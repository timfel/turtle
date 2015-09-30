// constraints.t -- Demonstration for Constraint Programming in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* This module tries to solve the crypto-arithmetic puzzle:
//* 
//*    O M A
//* +  O P A
//* --------
//*  P A A R
//* ========
//* 
//* using interval narrowing.  Of course, this will fail.  But at
//* least we can see that it is possible to try it.
//*
//* For a working example, see the file omaopa.t in this directory,
//* which solves this puzzle using integer constraints.

module constraints;

import io;

fun test0()
  var o: !real := var 0.0;
  var m: !real := var 0.0;
  var a: !real := var 0.0;
  var p: !real := var 0.0;
  var r: !real := var 0.0;


  require a >= 0.0;
  require a <= 9.0;
  require r >= 0.0;
  require r <= 9.0;
  require o >= 0.0;
  require o <= 9.0;
  require m >= 0.0;
  require m <= 9.0;
  require p = 1.0;
//  require p >= 1.0;
//  require p <= 9.0;
  require (o * 100.0 + m * 10.0 + a) + (o * 100.0 + p * 10.0 + a) =
          (p * 1000.0 + a * 100.0 + a * 10.0 + r);
  io.put ("o = "); io.put (!o); io.nl ();
  io.put ("m = "); io.put (!m); io.nl ();
  io.put ("a = "); io.put (!a); io.nl ();
  io.put ("p = "); io.put (!p); io.nl ();
  io.put ("r = "); io.put (!r); io.nl ();
end;

// This function gets called when the program is started, command line
// arguments are passed in the `args' array.  When the function
// returns, the process will be halted with the return value as its 
// exit code.
fun main(args: list of string): int
  test0();

  return 0;
end;

// End of constraints.t.
