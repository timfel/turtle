// hashtab0.t -- Test file for hash tables.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module hashtab0;

import io, hashtab<string, int>, hash, strings, option<int>;

fun main(argv: list of string): int
  var h: hashtab.hashtable;
  h := hashtab.make (hash.hash, strings.eq);
  hashtab.insert (h, "Martin", 25);
  hashtab.insert (h, "Ilka", 21);
  var n: option.option := hashtab.lookup (h, "Martin");
  if option.some? (n) then
    io.put (option.data (n));
    io.nl ();
  else
    io.put ("not found\n");
  end;
  n := hashtab.lookup (h, "Ilka");
  if option.some? (n) then
    io.put (option.data (n));
    io.nl ();
  else
    io.put ("not found\n");
  end;

  hashtab.delete (h, "Martin");
  n := hashtab.lookup (h, "Martin");
  if option.some? (n) then
    io.put (option.data (n));
    io.nl ();
  else
    io.put ("not found\n");
  end;
  n := hashtab.lookup (h, "Ilka");
  if option.some? (n) then
    io.put (option.data (n));
    io.nl ();
  else
    io.put ("not found\n");
  end;
  return 0;
end;

// End of hashtab0.t.
