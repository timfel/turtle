// triples0.t -- Test file for the module `triples'.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module triples0;

import io, triples<int, string, bool>;

fun main(argv: list of string): int
  var p: (int, string, bool);

  p := 2, "Hello", true;
  io.put (triples.first (p)); io.nl ();
  io.put (triples.second (p)); io.nl ();
  io.put (triples.third (p)); io.nl ();
  return 0;
end;

// End of triples0.t.
