// overloading0.t - test file for overload resolution
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module overloading0;

fun main(argv: list of string): int
  var x: int;
  var y: int;
  var res1: int;

  var x: real;
  var y: real;
  var res2: real;

  var z: int;
  var res3: int;

  x := 1;
  y := 2;

  x := 1.0;
  y := 2.0;

  z := 3;

  res1 := x + y;
  res2 := x + y;
  res3 := x * z;

  return 0;
end;

// End of overloading0.t.
