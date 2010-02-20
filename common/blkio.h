/*
 * Declarations relating to block IO.
 */

typedef struct DevInfo DevInfo;

extern DevInfo *blkio_open(char *path);
extern void blkio_describe(DevInfo *dev_info, char *str, int size);
extern int blkio_block_size(DevInfo *dev_info);
extern uint64_t blkio_total_blocks(DevInfo *dev_info);
extern uint64_t blkio_read(DevInfo *dev_info, void *buf, uint64_t offset, uint64_t count);
