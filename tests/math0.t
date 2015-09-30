// math0.t -- Test file for the math library module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module math0;

import io, math;

fun test_sin ()
  var x: real;
  io.put ("* Testing sin\n");
  x := 0.0;
  io.put (math.sin (x)); io.nl ();
  x := math.pi;
  io.put (math.sin (x)); io.nl ();
end;

fun test_asin ()
  var x: real;
  io.put ("* Testing asin\n");
  x := 0.0;
  io.put (math.asin (math.sin (x))); io.nl ();
  x := math.pi;
  io.put (math.asin (math.sin (x))); io.nl ();
end;

fun test_cos ()
  var x: real;
  io.put ("* Testing cos\n");
  x := 0.0;
  io.put (math.cos (x)); io.nl ();
  x := math.pi;
  io.put (math.cos (x)); io.nl ();
end;

fun test_acos ()
  var x: real;
  io.put ("* Testing acos\n");
  x := 0.0;
  io.put (math.acos (math.cos (x))); io.nl ();
  x := math.pi;
  io.put (math.acos (math.cos (x))); io.nl ();
end;

fun test_tan ()
  var x: real;
  io.put ("* Testing tan\n");
  x := 0.0;
  io.put (math.tan (x)); io.nl ();
  x := 1.0;
  io.put (math.tan (x)); io.nl ();
  x := math.pi;
  io.put (math.tan (x)); io.nl ();
end;

fun test_atan ()
  var x: real, y: real;
  io.put ("* Testing atan\n");
  x := 0.0;
  io.put (math.atan (math.tan (x))); io.nl ();
  x := math.pi;
  io.put (math.atan (math.tan (x))); io.nl ();

  x := 0.0;
  y := 0.0;
  io.put (math.atan (x, y)); io.nl ();
end;

fun main(argv: list of string): int
  io.put ("bla"); io.nl ();
  io.put (0.0); io.nl ();
  io.put (1.2); io.nl ();
  io.put ("bla"); io.nl ();
  test_sin ();
  test_asin ();
  test_cos ();
  test_acos ();
  test_tan ();
  test_atan ();
  return 0;
end;

// End of math0.t.
