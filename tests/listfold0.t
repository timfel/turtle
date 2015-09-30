// listfold0.t -- Test file for the module `listfold'.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module listfold0;

import io, listfold<int>;

fun main(argv: list of string): int
  fun sub(x: int, y: int): int
    return x - y;
  end;
  io.put (listfold.foldl (sub, [4, 3, 1])); io.nl ();
  io.put (listfold.foldr (sub, [4, 3, 1])); io.nl ();
  return 0;
end;

// End of listfold0.t.
