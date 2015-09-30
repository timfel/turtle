// lex0.t -- Test file for lexical analysis in the compiler.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module lex0;

import io;

fun test_literals ()
  var c: char, s: string, i: int, r: real, b: bool, l: list of int;

  c := '!';
  c := ' ';
  c := '\n';
  c := '\r';
  c := '\t';
  c := '\b';
  c := 'A';
  c := 'g';
  c := '0';

  s := "Hallo Welt";
  s := "Hallo Welt!";
  s := "";
  s := "\n";
  s := "\r";
  s := "\t";
  s := "\b";

  i := 0;
  i := 1;
  i := -1;
  i := - 1;
  i := 1000;
// FIXME: Fix the following test cases:
//  i := 1e2;
//  i := 1E2;

  r := 0.0;
  r := 1.0;
  r := -1.0;
  r := - 1.0;
  r := 1000.0;
  r := 1000.11;

  b := true;
  b := false;
  
  l := null;
  l := 1 :: null;
end;

fun main(argv: list of string): int
  test_literals ();
  return 0;
end;

// End of [...].t.
