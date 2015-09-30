// compare.t -- Comparison functions.
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

/** The module @code{compare} exports several comparison functions,
    where each of the functions compares two values of a basic data
    type.  These functions are especially useful as parameters for
    the higher-order functions, such as the sorting functions in
    the listsort or arraysort modules.  */

module compare;


//* Compare the values @var{x} and @var{y}, and return -1 if the first
//* argument is to be considered smaller, 0 if they are equal, and
//* 1 if the first argument is greater than the second.
//
public fun cmp (x: bool, y: bool): int
  if x > y then
    return 1;
  else
    if x < y then
      return -1;
    else
      return 0;
    end;
  end;
end;

//* ""
public fun cmp (x: int, y: int): int
  if x > y then
    return 1;
  else
    if x < y then
      return -1;
    else
      return 0;
    end;
  end;
end;

//* ""
public fun cmp (x: long, y: long): int
  if x > y then
    return 1;
  else
    if x < y then
      return -1;
    else
      return 0;
    end;
  end;
end;

//* ""
public fun cmp (x: real, y: real): int
  if x > y then
    return 1;
  else
    if x < y then
      return -1;
    else
      return 0;
    end;
  end;
end;

//* ""
public fun cmp (x: char, y: char): int
  if x > y then
    return 1;
  else
    if x < y then
      return -1;
    else
      return 0;
    end;
  end;
end;

//* ""
public fun cmp (x: string, y: string): int
  var idx: int := 0;
  var limitx: int := sizeof x;
  var limity: int := sizeof y;
  var limitm: int := limitx;

  if (limitm > limity) then
    limitm := limity;
  end;

  while (idx < limitm) do
    if x[idx] > y[idx] then
      return 1;
    else
      if x[idx] < y[idx] then
	return -1;
      end;
    end;
    idx := idx + 1;
  end;

  if (limitx > limity) then
    return 1;
  else
    if (limitx < limity) then
      return -1;
    else
      return 0;
    end;
  end;
  
  return 0;
end;

// End of compare.t.
