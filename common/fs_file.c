/*
 * Treat FAT 24 clusters chains as files.
 */

#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

#include "fs_int.h"

/* Work in units of 'chunks' by default. */
#define DEFAULT_BUFFER_SIZE 188

static DirEntry file_fake_root_dir_entry;

static DirEntry *
file_fake_root(FSInfo *fs)
{
	DirEntry *r = &file_fake_root_dir_entry;
	int i;

	if (r->type == 0)
	{
		r->type = DIR_ENTRY_ROOT;
		for (i = 0; i < sizeof(r->mtime); i++)
			r->mtime[i] = 0xff;
		r->start_cluster = htobe32(fs->root_dir_cluster);
		r->clusters = htobe32(1);
		r->unused_bytes_in_last_cluster = htobe32(fs->unused_bytes_in_root);
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
	int filesize_needs_fixup;

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
	case DIR_ENTRY_DOT_DOT:
	case DIR_ENTRY_RECYCLE:
		clusters = 1;
		/*
		 * We can't derive the directory size from these entries
		 * as the Toppy firmware doesn't update them. We need to
		 * rely on the '.' entry in the directory itself. For the
		 * time being, set the file size to the whole cluster.
		 * We'll fix it when we first read the file.
		 */
		unused = 0;
		filesize_needs_fixup = 1;
		break;
	case DIR_ENTRY_DOT:	/* '.' entries have valid sizes */
	case DIR_ENTRY_ROOT:	/* Our fake entry has valid sizes */
	case DIR_ENTRY_FILEA:
	case DIR_ENTRY_FILET:
		clusters = be32toh(entry->clusters);
		unused = be32toh(entry->unused_bytes_in_last_cluster);
		filesize_needs_fixup = 0;
		break;
	}
	file->filesize_needs_fixup = filesize_needs_fixup;
	file->filesize = clusters*fs->bytes_per_cluster - unused;
	file->num_clusters = clusters;
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

	if ((file->clusters = fs_fat_chain(fs, fs->root_dir_cluster, &file->num_clusters, file->filesize)) == 0)
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
	if ((file->clusters = fs_fat_chain(dir->fs, start_cluster, &file->num_clusters, file->filesize)) == 0)
	{
		free(file);
		return 0;
	}

	return file;
}

FileHandle *
file_open(FileHandle *dir, char *filename)
{
	DirEntry *entry;

	if ((entry = fs_dir_find(dir, filename)) == 0)
		return 0;
	return file_open_dir_entry(dir, entry);
}

FileHandle *
file_open_pathname(FSInfo *fs, FileHandle *dir, char *pathname)
{
	FileHandle *cur;
	char *s;
	char *e;
	int need_close = 0;

	if (dir)
	{
		cur = dir;
	}
	else
	{
		cur = file_open_root(fs);
		need_close = 1;
	}

	s = pathname;
	for (;;)
	{
		FileHandle *next;

		while (*s == '/')
			s++;
		if (!*s)
			break;
		e = strchr(s, '/');
		if (e)
			*e = 0;
		if ((next = file_open(cur, s)) == 0)
		{
			fs_warn("could not find '%s'", s);
			if (need_close)
				file_close(cur);
			return 0;
		}
		if (need_close)
			file_close(cur);
		cur = next;
		need_close = 1;
		if (e)
			*e = '/';
		else
			break;
		s = e+1;
	}
	return cur;
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

char *
file_read(FileHandle *file)
{
	char *buffer = file_buffer_alloc(file);
	int cluster_index;
	int cluster;
	int cluster_offset;
	int bytes;
	int bytes_left;

	if (file->offset >= file->filesize)
		return 0;

	cluster_index = file->offset / file->fs->bytes_per_cluster;
	cluster = file->clusters[cluster_index].cluster;
	cluster_offset = file->offset % file->fs->bytes_per_cluster;
	bytes = MIN(file->buffer_size, file->filesize-file->offset);

	if (!fs_read(file->fs, buffer, cluster, cluster_offset, bytes))
	{
		file->nread = 0;
		return 0;
	}

	if (file->filesize_needs_fixup)
	{
		DirEntry *dot;
		int clusters;
		int unused;
		int new_size;

		dot = (DirEntry *)buffer;
		clusters = be32toh(dot->clusters);
		unused = be32toh(dot->unused_bytes_in_last_cluster);
		new_size = clusters*file->fs->bytes_per_cluster - unused;
		file->clusters[file->num_clusters-1].bytes_used -= (file->filesize - new_size);
		file->filesize = new_size;
		bytes = MIN(new_size, bytes);
		file->filesize_needs_fixup = 0;
	}
	file->nread = bytes;
	file->offset += bytes;
	return buffer;
}
