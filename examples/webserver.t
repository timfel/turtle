// webserver.t -- Web server written in Turtle.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* This is a minimal web server, written in Turtle.

module webserver;

import io, sys.procs, sys.net, sys.files, sys.errno, binary, 
  strings, cmdline, internal.version, ints, sys.times, 
  http, config, dirlist, wiki, game;


//* Create a listening socket and bind it to the given @var{port}.
//
fun setup_server_socket (port: int): int
  var ret: int;
  var sockfd: int;

  sockfd := sys.net.socket (sys.net.PF_INET, sys.net.SOCK_STREAM, 0);
  check_ret (sockfd, "socket");
  ret := sys.net.bind (sockfd, sys.net.inetaddr (port));
  check_ret (ret, "bind");
  ret := sys.net.listen (sockfd, 8);
  check_ret (ret, "listen");
  return sockfd;
end;


//* Check whether @var{val} is less than zero, and if it is, print
//* an error message including the error message from
//* @code{sys.errno.errno}.
//
fun check_ret (val: int, op: string)
  if val < 0 then
    check_ret_no_exit (val, op);
    sys.procs.exit (1);
  end;
end;

//* Like @code{check_ret}, but do no exit the process, just write out
//* the error message.
//
fun check_ret_no_exit (val: int, op: string)
  if val < 0 then
    io.put (io.error, "webserver: ");
    io.put (io.error, op);
    io.put (io.error, ": ");
    io.put (io.error, sys.errno.strerror (sys.errno.errno));
    io.nl (io.error);
  end;
end;

var about_page_text: string := 
  "<p>This is a simple webserver, written completely in Turtle.  It has the following features:</p>\n<ul>\n<li>Configurable port number and document root.</li>\n<li>Builtin directory listing.</li>\n<li>Transfer logging.</li>\n<li>Can be run in forking or single-tasked mode</li>\n<li>This about page, which is server generated.</li>\n</ul>\n";

fun make_about_page (req: http.request): http.response
  var body: list of string;

  body := http.server_header ("About Turtle webserver") ::
           about_page_text ::
	   http.server_footer ();
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;

fun make_date_page (req: http.request): http.response
  var time: sys.times.tm := sys.times.gmtime (sys.times.time ());
  var date: string;
  var body: list of string;

  date := ints.to_string (sys.times.year (time) + 1900) + "-" +
          ints.to_string (sys.times.mon (time)) + "-" +
          ints.to_string (sys.times.mday (time)) + " " +
          ints.to_string (sys.times.hour (time)) + ":" +
          ints.to_string (sys.times.min (time)) + ":" +
          ints.to_string (sys.times.sec (time));

  body := http.server_header ("Date on the server") ::
          "<p>The current date on the server is:</p><p>" ::
           date ::
	   " UTC</p>" ::
	   http.server_footer ();
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;

var dynurl_handler: list of (string, fun(http.request): http.response) :=
  [("/about", make_about_page),
   ("/date", make_date_page),
   ("/wiki", wiki.handle_wiki_request),
   ("/game", game.handle_game_request)];

//* Handle a @code{GET} request by creating an appropriate response
//* object.
//
fun handle_get_or_post (req: http.request, addr: sys.net.sockaddr): 
                        http.response
  var f: int;
  var fname: string;
  var ressource: string;
  var i: int;
  ressource := http.ressource (req);
  fname := ressource;
  i := 0;
  while i < sizeof fname - 1 do
    if fname[i] = '.' and fname[i + 1] = '.' then
      return  http.fail (400, "Bad Request", 9, http.std_html_headers (),
                    http.bad_request_body (), -1L, req, http.file (req));
    end;
    i := i + 1;
  end;

  if ressource[0] <> '/' then
    ressource := "/" + ressource;
  end;
  fname := config.document_root + ressource;

  var stat: sys.files.stat := sys.files.stat (fname);
  var count: long := 0L;

  if stat <> null then
    if (sys.files.mode (stat) / 16384) % 2 = 1 then
      return dirlist.make_directory_listing (req, http.ressource (req));
    end;
    count := sys.files.size (stat);
  else
    var ll: list of (string, fun(http.request): http.response) := 
      dynurl_handler;
    while ll <> null do
      var dynurl: string;
      var handler: fun (http.request): http.response;
      dynurl, handler := hd ll;
      if strings.eq (ressource, dynurl) or
	(sizeof ressource > sizeof dynurl and
	 strings.eq (strings.substring (ressource, 0, sizeof dynurl), dynurl)
	 and ressource[sizeof dynurl] = '/')
      then
	return handler (req);
      end;
      ll := tl ll;
    end;
    return http.fail (404, "Not found", http.version (req), 
                      http.std_html_headers (),
		      http.not_found_body (), -1L, req, http.file (req));
  end;
  f := sys.files.open (fname);
  if f < 0 then
    return http.fail (404, "Not found", http.version (req), 
                      http.std_html_headers (),
		      http.not_found_body (), -1L, req, http.file (req));
  else
    return http.ok (http.version (req), http.file_headers (fname),
               [], count, req, http.file (req), f, null);
  end;
end;

fun to_string (addr: sys.net.sockaddr): string
  var b: binary.binary := sys.net.sockaddr_addr (addr);
  return ints.to_string (binary.get (b, 0)) + "." +
          ints.to_string (binary.get (b, 1)) + "." +
	  ints.to_string (binary.get (b, 2)) + "." +
	  ints.to_string (binary.get (b, 3));
end;

fun log (resp: http.response)
  var time: sys.times.tm := sys.times.gmtime (sys.times.time ());
  var req: http.request := http.req (resp);
  io.put (to_string (http.addr (req))); io.put (" - - [");
  io.put (sys.times.year (time) + 1900); io.put ("-");
  io.put (sys.times.mon (time)); io.put ("-");
  io.put (sys.times.mday (time)); io.put (" ");
  io.put (sys.times.hour (time)); io.put (":");
  io.put (sys.times.min (time)); io.put (":");
  io.put (sys.times.sec (time)); io.put ("] ");
  if http.get? (req) then
    io.put ("\"GET ");
    io.put (http.ressource (req));
    io.put ("\" ");
  elsif http.post? (req) then
    io.put ("\"POST ");
    io.put (http.ressource (req));
    io.put ("\" ");
  end;
  if http.fail? (resp) then
    io.put (http.code (resp));
  else
    io.put (200);
  end;
  io.put (" "); io.put (http.content_length (resp));
  io.nl ();
end;

fun handle_connection (client_socket: int, addr: sys.net.sockaddr)
  var ret: int;
  var req: http.request, resp: http.response;

  req := http.read_request (client_socket, addr);
  if http.get? (req) then
    resp := handle_get_or_post (req, addr);
  elsif http.post? (req) then
    resp := handle_get_or_post (req, addr);
  else
    resp := http.fail (400, "Bad Request", 9, http.std_html_headers (),
                  http.bad_request_body (), -1L, req, http.file (req));
  end;
  http.send_response (resp);
  ret := sys.files.close (client_socket);
  check_ret (ret, "close");

  log (resp);
end;

fun main_loop (accept_socket: int)
  var ret: int, status: int, client_socket: int;
  var addr: sys.net.sockaddr;
  var pid: int;
  var children: list of int := [];

  while true do

    // First, we try to free all of the children which have already 
    // exited.
    //
    var c: list of int := [];

    while children <> null do
      pid := hd children;
      children := tl children;
      ret, status := sys.procs.waitpid (pid, sys.procs.WNOHANG);
      if ret < 0 then
	check_ret_no_exit (ret, "waitpid");
      elsif ret = 0 then
	c := pid :: c;
      else
	if config.debug_mode then
	  io.put ("webserver: child process has exited: ");
	  io.put (ret);
	  if sys.procs.WIFEXITED (status) then
	    io.put (" (exit code: ");
	    io.put (sys.procs.WEXITSTATUS (status));
	    io.put (")");
	  elsif sys.procs.WIFSIGNALED (status) then
	    io.put (" (signal: ");
	    io.put (sys.procs.WTERMSIG (status));
	    io.put (")");
	  end;
	  io.nl ();
	end;
      end;
    end;
    children := c;

    // Accept a client connection.
    //
    client_socket, addr := sys.net.accept (accept_socket);
    check_ret (client_socket, "accept");

    if config.fork_children then
      // Create a child process to handle the connection.
      //
      pid := sys.procs.fork ();
      check_ret (pid, "fork");

      if pid = 0 then
	// Now, if we are in the child, close the file descriptor for
	// the accept socket, handle the request and exit successfully.
	//
	ret := sys.files.close (accept_socket);
	check_ret (ret, "close");
	handle_connection (client_socket, addr);
	sys.procs.exit (0);
      else
	// In the parent, add the newly created child to the list of
	// child processes (so that we can clean them up when they are
	// done), close the accepted socket and go back to the beginning
	// of the loop, accepting more connections.
	//
	children := pid :: children;
	ret := sys.files.close (client_socket);
	check_ret (ret, "close");
      end;
    else
      handle_connection (client_socket, addr);
    end;
  end;
end;

var optspecs: list of cmdline.optspec :=
  [cmdline.flag ('h', "help"),
   cmdline.flag ('v', "version"),
   cmdline.option ('p', "port"),
   cmdline.option ('d', "docroot"),
   cmdline.option ('w', "wikiroot"),
   cmdline.flag ('s', "single-task"),
   cmdline.flag ('D', "debug")];

fun help ()
  io.put ("usage: webserver [OPTION...]\n");
  io.put ("  -h, --help          display this help and exit\n");
  io.put ("  -v, --version       display version information and exit\n");
  io.put ("  -p, --port=NUM      listen on port number NUM\n");
  io.put ("  -d, --docroot=PATH  use PATH as the document root\n");
  io.put ("  -w, --wikiroot=PATH use PATH as the wiki root\n");
  io.put ("  -s, --single-task   do not fork children for requests\n");
  io.put ("Report bugs to mgrabmue@cs.tu-berlin.de\n");
end;

fun version ()
  io.put ("webserver version ");
  io.put (http.server_version);
  io.put (" (Turtle version ");
  io.put (internal.version.version ());
  io.put (")\n");
end;

fun pathify (s: string): string
  if sizeof s > 0 and s[sizeof s - 1] <> '/' then
    return s + "/";
  else
    return s;
  end;
end;

fun main(argv: list of string): int
  var accept_socket: int;
  var ret: int;
  var opts: list of cmdline.option := cmdline.getopt (optspecs, argv);

  while opts <> null do
    var opt: cmdline.option := hd opts;

    if cmdline.flag? (opt) then
      if (cmdline.flag_char (opt) = 'v') then
	version ();
	return 0;
      elsif (cmdline.flag_char (opt) = 'h') then
	help ();
	return 0;
      elsif (cmdline.flag_char (opt) = 's') then
	config.fork_children := false;
      elsif (cmdline.flag_char (opt) = 'D') then
	config.debug_mode := true;
      elsif (cmdline.flag_char (opt) = '?') then
	help ();
	return 1;
      end;
    elsif cmdline.option? (opt) then
      if cmdline.option_char (opt) = 'p' then
	config.server_port := ints.from_string (cmdline.argument (opt));
      elsif cmdline.option_char (opt) = 'd' then
	config.document_root := pathify (cmdline.argument (opt));
      elsif cmdline.option_char (opt) = 'w' then
	config.wiki_root := pathify (cmdline.argument (opt));
      elsif (cmdline.flag_char (opt) = '?') then
	help ();
	return 1;
      end;
    else
      help ();
      return 1;
    end;
    opts := tl opts;
  end;

  accept_socket := setup_server_socket (config.server_port);
  main_loop (accept_socket);
  ret := sys.files.close (accept_socket);
  check_ret (ret, "close");

  return 0;
end;

// End of webserver.t.
