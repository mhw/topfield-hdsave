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

typedef struct {
	uint32_t magic;
	char identifier[28];
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

	if (!fs_check_hd_identifier(sb1, sb2))
	{
		free(sb_buffer);
		return 0;
	}

	return 1;
}

static int
fs_check_hd_identifier(SuperBlock *sb1, SuperBlock *sb2)
{
	int i = sizeof(fs_valid_identifiers)/sizeof(fs_valid_identifiers[0])-1;

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

	if (memcmp(sb1->identifier, sb2->identifier, sizeof(sb1->identifier)) != 0)
	{
		fs_error("super block identifiers do not match");
		return 0;
	}

	return 1;
}
