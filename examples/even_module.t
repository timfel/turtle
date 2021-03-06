// even_module.t - Tail-recursive calls across module boundaries.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// Together with the module `odd_module', this module demonstrates
// how Turtle handles tail-recursive calls across module boundaries
// without stack growth.

module even_module;

import odd_module;

// If `i' is zero, return false, otherwise call the function
// `odd_module.odd' with `i' minus one.  Also increase the count of
// function calls.
//
fun even (i: int): bool
  if i = 0 then
    return true;
  else
    odd_module.count := odd_module.count + 1;
    return odd_module.odd (i - 1);
  end;
end;

// This function is called on program startup.  It sets the function
// call counter `count' and the function variable `even' in module
// `odd' and then starts the even/odd computation.
//
fun main (args: list of string): int
  var b: bool;
  odd_module.count := 0;
  odd_module.even := even;
  b := even (100020221);
  return 0;
end;

// End of even_module.t.
