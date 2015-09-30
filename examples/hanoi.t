// hanoi.t -- Towers of Hanoi in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// The famous Towers of Hanoi in Turtle.

module hanoi;

import io, internal.timeout;

fun hanoi (n: int)
  fun move_them (n: int, from: int, to: int, helper: int)
    if n > 1 then
      move_them (n - 1, from, helper, to);
      move_them (n - 1, helper, to, from);
    end;
  end;

  move_them (n, 0, 1, 2);
end;

fun main (args: list of string): int
  internal.timeout.set (fun () io.put ("."); end);

  io.put ("hanoi (25): ");
  hanoi (25);
  io.put (" Done.");
  io.nl ();

  return 0;
end;

// End of hanoi.t.
