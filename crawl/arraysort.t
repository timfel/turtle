// arraysort.t -- Array sorting.
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

module arraysort<A>;

import arrays<A>;



//* Sort the array @var{a}, using @code{cmp} as the comparison function.
//* The parameter @var{a} is modified for performing the sort.
//
public fun sort (a: array of A, cmp: fun (A, A): int)

  // Quicksort.  Private function for implementing the public `sort'
  // function above.
  //
  fun sort (i: int, j: int)
    fun swap (i: int, j: int)
      a[i], a[j] := a[j], a[i];
    end;

    fun pivot (i: int, j: int): A
      var m: int;
      m := (i + j) / 2;
      return a[m];
    end;

    if i >= j then
      return;
    else
      if i + 1 = j then
	if cmp (a[i], a[j]) > 0 then
	  swap (i, j);
	end;
      else
	var w: int, b: int, r: int;
	var pivot: A;

	pivot := pivot (i, j);
	w := i;
	b := i - 1;
	r := j + 1;
      
        while w < r do
	  var c: int;
	  c := cmp (a[w], pivot);
	  if c < 0 then
	    swap (w, b + 1);
	    b := b + 1;
	    w := w + 1;
	  else
	    if c > 0 then
	      swap (w, r - 1);
	      r := r - 1;
	    else
	      w := w + 1;
	    end;
	  end;
	end; // while

	sort (i, b);
	sort (r, j);
      end;
    end;
  end;

  sort (0, arrays.length (a) - 1);
end;

// End of arraysort.t.
