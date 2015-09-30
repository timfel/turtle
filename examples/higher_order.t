// higher_order.t - Higher-order functions in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
// This module demonstrates the higher-order features of Turtle.

module higher_order;

fun test(z: int): fun(): fun(): int
  return fun (): fun(): int
           var y: int;
	   y := 5;
           return fun (): int
	            var x: int;
		    x := 3;
	            return x + z + y;
		  end;
         end;
end;

fun main(args: list of string): int
  var c1: fun(): fun(): int;
  var c2: fun(): int;
  var c3: int;

  c1 := test(23);
  c2 := c1();
  c3 := c2();
  return 0;
end;

// End of higher_order.t.
