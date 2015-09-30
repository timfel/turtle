// compose.t - Function composition.
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
//* This module exports one function, @code{compose}, which implements
//* function composition.

module compose<arg, inter, res>;


//* Return a function of one argument, which first applies @var{g}
//* to its argument and then @var{f} to the result of this
//* function application.
//
public fun compose(f: fun(inter): res, g: fun(arg): inter): fun(arg): res
  return fun (i: arg): res
           return f(g(i));
	 end;
end;

// End of compose.t.
