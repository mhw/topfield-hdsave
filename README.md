Topfield HDSave
===============

Basic Idea
----------

HDSave has a couple of parts. The first part is a TAP that runs on the
Topfield device making periodic backups of the directory structure and
extent locations of all files on the Topfield hard disc (this backup we
will refer to as the 'disk map'). The second part is a program that runs
on a PC (under Windows, Linux or OS X) and allows files to be recovered
from a disk by using the extent locations captured by the TAP. The idea
is that the disk map represents a complete enough backup of the Topfield 
disk structure that the majority of the underlying recordings could be
recovered in the event that the disk structure gets trashed by the crash.

Some Implementation Thoughts
----------------------------

* The TAP needs to be written in C. It will be easier for me to build
  the code on a Linux machine with a Topfield disk plugged in to it,
  do the core of the directory interpretation code needs to be abstracted
  from the bits that implement reading raw disk blocks.
* This abstraction layer can also be used to make the command line
  recovery tool portable between Unix and other operating systems.
* The disk structure dumper should be runnable as a command line program
  under Linux for testing. It should be simple enough to wrap the core
  disk structure dumping logic into either a CLI program or a TAP.

Disk Map Structure and Location
-------------------------------

* The disk map should be written into a file in the Topfield filesystem
  when running as a TAP. This would allow the disk map to be copied off
  the device periodically by an attached WL500g or similar device.
* The disk map file should be overwritten rather than recreated each time.
  This would allow an older backup of the disk map to provide the location
  of a more recent disk map on the device itself.
* The disk map could also be written to the last sectors of the disk if
  these sectors are unused, on the basis that they are likely to remain
  unused and will be easy to find after a disk structure failure.
* If the last sectors are used it may then be more useful to recreate the
  disk map file each time it is written to maximise the number of copies
  of the disk map that might be left on the disk in the event of a crash.

The disk map should have header and trailer lines that allow the map
to be found and validated. If there is no off-disk backup of the disk
map and the last sectors contain file data a disk scan may be able to
locate the sectors that contained the file copy of the disk map by
looking for the disk map header. By matching something like a timestamp
between a header line and a footer line it should be possible to verify
that the whole disk map has been recorded. A checksum might also be a
good idea.

If the disk map spans more than one disk block (whatever the basic Topfield
disk block size is) it is possible that the disk map could be split into
smaller fragments. This might mean that it is better to write header and
trailer lines around each block-sized chunk of the disk map.

The disk map should be plain text. In the simplest case it should be
possible to take the raw block offsets and lengths from a copy of the disk
map and use the Unix 'dd' command to extract the file data from the disk.
