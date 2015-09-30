// listreduce0.t -- Test file for the module `listreduce'.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module listreduce0;

import io, listreduce<int, int>;

fun main(argv: list of string): int
  fun sub (x: int, y: int): int
    return x - y;
  end;
  io.put (listreduce.reducel (sub, 12, [2, 5, 3])); io.nl ();
  io.put (listreduce.reducer (sub, 12, [2, 5, 3])); io.nl ();
  return 0;
end;

// End of listreduce0.t.
