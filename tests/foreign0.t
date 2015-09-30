// foreign0.t -- Testing the `foreign' construct.
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

module foreign0;

import testsuite, strings;

const x: int := foreign "4";

fun test_foreign ()
  testsuite.open ("`foreign' keyword");
  testsuite.test ("integer value", fun (): bool return x = 1; end);
  testsuite.test ("string variable",
    fun (): bool return strings.eq (foreign "ttl_null_pointer_exception",
                                    "null-pointer-exception"); end);
  testsuite.close ();
end;

fun main(args: list of string): int
  testsuite.start ("foreign0");
  test_foreign ();
  return testsuite.summary ();
end;

// End of foreign0.t.
