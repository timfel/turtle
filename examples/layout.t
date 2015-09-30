// layout.t -- A simple layout example.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* This program solves a trivial layout problem, which consists of
//* two columns of widths `lw' and `rw', which should be placed on
//* a page of width `pw', seperated by a gap of width `gap', which
//* shall occupy 1/20th of the page width.
//*
//*               /\ gap
//* +-------------+ +-------------+
//* |             | |             |
//* |             | |             |
//* |             | |             |
//* |             | |             |
//* +-------------+ +-------------+
//*  \--- lw ----/   \--- rw ----/
//*  \------------ pw ------------/

module layout;

import io;

public fun main(argv: list of string): int
  // Gap between the columns.
  var gap: !real := var 0.0;
  // Page width.
  var pw: !real := var 0.0;
  // Left and right column.
  var lw: !real := var 0.0;
  var rw: !real := var 0.0;

  require pw = 19.0;
  require 20.0 * gap = pw:1;
  require lw + gap + rw = pw : 1;
  require lw >= 0.0 : 4;
  require rw >= 0.0 : 4;

  io.put ("gap = "); io.put (!gap); io.nl ();
  io.put ("lw = "); io.put (!lw); io.nl ();
  io.put ("rw = "); io.put (!rw); io.nl ();
  io.put ("pw = "); io.put (!pw); io.nl ();
  return 0;
end;

// End of layout.t.
