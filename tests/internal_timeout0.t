// internal_timeout0.t -- Test file for the `internal.timeout' module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module internal_timeout0;

import io, internal.timeout;

fun timer ()
  io.put ("** timer called\n");
end;

fun main(argv: list of string): int
  var x: int := 5000000;
  internal.timeout.set (timer);
  while x > 0 do
    x := x - 1;
  end;
  return 0;
end;

// End of internal_timeout0.t.
