/*
 * Decode directory clusters.
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
			if (fn && !fn(dir->fs, arg, entry, i))
				return entry;
	return 0;
}

static int
fs_dir_entry_filename_match(FSInfo *fs, void *filename, DirEntry *entry, int index)
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
