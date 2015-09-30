// exceptions0.t -- Test file for raising and handling exceptions.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module exceptions0;

import io, exceptions;

fun raise_it ()
  io.put ("about to raise subscript exception\n");
  exceptions.raise (exceptions.subscript_ex ());
  io.put ("Oops, something is wrong\n");
end;

fun handle_it (s: string)
  io.put ("exception: ");
  io.put (s);
  io.nl ();
  io.put ("about to raise from exception handler\n");
  exceptions.handle (raise_it, handle_it2);
end;

fun handle_it2 (s: string)
  io.put ("exception: ");
  io.put (s);
  io.nl ();
end;

fun main(argv: list of string): int
  io.put ("About to set up exception handler\n");
  exceptions.handle (raise_it, handle_it);
  io.put ("Gotit\n");
  return 0;
end;

// End of exceptions0.t.
