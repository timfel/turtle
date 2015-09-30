// internal/binary.t -- Turtle module for binary value handling
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
//* Library module for utility functions for binary values.

module internal.binary;


// This is the experimental interface to binary arrays, that is, to
// array of bytes.  They are supported by the runtime system, but
// currently there is no way to create binary arrays in Turtle.
//
// This datatype is a dummy declaration and provided so that the
// user can declare variables of this type.
//

//* Binary array data type.  Values of this type can be created by
//* calling @code{make_binary}.
//
public datatype binary = binary;


//* These functions create binary arrays of a given size, extract an
//* element from these arrays or stores an integer into a specified
//* location.  @code{binary_size} returns the number of elements in the
//* binary array @var{b}.
//
public fun make (size: int): binary;  // Handcoded in binary.t.i.
//* ""
public fun get (b: binary, index: int): int;  // Handcoded in binary.t.i.
//* ""
public fun set (b: binary, index: int, val: int);// Handcoded in binary.t.i.
//* ""
public fun size (b: binary): int;  // Handcoded in binary.t.i.


// End of internal/binary.t.
