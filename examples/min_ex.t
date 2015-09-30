// min_ex.t -- Demonstration for Constraint Programming in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

//* Commentary:
//*
//* This program shows how to write the minimum function using
//* constraints.
//*
//* NOTE: This is not yet functional, because constraint hierarchies
//* do not work yet for finite domain constraints.

module min_ex;

import io, ints;

fun min (x: int, y: int): int
  var m: ! int := var ints.max;

  require m <= x and m <= y;
  return !m;
end;

fun main(args: list of string): int
  io.put ("min (3, 2) = ");
  io.put (min (3, 2));
  io.nl ();
  io.put ("min (2, 3) = ");
  io.put (min (2, 3));
  io.nl ();
  return 0;
end;

// End of min_ex.t.
