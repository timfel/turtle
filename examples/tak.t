// tak.t -- Takeuchi function in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// The famous Takeuchi benchmark function in Turtle.

module tak;

import io, internal.timeout;

fun tak (x: int, y: int, z: int): int
  if y >= x then
    return z;
  else
    return tak (tak (x - 1, y, z),
                tak (y - 1, z, x),
		tak (z - 1, x, y));
  end;
end;

fun main (args: list of string): int
  var i: int, x: int := 500;

  internal.timeout.set (fun () io.put ("."); end);
  io.put (x);
  io.put (" * tak (18, 12, 6):  ");
  while x > 0 do
    i := tak (18, 12, 6);
    x := x - 1;
  end;
  io.put (" Done.");
  io.nl ();

  return 0;
end;

// End of tak.t.
