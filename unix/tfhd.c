/*
 * Command line program for manipulating Topfield hard disks.
 */

#include <stdio.h>
#include <unistd.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

static void info_cmd(int argc, char *argv[]);

typedef struct {
	char *device_path;
	char *disk_map;
	char *command;
} Options;

static Options opts;

static void
usage(void)
{
	fputs("usage: tfhd [options] <command> [args...]\n", stderr);
	fputs("options:\n", stderr);
	fputs("\t-f DEVICE\tTopfield disk to manipulate\n", stderr);
	fputs("\t-m FILE\t\tUse a previously saved map file\n", stderr);
	fputs("commands:\n", stderr);
	fputs("\tinfo\t\tPrint basic information about the disk\n", stderr);
	exit(EXIT_FAILURE);
}

static int
parse_options(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "f:m:")) != -1)
	{
		switch (opt)
		{
		case 'f':
			opts.device_path = optarg;
			break;
		case 'm':
			opts.disk_map = optarg;
			break;
		default:
			usage();
		}
	}

	if (optind >= argc)
		usage();
	opts.command = argv[optind];
	return optind;
}

int
main(int argc, char *argv[])
{
	argc = parse_options(argc, argv);

	if (strcmp(opts.command, "info") == 0)
	{
		info_cmd(argc, argv);
	}
	else
	{
		usage();
	}
}

static void
info_cmd(int argc, char *argv[])
{
	FSInfo *fs_info;

	if ((fs_info = fs_open(opts.device_path)) == 0)
	{
		fprintf(stderr, "%s\n", get_error());
		return;
	}

	printf("device: %s\n", fs_info->dev_info->path);
	printf("sector size: %d\n", fs_info->dev_info->sector_size);
	printf("blocks: %ld\n", fs_info->dev_info->blocks);
}
