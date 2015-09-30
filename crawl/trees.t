// trees.t -- Binary trees.
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
//* Functions for creating and inspecting binary trees of type
//* @code{A}, where @var{A} must be instantiated when importing
//* this module.

module trees<A>;


//* Data type for binary trees.  The empty tree is created with a
//* call to the constructor @code{nil}, and a non-empty tree with
//* the constructor @code{node}.
//
public datatype tree<A> = nil or
                          node (data: A, left: tree, right: tree);

//* Utility function for creating a singleton tree.
//
public fun node (d: A): tree
  return node (d, nil (), nil ());
end;


//* Iterate over the tree @var{t} in preorder, inorder or
//* postorder, respectively and call function @var{f} at every
//* node, with the data element of the node as argument.
//
public fun preorder (t: tree, f: fun(A))
  if node? (t) then
    preorder (left (t), f);
    f (data (t));
    preorder (right (t), f);
  end;
end;
//* ""
public fun inorder (t: tree, f: fun(A))
  if node? (t) then
    inorder (left (t), f);
    f (data (t));
    inorder (right (t), f);
  end;
end;
//* ""
public fun postorder (t: tree, f: fun(A))
  if node? (t) then
    postorder (left (t), f);
    f (data (t));
    postorder (right (t), f);
  end;
end;


//* Create a deep copy of the tree @var{t}.
//
public fun copy (t: tree): tree
  if nil? (t) then
    return nil ();
  else
    return node (data (t), left (t), right (t));
  end;
end;

// End of trees.t.
