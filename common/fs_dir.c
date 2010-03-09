/*
 * Decode directory clusters.
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
#include <string.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

DirEntry *
fs_dir_each_entry(FileHandle *dir, EachDirEntryFn fn, void *arg)
{
	int i;
	DirEntry *entry;

	while (file_read(dir) > 0)
		for (entry = (DirEntry *)dir->buffer, i = 0;
				entry < (DirEntry *)(dir->buffer+dir->nread);
				entry++, i++)
			if (fn && !fn(dir, arg, entry, i))
				return entry;
	return 0;
}

static int
fs_dir_entry_filename_match(FileHandle *dir, void *filename, DirEntry *entry, int index)
{
	if (strcmp(filename, entry->filename) == 0)
		return 0;
	else
		return 1;
}

DirEntry *
fs_dir_find(FileHandle *dir, char *filename)
{
	fs_dir_each_entry(dir, fs_dir_entry_filename_match, filename);
}
