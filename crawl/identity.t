// identity.t -- The identity function.
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
//* This module exports the function @code{id}, which is the identity
//* on values of the type given as the module parameter @var{A}.

module identity<A>;


//* Return the argument @var{data} unchanged. This function is especially
//* useful for higher-order functions which expect a function to apply to
//* all members of a collection, such as @code{map}.  For example,
//*
//* @example
//* l := listmap.map (identity.id, l)
//* @end example
//*
//* can be used to copy a list.
//
public fun id (data: A): A
  return data;
end;

// End of identity.t.
