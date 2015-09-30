// overloading1.t - test file for overload resolution
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module overloading1;

fun min (x: int, y: int): int
  return x;
end;

fun mini ()
end;

fun min (x: real, y: real): real
  return x;
end;

fun main(argv: list of string): int
  var x: int;
  var y: int;
  var res: int;
  var res1: int;

  var x: real;
  var y: real;
  var res: real;
  var res2: real;

  // Typing is determined by lhs of assignment.
  res1 := min (x, y);
  res1 := min (min (x, y), min (y, x));
  res2 := min (x, y);
  res2 := min (min (x, y), min (y, x));

  // Typing is determined by argument value.
  res := min (x, 1);
  res := min (x, 1.0);
  res := min (2, 1);
  res := min (2.3, 1.0);

  res1 := min (x, y);
  res := min (min (1, y), min (y, 3));
  res2 := min (x, y);
  res2 := min (min (x, y), min (y, x));

  mini();
  
  return 0;
end;

// End of overloading1.t.
