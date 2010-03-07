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
	int nread;
	int filesize_needs_fixup;
	uint64_t filesize;
	uint64_t offset;
	int num_clusters;
	Cluster *clusters;
} FileHandle;

typedef struct {
	uint8_t type;
	char mtime[7];
	uint32_t start_cluster;
	uint32_t clusters;
	uint32_t unused_bytes_in_last_cluster;
	char filename[64];
	char service_name[31];
	char unused1;
	uint32_t attributes;
	uint16_t flags;
	char unused2[2];
	uint8_t unused3;
	uint8_t s3_crc;
	uint16_t bytes_in_last_block;
} DirEntry;

/*
 * Values for DirEntry.type
 */
#define DIR_ENTRY_FILEA		0xd0
#define DIR_ENTRY_FILET		0xd1
#define DIR_ENTRY_DOT_DOT	0xf0
#define DIR_ENTRY_DOT		0xf1
#define DIR_ENTRY_SUBDIR	0xf2
#define DIR_ENTRY_RECYCLE	0xf3
#define DIR_ENTRY_UNUSED	0xff

/*
 * Special value for our fake root directory entry. Not used by on-disk
 * structure.
 */
#define DIR_ENTRY_ROOT		0xef

/* fs.c */

extern void fs_error(char *fmt, ...);
extern void fs_warn(char *fmt, ...);

extern DiskInfo *disk_open(char *path);
extern void disk_close(DiskInfo *disk);

extern FSInfo *fs_open_disk(DiskInfo *disk);
extern void fs_close(FSInfo *fs);

/* fs_dir.c */

typedef int (*EachDirEntryFn)(FSInfo *fs, void *arg, DirEntry *entry, int index);

extern DirEntry *fs_dir_each_entry(FileHandle *dir, EachDirEntryFn fn, void *arg);
extern DirEntry *fs_dir_find(FileHandle *dir, char *filename);

/* fs_dir_ls.c */

extern int fs_dir_ls(FSInfo *fs, char *path);

/* fs_file.c */

extern FileHandle *file_open_root(FSInfo *fs);
extern FileHandle *file_open_dir_entry(FileHandle *dir, DirEntry *entry);
extern FileHandle *file_open(FileHandle *dir, char *filename);
extern FileHandle *file_open_pathname(FSInfo *fs, FileHandle *dir, char *pathname);
extern void file_close(FileHandle *file);
extern char *file_read(FileHandle *file);

/* fs_fat.c */

extern Cluster *fs_fat_chain(FSInfo *fs, int start_cluster, int *cluster_count, uint64_t filesize) ;

/* fs_io.c */

extern void *fs_read(FSInfo *fs, void *buf, int cluster, int cluster_offset, int bytes);
extern void fs_swap_bytes(void *buf, int bytes);
