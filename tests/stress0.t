// stress0.t -- Stress tests for the garbage collector.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module stress0;

import io, internal.version;

fun main(args: list of string): int
  var x: int, y: int;
  var lr: list of real := null;
  var ls: list of string := null;
  x := 10000;
  while x > 0 do
    lr := 3.14 :: lr;
    x := x - 1;
  end;
  x := 10000;
  while x > 0 do
    ls := string 256 of '!' :: ls;
    x := x - 1;
  end;
  x := 80;
  while x > 0 do
    var r: real := 3.14;
    var l: list of int := null;
    var lls: list of string := [];

    y := 2000;
    while y > 0 do
      lls := "foo" :: lls;
      y := y - 1;
    end;

    y := 10000;
    while y > 0 do
      l := y :: l;
      y := y - 1;
    end;
    io.put (".");
    y := 100000;
    while y > 0 do
      var s: string := internal.version.version ();
      y := y - 1;
    end;
    if r <> 3.14 then
      return 1;
    end;
    x := x - 1;
  end;
  io.nl ();
  while lr <> null do
    if hd lr <> 3.14 then
      return 1;
    end;
    lr := tl lr;
  end;
  while ls <> null do
    var str: string := hd ls;
    if sizeof str <> 256 then
      return 1;
    end;
    var idx: int := 0;
    while idx < sizeof str do
      if str[idx] <> '!' then
	return 1;
      end;
      idx := idx + 1;
    end;
    ls := tl ls;
  end;
  io.put ("Success.\n");
  return 0;
end;

// End of stress0.t.
