Topfield HDSave
===============

by Mark Wilkinson <mhw+hdsave@kremvax.net>

<http://github.com/mhw/topfield-hdsave/>

Current State
-------------

HDSave is still in development and should be considered alpha quality.
The Unix command line program (`tfhd`) is able to list the contents
of directories and copy files from the Topfield hard disk to the host
filesystem. This might be enough functionality for you to warrant the
risk of letting this code loose on your hard disk. To make you feel
more comfortable I'll point out that the raw disk device is opened
read-only, so it should not be possible for `tfhd` to alter the disk's
contents. Your mileage may vary though.

All development work and testing has been done on a 64-bit Intel
processor; the software has not been tested on a 32-bit system yet.

Work on the TAP mentioned below has not progressed beyond a Makefile
that compiles some of the source code with the Topfield MIPS tool chain.

Downloading and Building
------------------------

For the time being you will need to compile HDSave yourself. You can
download a source tarball or zip file, or check the code out using
[git](http://git-scm.com/), from the Github URL above.

Once you have a copy of the source code you should be able to build
the `tfhd` command line program by running `make` in the `unix`
subdirectory:

    $ cd unix
    $ make
    [...]
    $ ./tfhd
    usage: tfhd [options] <command> [args...]
    options:
            -f DEVICE       Topfield disk to manipulate
            -m FILE         Use a previously saved map file
            -s SIZE         Set disk size instead of probing device
    commands:
            info            Print basic information about the disk
            ls [dir]        List contents of a directory
            cp <src> <dst>  Copy contents of a file to host filesystem
    $

If it fails to compile, please let me know what went wrong. Better
still, if you get it to build on a new platform either fork the code
on github, commit your changes and then send me a pull request, or
just send me a patch and I'll incorporate it.

Using `tfhd`
------------

To use `thfd` you need to connect your Topfield hard disk to your PC.
I use USB caddies to do this, but directly attaching the disk to the
IDE or SATA bus should also work. As this necessitates removing the
hard disk from the Toppy I need to say this: Be careful of high
voltages on the power supply components when working inside the Toppy.
Removing the cover of your Toppy will void any warranty on the hardware.
You follow these instructions at your own risk.

`tfhd` needs read access to the raw disk device corresponding to the
Topfield hard disk. I recommend you grant read access to all users on
your system in preference to running `tfhd` as the `root` user. On
an Ubuntu system you can do that like this:

    $ sudo chmod o+r /dev/sdb

replacing `/dev/sdb` with the appropriate raw device. On Ubuntu systems
the device files are recreated each time the device is plugged in or
the system rebooted, so these permission changes are not permanent.

You should now be able to use tfhd to list the contents of the
directories:

    $ ./tfhd -f /dev/sdb ls
    __RECYCLE__/
    DataFiles/
    ProgramFiles/
    MP3/
    $ ./tfhd -f /dev/sdb ls /DataFiles
    [...]
    $

You can copy a file from the Topfield hard disk to the host filesystem
like this:

    $ ./tfhd -f /dev/sdb cp /ProgramFiles/Auto\ Start/MyStuff.tap MyStuff.tap
    $

Note that you need to quote spaces in filenames to prevent the shell
from splitting them into separate arguments.

Future Plans
------------

The intention is for HDSave to have a couple of parts. The first part
is a TAP that runs on the Topfield device making periodic backups of
the directory structure and extent locations of all files on the
Topfield hard disc (this backup we will refer to as the 'disk map').
The second part is a program that runs on a PC (under Windows, Linux or
OS X) and allows files to be recovered from a disk by using the extent
locations captured by the TAP. The idea is that the disk map represents
a complete enough backup of the Topfield disk structure that the majority
of the underlying recordings could be recovered in the event that the
disk structure gets trashed by the crash.

Some Implementation Thoughts
----------------------------

* The TAP needs to be written in C. It will be easier for me to build
  the code on a Linux machine with a Topfield disk plugged in to it,
  so the core of the directory interpretation code needs to be abstracted
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
* The disk map can be found by looking at the first block of each cluster
  to see if it looks like a disk map file. The time to read the first
  block of each cluster should be reasonably small.
* The disk map file should be written to a new file each time to maximise
  the number of clusters that will contain a copy of the disk map from
  some point in time.
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

If the disk map spans more than one cluster it is possible that the
disk map could be split into smaller fragments. This might mean that
it is better to write header and trailer lines around each block-sized
chunk of the disk map.

The disk map should be plain text. In the simplest case it should be
possible to take the raw block offsets and lengths from a copy of the disk
map and use the Unix 'dd' command to extract the file data from the disk.

Thanks
------

Thanks to:

* Firebird for documenting the disk structure, and for firebirdlib.
* R2-D2 for explaining directory entries when I hadn't read the
  documentation properly.

License
-------

Copyright 2010 Mark H. Wilkinson

HDSave is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HDSave is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with HDSave.  If not, see <http://www.gnu.org/licenses/>.
