// bintree.t -- Binary trees for Turtle.
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
//* Simple, but working binary tree implementation for building search
//* trees.  A binary tree is parametrized by two types, one for the key
//* and one for the associated value.  Both the insertion and the
//* search function require a comparison function to be passed, so that
//* an order on the keys can be established.
//*
//* The module expects two module parameters, @var{A} and @var{B},
//* where @var{A} is the type of the keys in the search tree and
//* @var{B} is the type of the associated values.

module bintree<A, B>;

import io, option<B>;


//* The binary search trees are made up of this data type.
//
public datatype tree<A, B> = empty or
                             leaf(element: A, data: B) or
			     node(left: tree, right: tree, key: A);


//* Return a new search tree in which all key/value pairs from the
//* input tree @var{t} are stored and additionally a pair of the
//* parameters @var{key} and @var{data} is stored.  @var{cmp} is
//* a comparison function used to determine the order in the tree.
//* @var{cmp} is expected to return a value less than 0 if the first
//* argument is smaller than the second, a value greater than 0 if the
//* first argument is greater and 0 if they are equal.
//
public fun insert (t: bintree.tree<A, B>, key: A, data: B, 
                   cmp: fun (A, A): int): bintree.tree<A, B>
  var c: int;

  if empty? (t) then
    return leaf (key, data);
  else
    if leaf? (t) then
      c := cmp (key, element (t));
      if c = 0 then
	return leaf (key, data);
      else
	if c < 0 then
	  return node (leaf (key, data), t, key);
	else
	  return node (t, leaf (key, data), element (t));
	end;
      end;
    else // node? (t) <=> true
      c := cmp (key, key (t));
      if c <= 0 then
	return node (insert (left (t), key, data, cmp), right (t), key (t));
      else
	return node (left (t), insert (right (t), key, data, cmp), 
	             key (t));
      end;
    end;
  end;
end;


//* Search the binary search tree @var{t} for an entry with key 
//* @var{key}.  Return @code{option.some (data)} with the data value
//* associated with @var{key} if @var{key} was found, otherwise
//* return @code{option.none ()}.  Note that it is not specified
//* which data value will be returned if the key @var{key} appears
//* more than once in the tree.
//
public fun find (t: bintree.tree<A, B>, key: A, cmp: fun (A, A): int): 
          option.option<B>
  if empty? (t) then
    return option.none ();
  else
    if leaf? (t) then
      if cmp (key, element (t)) = 0 then
	return option.some (data (t));
      else
	return option.none ();
      end;
    else
      var c: int := cmp (key, key (t));
      if c <= 0 then
	return find (left (t), key, cmp);
      else
	return find (right (t), key, cmp);
      end;
    end;
  end;
end;

// End of bintree.t.
