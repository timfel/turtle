// tupletest.t - test file for tuple handling.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module tupletest;

import io;

fun swap (x: int, y: int): (int, int)
  return y, x;
end;

fun main(argv: list of string): int
  var i1: int, i2: int;
  var s1: string, s2: string;

  i1 := 23;
  i2 := 42;

  s1 := "hello";
  s2 := "world";

  i1, s1 := i2, s2;

  io.put (i1); io.put (" ");
  io.put (s1); io.nl ();
  io.put (i2); io.put (" ");
  io.put (s2); io.nl ();

  i1 := 23;
  i2 := 42;

  s1 := "hello";
  s2 := "world";

  var ai1: array of int, ai2: array of int;
  var as1: array of string, as2: array of string;

  ai1 := array 10 of 0;
  ai2 := array 10 of 0;
  as1 := array 10 of "";
  as2 := array 10 of "";

  ai1[0], as1[0] := i1, s1;
  ai2[0], as2[0] := i2, s2;

  io.put (ai1[0]); io.put (" ");
  io.put (as1[0]); io.nl ();
  io.put (ai2[0]); io.put (" ");
  io.put (as2[0]); io.nl ();

  var is1: (int, string), is2: (int, string);

  is1 := i1, s1;
  is2 := i2, s2;

  i1, s1 := is2;
  i2, s2 := is1;

  io.put (i1); io.put (" ");
  io.put (s1); io.nl ();
  io.put (i2); io.put (" ");
  io.put (s2); io.nl ();

  var x: int, y: int;
  x := 1;
  y := -1;
  io.put ("x = "); io.put (x); io.nl ();
  io.put ("y = "); io.put (y); io.nl ();
  x, y := y, x;
  io.put ("x = "); io.put (x); io.nl ();
  io.put ("y = "); io.put (y); io.nl ();
  x, y := swap (x, y);
  io.put ("x = "); io.put (x); io.nl ();
  io.put ("y = "); io.put (y); io.nl ();
  return 0;
end;

// End of tupletest.t.
