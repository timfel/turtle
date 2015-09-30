// union.t -- Union type for the builtin data types.
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
//* This module exports an union type for the builtin data types.

module union;


//* This data type is intended to be used for functions which
//* should be able to deal with all or some of the builtin data
//* types.
public datatype union = i (i: int) or
                        l (l: long) or
                        r (r: real) or
			b (b: bool) or
			c (c: char) or
			s (s: string);

// End of union.t.
