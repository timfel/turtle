// helloclient.t -- Test file for the `sys.net' module.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* This is a minimal example for network programming in Turtle.
//* The program fetches a web page from localhost.

module helloclient;

import io, sys.procs, sys.net, sys.files, sys.users, sys.errno, binary, 
  lists<string>, strings;

fun check_ret (val: int, op: string)
  if val < 0 then
    io.put (io.error, "helloclient: ");
    io.put (io.error, op);
    io.put (io.error, ": ");
    io.put (io.error, sys.errno.strerror (sys.errno.errno));
    io.nl (io.error);
    sys.procs.exit (1);
  end;
end;

fun main(argv: list of string): int
  var sockfd: int, ret: int;
  var user: string;
  var getmsg: string;
  var buf: binary.binary := binary.make (1024);
  var addr: sys.net.sockaddr;

  if lists.length (argv) < 2 then
    user := sys.users.getlogin ();
  else
    user := hd tl argv;
  end;
  if lists.length (argv) < 3 then
    addr := sys.net.inetaddr (5454, 127, 0, 0, 1);
  else
    var dest: string := hd tl tl argv;
    var b: binary.binary := sys.net.gethostbyname (dest);
    if b = null then
      io.put (io.error, "helloclient: gethostbyname: cannot find host: ");
      io.put (io.error, dest);
      io.nl (io.error);
      sys.procs.exit (1);
    end;
    addr := sys.net.inetaddr (5454, b);
  end;
  getmsg := strings.append (user, "\n");

  sockfd := sys.net.socket (sys.net.PF_INET, sys.net.SOCK_STREAM, 0);
  check_ret (sockfd, "socket");
  ret := sys.net.connect (sockfd, addr);
  check_ret (ret, "connect");
  ret := sys.files.write (sockfd, binary.from_string (getmsg), sizeof getmsg);
  check_ret (ret, "write");
  ret := sys.files.read (sockfd, buf, binary.size (buf));
  check_ret (ret, "read");
  while ret > 0 do
    io.put (strings.substring (binary.to_string (buf), 0, ret));
    ret := sys.files.read (sockfd, buf, binary.size (buf));
    check_ret (ret, "read");
  end;
  ret := sys.files.close (sockfd);
    check_ret (ret, "close");
  return 0;
end;

// End of helloclient.t.
