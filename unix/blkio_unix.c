/*
 * Implementation of block IO routines on Unix platforms.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fs.h>

#include "port.h"
#include "common.h"
#include "blkio.h"

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

	if (ioctl(dev_info->fd, BLKSSZGET, &dev_info->sector_size) == -1)
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
	dev_info->blocks = dev_size/dev_info->sector_size;

	return dev_info;
}
