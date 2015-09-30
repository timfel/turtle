// helloserver.t -- Test file for the `sys.net' module.
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

module helloserver;

import io, sys.procs, sys.net, sys.files, sys.errno, binary, 
  lists<string>, strings;

fun check_ret (val: int, op: string)
  if val < 0 then
    io.put (io.error, "helloserver: ");
    io.put (io.error, op);
    io.put (io.error, ": ");
    io.put (io.error, sys.errno.strerror (sys.errno.errno));
    io.nl (io.error);
    sys.procs.exit (1);
  end;
end;

fun main(argv: list of string): int
  var sockfd: int, ret: int, clientfd: int;
  var addr: sys.net.sockaddr;
  var name: string;
  var buf: binary.binary := binary.make (128);

  sockfd := sys.net.socket (sys.net.PF_INET, sys.net.SOCK_STREAM, 0);
  check_ret (sockfd, "socket");
  
  ret := sys.net.bind (sockfd, sys.net.inetaddr (5454));
  check_ret (ret, "bind");

  ret := sys.net.listen (sockfd, 8);
  check_ret (ret, "listen");

  clientfd, addr := sys.net.accept (sockfd);
  check_ret (clientfd, "accept");

  ret := sys.files.read (clientfd, buf, binary.size (buf));
  check_ret (ret, "read");

  name := strings.substring (binary.to_string (buf), 0, ret - 1);
  ret := sys.files.write (clientfd, binary.from_string ("Hello, "), 7);
  check_ret (ret, "write");
  ret := sys.files.write (clientfd, binary.from_string (name), sizeof name);
  check_ret (ret, "write");
  ret := sys.files.write (clientfd, binary.from_string ("!\n"), 2);

  ret := sys.files.close (clientfd);
  check_ret (ret, "close (1)");

  ret := sys.files.close (sockfd);
  check_ret (ret, "close (2)");
  return 0;
end;

// End of helloserver.t.
