// array0.t -- Test file for arrays: Sorting.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module array0;

import io, arraysort<int>, compare;

fun wr_array (a: array of int)
  var i: int;
  i := 0;
  io.put ("{");
  while i < sizeof a do
    io.put (a[i]);
    i := i + 1;
    if i < sizeof a then
      io.put (", ");
    end;
  end;
  io.put ("}\n");
end;

fun main(argv: list of string): int
  var a: array of int;

  a := {3, 2, 5, 1, 9, 6, 0, 7, 4, 8};

  wr_array (a);
  arraysort.sort (a, compare.cmp);
  wr_array (a);

  return 0;
end;

// End of array0.t.
