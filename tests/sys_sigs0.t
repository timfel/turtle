// sys_sigs0.t -- Test file for the `sys.sigs' module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module sys_sigs0;

import io, sys.sigs, sys.procs;

fun intr (no: int)
  io.put ("received signal no. ");
  io.put (no);
  io.nl ();
end;

fun main(argv: list of string): int
  sys.sigs.signal (sys.sigs.SIGUSR1, intr);
  sys.procs.kill (sys.procs.getpid (), sys.sigs.SIGUSR1);
  io.put ("Do it\n");
  return 0;
end;

// End of sys_sigs0.t.
