// listmap.t -- List mapping function for Turtle.
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
//* The module file @file{listmap} exports higher-order functions for
//* iterating over lists.  It is a generic module with two module
//* parameters, which are the types of the list elements.  The name of the
//* first parameter is @var{A} and denotes the element type of the
//* input lists, the name of the second is @var{B} and stands for the
//* element type of the output lists of @code{map}.

module listmap<A, B>;

import lists<A>;


//* Apply the function @var{f} to every element of @var{l}, return a list
//* containing the results of the function applications.
//* The order in which @var{f} is applied to the list elements is not 
//* specified.
//
public fun map(f: fun(A): B, l: list of A): list of B
  if lists.empty? (l) then
    return null;
  else
    return  f (hd l) :: map (f, tl l);
  end;
end;

// End of listmap.t.
