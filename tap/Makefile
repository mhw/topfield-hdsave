TOOLS=/usr/local/topfield-gcc

CC=$(TOOLS)/bin/mips-gcc
CFLAGS=-g -I. -I../common

VPATH=.:../common

COMMON=common.o fs.o fs_fat.o fs_io.o
OBJS=$(COMMON)

hdsave.tap: $(OBJS)

clean:
	rm hdsave.tap $(OBJS)

tfhd.o:		fs.h blkio.h common.h port.h
common.o:	common.h port.h
fs.o:		fs.h blkio.h common.h port.h
blkio_unix.o:	blkio.h common.h port.h
common_unix.o:	common.h port.h
