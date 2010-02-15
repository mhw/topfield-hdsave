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

struct DevInfo
{
	char *path;
	int fd;
	int block_size;
	uint64_t blocks;
};

DevInfo *
blkio_open(char *path)
{
	DevInfo *dev_info;
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

	if (ioctl(dev_info->fd, BLKSSZGET, &dev_info->block_size) == -1)
	{
		error("blkio_open", "ioctl(BLKSSZGET) failed");
		free(dev_info);
		return 0;
	}

	if (ioctl(dev_info->fd, BLKGETSIZE64, &dev_size) == -1)
	{
		error("blkio_open", "ioctl(BLKGETSIZE64) failed");
		free(dev_info);
		return 0;
	}
	dev_info->blocks = dev_size/dev_info->block_size;

	return dev_info;
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
