#include "port.h"
#include "common.h"
#include "blkio.h"
#include "fs.h"

void *
fs_read(FSInfo *fs, void *buf, int cluster, int cluster_offset, int bytes)
{
	off_t offset;

	if (cluster < -1)
	{
		fatal("fs_blk_offset", "invalid cluster number %d", cluster);
		return 0;
	}

	if (cluster_offset < 0
		|| cluster_offset > fs->bytes_per_cluster)
	{
		fatal("fs_blk_offset", "invalid offset within cluster %d", cluster_offset);
		return 0;
	}

	offset = (cluster+1)*fs->bytes_per_cluster+cluster_offset;
	if (!blkio_read(fs->disk->dev, buf, offset, bytes))
	{
		return 0;
	}
	return buf;
}

void
fs_swap_bytes(void *buf, int bytes)
{
	uint32_t *s = (uint32_t *)buf;
	uint32_t *e = (uint32_t *)(buf+bytes);

	while (s < e) {
		*s = bswap_32(*s);
		s++;
	}
}
