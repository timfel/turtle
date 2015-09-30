// queens_cip.t -- N-Queens problem in Turtle, solved using constraints.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// N-Queens example program.  This program calculates all possibilities
// to place N queens_cip on a chessboard of size N x N, so that no 
// queen attacks any other.
//
// Adapted from an example in the paper:
//   Krzystof R. Apt and Andrea Schaerf:
//   The Alma Project, or How First Order Logic Can Help Us in
//   Imperative Programming

module queens_cip;

import io;

var size: int;

var i: int, j: int;
var board: array of !int;

// Print the current board configuration to standard out.
//
fun print_board ()
  var i: int, j: int;

  i := 0;
  while i < size do
    j := 0;
    while j < size do
      if board[i] = j then
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


fun find()
  i := 0;
  while i < size - 1 do
    j := i + 1;
    while j < size do
      require board[i] <> board[j];
      require board[i] <> board[j] + j - i;
      require board[i] <> board[j] + i - j;
      j := j + 1;
    end;
    i := i + 1;
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

  // Create board representation.
  //
  board := array size of 0;

  // Find a solution.
  //
  find ();

  print_board ();  
  return 0;
end;

// End of queens_cip.t.
