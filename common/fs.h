/*
 * Decode information on a Topfield FAT24 format disk.
 */

typedef struct {
	DevInfo *dev_info;
	int block_size;
	int blocks_per_cluster;
	int root_dir_cluster;
	int used_clusters;
	int unused_bytes_in_root;
	int fat_crc32;
} FSInfo;

extern FSInfo *fs_open(char *path);
