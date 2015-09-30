// sys_files0.t -- Test file for the `sys.files' module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module sys_files0;

import io, sys.files, sys.errno, binary;

var buf_size: int := 4096;

fun main(argv: list of string): int
  var b: binary.binary := binary.make (buf_size);
  var fd1: int, fd2: int;
  var rd: int, wr: int;

  fd1 := sys.files.open ("sys_files0");
  if fd1 < 0 then
    io.put (io.error, "cannot open source file: ");
    io.put (io.error, sys.errno.strerror (sys.errno.errno));
    io.nl (io.error);
  end;
  fd2 := sys.files.create ("sys_files0.cp");
  if fd2 < 0 then
    io.put (io.error, "cannot open destination file: ");
    io.put (io.error, sys.errno.strerror (sys.errno.errno));
    io.nl (io.error);
  end;
  rd := sys.files.read (fd1, b, buf_size);
  while rd > 0 do
    wr := sys.files.write (fd2, b, rd);
    if wr < 0 then
      io.put (io.error, "cannot write to destination file: ");
      io.put (io.error, sys.errno.strerror (sys.errno.errno));
      io.nl (io.error);
    end;
    rd := sys.files.read (fd1, b, buf_size);
  end;
  if rd < 0 then
    io.put (io.error, "cannot read from source file: ");
    io.put (io.error, sys.errno.strerror (sys.errno.errno));
    io.nl (io.error);
  end;
  sys.files.close (fd1);  
  sys.files.close (fd2);  
  
  if sys.files.unlink ("sys_files0.cp") < 0 then
    io.put (io.error, "cannot delete destination file: ");
    io.put (io.error, sys.errno.strerror (sys.errno.errno));
    io.nl (io.error);
  end;
 return 0;
end;

// End of sys_files0.t.
