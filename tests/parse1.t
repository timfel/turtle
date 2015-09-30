// parse1.t -- Test file for syntax analysis in the compiler.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// *NOTE* Compilation of this file is expected to fail.
// The compiler should report 6 errors with the following command line:
//
// ../turtle/turtle -p../crawl parse1.t 2>&1|wc -l
//      6

module parse1;

fun fun0 ()
  var s: string;

  s := "forgotten semicolon after assignment"
  s := "correct";
  s := "forgotten semicolon at end of statement list"
end;

fun main(argv: list of string): int
  fun0 ();
  return 0;
end;

// End of parse1.t.
