// trees.t -- User-defined datatype demonstration..
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

module trees;

import io;

// This is a binary tree.
datatype tree = leaf(value: int) or
                node(left: tree, right: tree, key: int);


// Return the maximum of two integer values.
fun max(x: int, y: int): int
  if (x > y) then
    return x;
  else
    return y;
  end;
end;

// Return the number of leafs in the binary tree t.
fun leafs(t: tree): int
  if leaf?(t) then
    return 1;
  else
    return leafs(left(t)) + leafs(right(t));
  end;
end;

// Return the depth of tree t.
fun depth(t: tree): int
  if leaf?(t) then
    return 1;
  else
    return 1 + max(depth(left(t)), depth(right(t)));
  end;
end;

fun main (argv: list of string): int
  var t: tree := node (node (leaf (3), node (leaf (4), leaf (8), 2), 13),
                       node (node (leaf (12), leaf(-5), -23), leaf (0), 11),
		       28);

  io.put ("depth: "); io.put (depth (t)); io.nl ();
  io.put ("leafs: "); io.put (leafs (t)); io.nl ();

  return 0;
end;

// End of trees.t.
