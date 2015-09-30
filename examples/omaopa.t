// omaopa.t -- Demonstration for Constraint Programming in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* This module solves the crypto-arithmetic puzzle:
//* 
//*    O M A
//* +  O P A
//* --------
//*  P A A R
//* ========

module omaopa;

import io;

fun test0()
  var o: !int := var 0;
  var m: !int := var 0;
  var a: !int := var 0;
  var p: !int := var 0;
  var r: !int := var 0;

  require a >= 0;
  require a <= 9;
  require r >= 0;
  require r <= 9;
  require o >= 0;
  require o <= 9;
  require m >= 0;
  require m <= 9;
  require p = 1;
  require (o * 100 + m * 10 + a) + (o * 100 + p * 10 + a) =
          (p * 1000 + a * 100 + a * 10 + r);
  io.put ("o = "); io.put (!o); io.nl ();
  io.put ("m = "); io.put (!m); io.nl ();
  io.put ("a = "); io.put (!a); io.nl ();
  io.put ("p = "); io.put (!p); io.nl ();
  io.put ("r = "); io.put (!r); io.nl ();
end;

fun main(args: list of string): int
  test0();
  return 0;
end;

// End of omaopa.t.
