// arraytest.t -- Test file for array handling.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module arraytest;

import io;

fun main(argv: list of string): int
  var ai: array of int;
  var ab: array of bool;

  ai := {1, 4, 2};
  ab := {true, false, true, true};

  io.put (ai[0]); io.put (" ");
  io.put (ai[1]); io.put (" ");
  io.put (ai[2]); io.put (" ");
  io.nl ();

  io.put (ab[0]); io.put (" ");
  io.put (ab[1]); io.put (" ");
  io.put (ab[2]); io.put (" ");
  io.put (ab[3]); io.put (" ");
  io.nl ();

  ai := array 2 of 1;
  ab := array 3 of true;

  io.put (ai[0]); io.put (" ");
  io.put (ai[1]); io.put (" ");
  io.nl ();

  io.put (ab[0]); io.put (" ");
  io.put (ab[1]); io.put (" ");
  io.put (ab[2]); io.put (" ");
  io.nl ();

  ai[1] := 2;
  ab[2] := false;

  io.put (ai[0]); io.put (" ");
  io.put (ai[1]); io.put (" ");
  io.nl ();

  io.put (ab[0]); io.put (" ");
  io.put (ab[1]); io.put (" ");
  io.put (ab[2]); io.put (" ");
  io.nl ();

  return 0;
end;

// End of arraytest.t.
