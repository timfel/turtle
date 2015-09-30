// listsearch.t -- List search utilities.
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
//* Function for searching in lists of type
//*
//* @example
//* list of @var{A}
//* @end example
//*
//* where @var{A} must be instantiated when importing this module.
//*
//* The comparison function @var{cmp} is used for comparing elements of the
//* input arrays.  @var{cmp} is expected to return a value less than 0 if
//* the first argument is to be considered smaller than the second, a
//* value greater than 0 if it is greater, and exactly 0 if the two
//* arguments are equivalent.

module listsearch<A>;

import lists<A>;


//* Search linearly through the list @var{l}, until an element equal
//* to @var{elem} is found.  Use @var{cmp} for comparing the elements of
//* the list to @var{elem}.  Return the first list cell whose head is
//* equal to @var{elem}, or @code{null} if not found.
//
public fun lsearch (l: list of A, elem: A, cmp: fun (A, A): int): list of A
  while l <> null do
    if cmp (elem, hd l) = 0 then
      return l;
    end;
    l := tl l;
  end;
  return null;
end;

// End of listsearch.t.
