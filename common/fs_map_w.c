/*
 * Write a disk map file.
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
#include <stdarg.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

static FILE *map;

static int
map_open_write(char *path)
{
	if ((map = fopen(path, "w")) == 0)
		return 0;

	return 1;
}

static void
map_printf(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(map, fmt, ap);
	va_end(ap);
}

static void
map_close()
{
	if (map)
		fclose(map);
	map = 0;
}

static int walk_dir(FileHandle *dir);

static int
walk_dir_entry(FileHandle *dir, void *arg, DirEntry *entry, int index)
{
	FileHandle *subdir;

	switch (entry->type)
	{
	case DIR_ENTRY_UNUSED:
	case DIR_ENTRY_DOT_DOT:
	case DIR_ENTRY_DOT:
	case DIR_ENTRY_RECYCLE:
		break;
	case DIR_ENTRY_FILEA:
	case DIR_ENTRY_FILET:
		map_printf("%s: \n", entry->filename);
		break;
	case DIR_ENTRY_SUBDIR:
		map_printf("%s: {\n", entry->filename);
		if ((subdir = file_open_dir_entry(dir, entry)) == 0)
			return 0;
		if (!walk_dir(subdir))
			return 0;
		file_close(subdir);
		map_printf("}\n", entry->filename);
		break;
	default:
		fs_error("unrecognised directory entry type %d", entry->type);
		return 0;
	}
	return 1;
}

static int
walk_dir(FileHandle *dir)
{
	DirEntry *bad;

	if ((bad = fs_dir_each_entry(dir, walk_dir_entry, 0)) != 0)
	{
		fprintf(stderr, "fs_walk_dir failed at %s\n", bad->filename);
		return 0;
	}
	return 1;
}

int
map_write(FSInfo *fs, char *path)
{
	FileHandle *root;
	FileHandle *bad;

	if (!map_open_write(path))
		return 0;
	if ((root = file_open_root(fs)) == 0)
		return 0;
	walk_dir(root);
	file_close(root);
	map_close();
	return 1;
}
