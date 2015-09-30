// arraysearch.t -- Array search utilities.
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
//* This module exports a functions for sorting objects of type
//*
//* @example
//* array of @var{A}
//* @end example
//*
//* where @var{A} must be instantiated when importing this module.
//*
//* The comparison function @code{cmp} is used for comparing elements of the
//* input arrays.  @code{cmp} is expected to return a value less than 0 if
//* the first argument is to be considered smaller than the second, a
//* value greater than 0 if it is greater, and exactly 0 if the two
//* arguments are equivalent.

module arraysearch<A>;

import arrays<A>;


//* Search linearly through the array @var{a}, until an element equal
//* to @var{elem} is found.  Use @var{cmp} for comparing the elements of
//* the array to @var{elem}.  Return the index of the first occurence
//* if found; return -1 otherwise.
//*
//* Note that if @var{elem} appear more than once in the array, the
//* index of the first occurence is returned.
//
public fun lsearch (a: array of A, elem: A, cmp: fun (A, A): int): int
  var i: int;
  var limit: int;

  limit := arrays.length (a);
  i := 0;
  while i < limit do
    if cmp (a[i], elem) = 0 then
      return i;
    end;
    i := i + 1;
  end;
  return -1;
end;


//* Binary search.  Search in the array @var{a} for an element equal to 
//* @var{elem}, using @var{cmp} as the comparison function.  Return the
//* index of the element if found, return -1 otherwise.
//*
//* The array @var{a} must be sorted according to the comparison function
//* @var{cmp}, otherwise the search will most probably fail, even if
//* @var{elem} does appear in @var{a}.
//*
//* Note that if @var{elem} appear more than once in the array, 
//* it is not specified the index of which will be returned.
//
public fun bsearch(a: array of A, elem: A, cmp: fun (A, A): int): int
  var l: int, r: int, m: int;

  l := 0;
  r := arrays.length (a);
  while l < r do
    var c: int;

    m := (l + r) / 2;
    c := cmp (elem, a[m]);
    if c < 0 then
      r := m;
    else
      if c > 0 then
	l := m + 1;
      else
	return m;
      end;
    end;
  end;
  return -1;
end;

// End of arraysearch.t.
