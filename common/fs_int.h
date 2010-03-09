/*
 * Structures used in on-disk filesystem.
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

#define FS_MAGIC 0x07082607

#define FS_VERSION 0x0101

typedef struct {
	uint32_t magic;
	char identifier[28];
	uint16_t version;
	uint16_t sectors_per_cluster;
	// firebird's doc suggests this is a 16 bit value
	uint16_t root_dir_cluster;
	uint16_t unused_1;
	uint32_t used_clusters;
	uint32_t unused_bytes_in_root;
	uint32_t fat_crc32;
} SuperBlock;
