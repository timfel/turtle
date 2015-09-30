// hash.t -- Hash functions for basic data types.
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
//* This module exports hash functions for the basic data types.

module hash;

import core, chars, longs;


//* Calculate a hash value for the given argument.  The returned 
//* value is in the range 0@dots{}@code{ints.max}.
//
public fun hash (i: int): int
  if i < 0 then
    return -i;
  else
    return i;
  end;
end;
//* ""
public fun hash (l: long): int
  var i: int := longs.to_int (l);
  if i < 0 then
    return -i;
  else
    return i;
  end;
end;
//* ""
public fun hash (r: real): int
  var i: int := core.real_to_int (r);
  if i < 0 then
    return -i;
  else
    return i;
  end;
end;
//* ""
public fun hash (b: bool): int
  if b then
    return 1;
  else
    return 0;
  end;
end;
//* ""
public fun hash (c: char): int
  return chars.ord (c);
end;
//* ""
public fun hash (s: string): int
  var x: int := 0;
  var h: int := 234562;
  while x < sizeof s do
    h := h * 11 + chars.ord (s[x]);
    x := x + 1;
  end;
  if h < 0 then
    return -h;
  else
    return h;
  end;
end;

// End of hash.t.
