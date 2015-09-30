// fac.t - Factorial in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module fac;

import io;

// Calculate n!, the factorial of n.
//
fun fac (n: int): int
  if n = 0 then
    return 1;
  else
    return n * fac (n - 1);
  end;
end;

// The function `main(list of string): int' will be automatically
// called when the program starts up.  Remember that the main module
// of a program must be compiled with the `--main=NAME' switch.
//
fun main (args: list of string): int
  io.putln (fac (10));
  return 0;
end;

// End of fac.t.
