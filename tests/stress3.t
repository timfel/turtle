// stress3.t -- Stress tests for the garbage collector.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module stress3;

import io, random, ints, listmap<string, string>,
  internal.gc;

fun main(args: list of string): int
fun transform (l: list of string): list of string
  var res: list of string := [];
  while l <> null do
    res := hd l + ".text" :: res;
    l := tl l;
  end;
  return res;
end;

  var names: list of string := [];
  var x: int;

//  internal.gc.garbage_collect ();  
  x := 10000;
  while x > 0 do
    names := ints.to_string (random.rand ()) :: names;
    x := x - 1;
  end;

//  internal.gc.garbage_collect ();  
  names := listmap.map (fun (s: string): string
                          return s + ".text"; 
			end, names);

  while names <> null do
    io.put (hd names); io.nl ();
    names := tl names;
  end;
  return 0;
end;

// End of stress3.t.
