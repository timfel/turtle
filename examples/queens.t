// queens.t -- N-Queens problem in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// N-Queens example program.  This program calculates all
// possibilities to place N queens on a chessboard of size N x N, so
// that no queen attacks any other.
//
// Adapted from the example program QUEENS.TIG in:
// Andrew W. Appel: Modern compiler implementation in C

module queens;

import io;

var row: array of int, col: array of int;
var diag1: array of int, diag2: array of int;

var size: int;
var solutions: int;

// Print the current board configuration to standard out.
//
fun print_board ()
  var i: int, j: int;

  i := 0;
  while i < size do
    j := 0;
    while j < size do
      if col[i] = j then
	io.put (" o");
      else
	io.put (" .");
      end;
      j := j + 1;
    end;
    io.nl ();
    i := i + 1;
  end;
  io.nl ();
end;


// Try to place queen `c' onto the board.
//
fun try (c: int)
  if c = size then
    print_board ();
    solutions := solutions + 1;
  else
    var r: int;
    r := 0;
    while r < size do
      if (row[r] = 0) and (diag1[r + c] = 0) and 
	(diag2[r + (size - 1) - c] = 0) 
      then
	row[r] := 1;
	diag1[r + c] := 1;
	diag2[r + (size - 1) - c] := 1;
	col[c] := r;
	try (c + 1);
	row[r] := 0;
	diag1[r + c] := 0;
	diag2[r + (size - 1) - c] := 0;
      end;
      r := r + 1;
    end;
  end;
end;


// The function `main(list of string): int' will be automatically
// called when the program starts up.  Remember that the main module
// of a program must be compiled with the `--main=NAME' switch.
//
fun main (argv: list of string): int

  // Change the constant in the following line to try out the program
  // for another value of N.
  //
  size := 8;

  // Initialize solution counter.
  solutions := 0;

  // Create the arrays representing the board.
  row := array size of 0;
  col := array size of 0;

  // Create auxiliary data structures.
  diag1 := array size * 2 - 1 of 0;
  diag2 := array size * 2 - 1 of 0;

  // Start by placing the first queen.
  try (0);

  io.put (solutions);
  io.put (" solutions found.\n");
  return 0;
end;

// End of queens.t.
