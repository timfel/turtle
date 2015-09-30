// fac_iterative.t - Iterative factorial in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module fac_iterative;

import io;

// Calculate n!, the factorial of n.
//
fun fac_iterative (x: int): int
  var res: int;

  res := 1;
  while x > 0 do
    res := res * x;
    x := x - 1;
  end;
  return res;
end;

// The function `main(list of string): int' will be automatically
// called when the program starts up.  Remember that the main module
// of a program must be compiled with the `--main=NAME' switch.
//
fun main (args: list of string): int
  io.putln (fac_iterative (10));
  return 0;
end;

// End of fac_iterative.t.
