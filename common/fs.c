/*
 * Decode information on a Topfield FAT24 format disk.
 */

#include <stdio.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"


/*
 * Structures used in on-disk filesystem.
 */

#define FS_MAGIC 0x07082607

static char *fs_valid_identifiers[] = {
	"TOPFIELD TF5000PVR HDD",
	// Currently untested on 4000 series disks
	// "TOPFIELD PVR HDD",
};

#define FS_VERSION 0x0101

typedef struct {
	uint32_t magic;
	char identifier[28];
	uint16_t version;
	uint16_t sectors_per_cluster;
	// firebird's doc suggests this is a 16 bit value
	uint16_t root_dir_cluster;
	uint16_t unused_1;
	uint32_t used_clusters;
	uint32_t unused_bytes_in_root;
	uint32_t fat_crc32;
} SuperBlock;

static int fs_read_super_blocks(FSInfo *fs_info);
static int fs_check_hd_identifier(SuperBlock *sb1, SuperBlock *sb2);

static void
fs_error(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verror("filesystem", fmt, ap);
	va_end(ap);
}

FSInfo *
fs_open(char *path)
{
	FSInfo *fs_info;

	if ((fs_info = malloc(sizeof(FSInfo))) == 0)
	{
		no_memory("fs_open");
		return 0;
	}

	if ((fs_info->dev_info = blkio_open(path)) == 0)
	{
		free(fs_info);
		return 0;
	}

	/*
	 * Documentation suggests this is hard-coded regardless of
	 * device characteristics.
	 */
	fs_info->block_size = 512;

	if (!fs_read_super_blocks(fs_info))
	{
		free(fs_info);
		return 0;
	}

	return fs_info;
}

static void
fs_swap_bytes(void *buf, int count)
{
	uint32_t *s = (uint32_t *)buf;
	uint32_t *e = (uint32_t *)(buf+count);

	while (s < e) {
		*s = bswap_32(*s);
		s++;
	}
}

static int
fs_read_super_blocks(FSInfo *fs_info)
{
	char *sb_buffer;
	SuperBlock *sb1;
	SuperBlock *sb2;

	if ((sb_buffer = malloc(2*fs_info->block_size)) == 0)
	{
		no_memory("fs_read_super_block");
		return 0;
	}

	if (!blkio_read(fs_info->dev_info, sb_buffer, 0, 2*fs_info->block_size))
	{
		free(sb_buffer);
		return 0;
	}

	sb1 = (SuperBlock *)sb_buffer;
	sb2 = (SuperBlock *)(sb_buffer+fs_info->block_size);

	fs_swap_bytes(sb1, sizeof(SuperBlock));
	fs_swap_bytes(sb2, sizeof(SuperBlock));

	if (be32toh(sb1->magic) != FS_MAGIC)
	{
		fs_error("super block 1 magic 0x%" PRIx32 " != expected 0x%" PRIx32, be32toh(sb1->magic), FS_MAGIC);
		free(sb_buffer);
		return 0;
	}

	if (be32toh(sb2->magic) != FS_MAGIC)
	{
		fs_error("super block 2 magic 0x%" PRIx32 " != expected 0x%" PRIx32, be32toh(sb1->magic), FS_MAGIC);
		free(sb_buffer);
		return 0;
	}

	if (memcmp(sb1, sb2, fs_info->block_size) != 0)
	{
		fs_error("super blocks do not match");
		free(sb_buffer);
		return 0;
	}

	if (!fs_check_hd_identifier(sb1, sb2))
	{
		free(sb_buffer);
		return 0;
	}

	if (be16toh(sb1->version) != FS_VERSION)
	{
		fs_error("unrecognised filesystem version number 0x%" PRIx16, be16toh(sb1->version));
		free(sb_buffer);
		return 0;
	}

	fs_info->blocks_per_cluster = be16toh(sb1->sectors_per_cluster);
	fs_info->root_dir_cluster = be16toh(sb1->root_dir_cluster);
	fs_info->used_clusters = be32toh(sb1->used_clusters);
	fs_info->unused_bytes_in_root = be32toh(sb1->unused_bytes_in_root);
	fs_info->fat_crc32 = be32toh(sb1->fat_crc32);

	free(sb_buffer);

	return 1;
}

static int
fs_check_hd_identifier(SuperBlock *sb1, SuperBlock *sb2)
{
	int i = elementsof(fs_valid_identifiers)-1;

	while (i >= 0)
	{
		if (strcmp(sb1->identifier, fs_valid_identifiers[i]) == 0)
			break;
		i--;
	}

	if (i < 0)
	{
		fs_error("super block identifier not recognised");
		return 0;
	}

	return 1;
}
