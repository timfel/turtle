// rand0.t -- Test file for the random module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module rand0;

import io, random;

fun main(argv: list of string): int
  var x: int;

  random.seed (23);
  io.put (random.rand ()); io.nl ();
  io.put (random.rand ()); io.nl ();
  io.put (random.rand ()); io.nl ();
  io.put (random.rand ()); io.nl ();
  return 0;
end;

// End of rand0.t.
