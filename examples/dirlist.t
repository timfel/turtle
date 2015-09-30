// dirlist.t -- Directory listing for the Turtle web server.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* Directory listing for the Turtle web server.

module dirlist;

import io, strings, lists<string>, compare, sys.procs, 
  sys.errno, listmap<string, string>, sys.dirs, sys.files, http, config;


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


//* Return @code{true} if the file called @var{fname} does exist
//* and is a directory, return @code{false} otherwise.
//
fun isdir (fname: string): bool
  var stat: sys.files.stat := sys.files.stat (config.document_root + fname);
  if stat <> null then
    if (sys.files.mode (stat) / 16384) % 2 = 1 then
      return true;
    end;
  end;
  return false;
end;


//* Create a directory listing for the ressource @var{ofname} and
//* return it as a HTTP response object.
//
public fun make_directory_listing (req: http.request, ofname: string): 
                                   http.response
  var res: list of string := [];
  var fname: string;

  if ofname[0] <> '/' then
    fname := strings.append (config.document_root, "/", ofname);
  else
    fname := strings.append (config.document_root, ofname);
  end;
  if ofname[sizeof ofname - 1] <> '/' then
    ofname := strings.append (ofname, "/");
  end;

  var d: sys.dirs.dir := sys.dirs.opendir (fname);
  d := sys.dirs.opendir (fname);
  if d = null then
    return http.fail (404, "Not found", http.version (req), 
                      http.std_html_headers (),
		      http.not_found_body (), -1L, req, http.file (req));
  else
    var e: string := sys.dirs.readdir (d);
    e := sys.dirs.readdir (d);
    while e <> null do
      if not strings.eq (e, ".") then
        res := lists.insert (e, res, compare.cmp);
      end;
      e := sys.dirs.readdir (d);
    end;
    var ret: int := sys.dirs.closedir (d);
    check_ret (ret, "closedir");
    res := listmap.map (fun (s: string): string
                          if isdir (ofname + s) then
			    return "<li><a href=\"" + ofname + s +
			    "\"><tt>" + s + "</tt></a><tt> -></tt></li>\n";
			  else
			    return "<li><a href=\"" + ofname + s +
			    "\"><tt>" + s + "</tt></a></li>\n";
			  end;
			end,
			res);
    res := strings.append (http.server_header (strings.append ("Index of ", ofname)), "<UL>") :: res;

    res := lists.append (res, "</UL>" :: http.server_footer ());
    return http.ok (http.version (req), http.std_html_headers (),
               res, -1L, req, http.file (req), -1, null);
  end;
end;

// End of dirlist.t.
