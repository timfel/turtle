// pairs.t -- Tuple utilities.
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
//* Utility functions for tuples of type
//* 
//* @example
//* (A, B)
//* @end example
//* 
//* where @var{A} and @var{B} must be instantiated when importing
//* this module.

module pairs<A, B>;


//* This datatype wraps a pair of values in an user-defined data type.
//
public datatype pair<A, B> = pair (first: A, second: B);


//* These are conversion functions between the user-defined
//* @code{pair} data type and 2-tuples.
//
public fun unpair (p: pair): (A, B)
  return first (p), second (p);
end;
//* ""
public fun pair (p: (A, B)): pair
  var a: A, b: B;
  a, b := p;
  return pair (a, b);
end;


//* Return the first or second component of the argument tuple,
//* respectively.
//
public fun first (p: (A, B)): A
  var a: A, b: B;
  a, b := p;
  return a;
end;
//* ""
public fun second (p: (A, B)): B
  var a: A, b: B;
  a, b := p;
  return b;
end;

// End of pairs.t.
