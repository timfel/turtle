// fun0.t -- Test file for functions.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module fun0;

import io;

fun return_nested (): fun(): int
  fun f (): int
    io.put ("* in nested function (1)\n");
    return 1;
  end;
  io.put ("* about to return nested function\n");
  return f;
end;

fun test_nested ()

  fun nested ()
    io.put ("* in nested function (2)\n");
  end;

  io.put ("* about to call nested function\n");
  nested ();
  io.put ("* back from call nested function\n");

  var f: fun(): int;
  f := return_nested ();
  var x: int;
  x := f ();
  io.put (x); io.nl ();
end;

fun main(argv: list of string): int
  test_nested ();
  return 0;
end;

// End of fun0.t.
