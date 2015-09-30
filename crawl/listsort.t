// listsort.t -- List sorting.
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
//* Function for sorting lists of type
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

module listsort<A>;

import lists<A>;


//* Sort the list @var{a}, using @var{cmp} as the comparison function.
//* The parameter @var{a} is not modified for performing the sort,
//* instead a freshly allocated list is returned.
//
public fun sort (a: list of A, cmp: fun (A, A): int): list of A

  // These two helper functions return suitable predicate
  // functions for the filter function from the `lists'
  // module.
  //
  fun smaller (elem1: A): fun (A): bool
    return fun (elem2: A): bool
             return cmp (elem1, elem2) > 0;
           end;
  end;
  fun greatereq (elem1: A): fun (A): bool
    return fun (elem2: A): bool
             return cmp (elem1, elem2) <= 0;
           end;
  end;

  // Recursive helper function.  This is a simple quicksort, 
  // pivoting on the first list element.
  //
  fun sort (a: list of A): list of A
    if a = null or tl a = null then
      return a;
    elsif tl tl a = null then
      if cmp (hd a, hd tl a) > 0 then
	return [hd tl a, hd a];
      else
	return a;
      end;
    else
      var l1: list of A := lists.filter (smaller (hd a), tl a);
      var l2: list of A := lists.filter (greatereq (hd a), tl a);
      return lists.append (sort (l1), hd a :: sort (l2));
    end;
  end;

  return sort (a);
end;

// End of listsort.t.
