/*
 * Structures used in on-disk filesystem.
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
#define DIR_ENTRY_END		0x00

/* fs_file.c */

extern FileHandle *file_open_dir_entry(FileHandle *dir, DirEntry *entry);
