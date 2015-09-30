// overloading.t -- Overloading demonstration.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// This module shows how functions and variables can be overloaded in
// Turtle.

module overloading;

import io;

fun test ()
  var a: int;
  var a: real;

  io.put ("in test ()\n");

  a := 1;
  a := 1.1;
end;

fun test (d: real)
  io.put ("in test (real)\n");
end;

fun test (x: int)
  io.put ("in test (int)\n");
  test ();
  test (3.14);
end;

// The function `main(list of string): int' will be automatically
// called when the program starts up.  Remember that the main module
// of a program must be compiled with the `--main=NAME' switch.
//
fun main (args: list of string): int
  test ();
  test (1);
  return 0;
end;

// End of overloading.t.
