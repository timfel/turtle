// hello2.t -- Extended ``Hello World!'' in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// The famous ``Hello World!'' program in the Turtle version, enhanced
// with command line options.

module hello;

import io, cmdline, internal.version;

var optspecs: list of cmdline.optspec :=
  [cmdline.flag ('h', "help"),
   cmdline.flag ('v', "version")];

var version: string := "1.0";

fun help ()
  io.put ("usage: hello2 [OPTION...] [NAME]\n");
  io.put ("  -h, --help       display this help and exit\n");
  io.put ("  -v, --version    display version information and exit\n");
  io.put ("Report bugs to mgrabmue@cs.tu-berlin.de\n");
end;

fun version ()
  io.put ("hello2 version ");
  io.put (version);
  io.put (" (Turtle version ");
  io.put (internal.version.version ());
  io.put (")\n");
end;

// The function `main(list of string): int' will be automatically
// called when the program starts up.  Remember that the main module
// of a program must be compiled with the `--main=NAME' switch.
//
fun main (args: list of string): int
  var opts: list of cmdline.option := cmdline.getopt (optspecs, args);
  var name: string := "Stranger";

  while opts <> null do
    var opt: cmdline.option := hd opts;

    if cmdline.flag? (opt) then
      if (cmdline.flag_char (opt) = 'v') then
	version ();
	return 0;
      else
	if (cmdline.flag_char (opt) = 'h') then
	  help ();
	  return 0;
	else
	  if (cmdline.flag_char (opt) = '?') then
	    return 1;
	  end;
	end;
      end;
    else
      name := cmdline.param (opt);
    end;
    opts := tl opts;
  end;

  io.put ("Hello ");
  io.put (name);
  io.put ("!");
  io.nl ();

  return 0;
end;

// End of hello.t.
