/*
 * Declarations relating to block IO.
 */

typedef struct {
	char *path;
	int fd;
	int sector_size;
	uint64_t blocks;
} DevInfo;

extern DevInfo *blkio_open(char *path);
