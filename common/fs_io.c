/*
 * Basic block IO interface.
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

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

void *
fs_read(FSInfo *fs, void *buf, int cluster, int cluster_offset, int bytes)
{
	off_t offset;

	if (cluster < -1)
	{
		fatal("fs_read", "invalid cluster number %d", cluster);
		return 0;
	}

	if (cluster_offset < 0
		|| cluster_offset > fs->bytes_per_cluster)
	{
		fatal("fs_read", "invalid offset within cluster %d", cluster_offset);
		return 0;
	}

	if (bytes % sizeof(uint32_t) != 0)
	{
		fatal("fs_read", "attempt to read %d bytes which isn't a whole number of 32-bit words", bytes);
		return 0;
	}

	offset = (off_t)(cluster+1)*fs->bytes_per_cluster+cluster_offset;
	if (blkio_read(fs->disk->dev, buf, offset, bytes) == -1)
	{
		return 0;
	}

	fs_swap_bytes(buf, bytes);

	return buf;
}

void
fs_swap_bytes(void *buf, int bytes)
{
	uint32_t *s = (uint32_t *)buf;
	uint32_t *e = (uint32_t *)(buf+bytes);

	while (s < e) {
		*s = bswap_32(*s);
		s++;
	}
}
