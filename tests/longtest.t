// longtest.t -- Test file for long integer arithmetic.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module longtest;

import io, longs, ints;

fun main(argv: list of string): int
  var i: long, j: long;

  i := 42L;
  j := 23L;

  io.put (i + j); io.nl ();
  io.put (i - j); io.nl ();
  io.put (i * j); io.nl ();
  io.put (i / j); io.nl ();
  io.put (i % j); io.nl ();

  io.put (longs.from_int (ints.min)); io.nl ();  
  io.put (longs.from_int (ints.max)); io.nl ();  
  io.put (2147483647L); io.nl ();  
  io.put (-2147483648L); io.nl ();  
  return 0;
end;

// End of longtest.t.
