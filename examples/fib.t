// fib.t - Fibonacci numbers in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module fib;

// Calculate fib(n), the number of pairs of rabbits after n years.
//
fun fib (x: int): int
  if x <= 1 then
    return 1;
  else
    return fib (x - 1) + fib (x - 2);
  end;
end;

// The function `main(list of string): int' will be automatically
// called when the program starts up.  Remember that the main module
// of a program must be compiled with the `--main=NAME' switch.
//
fun main (args: list of string): int
  var fib: int;
  fib := fib (34);
  return 0;
end;

// End of fib.t.
