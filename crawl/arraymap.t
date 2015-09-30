// arraymap.t -- Array mapping function for Turtle.
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
//* Utility function for applying functions to arrays of type
//*
//* @example
//* array of @var{A}.
//* @end example
//*
//* @code{Map} returns values of type
//*
//* @example
//* array of @var{B}.
//* @end example
//*
//* @var{A} and @var{B} are type variables which must be instantiated when 
//* importing this module.

module arraymap<A, B>;

import arrays<A>;


//* Apply the function @var{f} to every element of the array @var{a},
//* return an
//* array containing the results of the function applications.  The
//* order in which @var{f} is applied to the array elements is not
//* specified.
//
public fun map(f: fun(A): B, l: array of A): array of B
  var b: B; // Needed as initializer in array constructor below.
  var idx: int := 0; 
  var limit: int := arrays.length (l);
  var res: array of B := array limit of (b);

  while idx < limit do
    res[idx] := f (l[idx]);
    idx := idx + 1;
  end;
  return res;
end;

// End of arraymap.t.
