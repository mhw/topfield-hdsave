/*
 * List contents of a directory.
 */

#include <stdio.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

#include "fs_int.h"

int
fs_dir_ls(FSInfo *fs, char *path, int opt_long)
{
	FileHandle *dir;
	FileHandle *entry;
	DirEntry *dir_entry;
	int i;

	if ((dir = file_open_pathname(fs, 0, path)) == 0)
		return 0;

	while (file_read(dir) > 0)
	{
		dir_entry = (DirEntry *)dir->buffer;

		while (dir_entry < (DirEntry *)(dir->buffer+dir->nread)) {
		        int is_dir = 1;

			switch (dir_entry->type)
			{
			case DIR_ENTRY_UNUSED:
			case DIR_ENTRY_DOT_DOT:
			case DIR_ENTRY_DOT:
				break;
			case DIR_ENTRY_FILEA:
			case DIR_ENTRY_FILET:
			        is_dir = 0;
			        /* fall through */
			case DIR_ENTRY_SUBDIR:
			case DIR_ENTRY_RECYCLE:
			        if (!opt_long)
			        {
					printf("%s%s\n", dir_entry->filename, is_dir? "/" : "");
					break;
				}

				if ((entry = file_open_dir_entry(dir, dir_entry)) == 0)
				{
					file_close(dir);
					return 0;
				}

				/*
				 * Read the first bit of the file if it needs
				 * the file size fixing up.
				 */
				if (entry->filesize_needs_fixup)
				{
					if (!file_read(entry))
					{
						file_close(entry);
						file_close(dir);
						return 0;
					}
				}

				printf("%s %10s %s\n", is_dir? "d" : "-",
						format_disk_size(entry->filesize),
						dir_entry->filename);

				file_close(entry);
				break;
			default:
				fs_error("unrecognised directory entry type %d", dir_entry->type);
				file_close(dir);
				return 0;
			}
			dir_entry++;
		}
	}

	file_close(dir);
	return 1;
}
