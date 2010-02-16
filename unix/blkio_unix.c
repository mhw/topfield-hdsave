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
		error("blkio_open", "could not open device");
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
	snprintf(str, size, "device: %s", dev_info->path);
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
