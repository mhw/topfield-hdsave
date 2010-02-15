/*
 * Decode information on a Topfield FAT24 format disk.
 */

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

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

	return fs_info;
}
