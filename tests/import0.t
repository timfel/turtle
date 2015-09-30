// import0.t -- Testing the `import' clause.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

//* Please note that this test file is extremely
//* implementation-dependant, so please please do not use it as a
//* template for your own 

module import0;

import testsuite, io, listmap<io.file, int>, lists<io.file>;

fun test_foreign ()
  testsuite.open ("call imported function");
  var l: list of int := 
  listmap.map (fun (f: io.file): int return io.fd (f); end, [io.output,
  io.input]);
  lists.foreach (fun (f: io.file) io.putln (io.fd (f)); end, [io.output, io.error, io.input]);
  testsuite.close ();
end;

fun main(args: list of string): int
  testsuite.start ("import0");
  test_foreign ();
  return testsuite.summary ();
end;

// End of import0.t.
