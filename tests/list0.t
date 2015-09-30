// list0.t -- Test file for lists: Sorting.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module list0;

import io, listsort<int>, compare;

fun wr_list (a: list of int)
  io.put ("[");
  while a <> null do
    io.put (hd a);
    a := tl a;
    if a <> null then
      io.put (", ");
    end;
  end;
  io.put ("]\n");
end;

fun main(argv: list of string): int
  var a: list of int;

  a := [3, 2, 5, 1, 9, 6, 0, 7, 4, 8];

  wr_list (a);
  a := listsort.sort (a, compare.cmp);
  wr_list (a);

  return 0;
end;

// End of list0.t.
