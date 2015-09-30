// http.t -- HTTP definitions for the Turtle Webserver.
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

module http;

import io, sys.net, sys.files, sys.times, binary, chars, strings, 
  lists<string>, lists<(string, string)>, sys.procs, sys.errno, ints,
  longs, html;


//* Version of the webserver.  This is printed on server-generated
//* web pages, such as error pages or directory listings and is
//* printed when the server is started with the @code{--version}
//* option.
public const server_version: string := "1.0";


//* Size of the buffer for copying files to the client connection.
//* The faster this is, the faster are files delivered, but more
//* memory is required.
//
const buffer_size: int := 1024 * 4;


//* Pre-allocated buffer for copying file contents.
//
const buffer: binary.binary := binary.make (buffer_size);


//* This data type represents requests from the clients.
//* 
//* The common fields are:
//* @table @code
//* @item addr
//* The IP number of the requesting client.
//* 
//* @item file
//* The file object for the connection to the requesting client.
//* 
//* @item ressource
//* The requested ressource.
//* 
//* @item version
//* The HTTP version of the request.  9 stands for 0.9, 10 for 1.0,
//* 11 for 1.1 etc.
//* 
//* @item headers
//* A list of key-value pairs representing the headers in the request.
//* 
//* @item body
//* A list of key-value pairs containing the arguments of a POST
//* request.
//* @end table
//* 
public datatype request = invalid (addr: sys.net.sockaddr, file: io.file) or
                          get (addr: sys.net.sockaddr, 
			       ressource: string, version: int, 
			       headers: list of (string, string),
			       body: list of (string, string),
			       file: io.file) or
                          post (addr: sys.net.sockaddr, 
			        ressource: string, version: int, 
				headers: list of (string, string),
				body: list of (string, string),
				file: io.file);


//* This data type represents the response to send back to the client.
//* 
//* @table @code
//* @item code
//* For an error response, this is the HTTP error code.
//* 
//* @item msg
//* For an error response, this is the human-readable error message.
//* 
//* @item headers
//* The list of key-value pairs to send back as the response headers.
//* 
//* @item body
//* The body of the response.  This is a list of strings which will
//* be concatenated for sending.  It is a list for more efficient
//* constructing of the response body.
//* 
//* @item content_length
//* The length of the response body.  If this is set to -1, it will be
//* calculated from the contents of the @code{body} field.
//* 
//* @item req
//* The @code{request} object to which this is the response.
//* 
//* @item file
//* The file object for the connection to the client.
//* 
//* @item passthrough
//* If this is greater than or equal to zero, it will be
//* interpreted as a file descriptor.  When sending the response,
//* the contents of the file will be sent to the client. Note that
//* the @code{body} field of the response must be @code{null}, or
//* this field will be ignored.
//* @end table
//* 
//* 
public datatype response = fail (code: int, msg: string, version: int,
                                 headers: list of (string, string),
				 body: list of string,
				 content_length: long,
				 req: request, file: io.file) or
			   ok (version: int, 
			       headers: list of (string, string),
			       body: list of string,
			       content_length: long,
			       req: request,
			       file: io.file,
			       passthrough: int,
			       html: html.tag);


//* Send the header lines given in the response object @var{resp}.
//* Additionally, send a @code{Content-length} header with the
//* content length of the body.  If the content length field is not
//* set yet (that is, if it is -1), it is calculated before sending
//* the header.
//
public fun send_headers (resp: response)
  var l: list of (string, string) := headers (resp);

  if content_length (resp) < 0L then
    var count: long := 0L;
    var s: list of string := body (resp);
    while s <> null do
      count := count + longs.from_int (sizeof hd s);
      s := tl s;
    end;
    content_length! (resp, count);
  end;

  while l <> null do
    var key: string, val: string;
    key, val := hd l;
    io.put (file (resp), key);
    io.put (file (resp), ": ");
    io.put (file (resp), val);
    io.put (file (resp), "\r\n");
    l := tl l;
  end;
  io.put (file (resp), "Content-length: ");
  io.put (file (resp), content_length (resp));
  io.put (file (resp), "\r\n");
end;


//* Send the body of the response @var{resp}.
//* 
//* If the body is not given as a list of strings, we check whether
//* a file descriptor was stored in field @code{passthrough}.  If
//* yes, this means that we should read the contents of this file
//* and send it down the connection, without storing all of the
//* file contents in memory.  This is much more efficient.
//
public fun send_body (resp: response)
  var l: list of string := body (resp);

  if l <> null then
    while l <> null do
      io.put (file (resp), hd l);
      l := tl l;
    end;
  elsif passthrough (resp) >= 0 then
    var res: int;
    var f: int := passthrough (resp);
    var outf: int := io.fd (file (resp));
    io.flush (file (resp));
    res := sys.files.read (f, buffer, buffer_size);
    while res > 0 do
      res := sys.files.write (outf, buffer, res);
      check_ret (res, "write");
      res := sys.files.read (f, buffer, buffer_size);
    end;
    check_ret (res, "read");
    res := sys.files.close (f);
    check_ret (res, "close");
  end;
end;


//* Return the value of the header field with the name @var{s}.
//* @var{h} is the list of headers, where each header is
//* represented as a key-value pair.  The key comparison is
//* case-independent.
//
public fun header_value (s: string, h: list of (string, string)): string
  s := strings.upcase (s);
  while h <> null do
    var n: string, v: string;
    n, v := hd h;
    if strings.eq (strings.upcase (n), s) then
      return v;
    end;
    h := tl h;
  end;
  return null;
end;


//* The same as header_value, but convert the return value to an
//* integer.  If the header with the given name is not present,
//* return the value of @var{default}.
//
fun header_int_value (s: string, h: list of (string, string), 
                      default: int): int
  var v: string := header_value (s, h);
  if v = null then
    return default;
  else
    return ints.from_string (v);
  end;
end;


//* Interpret the two characters @var{c1} and @var{c2} as
//* hexadecimal digits and return the corresponding integer value.
//* @var{c1} is the most significant digit.
//* 
fun to_int (c1: char, c2: char): int
  var v1: int, v2: int;

  if c1 >= '0' and c1 <= '9' then
    v1 := chars.ord (c1) - chars.ord ('0');
  else
    v1 := chars.ord (c1) - chars.ord ('A') + 10;
  end;
  if c2 >= '0' and c2 <= '9' then
    v2 := chars.ord (c2) - chars.ord ('0');
  else
    v2 := chars.ord (c2) - chars.ord ('A') + 10;
  end;
  return v1 * 16 + v2;
end;


//* URL-decode the string @var{s}.
//
fun decode (s: string): string
  var cl: list of char := strings.explode (s);
  var rl: list of char := [];
  while cl <> null do
    if hd cl = '%' and tl cl <> null and tl tl cl <> null then
      var val: int := to_int (hd tl cl, hd tl tl cl);
      cl := tl tl tl cl;
      rl := chars.chr (val) :: rl;
    elsif hd cl = '+' then
      rl := ' ' :: rl;
      cl := tl cl;
    else
      rl := hd cl :: rl;
      cl := tl cl;
    end;
  end;
  return strings.rimplode (rl);
end;


//* Interpret the string @var{s} as a CGI-encoded request body and
//* convert it to a list of name-value pairs.
//
fun parse_vals (s: string): list of (string, string)
  var amp_ind: int := strings.index (s, '&');
  var eq_ind: int;
  var res: list of (string, string) := [];
  var val: string;

  while amp_ind >= 0 do
    val := strings.substring (s, 0, amp_ind);
    s := strings.substring (s, amp_ind + 1, sizeof s);
    amp_ind := strings.index (s, '&');

    eq_ind := strings.index (val, '=');
    res := (strings.substring (val, 0, eq_ind), 
            decode (strings.substring (val, eq_ind + 1, sizeof val))) :: res;
//    io.putln (strings.substring (val, 0, eq_ind));
//    io.putln (decode (strings.substring (val, eq_ind + 1, sizeof val)));
  end;
  val := s;
  eq_ind := strings.index (val, '=');
  res := (strings.substring (val, 0, eq_ind), 
          decode(strings.substring (val, eq_ind + 1, sizeof val))) :: res;
//  io.putln (strings.substring (val, 0, eq_ind));
//  io.putln (decode (strings.substring (val, eq_ind + 1, sizeof val)));
  return lists.reverse (res);
end;


//* Read an HTTP request from the socket @var{sock} and turn it
//* into a request object.  If the request is somehow bad, an
//* @code{invalid} object is returned, otherwise a request object
//* of the variant matching the request.
//
public fun read_request (sock: int, addr: sys.net.sockaddr): request
  var c: char;
  var cl: list of char := [];
  var words: list of string;
  var version: int;
  var method: string, ressource: string;
  var headers: list of (string, string);
  var f: io.file := io.file (sock, string 128 of ' ', 0, 0, true);
  var vals: list of (string, string) := []; /* Body values.  */

  c := io.get (f);
  while c <> '\n' and c <> chars.EOF do
    if c <> '\r' then
      cl := c :: cl;
    end;
    c := io.get (f);
  end;
  words := strings.split (strings.rimplode (cl), ' ');
  if lists.length (words) = 2 then
    version := 9;
  elsif lists.length (words) = 3 then
    if strings.eq (hd tl tl words, "HTTP/1.0") then
      version := 10;
    elsif strings.eq (hd tl tl words, "HTTP/1.1") then
      version := 11;
    else
      version := 9;
    end;
  else
    return invalid (addr, f);
  end;
  method := hd words;
  ressource := hd tl words;

  // Read headers.
  cl := ['a'];
  while cl <> null do
    cl := [];
    c := io.get (f);
    while c <> chars.EOF and c <> '\n' do
      if c <> '\r' then
	cl := c :: cl;
      end;
      c := io.get (f);
    end;
    if cl <> null then
      var header: string := strings.rimplode (cl);
      var eq_ind: int := strings.index (header, ':');
      if eq_ind > 0 then
	words := strings.split (strings.rimplode (cl), ':');
	headers := (strings.substring (header, 0, eq_ind),
	strings.substring (header, eq_ind + 1, sizeof header)) :: headers;
      end;
    end;
  end;
  if strings.eq (method, "GET") then
    var y: int := strings.index (ressource, '?');
    var b: string;

    if y >= 0 then
      b := strings.substring (ressource, y + 1, sizeof ressource);
      ressource := strings.substring (ressource, 0, y);
      vals := parse_vals (b);
    end;
    return get (addr, ressource, version, lists.reverse (headers), vals, f);
  elsif strings.eq (method, "POST") then
    var cl: int := header_int_value ("CONTENT-LENGTH", headers, -1);
    if cl > 0 then
      var x: int := 0;
      var ccl: list of char := [];

      c := io.get (f); 			 /* Skip '\n'.  */
      while c <> chars.EOF and x < cl do
	if c <> '\r' then
	  ccl := c :: ccl;
	end;
	if x < cl - 1 then
	  c := io.get (f);
	end;
	x := x + 1;
      end;
      var s1: string := strings.rimplode (ccl);
      vals := parse_vals (s1);
    end;
    return post (addr, ressource, version, lists.reverse (headers), 
                 vals, f);
  else
    return invalid (addr, f);
  end;
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


//* This list maps file extensions to content types.  It is used to
//* guess a good @code{Content-type} header when transferring
//* files.
//
var map: list of (string, string) :=
  [("mp2", "audio/x-mpeg"),
   ("mpa", "audio/x-mpeg"),
   ("abs", "audio/x-mpeg"),
   ("mpega", "audio/x-mpeg"),
   ("mpeg", "video/mpeg"),
   ("mpg", "video/mpeg"),
   ("mpe", "video/mpeg"),
   ("mpv", "video/mpeg"),
   ("vbs", "video/mpeg"),
   ("mpegv", "video/mpeg"),
   ("bin", "application/octet-stream"),
   ("com", "application/octet-stream"),
   ("dll", "application/octet-stream"),
   ("bmp", "image/x-MS-bmp"),
   ("exe", "application/octet-stream"),
   ("mid", "audio/x-midi"),
   ("midi", "audio/x-midi"),
   ("htm", "text/html"),
   ("html", "text/html"),
   ("txt", "text/plain"),
   ("gif", "image/gif"),
   ("tar", "application/x-tar"),
   ("jpg", "image/jpeg"),
   ("jpeg", "image/jpeg"),
   ("png", "image/png"),
   ("ra", "audio/x-pn-realaudio"),
   ("ram", "audio/x-pn-realaudio"),
   ("sys", "application/octet-stream"),
   ("wav", "audio/x-wav"),
   ("xbm", "image/x-xbitmap"),
   ("zip", "application/x-zip"),
   ("deb", "application/x-debian-package"),
   ("ps", "application/postscript")];

//* Return the value for the @code{Content-type} header field,
//* based on the file extension of file name @var{fname}.
//
fun get_content_type (fname: string): string
  var i: int := strings.rindex (fname, '.');
  if i >= 0 then
    var s: string := strings.substring (fname, i + 1, sizeof fname);
    var m: list of (string, string) := map;
    var t1: string, t2: string;
    while m <> null do
      t1, t2 := hd m;
      if strings.eq (s, t1) then
	return t2;
      end;
      m := tl m;
    end;
  end;
  return "application/octet-stream";
end;


//* Return the standard headers, and a @code{Content-type: text/html}
//* header.  This is used for error messages which are always HTML.
//
public fun std_html_headers (): list of (string, string)
  return ("Content-type", "text/html") :: std_headers ();
end;


//* Return the headers which should go out with every response.
//
public fun std_headers (): list of (string, string)
  var date: string := sys.times.asctime (sys.times.gmtime (sys.times.time ()));
  date := strings.substring (date, 0, sizeof date - 1);
  return [("Server", "Turtle webserver " + server_version),
          ("Connection", "close"),
	  ("Date", date)];
end;


//* Return the headers for filename @var{fname}.  The only
//* difference to @code{std_headers} is that we try to figure out a
//* sensible value for the @code{Content-type} header.
//
public fun file_headers (fname: string): list of (string, string)
  return ("Content-type", get_content_type (fname)) :: std_headers ();
end;


//* Return a string which represents the head of all
//* server-generated web pages, such as error pages or directory
//* listings.
//
public fun server_header (title: string): string
  return "<HTML>\n<HEAD>\n<TITLE>" + title + 
         "</TITLE>\n</HEAD>\n<BODY bgcolor=\"white\" text=\"black\">\n<H1>" + title + "</H1>\n";
end;


//* Return a list of strings for the HTML code to attach to all
//* server-generated web pages, such as error pages or directory
//* listings.
//
public fun server_footer (): list of string
  return ["\n<HR>\n",
          "<ADDRESS>Turtle webserver version ",
	  server_version, "</ADDRESS>\n</BODY>\n</HTML>\n"];

end;


//* Return the body to send for a File Not Found error.
//
public fun not_found_body (): list of string
  return server_header ("404 Not Found") ::
  "<P>The requested ressource was not found.</P>\n" ::
  server_footer ();
end;


//* Return the body to send for a Bad Request error.
//
public fun bad_request_body (): list of string
  return server_header ("400 Bad Request") ::
  "<P>Malformed request.</P>" ::
  server_footer ();
end;


//* Send the response represented by the response object @var{resp}
//* to the client.
//
public fun send_response (resp: response)
  io.put (file (resp), "HTTP/" + ints.to_string (version (resp) / 10) +
          "." + ints.to_string (version (resp) % 10));
  if fail? (resp) then
    io.put (file (resp), 
            " " + ints.to_string (code (resp)) + " " + msg (resp));
  else
    io.put (file (resp), " 200 OK");
  end;
  io.put (file (resp), "\r\n");
  if ok? (resp) then
    if html (resp) <> null then
      body! (resp, html.flatten (html (resp)));
    end;
  end;
  send_headers (resp);
  io.put (file (resp), "\r\n");
  send_body (resp);
end;

public fun not_found_response (req: request): response
  return fail (404, "Not found", version (req), 
               std_html_headers (),
	       not_found_body (), -1L, req, file (req));
end;

public fun bad_request_response (req: request): response
  return fail (400, "Bad Request", version (req), 
               std_html_headers (),
	       bad_request_body (), -1L, req, file (req));
end;

// End of http.t.
