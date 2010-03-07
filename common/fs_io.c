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
		fatal("fs_read", "invalid cluster number %d", cluster);
		return 0;
	}

	if (cluster_offset < 0
		|| cluster_offset > fs->bytes_per_cluster)
	{
		fatal("fs_read", "invalid offset within cluster %d", cluster_offset);
		return 0;
	}

	if (bytes % sizeof(uint32_t) != 0)
	{
		fatal("fs_read", "attempt to read %d bytes which isn't a whole number of 32-bit words", bytes);
		return 0;
	}

	offset = (cluster+1)*fs->bytes_per_cluster+cluster_offset;
	if (blkio_read(fs->disk->dev, buf, offset, bytes) == -1)
	{
		return 0;
	}

	fs_swap_bytes(buf, bytes);

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
