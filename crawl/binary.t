// binary.t -- Turtle module for binary value handling
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
//* Support module for binary (byte-) arrays.

module binary;

import chars, internal.binary;


//* The @code{binary} data type is an alias for the type of the
//* same name from the module @code{internal.binary}, where the
//* real type and function definitions reside.
//
public type binary = internal.binary.binary;


//* These functions create binary arrays of a given size, extract an
//* element from these arrays or stores an integer into a specified
//* location.  @code{binary_size} returns the number of elements in the
//* binary array @var{b}.
//
public const make: fun (int): binary := internal.binary.make;
//* ""
public const get: fun (binary, int): int := internal.binary.get;
//* ""
public const set: fun (binary, int, int) := internal.binary.set;
//* ""
public const size: fun (binary): int := internal.binary.size;


//* Convert the binary array @var{b} to a string by simply converting
//* the integer values in @var{b} to characters using the function
//* @code{chars.chr}.
//
public fun to_string (b: internal.binary.binary): string
  var limit: int := internal.binary.size (b);
  var s: string := string limit of ' ';
  var x: int := 0;
  while x < limit do
    s[x] := chars.chr (internal.binary.get (b, x));
    x := x + 1;
  end;
  return s;
end;


//* Convert the string @var{s} to a binary array by converting the
//* characters in the string to their code values using the function
//* @code{chars.ord}.
//
public fun from_string (s: string): internal.binary.binary
  var limit: int := sizeof s;
  var b: internal.binary.binary := internal.binary.make (limit);
  var x: int := 0;
  while x < limit do
    internal.binary.set (b, x, chars.ord (s[x]));
    x := x + 1;
  end;
  return b;
end;

// End of binary.t.
