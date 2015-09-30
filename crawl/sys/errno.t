// sys/errno.t -- Operating system error handling.
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
//* Module for accessing operating system errors.

module sys.errno;


//* The variable @var{errno} contains the result code set by the
//* last operating system call.  A value of 0 means success, all
//* other values indicate failure.  The value can be translated to
//* a readable error message using the @code{strerror} function
//* below.
//
public var errno: int := 0;


//* Return a string describing the error code passed in the
//* argument @var{errnum}.
//
public fun strerror (errnum: int): string;

// End of sys/errno.t.
