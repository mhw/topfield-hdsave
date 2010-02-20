/*
 * Decode information on a Topfield FAT24 format disk.
 */

typedef struct {
	DevInfo *dev_info;
	int block_size;
	int cluster_size;
} FSInfo;

extern FSInfo *fs_open(char *path);
