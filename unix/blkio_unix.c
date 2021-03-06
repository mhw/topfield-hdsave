/*
 * Implementation of block IO routines on Unix platforms.
 *
 * Copyright 2010 Mark H. Wilkinson
 *
 * This file is part of HDSave.
 *
 * HDSave is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HDSave is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HDSave.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fs.h>

#include "port.h"
#include "common.h"
#include "blkio.h"

#include "blkio_unix.h"

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
	DevInfo *dev;
	struct stat dev_stat;
	uint64_t dev_size;

	if (!path)
	{
		error("blkio_open", "path is null");
		return 0;
	}

	if ((dev = malloc(sizeof(DevInfo))) == 0)
	{
		no_memory("blkio_open");
		return 0;
	}

	dev->path = path;
	if ((dev->fd = open(path, O_RDONLY)) == -1)
	{
		sys_error("blkio_open", "could not open '%s'", path);
		free(dev);
		return 0;
	}

	if (fstat(dev->fd, &dev_stat) == -1)
	{
		error("blkio_open", "fstat failed");
		free(dev);
		return 0;
	}

	if (S_ISREG(dev_stat.st_mode))
	{
		uint64_t size = size_override? size_override : dev_stat.st_size;

		dev->block_size = DEFAULT_BLOCK_SIZE;
		dev->blocks = size/dev->block_size;
		dev->bytes = size;
		return dev;
	}
	else if (S_ISBLK(dev_stat.st_mode))
	{
		if (ioctl(dev->fd, BLKSSZGET, &dev->block_size) == -1)
		{
			error("blkio_open", "ioctl(BLKSSZGET) failed");
			free(dev);
			return 0;
		}

		if (size_override)
		{
			dev->blocks = size_override/dev->block_size;
			dev->bytes = size_override;
		}
		else
		{
			if (ioctl(dev->fd, BLKGETSIZE64, &dev_size) == -1)
			{
				error("blkio_open", "ioctl(BLKGETSIZE64) failed");
				free(dev);
				return 0;
			}
			dev->blocks = dev_size/dev->block_size;
			dev->bytes = dev_size;
		}

		return dev;
	}

	error("blkio_open", "not a file or block device");
	free(dev);
	return 0;
}

void
blkio_close(DevInfo *dev)
{
	if (close(dev->fd) == -1)
		sys_error("blkio_close", "close failed");
	free(dev);
}

void
blkio_describe(DevInfo *dev, char *str, int size)
{
	snprintf(str, size, "%s: %s device - %" PRId64 " * %d byte blocks",
			dev->path,
			format_disk_size(dev->blocks*dev->block_size),
			dev->blocks,
			dev->block_size);
}

int
blkio_block_size(DevInfo *dev)
{
	return dev->block_size;
}

uint64_t
blkio_total_blocks(DevInfo *dev)
{
	return dev->blocks;
}

static EachBlockFn each_block_fn;

void
blkio_each_block_fn(EachBlockFn fn)
{
	each_block_fn = fn;
}

uint64_t
blkio_read(DevInfo *dev, void *buf, uint64_t offset, uint64_t count)
{
	int bytes;

	if (offset > dev->bytes)
	{
		error("blkio_read", "offset 0x%" PRIx64 " > disk size 0x%" PRIx64, offset, dev->bytes);
		return -1;
	}

	if (lseek(dev->fd, offset, SEEK_SET) == -1)
	{
		sys_error("blkio_read", "seek to 0x%" PRIx64 " failed", offset);
		return -1;
	}

	if ((bytes = read(dev->fd, buf, count)) == -1)
	{
		sys_error("blkio_read", "read failed");
		return -1;
	}

	if (bytes < count)
	{
		error("blkio_read", "short read - wanted 0x%" PRIx64 " bytes, got 0x%" PRIx64 " bytes", count, bytes);
		return -1;
	}

	if (each_block_fn)
		each_block_fn(dev, buf, offset, count);

	return bytes;
}
