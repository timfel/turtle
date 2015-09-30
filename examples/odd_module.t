// odd_module.t - Tail-recursive calls across module boundaries.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// Together with the module `even_module', this module demonstrates
// how Turtle handles tail-recursive calls across module boundaries
// without stack growth.

module odd_module;


// This variable is necessary because we can't import `even_module'.
// The reason is that Turtle does not allow circular module
// dependencies.  So `even_module' stores its `even' function into the
// variable `even' on startup, so that `odd' can call it.
//
public var even: fun(int): bool;

// This variable counts the calls to the functions `even' and `odd'.
//
public var count: int;

// If `i' is zero, return true, otherwise call the function
// `even_module.even' with `i' minus one.  Also increase the count of
// function calls.
//
public fun odd(i: int): bool
  if i = 0 then
    return false;
  else
    count := count + 1;
    return even (i - 1);
  end;
end;

// End of odd_module.t.
