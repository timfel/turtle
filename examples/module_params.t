// module_params.t - Paremetric module demonstration.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// This module demonstrates how parametric modules can be used
// in Turtle

module module_params;

import identity<int>, identity<bool>;

fun main(args: list of string): int
  var x: int, b: bool;
  
  x := 42;
  x := identity.id (x);
  b := identity.id (b);

  return 0;
end;

// End of module_params.t.
