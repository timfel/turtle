// sys_times0.t -- Test file for the `sys.times' module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module sys_times0;

import io, sys.times, sys.errno, strings;

fun put (tm: sys.times.tm)
  io.put (sys.times.year (tm) + 1900);
  io.put ("-");
  io.put (strings.lpad (strings.to_string (sys.times.mon (tm)), 2, '0'));
  io.put ("-");
  io.put (strings.lpad (strings.to_string (sys.times.mday (tm)), 2, '0'));
  io.put (" ");
  io.put (strings.lpad (strings.to_string (sys.times.hour (tm)), 2, '0'));
  io.put (":");
  io.put (strings.lpad (strings.to_string (sys.times.min (tm)), 2, '0'));
  io.put (":");
  io.put (strings.lpad (strings.to_string (sys.times.sec (tm)), 2, '0'));
  io.put (" istdst: ");
  io.put (sys.times.isdst (tm));
  io.put (" day-of-year: ");
  io.put (sys.times.yday (tm));
end;

fun main(argv: list of string): int
  var t: long := sys.times.time ();
  var tm: sys.times.tm;
  io.put ("Current time in seconds since the epoch: ");
  io.put (t);
  io.nl ();
  tm := sys.times.gmtime (t);
  io.put ("UTC time: ");
  put (tm);
  io.nl ();
  tm := sys.times.localtime (t);
  io.put ("Local time: ");
  put (tm);
  io.nl ();
  tm := sys.times.gmtime (t);
  io.put ("UTC time (asctime): ");
  io.put (sys.times.asctime (tm));
  tm := sys.times.localtime (t);
  io.put ("Local time (asctime): ");
  io.put (sys.times.asctime (tm));

  io.put ("Time sinces epoch (ctime): ");
  io.put (sys.times.ctime (t));
  return 0;
end;

// End of sys_times0.t.
