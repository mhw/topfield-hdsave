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
	int bytes_per_cluster;
	int root_dir_cluster;
	int used_clusters;
	int unused_bytes_in_root;
	int fat_crc32;
	uint8_t *fat;
} FSInfo;

typedef struct {
	int cluster;
	int bytes_used;
} Cluster;

typedef struct {
	FSInfo *fs;
	int buffer_size;
	char *buffer;
	uint64_t filesize;
	uint64_t offset;
	int num_clusters;
	Cluster *clusters;
} FileHandle;

/* fs.c */

extern void fs_error(char *fmt, ...);
extern void fs_warn(char *fmt, ...);

extern DiskInfo *disk_open(char *path);
extern void disk_close(DiskInfo *disk);

extern FSInfo *fs_open_disk(DiskInfo *disk);
extern void fs_close(FSInfo *fs);
extern int fs_read_directory(FSInfo *fs, char *path);

extern FileHandle file_open(FSInfo *fs, FileHandle *dir, char *filename);
extern void file_set_buffer_size(FileHandle *file, int buffer_size);
extern char *file_read(FileHandle *file);
extern void file_seek(FileHandle *file, uint64_t offset);
extern int file_close(FileHandle *file);

/* fs_fat.c */

extern Cluster *fs_fat_chain(FSInfo *fs, int start_cluster, int *cluster_count);

/* fs_io.c */

extern void *fs_read(FSInfo *fs, void *buf, int cluster, int cluster_offset, int bytes);
extern void fs_swap_bytes(void *buf, int bytes);
