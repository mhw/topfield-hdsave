/*
 * Command line program for manipulating Topfield hard disks.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

extern void blkio_set_size_override(uint64_t size);

static int info_cmd(int argc, char *argv[]);
static int ls_cmd(int argc, char *argv[]);
static int cp_cmd(int argc, char *argv[]);

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
	fputs("\tls [dir]\tList contents of a directory\n", stderr);
	fputs("\tcp <src> <dst>\tCopy contents of a file to host filesystem\n", stderr);
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
	int parsed;
	int success;

	parsed = parse_options(argc, argv);
	argc -= parsed;
	argv += parsed;

	if (opts.size_override)
	{
		uint64_t size = parse_disk_size(opts.size_override);
		blkio_set_size_override(size);
	}

	if (strcmp(opts.command, "info") == 0)
	{
		success = info_cmd(argc, argv);
	}
	else if (strcmp(opts.command, "ls") == 0)
	{
		success = ls_cmd(argc, argv);
	}
	else if (strcmp(opts.command, "cp") == 0)
	{
		success = cp_cmd(argc, argv);
	}
	else
	{
		usage();
	}

	if (success)
	{
		return 0;
	}
	else
	{
		fprintf(stderr, "%s\n", get_error());
		return 1;
	}
}

static int
info_cmd(int argc, char *argv[])
{
	DiskInfo *disk;
	FSInfo *fs;
	DevInfo *dev;
	char buf[80];
	int r;

	if ((disk = disk_open(opts.device_path)) == 0)
		return 0;
	if ((fs = fs_open_disk(disk)) == 0)
		return 0;

	dev = fs->disk->dev;
	blkio_describe(dev, buf, sizeof(buf));
	printf("%s\n", buf);
	printf("Filesystem cluster size: %d blocks\n", fs->blocks_per_cluster);
	printf("Root directory cluster: %d\n", fs->root_dir_cluster);
	printf("Used clusters: %d\n", fs->used_clusters);
	printf("Unused bytes in root: %d\n", fs->unused_bytes_in_root);
	fs_close(fs);
	disk_close(disk);
	return r;
}

static int
ls_cmd(int argc, char *argv[])
{
	DiskInfo *disk;
	FSInfo *fs;
	int r;

	if ((disk = disk_open(opts.device_path)) == 0)
		return 0;
	if ((fs = fs_open_disk(disk)) == 0)
		return 0;

	if (argc == 1)
		r = fs_dir_ls(fs, "/");
	else
		r = fs_dir_ls(fs, argv[1]);

	fs_close(fs);
	disk_close(disk);
	return r;
}

static int
cp_cmd(int argc, char *argv[])
{
	DiskInfo *disk;
	FSInfo *fs;
	FileHandle *file;
	int fd;

	if ((disk = disk_open(opts.device_path)) == 0)
		return 0;
	if ((fs = fs_open_disk(disk)) == 0)
		return 0;

	if ((file = file_open_pathname(fs, 0, argv[1])) == 0)
		return 0;
	if ((fd = creat(argv[2], 0666)) == -1)
	{
		sys_error("cp", "could not open '%s' for writing", argv[2]);
		return 0;
	}

	while (file_read(file) > 0)
	{
		if (write(fd, file->buffer, file->nread) == -1)
		{
			sys_error("cp", "could not write to '%s'", argv[2]);
			file_close(file);
			return 0;
		}
	}

	if (close(fd) == -1)
	{
		sys_error("cp", "could not write to '%s'", argv[2]);
		file_close(file);
		return 0;
	}
	fs_close(fs);
	disk_close(disk);
	return 1;
}
