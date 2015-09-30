// exceptions.t -- Raising and handling exceptions.
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
//* This module exports some functions for raising and handling
//* exceptions.
//* 
//* Exceptions are raised by calling the function
//* @code{exceptions.raise}, which has the same effect as
//* performing some illegal operation such as taking the head of
//* the empty list.  The argument to @code{raise} is the name of
//* the exception.
//* 
//* Exception handling is done by calling the function
//* @code{exceptions.handle}.  The first argument is a functions
//* which might possibly raise an exception, while the second is a
//* function which will be called when an exception occurs.  If no
//* exception is raised, @code{handle} returns without calling the
//* handler function.

module exceptions;

import internal.ex;


//* Raise an exception with name @var{s}.
//
public fun raise (s: string)
  internal.ex.raise (s);
end;


//* Call the function @var{thunk}.  If any exception is raised
//* while @var{thunk} is running, the function @var{handler} will
//* be called with the exception name as the only argument.  When
//* @var{handler} returns, it will return to the caller of
//* @code{exceptions.handle}, in the same way as the call would
//* return when @var{thunk} was returning without an exception.
//
public fun handle (thunk: (fun (): ()), handler: fun (string))
  internal.ex.handle (thunk, handler);
end;


//* The return value of these functions is the corresponding
//* exception name, which is the same that would be used if the
//* illegal operation would be performed.
//* 
//* That means that
//* 
//* @example
//* exceptions.raise (exceptions.subscript_ex ())
//* @end example
//* 
//* has the same effect as
//* 
//* @example
//* var s: string;
//* s[-1] := 'a'
//* @end example
//
public fun null_pointer_ex (): string
  return internal.ex.null_pointer_exception ();
end;
//* ""
public fun out_of_range_ex (): string
  return internal.ex.out_of_range_exception ();
end;
//* ""
public fun subscript_ex (): string
  return internal.ex.subscript_exception ();
end;
//* ""
public fun wrong_variant_ex (): string
  return internal.ex.wrong_variant_exception ();
end;
//* ""
public fun require_ex (): string
  return internal.ex.require_exception ();
end;

// End of exceptions.t.
