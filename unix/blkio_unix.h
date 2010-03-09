/*
 * Unix specific declarations relating to block IO.
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

typedef void (*EachBlockFn)(DevInfo *dev, void *buf, uint64_t offset, uint64_t count);

/* blkio_unix.c */

extern void blkio_each_block_fn(EachBlockFn fn);

/* blkio_sparse.c */

extern void blkio_open_sparse_clone(char *path);
extern void blkio_close_sparse_clone();
