// pairs0.t -- Test file for the module `pairs'.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module pairs0;

import io, pairs<int, string>;

fun main(argv: list of string): int
  var p: (int, string);

  p := 2, "Hello";
  io.put (pairs.first (p)); io.nl ();
  io.put (pairs.second (p)); io.nl ();
  return 0;
end;

// End of pairs0.t.
