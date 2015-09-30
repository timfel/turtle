// filenames0.t -- Test file for the module `filenames'.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module filenames0;

import io, filenames;

fun main(argv: list of string): int
  io.put (filenames.basename ("/etc/passwd")); io.nl ();

  io.put (filenames.basename ("./test.c")); io.nl ();
  io.put (filenames.basename ("./test.c", ".c")); io.nl ();
  io.put (filenames.basename ("./test.d", ".c")); io.nl ();
  io.put (filenames.basename ("./.c", ".c")); io.nl ();

  io.put (filenames.basename ("test.c")); io.nl ();
  io.put (filenames.basename ("test.c", ".c")); io.nl ();
  io.put (filenames.basename ("test.d", ".c")); io.nl ();
  io.put (filenames.basename (".c", ".c")); io.nl ();

  io.put (filenames.dirname ("/etc/passwd")); io.nl ();
  io.put (filenames.dirname ("/etc/../etc/passwd")); io.nl ();
  io.put (filenames.dirname ("/etc/")); io.nl ();
  io.put (filenames.dirname ("/")); io.nl ();
  return 0;
end;

// End of filenames0.t.
