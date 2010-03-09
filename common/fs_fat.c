/*
 * Decode the Topfield File Allocation Table (FAT).
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
#include <sys/param.h>

#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

typedef union {
	struct {
		uint8_t zero;
		uint8_t bytes[3];
	} raw;
	uint32_t value;
} FATEntry;

#define FAT_FREE         0xffffff
#define FAT_CHAIN_END    0xfffffe
#define FAT_CLUSTER_MASK 0x01ffff
#define FAT_MARK         0x400000

#define FAT_CLUSTER_IS_MARKED(value) ((value&0x800000)? ((value&0x7e0000)^0x7e0000) : (value&0x7e0000))
#define FAT_CLUSTER_UNMARKED(value) ((value&0x800000)? (value|0x7e0000) : (value&0x01ffff))

static int
fs_load_fat(FSInfo *fs)
{
	int fat_start = 256*fs->block_size;
	int fat_size = 768*fs->block_size;

	if ((fs->fat = malloc(fat_size)) == 0)
	{
		no_memory("fs_load_fat");
		return 0;
	}

	if (!fs_read(fs, fs->fat, -1, fat_start, fat_size))
	{
		free(fs->fat);
		fs->fat = 0;
		return 0;
	}

	return 1;
}

static int
fs_fat_entry(FSInfo *fs, int cluster)
{
	FATEntry fe;

	cluster *= 3;
	fe.raw.zero = 0;
	fe.raw.bytes[0] = fs->fat[cluster++];
	fe.raw.bytes[1] = fs->fat[cluster++];
	fe.raw.bytes[2] = fs->fat[cluster++];
	return be32toh(fe.value);
}

static void
fs_fat_set_entry(FSInfo *fs, int cluster, int value)
{
	FATEntry fe;

	fe.value = htobe32(value);
	cluster *= 3;
	fs->fat[cluster++] = fe.raw.bytes[0];
	fs->fat[cluster++] = fe.raw.bytes[1];
	fs->fat[cluster++] = fe.raw.bytes[2];
}

typedef int (*EachClusterFn)(FSInfo *fs, void *arg, int cluster, int index);

static int fs_fat_marking_clusters = 0;

/*
 * TODO: could mark each cluster as we visit it to detect loops in the
 * FAT more accurately.
 */
static int
fs_fat_each_cluster(FSInfo *fs, int start_cluster, EachClusterFn fn, void *arg)
{
	int cluster;
	int i;
	int next_cluster;
	int marked;

	cluster = start_cluster;
	i = 0;
	while (i < 131072) {
// printf("cluster %d: %d\n", i, cluster);
		if (fn && !fn(fs, arg, cluster, i))
			return 0;
		next_cluster = fs_fat_entry(fs, cluster);
// printf("raw next cluster %d (0x%x)\n", next_cluster, next_cluster);
		marked = FAT_CLUSTER_IS_MARKED(next_cluster);
		next_cluster = FAT_CLUSTER_UNMARKED(next_cluster);
// printf("next cluster %d (0x%x)%s\n", next_cluster, next_cluster, marked? " marked" : "");
		if (next_cluster == FAT_FREE)
		{
			fs_error("free cluster found in cluster chain");
			return -1;
		}
		else if (next_cluster == FAT_CHAIN_END)
			return i+1;
		if (next_cluster > 131071)
		{
			fs_error("FAT entry for cluster %d points to cluster %d which is out of range", cluster, next_cluster);
			return -1;
		}
		cluster = next_cluster;
		i++;
	}
	fs_error("more than 131072 clusters in chain starting with cluster %d: loop in FAT?", start_cluster);
	return -1;
}

static uint64_t fs_fat_remaining_bytes;

static int
fs_fat_record_cluster_fn(FSInfo *fs, void *arg, int cluster, int index)
{
	Cluster *clusters = (Cluster *)arg;
	int bytes_used;

	bytes_used = MIN(fs->bytes_per_cluster, fs_fat_remaining_bytes);
	clusters[index].cluster = cluster;
	clusters[index].bytes_used = bytes_used;
	fs_fat_remaining_bytes -= bytes_used;
	return 1;
}

Cluster *
fs_fat_chain(FSInfo *fs, int start_cluster, int *cluster_count, uint64_t filesize)
{
	Cluster *clusters;
	int i;
	int cluster;
	int num_clusters;

	if (!fs->fat && !fs_load_fat(fs))
		return 0;

	if ((num_clusters = fs_fat_each_cluster(fs, start_cluster, 0, 0)) < 0)
		return 0;

// printf("from cluster %d found %d clusters\n", start_cluster, num_clusters);

	if (num_clusters != *cluster_count)
	{
		fs_warn("found %d clusters in chain starting from cluster %d, was expecting %d clusters", num_clusters, start_cluster, cluster_count);
		*cluster_count = num_clusters;
	}

	if ((clusters = malloc((num_clusters)*sizeof(Cluster))) == 0)
	{
		no_memory("fs_fat_chain");
		return 0;
	}

	fs_fat_remaining_bytes = filesize;
	if (fs_fat_each_cluster(fs, start_cluster, fs_fat_record_cluster_fn, clusters) < 0)
	{
		free(clusters);
		return 0;
	}

	return clusters;
}
