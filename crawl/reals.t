// reals.t -- Turtle module for real data type related functions.
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
//* This is a library module which exports real data type related
//* functions.

module reals;

import core;


//* Return the minimum of two real values.
//
public fun min (x: real, y: real): real
  if x < y then
    return x;
  else
    return y;
  end;
end;


//* Return the maximum of two real values.
//
public fun max (x: real, y: real): real
  if x > y then
    return x;
  else
    return y;
  end;
end;


//* Convert the string value @var{s} to the real number it represents.
//*  If @var{s} does not represent any integer number,
//* the return value is unspecified.
//
public fun from_string (s: string): real
  return core.string_to_real (s);
end;


//* Convert the real number @var{r} to its string representation.
//
public fun to_string (r: real): string
  return core.real_to_string (r);
end;


//* Convert the integer value @var{i} to a real value.
//
public fun from_int (i: int): real
  return core.int_to_real (i);
end;


//* Convert the real value @var{r} to an integer value.  Decimal
//* places are stripped.
//
public fun to_int (r: real): int
  return core.real_to_int (r);
end;

//* Return true iff @var{i} is equal to zero, positive or negative,
//* respectively.
//
public fun zero? (r: real): bool
  return r = 0.0;
end;
//* ""
public fun positive? (r: real): bool
  return r > 0.0;
end;
//* ""
public fun negative? (r: real): bool
  return r < 0.0;
end;


//* Return the absolute value of @var{r}, that is, remove @var{r}'s
//* sign.
public fun abs (r: real): real
  if r < 0.0 then
    return -r;
  else
    return r;
  end;
end;


//* Return 1.0 if @var{r} is greater than zero, 0.0 if @var{r} is
//* equal to zero and -1.0 if @var{r} is less than zero.
//
public fun signum (r: real): real
  if r < 0.0 then
    return -1.0;
  elsif r > 0.0 then
    return 1.0;
  else
    return 0.0;
  end;
end;


//* Return @var{a} raised to the power of @var{b}.
//
public fun pow (a: real, b: int): real
  var res: real := 1.0;
  if b > 0 then
    while b > 0 do
      res := res * a;
      b := b - 1;
    end;
  elsif b > 0 then
    while b < 0 do
      res := res / a;
      b := b + 1;
    end;
  end;
  return res;
end;

// End of reals.t.
