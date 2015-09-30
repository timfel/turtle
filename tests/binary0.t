// binary0.t -- Test file for binary arrays..
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module binary0;

import io, binary, strings;

fun main(args: list of string): int
  var b: binary.binary;

  b := binary.make (10);

  binary.set (b, 1, 4);
  io.put (binary.get (b, 1));
  io.nl ();
  binary.set (b, 0, 226);
  io.put (binary.get (b, 0));
  io.nl ();

  var s: string := "Hello World!";
  b := binary.from_string (s);
  s := binary.to_string (b);
  if not strings.eq (s, "Hello World!") then
    return 1;
  end;
  return 0;
end;

// End of binary0.t.
