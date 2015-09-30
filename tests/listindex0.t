// listindex0.t -- Test file for the module `listindex'.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module listindex0;

import io, ints, lists<int>, listindex<int>;

fun main(argv: list of string): int
  if listindex.nth ([3, 2, 4, 5], 0) <> 3 then
    return 1;
  end;
  if listindex.nth ([3, 2, 4, 5], 1) <> 2 then
    return 1;
  end;
  if listindex.nth ([3, 2, 4, 5], 2) <> 4 then
    return 1;
  end;
  if listindex.nth ([3, 2, 4, 5], 3) <> 5 then
    return 1;
  end;

  if listindex.pos (ints.even?, [3, 2, 4, 1]) <> 1 then
    return 1;
  end;
  if listindex.pos (ints.negative?, [3, 2, 4, 1]) <> -1 then
    return 1;
  end;
  if lists.length (listindex.slice ([2, 5, -1, 3, -12], 2, 4)) <> 2 then
    return 1;
  end;
  return 0;
end;

// End of listindex0.t.
