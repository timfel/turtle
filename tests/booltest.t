// booltest.t -- test file for boolean operations.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module booltest;

import io;

fun main(argv: list of string): int
  var b: bool, q: bool;

  b := true;
  q := false;

  // Test basics: assignment, or, and, and not.
  //
  io.put (b); io.nl ();
  io.put (q); io.nl ();
  io.put (not b); io.nl ();
  io.put (not q); io.nl ();
  io.put (b or q); io.nl ();
  io.put (b and q); io.nl ();
  io.put (b or b); io.nl ();
  io.put (b and b); io.nl ();
  io.put (q or q); io.nl ();
  io.put (q and q); io.nl ();
  
  var x: int;

  x := 0;

  // Test short-circuiting.
  //
  if (x <> 0) and (42 / x = 1) then
    io.put ("Bug!\n");
  else
    io.put ("Okay.\n");
  end;
  if (x = 0) or (23 / x = 1) then
    io.put ("Okay.\n");
  else
    io.put ("Bug!\n");
  end;
  return 0;
end;

// End of booltest.t.
