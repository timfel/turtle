// sys/files.t -- File handling.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//  
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this package; see the file COPYING.  If not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.

// Commentary:
//
//* Module for handling files.

module sys.files;

import internal.binary, sys.errno;

//* File descriptors for standard input, output and error..
public const stdin: int := 0;
//* ""
public const stdout: int := 1;
//* ""
public const stderr: int := 2;


//* Open the existing file named @var{fname} for reading and return a file
//* descriptor; or return -1 on error.
//
public fun open (fname: string): int;


//* Create a new file named @var{fname} and open it for writing and
//* return a file descriptor; or return -1 on error.  An existing
//* file named @var{fname} will be overwritten, so use with
//* caution.
//
public fun create (fname: string): int;


//* Close the file associated with file descriptor @var{fd}, and
//* return 0 on success or -1 on error.
//
public fun close (fd: int): int;


//* Close the file associated with file descriptor @var{fd}. Ignore
//* any errors.
//
public fun close (fd: int)
  var res: int;
  res := close (fd);
end;


//* Write @var{len} bytes from the byte array @var{b} to the file
//* descriptor @var{fd}, starting at offset 0 of the byte array.
//* Return the number of bytes actually written, or -1 if an error
//* occurs.  The variable @code{sys.errno.errno} is set accordingly.
//
public fun write (fd: int, b: internal.binary.binary, len: int): int;


//* Read @var{len} bytes from the file descriptor @var{fd} into the
//* byte array @var{b}.  Return the number of bytes read, or -1 if
//* an error occurs.  The variable @code{sys.errno.errno} is set
//* accordingly.
//
public fun read (fd: int, b: internal.binary.binary, len: int): int;


//* Delete a name from the filesystem.  If that name was the alst
//* linkt to a file and no processes have the file open the file is
//* deleted and the space it was using is made available for reuse.
//* 
//* On success, zero is returned.  On error, -1 is returned and the
//* variable @code{sys.errno.errno} is set appropriately.
//
public fun unlink (filename: string): int;


public datatype stat = stat (dev: int, ino: int, mode: int, nlink: int,
                             uid: int, gid: int, rdev: int, size: long,
			     blksize: long, blocks: long, 
			     atime: long, mtime: long, ctime: long);


//* -
fun istat (filename: string, s: stat): stat;


public fun stat (filename: string): stat
  var s: stat := stat (0, 0, 0, 0, 0, 0, 0, 0L, 0L, 0L, 0L, 0L, 0L);
  return istat (filename, s);
end;

// End of sys/files.t.
