// core.t -- Low-level system interface module.
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
//* This file defines some useful low-level functions, but does not
//* implement all of them.  For some of the functions only the
//* declaration is given, and the implementation is written in C in the
//* file core.t.i.
//*
//* The module @code{core} is a very basic library module.  Very low-level
//* functions for input and output are provided.  The user should not use
//* these functions directly, but use one of the higher-level modules like
//* @code{io} instead.

// Note:
//
// This module must be compiled with the `--pragma=handcoded' option.
//
// For details on how to hand-code Turtle modules, see the file
// doc/handcoding.text.
 
module core;


//* Write the character @var{c} to the file descriptor @var{fd}.
//
public fun write_char (fd: int, c: char);


//* Read a character from file descriptor @var{fd}.
//* On end-of-file, the constant @code{chars.EOF} is returned.
//
public fun read_char (fd: int): char;


//* Return the character with character code @var{i}.
//
public fun chr (i: int): char;


//* Return the character code of the character @var{c}.
//
public fun ord (c: char): int;


//* Convert the real number @var{r} to its string representation.
//
public fun real_to_string (r: real): string;


//* Convert the string value @var{s} to a real value.  If @var{s} is
//* not a valid real number representation, the returned value is
//* undefined.
//
public fun string_to_real (s: string): real = "ttl_string_to_real";


//* Convert the long number @var{l} to its string representation.
//
public fun long_to_string (l: long): string;


//* Convert the string value @var{s} to a long value.  If @var{s} is
//* not a valid long number representation, the returned value is
//* undefined.
//
public fun string_to_long (s: string): long = "ttl_string_to_long";


//* Convert the integer value @var{i} to a real value.
//
public fun int_to_real (i: int): real;


//* Convert the real value @var{r} to an integer value, stripping off
//* any decimal places.
//
public fun real_to_int (r: real): int;


//* Convert the long value @var{l} to an integer value.
//
public fun long_to_int (l: long): int;


//* Convert the integer value @var{i} to a long value.
//
public fun int_to_long (i: int): long;


//* Convert the long value @var{l} to a real value.
//
public fun long_to_real (l: long): real;


//* Convert the real value @var{r} to a long value, stripping off
//* any decimal places.
//
public fun real_to_long (r: real): long;

// End of core.t.
