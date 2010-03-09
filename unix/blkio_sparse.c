/*
 * Write accessed disk blocks to a sparse clone file.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fs.h>

#include "port.h"
#include "common.h"
#include "blkio.h"

#include "blkio_unix.h"

/*
 * This module takes advantage of Unix filesystems' general support for
 * sparse files: areas of files that are never written to do not actually
 * consume disk blocks. We use this feature to create sparse clones of
 * Topfield disks, which are useful for development and testing.
 *
 * This module creates or updates a sparse clone by writing out copies
 * of any block accessed by the tfhd program while it runs. Typically
 * this is used in conjunction with the 'map' command that visits every
 * directory on the disk.
 */

static int sparse_clone_fd = -1;

void
blkio_write_sparse_clone(DevInfo *dev, void *buf, uint64_t offset, uint64_t count)
{
	int bytes;

	if (lseek(sparse_clone_fd, offset, SEEK_SET) == -1)
	{
		sys_error("blkio_write_sparse_clone", "seek to 0x%" PRIx64 " failed", offset);
		return;
	}

	if ((bytes = write(sparse_clone_fd, buf, count)) == -1)
	{
		sys_error("blkio_write_sparse_clone", "write failed");
		return;
	}

	if (bytes < count)
	{
		error("blkio_write_sparse_clone", "short write - expected to write 0x%" PRIx64 " bytes, actually wrote 0x%" PRIx64 " bytes", count, bytes);
		return;
	}
}

void
blkio_open_sparse_clone(char *path)
{
	if ((sparse_clone_fd = open(path, O_WRONLY|O_CREAT, 0666)) == -1)
	{
		sys_error("sparse_clone", "couldn't open %s", path);
		return;
	}
	blkio_each_block_fn(blkio_write_sparse_clone);
}

void
blkio_close_sparse_clone()
{
	if (sparse_clone_fd >= 0)
		close(sparse_clone_fd);
	sparse_clone_fd = 0;
}
