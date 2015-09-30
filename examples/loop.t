// loop.t -- Empty loop.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// This program only performs a loop.

module loop;

import io;

fun loop (number: int)
  while number > 0 do
    number := number - 1;
  end;
end;

fun main (args: list of string): int
  var x: int := 500000000;

  io.put ("loop (");
  io.put (x);
  io.put ("): ");
  loop (x);
  io.put (" Done.");
  io.nl ();

  return 0;
end;

// End of loop.t.
