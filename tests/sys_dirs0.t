// sys_dirs0.t -- Test file for the `sys.dirs' module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module sys_dirs0;

import io, sys.dirs;

fun main(argv: list of string): int
  var d: sys.dirs.dir;
  var res: int;

  d := sys.dirs.opendir ("/");
  if d <> null then
    var s: string;

    s := sys.dirs.readdir (d);
    while s <> null do
      io.put (s); io.nl ();
      s := sys.dirs.readdir (d);
    end;

    sys.dirs.rewinddir (d);

    s := sys.dirs.readdir (d);
    while s <> null do
      io.put (s); io.nl ();
      s := sys.dirs.readdir (d);
    end;

    res := sys.dirs.closedir (d);
  end;
  return 0;
end;

// End of sys_dirs0.t.
