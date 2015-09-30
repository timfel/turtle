// fun1.t -- Test file for functions.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module fun1;

import io, compose<int, int, int>;

fun sub1 (i: int): int
  return i - 1;
end;

fun add1 (i: int): int
  return i + 1;
end;

fun main(argv: list of string): int
  var x: int;
  x := 42;
  var identity: fun (int): int;

  identity := compose.compose (sub1, add1);

  io.put (x); io.put (" = ");
  io.put (identity (x)); io.nl ();
  return 0;
end;

// End of fun1.t.
