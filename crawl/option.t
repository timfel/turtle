// option.t -- Option data type.
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
//* This module exports one data type, @code{option}, and the corresponding
//* constructor, accessor and discriminator functions.
//*
//* The @code{option} type is intended to be the result of partial functions,
//* which can either succeed and return a useful value, or fail.
//* The former result will be of variant @code{some} with the useful
//* packaged in the @code{data} field, whereas the latter will yield a
//* value of variant @code{none}.
//*
//* This is (of course) inspired by the @code{option} type in ML.

module option<A>;


//* The @code{option} data type.  It either represents nothing
//* (variant @code{none}), or a value of the data type @var{A},
//* which is a parameter to this module.
//
public datatype option<A> = none or
                            some(data: A);

// End of option.t.
