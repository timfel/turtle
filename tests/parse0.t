// parse0.t -- Test file for syntax analysis in the compiler.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module parse0;

fun fun0 ()
  var s: string, c: char, i: int, r: real, b: bool;
  s := "simple assignment";
  c := '!';
  i := 1;
  r := 1.3;
  b := true;
end;

fun main(argv: list of string): int
  fun0 ();
  return 0;
end;

// End of parse0.t.
