/*
 * Declarations relating to block IO.
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

typedef struct DevInfo DevInfo;

extern DevInfo *blkio_open(char *path);
extern void blkio_close(DevInfo *dev);
extern void blkio_describe(DevInfo *dev, char *str, int size);
extern int blkio_block_size(DevInfo *dev);
extern uint64_t blkio_total_blocks(DevInfo *dev);
extern uint64_t blkio_read(DevInfo *dev, void *buf, uint64_t offset, uint64_t count);
