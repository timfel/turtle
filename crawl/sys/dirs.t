// sys/dirs.t -- File handling.
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
//* Module for handling directories.  The exported data type
//* @code{dirs} serves as a handle for directories and must be
//* obtained by calling the function @code{opendir}.  Calling
//* @code{readdir} repeatedly on a valid @code{dir} value will
//* return all entries of a directory, and the directory stream can
//* then be closed with @code{closedir} or reset to the beginning
//* with @code{rewinddir}.

module sys.dirs;

import internal.binary, sys.errno;


//* Directory handle for use with the directory functions below.
//
public datatype dir = _adir;


//* Open a directory stream corresponding with the directory called
//* @var{name}, and return a handle for that stream.  The stream is
//* positioned at the first entry in the directory.  Return a
//* handle for the opened stream on success, or @code{null} if an
//* error occurs.  The variable @code{sys.errno.errno} will be set
//* accordingly.
//
public fun opendir (name: string): dir;


//* Return a string representing the next directory entry in the
//* directory stream @var{d}.  Return @code{null} if the end of the
//* stream is reached or an error occurs.  @code{sys.errno.errno}
//* will be set accordingly.
//
public fun readdir (d: dir): string;


//* Close the directory stream associated with the handle @var{d}.
//* The stream descriptor @var{d} cannot be used anymore after this
//* call.  Return 0 on success or -1 on failure, and set
//* @code{sys.errno.errno} accordingly.
//
public fun closedir (d: dir): int;


//* Reset the position of the directory stream @var{d} to the
//* beginning of the directory.
//
public fun rewinddir (d: dir);

// End of sys/dirs.t.
