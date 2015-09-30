// sendmory.t -- Demonstration for Constraint Programming in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* This program solves the crypto-arithmetic puzzle:
//*
//*    S E N D
//* +  M O R E
//* ----------
//*  M O N E Y
//* ==========

module sendmory;

import io;

fun test0()
  var s: !int := var 0;
  var e: !int := var 0;
  var n: !int := var 0;
  var d: !int := var 0;
  var m: !int := var 0;
  var o: !int := var 0;
  var r: !int := var 0;
  var y: !int := var 0;

  require s >= 0;
  require s <= 9;
  require e >= 0;
  require e <= 9;
  require n >= 0;
  require n <= 9;
  require d >= 0;
  require d <= 9;
  require m >= 0;
  require m <= 9;
  require o >= 0;
  require o <= 9;
  require r >= 0;
  require r <= 9;
  require y >= 0;
  require y <= 9;

  require s <> e;
  require s <> n;
  require s <> d;
  require s <> m;
  require s <> o;
  require s <> r;
  require s <> y;
  require e <> n;
  require e <> d;
  require e <> m;
  require e <> o;
  require e <> r;
  require e <> y;
  require n <> d;
  require n <> m;
  require n <> o;
  require n <> r;
  require n <> y;
  require d <> m;
  require d <> o;
  require d <> r;
  require d <> y;
  require m <> o;
  require m <> r;
  require m <> y;
  require o <> r;
  require o <> y;
  require r <> y;
  require m > 0;
  require (s * 1000 + e * 100 + n * 10 + d) +
          (m * 1000 + o * 100 + r * 10 + e) =
          (m * 10000 + o * 1000 + n * 100 + e * 10 + y);

  io.put ("s = "); io.put (!s); io.nl ();
  io.put ("e = "); io.put (!e); io.nl ();
  io.put ("n = "); io.put (!n); io.nl ();
  io.put ("d = "); io.put (!d); io.nl ();
  io.put ("m = "); io.put (!m); io.nl ();
  io.put ("o = "); io.put (!o); io.nl ();
  io.put ("r = "); io.put (!r); io.nl ();
  io.put ("y = "); io.put (!y); io.nl ();
end;

fun main(args: list of string): int
  test0();
  return 0;
end;

// End of sendmory.t.
