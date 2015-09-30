// lex1.t -- Test file for lexical analysis in the compiler.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// *NOTE* Compilation of this file is expected to fail.
// The compiler should report 8 errors with the following command line:
//
// ../turtle/turtle -p../crawl lex1.t 2>&1|wc -l
//      8

module lex1;

fun main(argv: list of string): int
ä
  var c: char, s: string, r: real;
  c := '\1';
  s := "\1";
  s := "a\1b";
  s := "Unterminated string
  ;
  r := 1e2e2;
  r := 1.2.3;
ö
  return 1;
end;

// End of lex1.t.
