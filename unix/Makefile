CFLAGS=-g -I. -I../common -D_FILE_OFFSET_BITS=64

VPATH=.:../common

COMMON=common.o fs.o fs_dir.o fs_dir_ls.o fs_file.o fs_fat.o fs_io.o
OBJS=tfhd.o $(COMMON) blkio_unix.o common_unix.o

tfhd: $(OBJS)

clean:
	rm tfhd $(OBJS)

tfhd.o:		fs.h blkio.h common.h port.h
common.o:	common.h port.h
fs.o:		fs.h blkio.h common.h port.h
fs_dir.o:	fs.h blkio.h common.h port.h
fs_dir_ls.o:	fs.h blkio.h common.h port.h
fs_file.o:	fs.h blkio.h common.h port.h
fs_fat.o:	fs.h blkio.h common.h port.h
fs_io.o:	fs.h blkio.h common.h port.h
blkio_unix.o:	blkio.h common.h port.h
common_unix.o:	common.h port.h
