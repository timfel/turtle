// listreduce.t -- List reducing utilities.
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
//* The module file @file{listreduce} implements the functions
//* @code{reducel} and @code{reducer} for reducint functions over
//* lists.

module listreduce<from, to>;


//* Reduce the list @var{l} with function @var{f}, bracketing on
//* the left.
//* 
//* @example
//* fun sub (x: int, y: int): int
//*   return x - y;
//* end;
//* reducel (sub, 12, [2, 5, 3])
//* @result{}
//* 3 - (5 - (2 - 12))
//* @result{}
//* -12
//* @end example
//
public fun reducel (f: fun (from, to): to, init: to, l: list of from): to
  if l = null then
    return init;
  else
    return reducel (f, f (hd l, init), tl l);
  end;
end;


//* Reduce the list @var{l} with function @var{f}, bracketing on
//* the right.
//* 
//* @example
//* fun sub (x: int, y: int): int
//*   return x - y;
//* end;
//* reducel (sub, 12, [2, 5, 3])
//* @result{}
//* 2 - (5 - (3 - 12))
//* @result{}
//* -12
//* @end example
//
public fun reducer (f: fun (from, to): to, init: to, l: list of from): to
  if l = null then
    return init;
  else
    return f (hd l, reducer (f, init, tl l));
  end;
end;

// End of listreduce.t.
