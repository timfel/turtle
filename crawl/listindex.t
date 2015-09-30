// listindex.t -- List reducing utilities.
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
//* Some utility functions for lists which work on indices into the
//* list.

module listindex<A>;

import lists<A>;


//* Return the element at index @var{idx} in list @var{l}.
//
public fun nth (l: list of A, idx: int): A
  while idx > 0 do
    idx := idx - 1;
    l := tl l;
  end;
  return hd l;
end;


//* Return the index of the first element in list @var{l} which
//* satisfies predicate @var{p}.  Return -1 if no element satisfies
//* @var{p}.
//
public fun pos (p: fun (A): bool, l: list of A): int
  var idx: int := 0;
  while l <> null do
    if p (hd l) then
      return idx;
    end;
    idx := idx + 1;
    l := tl l;
  end;
  return -1;
end;


//* Return the sublist of @var{l} which starts at index @var{start}
//* (inclusive) and ends at index @var{ende} (exclusive).
//
public fun slice (l: list of A, start: int, ende: int): list of A
  var res: list of A := null;
  while start > 0 do
    start := start - 1;
    ende := ende - 1;
    l := tl l;
  end;
  while ende > 0 do
    res := hd l :: res;
    ende := ende - 1;
    l := tl l;
  end;
  return lists.reverse (res);
end;

// End of listindex.t.
