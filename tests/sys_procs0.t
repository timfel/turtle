// sys_procs0.t -- Test file for the module `sys.procs'.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module sys_procs0.t;

import io, sys.procs, sys.errno;

fun main(argv: list of string): int
  io.put ("pid: "); io.put (sys.procs.getpid ()); io.nl ();
  io.put ("ppid: "); io.put (sys.procs.getppid ()); io.nl ();
  io.put ("kill (43636, 0): "); io.put (sys.procs.kill (43636, 0)); io.nl ();
  var errnum: int := sys.errno.errno;
  io.put ("errno: "); io.put (errnum); io.nl ();
  io.put ("strerror (errno): "); io.put (sys.errno.strerror(errnum)); io.nl ();

  var child: int, status: int;

  child := sys.procs.fork ();
  if child = -1 then
    io.put ("cannot fork\n");
  elsif child = 0 then
    io.put ("in child, going to sleep (2)\n");
    sys.procs.sleep (2);
    io.put ("in child, exiting\n");
    sys.procs.exit (1);
  else
    io.put ("in parent, forked child "); io.put (child); io.nl ();
    child, status := sys.procs.wait ();
    io.put ("child "); io.put (child); io.put (" has exited\n");
    if sys.procs.WIFEXITED (status) then
      io.put ("normal exit, exit code ");
      io.put (sys.procs.WEXITSTATUS (status));
      io.nl ();
    elsif sys.procs.WIFSIGNALED (status) then
      io.put ("signaled\n");
    elsif sys.procs.WIFSTOPPED (status) then
      io.put ("stopped\n");
    end;
  end;
    
  child := sys.procs.fork ();
  if child = -1 then
    io.put ("cannot fork\n");
  elsif child = 0 then
    io.put ("in child, going to sleep (2)\n");
    sys.procs.sleep (2);
  else
    io.put ("in parent, forked child "); io.put (child); io.nl ();
    sys.procs.kill (child, 9);
    child, status := sys.procs.wait ();
    io.put ("child "); io.put (child); io.put (" has exited\n");
    if sys.procs.WIFEXITED (status) then
      io.put ("normal exit\n");
    elsif sys.procs.WIFSIGNALED (status) then
      io.put ("signaled\n");
    elsif sys.procs.WIFSTOPPED (status) then
      io.put ("stopped\n");
    end;
  end;
    
  child := sys.procs.fork ();
  if child = -1 then
    io.put ("cannot fork\n");
  elsif child = 0 then
    io.put ("in child, going execve (\"/bin/ls\", ...)\n");
    sys.procs.execve ("/bin/ls", ["/bin/ls", "-ld", "."]);
  else
    io.put ("in parent, forked child "); io.put (child); io.nl ();
    child, status := sys.procs.wait ();
    io.put ("child "); io.put (child); io.put (" has exited\n");
    if sys.procs.WIFEXITED (status) then
      io.put ("normal exit\n");
    elsif sys.procs.WIFSIGNALED (status) then
      io.put ("signaled\n");
    elsif sys.procs.WIFSTOPPED (status) then
      io.put ("stopped\n");
    end;
  end;
    
  return 0;
end;

// End of sys_procs0.t.
