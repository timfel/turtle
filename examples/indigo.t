// indigo.t -- Test file for the Indigo constraint solving algorithm.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* This is an adaption of an example for the Indigo local
//* propagation algorithm, as described in:
//* 
//* Alan Borning and Richard Anderson and Bjorn Freeman-Benson:
//* ``The Indigo Algorithm'', Department of Computer Science and
//* Engineering, University of Washington, TR 96-05-01.
//* 
//* This does nothing useful, it is just here to show that Turtle
//* computes the same results as Borning et al. in their article.

module indigo;

import io;

public fun main(argv: list of string): int
  var a: !real := var 0.0;
  var b: !real := var 0.0;
  var c: !real := var 0.0;
  var d: !real := var 0.0;

  require a >= 10.0;
  require b >= 20.0;
  require a + b = c;
  require c + 25.0  = d;
  require d <= 100.0 : 1;
  require a = 50.0 : 2;
  require a = 5.0 : 4;
  require b = 5.0 : 4;
  require c = 100.0 : 4;
  require d = 200.0 : 4;

  io.put ("a = "); io.put (!a); io.nl ();
  io.put ("b = "); io.put (!b); io.nl ();
  io.put ("c = "); io.put (!c); io.nl ();
  io.put ("d = "); io.put (!d); io.nl ();
  return 0;
end;

// End of indigo.t.
