// evenodd.t - Tail-recursive calls inside a module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// This module demonstrates how Turtle handles tail-recursive calls
// inside a module without stack growth.

module evenodd;

// This variable counts the calls to the functions `even' and `odd'.
//
var count: int;

// If `i' is zero, return true, otherwise call the function
// `even' with `i' minus one.  Also increase the count of function
// calls.
//
fun odd(i: int): bool
  if i = 0 then
    return false;
  else
//    count := count + 1;
    return even (i - 1);
  end;
end;

// If `i' is zero, return false, otherwise call the function
// `odd' with `i' minus one.  Also increase the count of function
// calls.
//
fun even (i: int): bool
  if i = 0 then
    return true;
  else
//    count := count + 1;
    return odd (i - 1);
  end;
end;

// This function is called on program startup.  It sets the function
// call counter `count' and then starts the even/odd computation.
//
fun main (args: list of string): int
  var b: bool;
  count := 0;
  b := even (100020221);
  return 0;
end;

// End of evenodd.t.
