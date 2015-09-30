// constraints0.t -- Test file for constrainable variable handling.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module constraints0;

import io;

// Test global constrainable variables, and constrainable arrays 
// and lists.
//
var foo: !bool;
var c: array of !bool := {var true, var false, var true};
var l: list of !real := [var 3.1, var 1.4, var 1.0];

fun main(argv: list of string): int
  // Test local constrainable variables and coercion from/to
  // constrainable variables.
  //
  var x: int;
  var y: !int;
  var z: int;
  var w: !char;
  var a: array of !int;

  z := 2;
  y := var 3;
  x := !y + z;
  io.put (x); io.nl ();

  foo := var true;
  io.put (!foo); io.nl ();

  a := array 10 of var 3;
  a[1] := var 2;
  x := !(a[2]);

  io.put (x); io.nl ();

  io.put (!(c[2])); io.nl ();
  io.put (!(hd tl l)); io.nl ();

  return 0;
end;

// End of constraints0.t.
