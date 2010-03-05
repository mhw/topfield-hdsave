/*
 * Treat FAT 24 clusters chains as files.
 */

#include <stdio.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

#include "fs_int.h"

/* Work in units of 'chunks' by default. */
#define DEFAULT_BUFFER_SIZE 188

static DirEntry file_fake_root_dir_entry = {
	DIR_ENTRY_SUBDIR,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0,
	1,
	0,
	'/', 0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

static DirEntry *
file_fake_root(FSInfo *fs)
{
	DirEntry *r = &file_fake_root_dir_entry;
	int i;

	if (r->type == 0)
	{
		r->type = DIR_ENTRY_SUBDIR;
		for (i = 0; i < sizeof(r->mtime); i++)
			r->mtime[i] = 0xff;
		r->start_cluster = htobe32(fs->root_dir_cluster);
		r->clusters = htobe32(1);
		r->unused_bytes_in_last_cluster = htobe32(0);
		r->filename[0] = '/';
	}

	return r;
}

static FileHandle *
file_handle_init(char *where, FSInfo *fs, DirEntry *entry)
{
	FileHandle *file;
	int clusters;
	int unused;

	if ((file = malloc(sizeof(FileHandle))) == 0)
	{
		no_memory(where);
		return 0;
	}

	file->fs = fs;
	file->buffer_size = DEFAULT_BUFFER_SIZE*fs->block_size;
	file->buffer = 0;
	file->nread = 0;
	switch (entry->type) {
	case DIR_ENTRY_UNUSED:
		fatal(where, "attempt to open unused DirEntry");
		free(file);
		return 0;
	case DIR_ENTRY_SUBDIR:
	case DIR_ENTRY_DOT:
	case DIR_ENTRY_DOT_DOT:
	case DIR_ENTRY_RECYCLE:
		file->filesize = fs->bytes_per_cluster;
		file->num_clusters = 1;
		break;
	case DIR_ENTRY_FILEA:
	case DIR_ENTRY_FILET:
		clusters = be32toh(entry->clusters);
		unused = be32toh(entry->unused_bytes_in_last_cluster);
		file->filesize = clusters*fs->bytes_per_cluster - unused;
		file->num_clusters = clusters;
		break;
	}
	file->offset = 0;

	return file;
}

FileHandle *
file_open_root(FSInfo *fs)
{
	DirEntry *root = file_fake_root(fs);
	FileHandle *file;

	if ((file = file_handle_init("file_open_root", fs, root)) == 0)
		return 0;

	if ((file->clusters = fs_fat_chain(fs, fs->root_dir_cluster, &file->num_clusters)) == 0)
	{
		free(file);
		return 0;
	}

	return file;
}

FileHandle *
file_open_dir_entry(FileHandle *dir, DirEntry *entry)
{
	FileHandle *file;
	int start_cluster;

	if ((file = file_handle_init("file_open_dir_entry", dir->fs, entry)) == 0)
		return 0;

	start_cluster = be32toh(entry->start_cluster);
	if ((file->clusters = fs_fat_chain(dir->fs, start_cluster, &file->num_clusters)) == 0)
	{
		free(file);
		return 0;
	}

	return file;
}

void
file_close(FileHandle *file)
{
	free(file->buffer);
	free(file->clusters);
	free(file);
}

static char *
file_buffer_alloc(FileHandle *file)
{
	if (!file->buffer)
	{
		if ((file->buffer = malloc(file->buffer_size)) == 0)
		{
			no_memory("file_buffer_alloc");
			return 0;
		}
	}

	return file->buffer;
}

static void
file_buffer_free(FileHandle *file)
{
	free(file->buffer);
	file->buffer = 0;
}

void
file_set_buffer_size(FileHandle *file, int buffer_size)
{
	file_buffer_free(file);
	file->buffer_size = buffer_size;
}

char *
file_read(FileHandle *file)
{
	char *buffer = file_buffer_alloc(file);
	int cluster = file->offset / file->fs->bytes_per_cluster;
	int cluster_offset = file->offset % file->fs->bytes_per_cluster;
	int bytes = file->buffer_size;
	int bytes_left = file->filesize-file->offset;

	if (bytes_left < bytes)
		bytes = bytes_left;

	if (!fs_read(file->fs, buffer, cluster, cluster_offset, bytes))
	{
		file->nread = 0;
		return 0;
	}
	else
	{
		file->nread = bytes;
		file->offset += bytes;
		return buffer;
	}
}
