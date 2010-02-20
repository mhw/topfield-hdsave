/*
 * Implementation of block IO routines on Unix platforms.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fs.h>

#include "port.h"
#include "common.h"
#include "blkio.h"

#define DEFAULT_BLOCK_SIZE 512

struct DevInfo
{
	char *path;
	int fd;
	int block_size;
	uint64_t blocks;
	uint64_t bytes;
};

static uint64_t size_override = 0;

void
blkio_set_size_override(uint64_t size)
{
	size_override = size;
	fprintf(stderr, "forcing device size to be %s\n",
			format_disk_size(size_override));
}

DevInfo *
blkio_open(char *path)
{
	DevInfo *dev_info;
	struct stat dev_stat;
	uint64_t dev_size;

	if (!path)
	{
		error("blkio_open", "path is null");
		return 0;
	}

	if ((dev_info = malloc(sizeof(DevInfo))) == 0)
	{
		no_memory("blkio_open");
		return 0;
	}

	dev_info->path = path;
	if ((dev_info->fd = open(path, O_RDONLY)) == -1)
	{
		sys_error("blkio_open", "could not open '%s'", path);
		free(dev_info);
		return 0;
	}

	if (fstat(dev_info->fd, &dev_stat) == -1)
	{
		error("blkio_open", "fstat failed");
		free(dev_info);
		return 0;
	}

	if (S_ISREG(dev_stat.st_mode))
	{
		uint64_t size = size_override? size_override : dev_stat.st_size;

		dev_info->block_size = DEFAULT_BLOCK_SIZE;
		dev_info->blocks = size/dev_info->block_size;
		dev_info->bytes = size;
		return dev_info;
	}
	else if (S_ISBLK(dev_stat.st_mode))
	{
		if (ioctl(dev_info->fd, BLKSSZGET, &dev_info->block_size) == -1)
		{
			error("blkio_open", "ioctl(BLKSSZGET) failed");
			free(dev_info);
			return 0;
		}

		if (size_override)
		{
			dev_info->blocks = size_override/dev_info->block_size;
			dev_info->bytes = size_override;
		}
		else
		{
			if (ioctl(dev_info->fd, BLKGETSIZE64, &dev_size) == -1)
			{
				error("blkio_open", "ioctl(BLKGETSIZE64) failed");
				free(dev_info);
				return 0;
			}
			dev_info->blocks = dev_size/dev_info->block_size;
			dev_info->bytes = dev_size;
		}

		return dev_info;
	}

	error("blkio_open", "not a file or block device");
	free(dev_info);
	return 0;
}

void
blkio_describe(DevInfo *dev_info, char *str, int size)
{
	snprintf(str, size, "%s: %s device with %d byte blocks",
			dev_info->path,
			format_disk_size(dev_info->blocks*dev_info->block_size),
			dev_info->block_size);
}

int
blkio_block_size(DevInfo *dev_info)
{
	return dev_info->block_size;
}

uint64_t
blkio_total_blocks(DevInfo *dev_info)
{
	return dev_info->blocks;
}

uint64_t
blkio_read(DevInfo *dev_info, void *buf, uint64_t offset, uint64_t count)
{
	int bytes;

	if (offset > dev_info->bytes)
	{
		error("blkio_read", "offset 0x%" PRIx64 " > disk size 0x%" PRIx64, offset, dev_info->bytes);
		return -1;
	}

	if (lseek(dev_info->fd, offset, SEEK_SET) == -1)
	{
		sys_error("blkio_read", "seek to 0x%" PRIx64 " failed", offset);
		return -1;
	}

	if ((bytes = read(dev_info->fd, buf, count)) == -1)
	{
		sys_error("blkio_read", "read failed");
		return -1;
	}

	if (bytes < count)
	{
		error("blkio_read", "short read - wanted 0x%" PRIx64 " bytes, got 0x%" PRIx64 " bytes", count, bytes);
		return -1;
	}

	return bytes;
}
