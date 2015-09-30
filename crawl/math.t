// math.t -- Mathematical functions.
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
//* This module provides mathematical constants and functions.
//
//

// Note:
//
// This module must be compiled with the `--pragma=handcoded' option.
//
// For details on how to hand-code Turtle modules, see the
// Turtle reference manual
 
module math;


//* This is the constant @code{pi}.
//
public const pi: real := 3.14159265358979323846;


//* These are the common trigonometric functions.  They are mapped
//* directly to the functions in the C library.
//
public fun sin (x: real): real;
//* ""
public fun asin (x: real): real;
//* ""
public fun cos (x: real): real;
//* ""
public fun acos (x: real): real;
//* ""
public fun tan (x: real): real;
//* ""
public fun atan (x: real): real;
//* ""
public fun atan (x: real, y: real): real;

// End of math.t.
