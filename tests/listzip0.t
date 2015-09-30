// listzip0.t -- Test file for the module `listzip'.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module listzip0;

import io, listzip<int, int, int>;

fun wr(l: list of int)
  io.put ("[");
  while l <> null do
    io.put (hd l);
    l := tl l;
    if l <> null then
      io.put (", ");
    end;
  end;
  io.put ("]\n");
end;

fun main(argv: list of string): int
  var l: list of int, l2: list of int, l3: list of int;
  fun add (x: int, y: int): int
    return x + y;
  end;
  fun div2 (x: int): (int, int)
    return x / 2, - (x / 2);
  end;
  l := listzip.zip (add, [2, 4, 1], [3, -1, -5]);
  wr (l);
  l2, l3 := listzip.unzip (div2, l);
  wr (l2);
  wr (l3);
  return 0;
end;

// End of listzip0.t.
