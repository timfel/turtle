// stringtest.t - test file for string handling.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module stringtest;

import io, strings;

fun put (l: list of string)
  io.put ("[");
  while l <> null do
    io.put ("\"");
    io.put (hd l);
    io.put ("\"");
    l := tl l;
    if l <> null then
      io.put (", ");
    end;
  end;
  io.put ("]");
end;

fun main(argv: list of string): int
  var s: string;

  s := "Hello";
  io.put (s); io.put (" (");
  io.put (strings.length (s)); io.put (")\n");

  io.put (strings.substring (s, 1, 4));
  io.nl ();

  put (strings.split ("root:x:0:0:root:/root:/bin/bash", ':'));
  io.nl ();
  put (strings.split (":x:0:0:root::/bin/bash", ':'));
  io.nl ();
  return 0;
end;

// End of stringtest.t.
