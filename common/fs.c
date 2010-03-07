/*
 * Decode information on a Topfield FAT24 format disk.
 */

#include <stdio.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

#include "fs_int.h"

/*
 * Terminology:
 *   block - one 512 byte disk area. sometimes called a sector.
 *   cluster - unit of filesystem disk allocation.
 *   FAT - File Allocation Table.
 *   chunk - 188 blocks.
 *
 * Disk space is allocated in units of clusters. Each cluster is a multiple
 * of 188 blocks in length, 188 blocks being 4 times the smallest whole number
 * of blocks that contain a whole number of 188 byte MPEG Transport Stream
 * packets. 47 * 512 byte blocks = 128 * 188 byte packets = 24064 bytes.
 * We call these 188 block units 'chunks'.
 *
 * Cluster usage is recorded in two file allocation tables, using a 3 byte
 * value to represent one cluster. Each 3 byte value indicates one of the
 * following possible uses of the cluster:
 *   0xffffff - cluster is unallocated.
 *   0xfffffe - cluster is the last one in a file.
 *   other    - value is the number of the next cluster in the file.
 * Hence the FAT links clusters into linked lists (sometimes called chains).
 *
 * Each FAT takes a maximum of 768 blocks, giving a maximum of 131072
 * 3 byte FAT entries and hence a maximum number of clusters in the
 * filesystem.
 */

static char *fs_valid_identifiers[] = {
	"TOPFIELD TF5000PVR HDD",
	// Currently untested on 4000 series disks
	// "TOPFIELD PVR HDD",
};

static int fs_blocks_per_cluster(DevInfo *dev);
static int fs_read_super_blocks(FSInfo *fs);
static int fs_check_hd_identifier(SuperBlock *sb1, SuperBlock *sb2);

void
fs_error(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verror("filesystem", fmt, ap);
	va_end(ap);
}

void
fs_warn(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarn(fmt, ap);
	va_end(ap);
}

DiskInfo *
disk_open(char *path)
{
	DiskInfo *disk;

	if ((disk = malloc(sizeof(DiskInfo))) == 0)
	{
		no_memory("disk_open");
		return 0;
	}

	if ((disk->dev = blkio_open(path)) == 0)
	{
		free(disk);
		return 0;
	}

	/*
	 * Documentation suggests block size is hard-coded in the
	 * Topfield firmware regardless of device characteristics.
	 */
	disk->block_size = 512;
	disk->blocks_per_cluster = fs_blocks_per_cluster(disk->dev);

	return disk;
}

void
disk_close(DiskInfo *disk)
{
	blkio_close(disk->dev);
	free(disk);
}

/*
 * We work out the number of blocks per cluster by working out how many
 * 188 block chunks we could address with each of the 131072 FAT entries.
 * As an example, for a 160Gb disk we have a total of 312,500,000 blocks.
 * This would allow 312,500,000 / (131072*188) = 12 188 block chunks
 * per FAT entry and a cluster size of 2256 blocks.
 *
 * This number is somewhat conservative though - the FAT would be fully
 * used but each cluster wastes some disk space because we've rounded
 * down. So instead we round up to the next whole number of chunks and
 * use fewer clusters in total to allocate the whole of the disk space.
 *
 * Finally, there is a minimum cluster size of 11 188 block chunks.
 */
static int
fs_blocks_per_cluster(DevInfo *dev)
{
	uint64_t blocks;
	int chunks_per_fat;

	blocks = blkio_total_blocks(dev);
	chunks_per_fat = blocks/(131072*188)+1;
	if (chunks_per_fat < 11)
		chunks_per_fat = 11;
	return chunks_per_fat*188;
}

FSInfo *
fs_open_disk(DiskInfo *disk)
{
	FSInfo *fs;

	if ((fs = malloc(sizeof(FSInfo))) == 0)
	{
		no_memory("fs_open_disk");
		return 0;
	}

	fs->disk = disk;
	fs->block_size = disk->block_size;

	if (!fs_read_super_blocks(fs))
	{
		free(fs);
		return 0;
	}

	return fs;
}

void
fs_close(FSInfo *fs)
{
	/*
	 * We don't close the disk here as multiple FSInfo instances
	 * may refer to the same DiskInfo.
	 */
	if (fs->fat)
		free(fs->fat);
	free(fs);
}

static int
fs_read_super_blocks(FSInfo *fs)
{
	char *sb_buffer;
	SuperBlock *sb1;
	SuperBlock *sb2;

	if ((sb_buffer = malloc(2*fs->block_size)) == 0)
	{
		no_memory("fs_read_super_block");
		return 0;
	}

	if (!fs_read(fs, sb_buffer, -1, 0, 2*fs->block_size))
	{
		free(sb_buffer);
		return 0;
	}

	sb1 = (SuperBlock *)sb_buffer;
	sb2 = (SuperBlock *)(sb_buffer+fs->block_size);

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

	if (memcmp(sb1, sb2, fs->block_size) != 0)
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

	fs->blocks_per_cluster = be16toh(sb1->sectors_per_cluster);
	if (fs->blocks_per_cluster != fs->disk->blocks_per_cluster)
	{
		fs_warn("superblock %d blocks per cluster does not match calculated %d blocks per cluster", fs->blocks_per_cluster, fs->disk->blocks_per_cluster);
	}
	fs->bytes_per_cluster = fs->blocks_per_cluster*fs->block_size;
	fs->root_dir_cluster = be16toh(sb1->root_dir_cluster);
	fs->used_clusters = be32toh(sb1->used_clusters);
	fs->unused_bytes_in_root = be32toh(sb1->unused_bytes_in_root);
	fs->fat_crc32 = be32toh(sb1->fat_crc32);

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
