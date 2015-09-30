// counter.t - Higher-order functions in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// This module demonstrates how to implement a counter using
// higher-order functions.

module counter;

fun makeCounter(): fun(): int
  var x: int;
  x := -1;
  return fun (): int
    x := x + 1;
    return x;
  end;
end;

fun test(): int
  var c1: fun(): int;
  var x: int;
  var y: int;

  y := 1000000;
  while (y > 0) do
    c1 := makeCounter();
    x := c1();
    x := c1();
    x := c1();
    x := c1();
    x := c1();
    x := c1();
    y := y - 1;
  end;

  return x;
end;

fun main(args: list of string): int
  var x: int;

  x := test ();
  return 0;
end;

// End of counter.t.
