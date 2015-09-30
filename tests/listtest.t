// listtest.t -- Test file for list handling.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module listtest;

import io, lists<int>, lists<string>, strings;

fun even (i: int): bool
  return i % 2 = 0;
end;

fun longer_than (len: int): fun(string): bool
  return fun (s: string): bool
           return strings.length (s) > len;
         end;
end;

fun main(argv: list of string): int
  var li: list of int;
  var ls: list of string;

  li := [3, 1, 4, 5, 9];
  ls := ["o", "sole", "mio"];

  io.put (lists.length (li));
  io.nl ();
  li := lists.filter (even, li);
  io.put (lists.length (li));
  io.nl ();
  io.put (lists.length (ls));
  io.nl ();
  ls := lists.filter (longer_than(1), ls);
  io.put (lists.length (ls));
  io.nl ();
  return 0;
end;

// End of listtest.t.
