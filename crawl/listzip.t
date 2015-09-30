// listzip.t -- List reducing utilities.
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
//* Functions for combining two lists into one element-wise, and
//* the reverse.

module listzip<from1, from2, to>;


//* Zip two lists of equal length together by applying the function
//* @var{f} to the corresponding elements of the lists and forming
//* the result lists from the function result(s).
//
public fun zip (f: fun (from1, from2): to, 
                l1: list of from1, l2: list of from2): list of to
  if l1 = null then
    return null;
  else
    return f (hd l1, hd l2) :: zip (f, tl l1, tl l2);
  end;
end;


//* Decompose the list @var{l} by applying the function @var{f} to
//* the successive list elements and returning the two lists of the
//* function results.
//
public fun unzip (f: fun (to): (from1, from2), l: list of to): 
                                               (list of from1, list of from2)
  if l = null then
    return (null, null);
  else
    var res1: from1;
    var res2: from2;
    var i1: list of from1;
    var i2: list of from2;
    res1, res2 := f (hd l);
    i1, i2 := unzip (f, tl l);
    return (res1 :: i1, res2 :: i2);
  end;
end;

// End of listzip.t.
