// constraints1.t -- Test file for constrainable fields.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module constraints1;

import io;

datatype cell = cell (val: !int) or 
                bla (val: int);

fun main(argv: list of string): int
  var c: cell := cell (var 1);
  io.put (!(val (c))); io.nl ();
  val! (c, var 2);
  io.put (!(val (c))); io.nl ();

  c := bla (3);
  io.put (val (c)); io.nl ();
  val! (c, 4);
  io.put (val (c)); io.nl ();
  return 0;
end;

// End of constraints1.t.
