/*
 * Command line program for manipulating Topfield hard disks.
 */

#include <stdio.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

int
main(int argc, char* argv[])
{
	FSInfo *fs_info;

	if ((fs_info = fs_open("/dev/sdb")) == 0)
	{
		fprintf(stderr, "%s\n", get_error());
		return 1;
	}

	printf("device: %s\n", fs_info->dev_info->path);
	printf("sector size: %d\n", fs_info->dev_info->sector_size);
	printf("blocks: %ld\n", fs_info->dev_info->blocks);
	return 0;
}
