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
fs_dir_ls(FSInfo *fs, char *path)
{
	FileHandle *root;
	FileHandle *dir;
	FileHandle *entry;
	DirEntry *dir_entry;
	int i;

	if ((root = file_open_root(fs)) == 0)
		return 0;

	if ((dir = file_open(root, "ProgramFiles")) == 0)
		return 0;

	while (file_read(dir) > 0)
	{
		dir_entry = (DirEntry *)dir->buffer;

		while (dir_entry < (DirEntry *)(dir->buffer+dir->nread)) {
			switch (dir_entry->type)
			{
			case DIR_ENTRY_UNUSED:
				break;
			case DIR_ENTRY_FILEA:
			case DIR_ENTRY_FILET:
			case DIR_ENTRY_DOT_DOT:
			case DIR_ENTRY_DOT:
			case DIR_ENTRY_SUBDIR:
			case DIR_ENTRY_RECYCLE:
				printf("%s ", dir_entry->filename);

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

				for (i = 0; i < entry->num_clusters; i++)
				{
					if (i > 0)
						putchar(',');
					printf("[%" PRId32 ",%" PRId32 "]", entry->clusters[i].cluster, entry->clusters[i].bytes_used);
				}
				putchar('\n');

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
