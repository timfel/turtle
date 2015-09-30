// sendmory2.t -- Demonstration for Constraint Programming in Turtle.
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
//*
//* This version employs more compact constraint statements than
//* sendmory.t by using constraint conjunctions and the user-defined
//* constraints `all_different' and `domain'.

module sendmory2;

import io;

//* This user-defined constraint places constraints on the elements
//* of @var{l} that they must be pair-wise different.
//*
constraint all_different (l: list of !int)
  while tl l <> null do
    var ll: list of !int := tl l;
    while ll <> null do
      require hd l <> hd ll;
      ll := tl ll;
    end;
    l := tl l;
  end;
end;

//* This constrains the variable @var{v} to take a value between
//* @var{min} and @var{max}, inclusive.
//*
constraint domain (v: !int, min: int, max: int)
  require v >= min and v <= max;
end;

fun test0()
  var s: !int := var 0;
  var e: !int := var 0;
  var n: !int := var 0;
  var d: !int := var 0;
  var m: !int := var 0;
  var o: !int := var 0;
  var r: !int := var 0;
  var y: !int := var 0;

  require domain (s, 0, 9) and domain (e, 0, 9) and domain (n, 0, 9) and
    domain (d, 0, 9) and domain (m, 1, 9) and domain (o, 0, 9) and
    domain (r, 0, 9) and domain (y, 0, 9) and
    all_different ([s, e, n, d, m, o, r, y]) and
    (s * 1000 + e * 100 + n * 10 + d) +
    (m * 1000 + o * 100 + r * 10 + e) =
    (m * 10000 + o * 1000 + n * 100 + e * 10 + y)
  in
    io.put ("s = "); io.put (!s); io.nl ();
    io.put ("e = "); io.put (!e); io.nl ();
    io.put ("n = "); io.put (!n); io.nl ();
    io.put ("d = "); io.put (!d); io.nl ();
    io.put ("m = "); io.put (!m); io.nl ();
    io.put ("o = "); io.put (!o); io.nl ();
    io.put ("r = "); io.put (!r); io.nl ();
    io.put ("y = "); io.put (!y); io.nl ();
  end;
end;

fun main(args: list of string): int
  test0();
  return 0;
end;

// End of sendmory2.t.
