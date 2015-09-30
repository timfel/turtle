// triples.t -- Tuple utilities.
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
//* (A, B, C)
//* @end example
//* 
//* where @var{A}, @var{B} and @var{C} must be instantiated when
//* importing this module.

module triples<A, B, C>;


//* This datatype wraps a 3-tuples of values in an user-defined
//* data type.
//
public datatype triple<A, B> = triple (first: A, second: B, third: C);


//* These are conversion functions between the user-defined
//* @code{triple} data type and 3-tuples.
//
public fun untriple (t: triple): (A, B, C)
  return first (t), second (t), third (t);
end;
//* ""
public fun triple (t: (A, B, C)): triple
  var a: A, b: B, c: C;
  a, b, c := t;
  return triple (a, b, c);
end;


//* Return the first, second or third component of the argument
//* tuple, respectively.
//
public fun first (p: (A, B, C)): A
  var a: A, b: B, c: C;
  a, b, c := p;
  return a;
end;
//* ""
public fun second (p: (A, B, C)): B
  var a: A, b: B, c: C;
  a, b, c := p;
  return b;
end;
//* ""
public fun third (p: (A, B, C)): C
  var a: A, b: B, c: C;
  a, b, c := p;
  return c;
end;

// End of triples.t.
