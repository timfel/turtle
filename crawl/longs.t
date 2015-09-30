// longs.t -- Turtle module for long data type related functions.
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
//* This is a library module which exports long data type related
//* functions.

module longs;

import core;

//* @code{min} is the smallest representable long value.
//
public const min: long := -2147483648L;


//
//* @code{max} is the largest representable long value.
public const max: long := 2147483647L;


//* Return the minimum of two long values.
//
public fun min (x: long, y: long): long
  if x < y then
    return x;
  else
    return y;
  end;
end;


//* Return the maximum of two long values.
//
public fun max (x: long, y: long): long
  if x > y then
    return x;
  else
    return y;
  end;
end;


//* Convert the string value @var{s} to the long number it represents.
//* If @var{s} does not represent any long integer number,
//* the return value is unspecified.
//
public fun from_string (s: string): long
  return core.string_to_long (s);
end;


//* Convert the long number @var{l} to its string representation.
//
public fun to_string (l: long): string
  return core.long_to_string (l);
end;


//* Convert the integer value @var{i} to a long value.
//
public fun from_int (i: int): long
  return core.int_to_long (i);
end;


//* Convert the long value @var{l} to an integer value.
//
public fun to_int (l: long): int
  return core.long_to_int (l);
end;


//* Conver the real number @var{r} to a long value, stripping off
//* any decimal places.
//
public fun from_real (r: real): long
  return core.real_to_long (r);
end;


//* Conver the long value @var{l} to a real value.
//
public fun to_real (l: long): real
  return core.long_to_real (l);
end;


//* Return true iff @var{l} is even, odd, equal to zero, positive
//* or negative, respectively.
//
public fun even? (l: long): bool
  return l % 2L = 0L;
end;
//* ""
public fun odd? (l: long): bool
  return l % 2L <> 0L;
end;
//* ""
public fun zero? (l: long): bool
  return l = 0L;
end;
//* ""
public fun positive? (l: long): bool
  return l > 0L;
end;
//* ""
public fun negative? (l: long): bool
  return l < 0L;
end;


//* Return the absolute value of @var{l}, that is, remove @var{l}'s
//* sign.
public fun abs (l: long): long
  if l < 0L then
    return -l;
  else
    return l;
  end;
end;


//* Return 1L if @var{l} is greater than zero, 0L if @var{l} is
//* equal to zero and -1L if @var{l} is less than zero.
//
public fun signum (l: long): long
  if l < 0L then
    return -1L;
  elsif l > 0L then
    return 1L;
  else
    return 0L;
  end;
end;


//* Return the predecessor or successor of @var{i}, respectively.
//
public fun pred (l: long): long
  return l - 1L;
end;
//* ""
public fun succ (l: long): long
  return l + 1L;
end;


//* Return @var{b} raised to the power of @var{e}.
//
public fun pow (b: long, e: int): long
  var r: long := 1L;
  while e > 0 do
    r := r * b;
    e := e - 1;
  end;
  return r;
end;

// End of longs.t.
