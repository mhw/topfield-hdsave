/*
 * Command line program for manipulating Topfield hard disks.
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

extern void blkio_set_size_override(uint64_t size);

typedef int (*CommandFn)(int argc, char *argv[]);

static int info_cmd(int argc, char *argv[]);
static int ls_cmd(int argc, char *argv[]);
static int cp_cmd(int argc, char *argv[]);

typedef struct {
	char *device_path;
	char *disk_map;
	char *size_override;
	CommandFn command_fn;
} Options;

static Options opts;

typedef struct {
        char *name;
        CommandFn fn;
} Command;

static Command commands[] = {
        { "info", info_cmd },
        { "ls", ls_cmd },
        { "cp", cp_cmd },
};

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
	int i;

	while ((opt = getopt(argc, argv, "+f:m:s:")) != -1)
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
	for (i = 0; i < elementsof(commands); i++)
	{
	        if (strcmp(commands[i].name, argv[optind]) == 0)
	        {
	                opts.command_fn = commands[i].fn;
	                break;
	        }
	}
	i = optind;
	optind = 0;     /* reset getopt so commands can also call it */
	return i;
}

static DiskInfo *disk;
static FSInfo *fs;

int
main(int argc, char *argv[])
{
	int parsed;
	int success = 1;

	parsed = parse_options(argc, argv);
	argc -= parsed;
	argv += parsed;

	if (opts.size_override)
	{
		uint64_t size = parse_disk_size(opts.size_override);
		blkio_set_size_override(size);
	}

	if (opts.command_fn)
	{
		if ((disk = disk_open(opts.device_path)) == 0)
		        success = 0;
		if (success && (fs = fs_open_disk(disk)) == 0)
		        success = 0;
		if (success)
			success = opts.command_fn(argc, argv);
		if (fs)
			fs_close(fs);
		if (disk)
			disk_close(disk);
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
info_cmd(int argc, char *arg[])
{
	DevInfo *dev;
	char buf[80];
	int r;

	dev = fs->disk->dev;
	blkio_describe(dev, buf, sizeof(buf));
	printf("%s\n", buf);
	printf("Filesystem cluster size: %d blocks\n", fs->blocks_per_cluster);
	printf("Root directory cluster: %d\n", fs->root_dir_cluster);
	printf("Used clusters: %d\n", fs->used_clusters);
	printf("Unused bytes in root: %d\n", fs->unused_bytes_in_root);
	return r;
}

static int
ls_cmd(int argc, char *argv[])
{
	int r;
	int opt;
	int opt_long;

	while ((opt = getopt(argc, argv, "l")) != -1)
	{
		switch (opt)
		{
		case 'l':
			opt_long = 1;
			break;
		default:
			fprintf(stderr, "usage: ls [-l] [dir]\n");
			return 1;
		}
	}

	if (optind == argc)
		r = fs_dir_ls(fs, "/", opt_long);
	else
		r = fs_dir_ls(fs, argv[optind], opt_long);
	return r;
}

static int
cp_cmd(int argc, char *argv[])
{
	FileHandle *file;
	int fd;

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
	return 1;
}
