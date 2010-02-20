CFLAGS=-g -I. -I../common -D_FILE_OFFSET_BITS=64

VPATH=.:../common

COMMON=common.o fs.o
OBJS=tfhd.o $(COMMON) blkio_unix.o common_unix.o

tfhd: $(OBJS)

tfhd.o:		fs.h blkio.h common.h port.h
common.o:	common.h port.h
fs.o:		fs.h blkio.h common.h port.h
blkio_unix.o:	blkio.h common.h port.h
common_unix.o:	common.h port.h
