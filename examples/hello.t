// hello.t -- ``Hello World!'' in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// The famous ``Hello World!'' program in the Turtle version.

module hello;

import io;

// The function `main(list of string): int' will be automatically
// called when the program starts up.  Remember that the main module
// of a program must be compiled with the `--main=NAME' switch.
//
fun main (args: list of string): int

  io.put ("Hello World!");
  io.nl ();

  return 0;
end;

// End of hello.t.
