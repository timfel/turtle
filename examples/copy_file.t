// copy_file.t -- Example file copying program.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* A simple @file{cp} replacement, for input/output and file
//* handling illustration.

module copy_file;

import io, chars, lists<string>;

fun main (args: list of string): int

  // Validate command line arguments.
  //
  if lists.length (args) <> 3 then
    io.put (io.error, "usage: copy_file INPUT OUTPUT\n");
    return 1;
  end;

  // Declare the file objects for the input and output files.
  //
  var inf: io.file, outf: io.file;

  // Open the input file and check for errors.
  //
  inf := io.open (hd tl args);
  if inf = null then
    io.put (io.error, hd tl args + ": cannot open input file\n");
    return 1;
  end;

  // Open the output file and check for errors.
  //
  outf := io.create (hd tl tl args);
  if outf = null then
    io.put (io.error, hd tl tl args + ": cannot open output file\n");
    return 1;
  end;

  // Read the input file until the end of file is reached, and
  // write each character to the output file.
  //
  var ch: char := io.get (inf);
  while ch <> chars.EOF do
    io.put (outf, ch);
    ch := io.get (inf);
  end;

  // Close the files.
  io.close (inf);
  io.close (outf);

  // Indicate success.
  return 0;
end;

// End of copy_file.t.
