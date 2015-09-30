// sys_users0.t -- Test file for the `sys.users' module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module sys_users0;

import io, sys.users;

fun main(argv: list of string): int
  var pw: sys.users.passwd;
  var gr: sys.users.group;
  var idx: int;

  io.put ("* Search for user `nobody'\n");
  pw := sys.users.getpwnam ("nobody");
  io.put (sys.users.name (pw));
  io.nl ();
  io.put ("* Search for user `nobody's uid\n");
  pw := sys.users.getpwuid (sys.users.uid (pw));
  io.put (sys.users.name (pw));
  io.nl ();
  io.put ("* Search for user `root'\n");
  pw := sys.users.getpwnam ("root");
  io.put (sys.users.name (pw));
  io.nl ();
  io.put ("* Search for user id 0\n");
  pw := sys.users.getpwuid (0);
  io.put (sys.users.name (pw));
  io.nl ();

  io.put ("* List all users\n");
  pw := sys.users.getpwent ();
  while pw <> null do
    io.put (sys.users.name (pw));
    io.nl ();
    pw := sys.users.getpwent ();
  end;

  io.put ("* Reset user data base\n");
  sys.users.setpwent ();

  io.put ("* List all users\n");
  pw := sys.users.getpwent ();
  while pw <> null do
    io.put (sys.users.name (pw));
    io.nl ();
    pw := sys.users.getpwent ();
  end;

  io.put ("* Close user data base\n");
  sys.users.endpwent ();

  io.put ("* Look for group `daemon'\n");
  gr := sys.users.getgrnam ("daemon");
  io.put (sys.users.name (gr));
  io.nl ();
  io.put ("* Look for group `daemon's gid\n");
  gr := sys.users.getgrgid (sys.users.gid (gr));
  io.put (sys.users.name (gr));
  io.nl ();
  io.put ("* Look for group `root'\n");
  gr := sys.users.getgrnam ("root");
  io.put (sys.users.name (gr));
  io.nl ();
  io.put ("* Look for group id 0\n");
  gr := sys.users.getgrgid (0);
  io.put (sys.users.name (gr));
  io.nl ();

  io.put ("* List all groups\n");
  gr := sys.users.getgrent ();
  while gr <> null do
    io.put (sys.users.name (gr));
    idx := 0;
    while idx < sizeof sys.users.members (gr) do
      io.put (" ");
      io.put (sys.users.members (gr)[idx]);
      idx := idx + 1;
    end;
    io.nl ();
    gr := sys.users.getgrent ();
  end;

  io.put ("* Reset user data base\n");
  sys.users.setgrent ();

  io.put ("* List all groups\n");
  gr := sys.users.getgrent ();
  while gr <> null do
    io.put (sys.users.name (gr));
    io.nl ();
    gr := sys.users.getgrent ();
  end;

  io.put ("* Close group data base\n");
  sys.users.endgrent ();

  return 0;
end;

// End of sys_users0.t.
