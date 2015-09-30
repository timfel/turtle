// ints.t -- Turtle module for integer data type related constants.
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
//* This is a library module exporting integer data type related 
//* constants and functions.

module ints;

import core, chars;


//* @code{min} is the smallest representable integer value.
//* This value may (and most probably will) differ from the minimum
//* integer value representable on the underlying hardware.
//
public const min: int := -536870912;


//* @code{max} is the largest representable integer value.
//* This value may (and most probably will) differ from the maximum
//* integer value representable on the underlying hardware.
//
public const max: int := 536870911;


//* Return the minimum of two integer values.
//
public fun min (x: int, y: int): int
  if x < y then
    return x;
  else
    return y;
  end;
end;


//* Return the maximum of two integer values.
//
public fun max (x: int, y: int): int
  if x > y then
    return x;
  else
    return y;
  end;
end;


//* Convert the string value @var{s} to the integer number it 
//* represents.  If @var{s} does not represent any integer number,
//* or if the resulting integer is not in the range 
//* @code{ints.min}@dots{}@code{inits.max}, the return value is 
//* unspecified.
//
public fun from_string (s: string): int
  var res: int := 0;
  var idx: int := 0;
  var limit: int := sizeof s;
  var negate: bool := false;

  // Skip leading whitespace.
  while (idx < limit) and (s[idx] = ' ') do
    idx := idx + 1;
  end;
  if s[idx] = '-' then
    negate := true;
    idx := idx + 1;
  elsif s[idx] = '+' then
    idx := idx + 1;
  end;
  if idx >= limit then
    return 0;
  end;
  while idx < limit do
    if (s[idx] < '0') or (s[idx] > '9') then
      return 0;
    end;
    // FIXME: Detect overflow.
    if res > ints.max / 10 then
      return 0;
    end;
    res := res * 10 + (chars.ord (s[idx]) - chars.ord ('0'));
    idx := idx + 1;
  end;
  if negate then
    res := -res;
  end;
  return res;
end;


//* Convert the integer value @var{i} to its string representation.
//
public fun to_string (i: int): string
  var x: int;
  var len: int;
  var s: string;
  var sign: bool;

  if i < 0 then
    if i = ints.min then
      return "-536870912";
    end;
    sign := true;
    i := -i;
  else
    sign := false;
  end;

  len := 0;
  x := i;
  while (x <> 0) do
    x := x / 10;
    len := len + 1;
  end;
  if (len = 0) then
    return "0";
  end;
  
  if sign then
    len := len + 1;
  end;

  s := string len of ' ';
  while (i <> 0) do
    len := len - 1;
    s[len] := chars.chr ((i % 10) + chars.ord ('0'));
    i := i / 10;
  end;

  if sign then
    s[0] := '-';
  end;
  return s;
end;


//* Convert the integer value @var{i} to a real value.
//
public fun to_real (i: int): real
  return core.int_to_real (i);
end;


//* Convert the real value @var{r} to an integer value.  Decimal
//* places are stripped.
//
public fun from_real (r: real): int
  return core.real_to_int (r);
end;


//* Return true iff @var{i} is even, odd, equal to zero, positive
//* or negative, respectively.
//
public fun even? (i: int): bool
  return i % 2 = 0;
end;
//* ""
public fun odd? (i: int): bool
  return i % 2 <> 0;
end;
//* ""
public fun zero? (i: int): bool
  return i = 0;
end;
//* ""
public fun positive? (i: int): bool
  return i > 0;
end;
//* ""
public fun negative? (i: int): bool
  return i < 0;
end;


//* Return the absolute value of @var{i}, that is, remove @var{i}'s
//* sign.
public fun abs (i: int): int
  if i < 0 then
    return -i;
  else
    return i;
  end;
end;


//* Return 1 if @var{i} is greater than zero, 0 if @var{i} is equal
//* to zero and -1 if @var{i} is less than zero.
//
public fun signum (i: int): int
  if i < 0 then
    return -1;
  elsif i > 0 then
    return 1;
  else
    return 0;
  end;
end;


//* Return the predecessor or successor of @var{i}, respectively.
//
public fun pred (i: int): int
  return i - 1;
end;
//* ""
public fun succ (i: int): int
  return i + 1;
end;

//* Return @var{b} raised to the power of @var{e}.
//
public fun pow (b: int, e: int): int
  var r: int := 1;
  while e > 0 do
    r := r * b;
    e := e - 1;
  end;
  return r;
end;

// End of ints.t.
