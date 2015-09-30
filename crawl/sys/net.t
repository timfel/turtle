// sys/net.t -- Network programming
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//  
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this package; see the file COPYING.  If not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.

// Commentary:
//
//* Module for network programming.
//* 
//* Currently, only IPv4 is supported, and only the most basic
//* operations are provided.

module sys.net;

import internal.binary, sys.errno;


//* These constants are to be used as the @var{domain} parameter to
//* the @code{socket} function.  Note that currently only IPv4
//* (@code{PF_INET}) sockets are supported.
public const PF_UNSPEC: int := foreign "TTL_INT_TO_VALUE (PF_UNSPEC)";
//* ""
public const PF_UNIX: int := foreign "TTL_INT_TO_VALUE (PF_UNIX)";
//* ""
public const PF_INET: int := foreign "TTL_INT_TO_VALUE (PF_INET)";
//* - Commented out because Solaris doesn't like it:
// public const PF_LOCAL: int := foreign "TTL_INT_TO_VALUE (PF_LOCAL)";

//* These are the socket type constants for calls to the
//* @code{socket} function.  Only stream sockets
//* (@code{SOCK_STREAM}) have been tested yet.
public const SOCK_STREAM: int := foreign "TTL_INT_TO_VALUE (SOCK_STREAM)";
//* ""
public const SOCK_DGRAM: int := foreign "TTL_INT_TO_VALUE (SOCK_DGRAM)";
//* ""
public const SOCK_RAW: int := foreign "TTL_INT_TO_VALUE (SOCK_RAW)";


//* Create an endpoint for communication and return a descriptor.
//* 
//* The @var{domain} parameter specifies a communication domain;
//* this selects the protocol family which will be used for
//* communication.  These families are defined as the @code{PF_*}
//* constants.
//* 
//* The socket has the indicated type @var{typ}, which specifies
//* the communication semantics.  The currently defined types are
//* the @code{SOCK_*} constants.
//* 
//* -1 is returned if an error occurs and @code{sys.errno.errno} is
//* set accordingly, othersie the return value is a descriptor
//* referencing the socket.
//
public fun socket (domain: int, typ: int, protocol: int): int;


//* Data type for specifying network addresses.  Currently only
//* IPv4 is supported.
public datatype sockaddr = inet (port: int, addr: internal.binary.binary);

//* Return the IP number or the port of the socket address
//* @var{addr}, respectively.
public fun sockaddr_addr (addr: sockaddr): internal.binary.binary
  return addr (addr);
end;
//* ""
public fun sockaddr_port (addr: sockaddr): int
  return port (addr);
end;

//* Give the socket @var{sockfd} the local address @var{addr}.
//* This is necessary for a stream socket may receive connections
//* with @code{accept}.
//
public fun bind (sockfd: int, addr: sockaddr): int;


//* Establish a connection for socket descriptor @var{sockfd} to
//* the server specified by @var{serv_addr}.  The return value is
//* -1 if an error occurs and zero on success.
//
public fun connect (sockfd: int, serv_addr: sockaddr): int;


//* Before accepting connections with @code{accept}, the
//* willingness to accept incoming connections and a queue limit
//* for incoming connections are specified with @code{listen}.
//
public fun listen (sockfd: int, backlog: int): int;


//* Accept a client connection on the given socket @var{sockfd}.
//* Return a pair of a socket descriptor for the connection to the
//* client, and the address of the client.
//
public fun accept (sockfd: int): (int, sockaddr)
  var sa: sockaddr := inetaddr (0, 0, 0, 0, 0);
  var ret: int := iaccept (sockfd, sa);
  return ret, sa;
end;
//* -
// Support function for @code{accept}.
//
fun iaccept (sockfd: int, sockaddr: sockaddr): int;


//* Create an internet address structure with a given @var{port}
//* and the given bytes of the internet address.  The bytes are to
//* be passed highest-order byte first, for example:
//* 
//* @example
//* "127.0.0.1"
//* @result{}
//* 127, 0, 0, 1
//* @end example
//* 
//* The second version of the function will set the address to zero
//* and can be used when binding an address for a server socket
//* where the address should be the default.
//
public fun inetaddr (port: int, b0: int, b1: int, b2: int, b3: int): sockaddr
  var b: internal.binary.binary := internal.binary.make (4);
  internal.binary.set (b, 0, b0);
  internal.binary.set (b, 1, b1);
  internal.binary.set (b, 2, b2);
  internal.binary.set (b, 3, b3);
  return inet (port, b);
end;
//* ""
public fun inetaddr (port: int): sockaddr
  return inetaddr (port, 0, 0, 0, 0);
end;
//* ""
public fun inetaddr (port: int, b: internal.binary.binary): sockaddr
  return inet (port, b);
end;

//* Return the IPv4 address of host @var{name}.  @var{name} must be
//* either a valid host name or an internet address in dotted
//* decimal notation.
//* 
//* The return value is a byte array representing the internet
//* address or @code{null}, if the host name cannot be resolved.
//
public fun gethostbyname (name: string): internal.binary.binary
  var b: internal.binary.binary := internal.binary.make (4);
  if igethostbyname (name, b) then
    return b;
  else
    return null;
  end;
end;
//* -
fun igethostbyname (name: string, b: internal.binary.binary): bool;

// End of sys/net.t.
