// inttest.t - test file for integer arithmetic.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module inttest;

import io;

fun main(argv: list of string): int
  var i: int, j: int;

  i := 42;
  j := 23;

  io.put (i + j); io.nl ();
  io.put (i - j); io.nl ();
  io.put (i * j); io.nl ();
  io.put (i / j); io.nl ();
  io.put (i % j); io.nl ();
  
  return 0;
end;

// End of inttest.t.
