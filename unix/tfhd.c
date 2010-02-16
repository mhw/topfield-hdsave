/*
 * Command line program for manipulating Topfield hard disks.
 */

#include <stdio.h>
#include <unistd.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

extern void blkio_set_size_override(uint64_t size);

static void info_cmd(int argc, char *argv[]);

typedef struct {
	char *device_path;
	char *disk_map;
	char *size_override;
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
	fputs("\t-s SIZE\t\tSet disk size instead of probing device\n", stderr);
	fputs("commands:\n", stderr);
	fputs("\tinfo\t\tPrint basic information about the disk\n", stderr);
	exit(EXIT_FAILURE);
}

static int
parse_options(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "f:m:s:")) != -1)
	{
		switch (opt)
		{
		case 'f':
			opts.device_path = optarg;
			break;
		case 'm':
			opts.disk_map = optarg;
			break;
		case 's':
			opts.size_override = optarg;
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

	if (opts.size_override)
	{
		uint64_t size = parse_disk_size(opts.size_override);
		blkio_set_size_override(size);
	}

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
	DevInfo *dev_info;
	char buf[80];

	if ((fs_info = fs_open(opts.device_path)) == 0)
	{
		fprintf(stderr, "%s\n", get_error());
		return;
	}

	dev_info = fs_info->dev_info;
	blkio_describe(dev_info, buf, sizeof(buf));
	printf("%s\n", buf);
	printf("block size: %d\n", blkio_block_size(dev_info));
	printf("blocks: %ld\n", blkio_total_blocks(dev_info));
}
