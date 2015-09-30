// internal/ex.t -- Exception raising and handling.
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
//* Low-level module for raising and handling exceptions.  Do not use this
//* module directly, rather use the standard library module
//* @code{exception} (@pxref{exceptions module}).

module internal.ex;


//* Raise an exception with name @var{s}.
//
public fun raise (s: string);


//* Call the function @var{thunk}.  If any exception is raised
//* while @var{thunk} is running, the function @var{handler} will
//* be called with the exception name as the only argument.  When
//* @var{handler} returns, it will return to the caller of
//* @code{ex.handle}, in the same way as the call would return when
//* @var{thunk} was returning without an exception.
//
public fun handle (thunk: (fun (): ()), handler: fun (string));


//* The return value of these functions is the corresponding
//* exception name, which is the same that would be used if the
//* illegal operation would be performed.
//* 
//* That means that
//* 
//* @example
//* internal.ex.raise (internal.ex.subscript_ex ())
//* @end example
//* 
//* has the same effect as
//* 
//* @example
//* var s: string;
//* s[-1] := 'a'
//* @end example
//
public fun null_pointer_exception (): string;
//* ""
public fun out_of_range_exception (): string;
//* ""
public fun subscript_exception (): string;
//* ""
public fun wrong_variant_exception (): string;
//* ""
public fun require_exception (): string;

// End of internal/ex.t.
