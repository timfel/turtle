// bstrees0.t -- Test file for the module `bstrees'.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module bstrees0;

import io, bstrees<string, int>, compare, option<int>;

fun main(argv: list of string): int
  var t: bstrees.tree<string, int>;

  t := bstrees.tree (compare.cmp);

  t := bstrees.insert (t, "Martin", 25);
  t := bstrees.insert (t, "bla", 42);
  t := bstrees.insert (t, "Ilka", 25);

  io.put (option.data (bstrees.search (t, "Martin"))); io.nl ();
  io.put (option.data (bstrees.search (t, "Ilka"))); io.nl ();
  if option.none? (bstrees.search (t, "foo")) then
    io.put ("Okay.\n");
  end;
  if option.some? (bstrees.search (t, "bla")) then
    io.put ("Okay.\n");
  end;
  return 0;
end;

// End of bstrees0.t.
