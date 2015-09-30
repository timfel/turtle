// overloading2.t - test file for overload resolution
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module overloading2;

fun mini ()
end;

fun mini (i: int)
end;

fun main(argv: list of string): int
  var x: int;
  mini ();
  mini (x);
  mini (1);
  return 0;
end;

// End of overloading2.t.
