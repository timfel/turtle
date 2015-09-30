// lists.t -- List utilities.
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
//* The module file @file{lists} exports list manipulation functions.  It
//* is a generic module with one module parameter, which is the type of
//* the list elements.  The name of this parameter is @var{A}.

module lists<A>;

import lists<int>;

//* Return true if @var{l} is the empty list, false otherwise.
//
public fun empty? (l: list of A): bool
  return l = null;
end;


//* Cons the element @var{a} onto the head of list @var{b}.
//
public fun cons (a: A, b: list of A): list of A
  return a :: b;
end;


//* Return the head of list @var{l}.  An exception is raised if @var{l} is
//* empty.
//
public fun head (l: list of A): A
  return hd l;
end;


//* Return the tail of list @var{l}.  An exception is raised if @var{l} is
//* empty.
//
public fun tail (l: list of A): list of A
  return tl l;
end;


//* Return the last element of list @var{l}.  An exception is
//* raised if @var{l} is empty.
//
public fun last (l: list of A): A
  while tl l <> null do
    l := tl l;
  end;
  return hd l;
end;


//* Return the initial sequence of list @var{l}, excluding the last
//* element.  An exception is raised if @var{l} is empty.
//
public fun init (l: list of A): list of A
  if tl l <> null then
    return hd l :: init (tl l);
  else
    return [];
  end;
end;


//* Append the lists @var{a} and @var{b}.
//
public fun append (a: list of A, b: list of A): list of A
  if a = null then
    return b;
  else
    return hd a :: append (tl a, b);
  end;
end;


//* Return a list which is the concatenation of the lists in
//* @var{lists}.
//
public fun concat (lists: list of list of A): list of A
  var result: list of A := [];
  while lists <> null do
    var l: list of A := hd lists;
    lists := tl lists;
    while l <> null do
      result := hd l :: result;
      l := tl l;
    end;
  end;
  return reverse (result);
end;


//* Return the length of list @var{a}.
//
public fun length (a: list of A): int
  if a = null then
    return 0;
  else
    return 1 + length (tl a);
  end;
end;


// Iterative helper function for `reverse'.
//* -
fun reverse_it (l: list of A, res: list of A): list of A
  if l = null then
    return res;
  else
    return reverse_it (tl l, hd l :: res);
  end;
end;


//* Return a list with the elements of @var{l} in reserved order.
//
public fun reverse (l: list of A): list of A
  return reverse_it (l, null);
end;


//* Apply the procedure @var{p} to each element of the list @var{l}, 
//* in order.  For a function which returns a list of the results
//* of the function application, see module @code{listmap}
//* (@pxref{listmap module}).
//
public fun foreach (p: fun(A), l: list of A)
  while (not lists.empty? (l)) do
    p (hd l);
    l := tl l;
  end;
end;


//* Return an integer list of @var{count} elements, with the number 0 to
//* @var{count}-1 as the list elements.
//
public fun iota (count: int): list of int
  var l: list of int;
  l := null;
  while count > 0 do
    count := count - 1;
    l := count :: l;
  end;
  return l;
end;


//* Return a list with all elements from @var{l} which satisfy predicate
//* @var{p}, in the same order as in @var{l}.
//
public fun filter (p: fun(A): bool, l: list of A): list of A
  if l = null then
    return null;
  else
    if p (hd l) then
      return hd l :: filter (p, tl l);
    else
      return filter (p, tl l);
    end;
  end;  
end;


//* Insert the element @var{s} into the list @var{l}, maintaining
//* the order as defined by the comparison function @var{cmp} which
//* takes two elements of type A and returns a value less then 0 if
//* the first is smaller than the second, 0 if they are to be
//* considered equal and a value greater than 0 if the second is
//* greater.
//
public fun insert (s: A, l: list of A, cmp: fun (A, A): int): list of A
  if l = null then
    return [s];
  else
    if cmp (s, hd l) <= 0 then
      return s :: l;
    else
      return hd l :: insert (s, tl l, cmp);
    end;
  end;
end;


//* Return the index of element @var{elem} in list @var{l},
//* according to the comparison function @var{cmp}.  Return -1 if
//* @var{elem} does not appear in @var{l}.
//
public fun index (elem: A, l: list of A, cmp: fun (A, A): int): int
  var idx: int := 0;
  while l <> null do
    if cmp (elem, hd l) = 0 then
      return idx;
    end;
    idx := idx + 1;
    l := tl l;
  end;
  return -1;
end;


//* Return a list of the indices of all appearences of @var{elem}
//* in @var{l}, according to the comparison function @var{cmp}.
//* Return the empty list of @var{elem} does not appear in @var{l}.
//
public fun indices (elem: A, l: list of A, cmp: fun (A, A): int): list of int
  var idx: int := 0;
  var res: list of int := [];
  var r: list of int := [];
  while l <> null do
    if cmp (elem, hd l) = 0 then
      res := idx :: res;
    end;
    idx := idx + 1;
    l := tl l;
  end;
  return reverse (res);
end;


//* Create a list of length @var{l}, where all list elemens are
//* initialized to @var{elem}.
//
public fun replicate (elem: A, len: int): list of A
  var result: list of A := [];
  while len > 0 do
    result := elem :: result;
    len := len - 1;
  end;
  return result;
end;


//* Create a copy of the list @var{l}.  Note that only the spine of
//* the list is copied, not the elements.
//
public fun copy (l: list of A): list of A
  var result: list of A := [];
  while l <> null do
    result := hd l :: result;
    l := tl l;
  end;
  return reverse (result);
end;


//* Take the first @var{n} elements from the list @var{l}, dropping
//* the following elements.
public fun take (l: list of A, n: int): list of A
  var r: list of A := null;
  while n > 0 do
    r := hd l :: r;
    l := tl l;
    n := n - 1;
  end;
  return reverse (l);
end;


//* Drop the first @var{n} elements from list @var{l} and return
//* the remaining list.
//
public fun drop (l: list of A, n: int): list of A
  while n > 0 do
    l := tl l;
    n := n - 1;
  end;
  return l;
end;

// End of lists.t.
