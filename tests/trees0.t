// trees0.t -- Test file for the module `trees'.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module trees0;

import io, trees<int>;

fun main(argv: list of string): int
  var t: trees.tree<int>;

  t := trees.node (2);

  io.put (trees.data (t)); io.nl ();

  t := trees.node (3, trees.node (4), trees.node (1));
  trees.inorder (t, fun (d: int) io.put (d); io.nl (); end);
  return 0;
end;

// End of trees0.t.
