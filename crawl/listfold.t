// listfold.t -- List folding utilities.
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
//* The module file @file{listfold} implements the functions
//* @code{foldl} and @code{foldr} for folding functions over lists.

module listfold<A>;

//* Fold the function @var{f} from left to right over the list
//* @var{l} and return the result.
//* 
//* @example
//* fun sub(x: int, y: int): int
//*   return x - y;
//* end;
//* foldl (sub, [4, 3, 1])
//* @result{}
//* (4 - 3) - 1
//* @result{}
//* 0
//* @end example
//
public fun foldl (f: fun(A, A): A, l: list of A): A
  return ifoldl (f, hd l, tl l);
end;
//* -
public fun ifoldl (f: fun(A, A): A, accu: A, l: list of A): A
  if l = null then
    return accu;
  else
    return ifoldl (f, f (accu, hd l), tl l);
  end;
end;

//* Fold the function @var{f} from to left right over the list
//* @var{l} and return the result.
//* 
//* @example
//* fun sub(x: int, y: int): int
//*   return x - y;
//* end;
//* foldr (sub, [4, 3, 1])
//* @result{}
//* 4 - (3 - 1)
//* @result{}
//* 2
//* @end example
//
public fun foldr (f: fun(A, A): A, l: list of A): A
  if tl l = null then
    return hd l;
  else
    return f (hd l, foldl (f, tl l));
  end;
end;

// End of listfold.t.
