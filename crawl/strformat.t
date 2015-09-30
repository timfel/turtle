// strformat.t -- Turtle module for string formatting.
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
//* Library module for string formatting.

module strformat;

import strings, lists<char>, union, ints, longs, reals, bools, chars;


//* String formatting function.  This is similar to the
//* @code{sprintf()} function in C, but much more limited.
//* @var{fmt} is expected to be a string with embedded formatting
//* instruction.  A formatting instruction is a @code{%} character
//* followed by an @code{s} character.  In the result string, each
//* occurence of a formatting instruction is replaced by the
//* corresponding element in the argument list.
//
public fun fmt (fmt: string, args: list of string): string
  var fmtl: list of char := strings.explode (fmt);
  var resl: list of char := null;
  var argl: list of char;

  while fmtl <> null do
    if hd fmtl = '%' then
      if hd tl fmtl = 's' then
	argl := strings.explode (hd args);
	args := tl args;
	resl := lists.append (lists.reverse (argl), resl);
      end;
      fmtl := tl tl fmtl;
    else
      resl := hd fmtl :: resl;
      fmtl := tl fmtl;
    end;
  end;
  return strings.rimplode (resl);
end;

//* Convenience versions of the function above, to be used for one
//* or two argument strings.
//
public fun fmt (fmt: string, s: string): string
  return fmt (fmt, [s]);
end;
//* ""
public fun fmt (fmt: string, s1: string, s2: string): string
  return fmt (fmt, [s1, s2]);
end;

//* Generalized version of the formatting function.  This accepts a
//* list of @code{union.union} values, which can hold different
//* data values.  The supported formatting sequences are:
//* 
//* @table @code
//* @item %s
//* Format a string.
//* 
//* @item %i, %d
//* Format an integer value.
//* 
//* @item %l
//* Format a long value.
//* 
//* @item %r
//* Format a real value.
//* 
//* @item %c
//* Format a character.
//* 
//* @item %b
//* Format a boolean value.
//* @end table
//* 
public fun fmt (fmt: string, args: list of union.union): string
  var fmtl: list of char := strings.explode (fmt);
  var resl: list of char := null;
  var argl: list of char;

  while fmtl <> null do
    if hd fmtl = '%' then
      fmtl := tl fmtl;
      if hd fmtl = 's' then
	argl := strings.rexplode (union.s (hd args));
	args := tl args;
      elsif hd fmtl = 'i' or hd fmtl = 'd' then
	argl := strings.rexplode (ints.to_string (union.i (hd args)));
	args := tl args;
      elsif hd fmtl = 'l' then
	argl := strings.rexplode (longs.to_string (union.l (hd args)));
	args := tl args;
      elsif hd fmtl = 'c' then
	argl := strings.rexplode (chars.to_string (union.c (hd args)));
	args := tl args;
      elsif hd fmtl = 'b' then
	argl := strings.rexplode (bools.to_string (union.b (hd args)));
	args := tl args;
      elsif hd fmtl = 'r' then
	argl := strings.rexplode (reals.to_string (union.r (hd args)));
	args := tl args;
      else
	argl := null;
      end;
      resl := lists.append (argl, resl);
      fmtl := tl fmtl;
    else
      resl := hd fmtl :: resl;
      fmtl := tl fmtl;
    end;
  end;
  return strings.rimplode (resl);
end;

// End of strformat.t.
