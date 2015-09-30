// bstrees.t -- Binary search trees.
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
//* Functions for creating and inspecting binary search trees with
//* a key of type @code{A}, and a data object of type @var{B},
//* where @var{A} and @var{B} must be instantiated when importing
//* this module.

module bstrees<A, B>;

import trees<(A, B)>, pairs<A, B>, option<B>;


//* Data type for binary search trees.
//
public datatype tree<A, B> = tree(cmpfn: fun (A, A): int, 
                                  root: trees.tree<(A, B)>);

public fun tree (cmpfn: fun (A, A): int): bstrees.tree<A, B>
  return tree (cmpfn, trees.nil ());
end;

//* -
public fun insert (cmp: fun(A, A): int, t: trees.tree<(A, B)>, key: A, 
                   val: B): trees.tree<(A, B)>
  if trees.nil? (t) then
    return trees.node ((key, val));
  else
    if cmp (key, pairs.first (trees.data (t))) <= 0 then
      return trees.node (trees.data (t), insert (cmp, trees.left (t), key, 
                                                 val), trees.right (t));
    else
      return trees.node (trees.data (t), trees.left (t),
                         insert (cmp, trees.right (t), key, val));
    end;
  end;
end;


//* Insert the pair (@var{key}, @var{val}) into the binary search
//* tree @var{t} and return the updated tree.  The old copy of the
//* search tree remains usable; the binary search tree implemented
//* by this module is persistent.
//
public fun insert (t: bstrees.tree<A, B>, key: A, val: B): bstrees.tree<A, B>
  return tree (cmpfn (t), insert (cmpfn (t), root (t), key, val));
end;


//* -
public fun search (cmp: fun (A, A): int, t: trees.tree<(A, B)>, key: A): 
                   option.option<B>
  if trees.nil? (t) then
    return option.none ();
  else
    var res: int := cmp (key, pairs.first (trees.data (t)));
    if res < 0 then
      return search (cmp, trees.left (t), key);
    elsif res > 0 then
      return search (cmp, trees.right (t), key);
    else
      return option.some (pairs.second (trees.data (t)));
    end;
  end;
end;


//* Search the binary search tree @var{t} for the key @var{key} and
//* return the associated value, if found, packaged in an
//* @code{option.option} type.  If not found, the variant
//* @code{option.none} is returned.
//
public fun search (t: bstrees.tree<A, B>, key: A): option.option<B>
  return search (cmpfn (t), root (t), key);
end;

// End of bstrees.t.
