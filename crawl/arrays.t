// arrays.t -- Array utilities.
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
//* Utility functions for handling objects of type
//*
//* @example
//* array of @var{A}
//* @end example
//*
//* where @var{A} must be instantiated when importing this module.

module arrays<A>;


//* Return the number of elements of array `a'.
//
public fun length (a: array of A): int
  return sizeof a;
end;


//* Apply the procedure @var{p} to each element of the array @var{a}, 
//* in order.
//
public fun foreach (p: fun(A), a: array of A)
  var idx: int := 0;
  var limit: int := length (a);
  while idx < limit do
    p (a[idx]);
    idx := idx + 1;
  end;
end;


//* Create a copy of array @var{a}.  Note that only the array
//* holding the elements is copied, not the elements themselves.
//* The result is the same as @var{a} if @var{a} is empty.
//
public fun copy (a: array of A): array of A
  if sizeof a > 0 then
    var tmp: array of A;
    var idx: int := 1;

    tmp := array sizeof a of a[0];
    while idx < sizeof a do
      tmp[idx] := a[idx];
      idx := idx + 1;
    end;
    return tmp;
  else
    return a;
  end;
end;


//* Return a new array which contains the elements of @var{a}, but
//* in reverse order.  The result is the same as @var{a} if @var{a}
//* is empty.
//
public fun reverse (a: array of A): array of A
  if sizeof a > 0 then
    var tmp: array of A;
    var idx1: int := 0;
    var idx2: int := sizeof a - 1;

    tmp := array sizeof a of a[0];
    while idx1 < sizeof a do
      tmp[idx1] := a[idx2];
      idx1 := idx1 + 1;
      idx2 := idx2 - 1;
    end;
    return tmp;
  else
    return a;
  end;
end;


//* Create an array of @var{len} elements, where each element is
//* initialized to @var{elem}.
//
public fun replicate (elem: A, len: int): array of A
  return array len of elem;
end;

// End of arrays.t.
