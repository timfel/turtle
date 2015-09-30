// sys_net0.t -- Test file for the `sys.net' module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module sys_net0;

import io, sys.procs, sys.net, sys.files, sys.errno, binary, strings;

fun check_ret (val: int, op: string)
  if val < 0 then
    io.put (io.error, "sys_net0: ");
    io.put (io.error, op);
    io.put (io.error, ": ");
    io.put (io.error, sys.errno.strerror (sys.errno.errno));
    io.nl (io.error);
    sys.procs.exit (1);
  end;
end;

fun main(argv: list of string): int
  var sockfd: int, ret: int;
  var getmsg: string := "GET / HTTP/1.0\r\n\r\n";
  var buf: binary.binary := binary.make (128);

  sockfd := sys.net.socket (sys.net.PF_INET, sys.net.SOCK_STREAM, 0);
  check_ret (sockfd, "socket");
  io.put ("socket fd: "); io.put (sockfd); io.nl ();
  ret := sys.net.connect (sockfd, sys.net.inetaddr (80, 127, 0, 0, 1));
  check_ret (ret, "connect");
  ret := sys.files.write (sockfd, binary.from_string (getmsg), sizeof getmsg);
  check_ret (ret, "write");
  ret := sys.files.read (sockfd, buf, binary.size (buf));
  check_ret (ret, "read");
  io.put ("read "); io.put (ret); io.put (" bytes: ");
  io.nl ();

  io.put (strings.substring (binary.to_string (buf), 0, ret));
  io.nl ();
  ret := sys.files.close (sockfd);
  check_ret (ret, "close");

  var addr: binary.binary;
  addr := sys.net.gethostbyname ("localhost");
  if addr <> null then
    io.put ("Host `localhost' found.\n");
  else
    io.put ("Host `localhost' not found.\n");
  end;
  return 0;
end;

// End of sys_net0.t.
