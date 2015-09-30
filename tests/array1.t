// array1.t -- Test file for arrays: Mapping.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module array1;

import io, arraymap<int, string>, strings;

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

fun wr_array (a: array of string)
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
  var a: array of int := {23, 42, -25, 1, 0};
  var b: array of string;

  wr_array (a);
  b := arraymap.map (strings.to_string, a);
  wr_array (b);

  return 0;
end;

// End of array1.t.
