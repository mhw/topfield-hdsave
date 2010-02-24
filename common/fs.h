/*
 * Decode information on a Topfield FAT24 format disk.
 */

typedef struct {
	DevInfo *dev;
	int block_size;
	int blocks_per_cluster;
} DiskInfo;

typedef struct {
	DiskInfo *disk;
	int block_size;
	int blocks_per_cluster;
	int root_dir_cluster;
	int used_clusters;
	int unused_bytes_in_root;
	int fat_crc32;
} FSInfo;

extern DiskInfo *disk_open(char *path);
extern void disk_close(DiskInfo *disk);

extern FSInfo *fs_open_disk(DiskInfo *disk);
extern void fs_close(FSInfo *fs);
extern int fs_read_directory(FSInfo *fs, char *path);
