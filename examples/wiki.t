// wiki.t -- Wiki functions for the Turtle web server.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* Wiki functionality for the Turtle web server.

module wiki;

import io, chars, ints, strings, lists<string>, lists<char>, http, config,
  sys.files, sys.times, sys.dirs, listmap<string, string>,
  listsort<string>, compare, sys.procs;


public fun wiki_page_count (): int
  var count: int := 0;
  var d: sys.dirs.dir := sys.dirs.opendir (config.wiki_root);
  if d = null then
    return 0;
  end;
  var s: string := sys.dirs.readdir (d);
  while s <> null do
    if not invalid_wiki_name (s) then
      count := count + 1;
    end;
    s := sys.dirs.readdir (d);
  end;
  var i: int := sys.dirs.closedir (d);
  return count;
end;


//* Return true if the wiki page called @var{s} exists, false otherwise.
//
fun wiki_exists (s: string): bool
  var f: io.file := io.open (config.wiki_root + s);
  if f <> null then
    io.close (f);
    return true;
  else
    return false;
  end;
end;


//* Append the reversed string @var{s} to the front of the
//* character list @var{rl}.  This is intended for constructing
//* character lists which will later be converted to strings using
//* @code{strings.rimplode()}.
//
fun append_string (s: string, rl: list of char): list of char
  return lists.append (lists.reverse (strings.explode (s)), rl);
end;


fun render_wiki_file (wiki_name: string, wiki_f: io.file): string
  var c: char := io.get (wiki_f);
  var res: list of char := [];
  var ul_open: bool := false;
  var pre_open: bool := false;
  var em_open: int := 0;
  var bold_open: int := 0;

  fun emit (c: char)
    res := c :: res;
  end;
  fun emit (s: string)
    res := lists.append (lists.reverse (strings.explode (s)), res);
  end;
  fun emit (l: list of char)
    res := lists.append (l, res);
  end;
  
  while true do
    // The following needs to be done at every newline which
    // starts a page.
    //
    if c = '\n' then
      if ul_open then
	emit ("</ul>\n");
	ul_open := false;
      end;
      if pre_open then
	emit ("</pre>\n");
	ul_open := false;
      end;
      emit ("<p>\n");
      c := io.get (wiki_f);
    end;

    // Beginning-of-line handling.
    //
    if c = '-' then			 /* ---- */
      c := io.get (wiki_f);
      if c = '-' then
	c := io.get (wiki_f);
	if c = '-' then
	  c := io.get (wiki_f);
	  if c = '-' then
	    c := io.get (wiki_f);
	    emit ("<hr>");
	  else
	    emit ("---");
	  end;
	else
	  emit ("--");
	end;
      else
	emit ("-");
      end;
    elsif c = '=' then			 /* == Title == */
      c := io.get (wiki_f);
      if c = '=' then
	c := io.get (wiki_f);
	emit ("<h2>");
	while c <> chars.EOF and c <> '=' and c <> '\n' do
	  emit (c);
	  c := io.get (wiki_f);
	end;
	if c = '=' then
	  c := io.get (wiki_f);
	  if c = '=' then
	    c := io.get (wiki_f);
	  end;
	end;
	emit ("</h2>");
      else
	emit ('=');
      end;
    elsif c = '*' then			 /* * List item.  */
      if not ul_open then
	emit ("<ul>");
	ul_open := true;
      end;
      emit ("\n<li>");
      c := io.get (wiki_f);
    elsif c = ' ' or c = '\t' then	 /* Preformatted text.  */
      if not pre_open then
	emit ("<pre>\n");
	pre_open := true;
      end;
      c := io.get (wiki_f);
    end;

    // Now we parse up to the end of the current line...
    //
    while c <> chars.EOF and c <> '\n' do
      var wiki_word: list of char := [];
      var small: int := 0;
      var caps: int := 0;

      // Parse everything up to an alphabetic letter.
      //
      while c <> chars.EOF and c <> '\n' and 
	strings.index ("ABCDEFGHIJKLMNOPQRSTUVWXYZ", c) < 0 and 
	strings.index ("abcdefghijklmnopqrstuvwxyz", c) < 0 do 

	// Check for emphasis markup.
	//
	if c = '\'' then
	  c := io.get (wiki_f);
	  if c = '\'' then
	    c := io.get (wiki_f);
	    if c = '\'' then
	      c := io.get (wiki_f);
	      if bold_open > 0 then
		bold_open := bold_open - 1;
		emit ("</strong>");
	      else
		bold_open := bold_open + 1;
		emit ("<strong>");
	      end;
	    else
	      if em_open > 0 then
		em_open := em_open - 1;
		emit ("</em>");
	      else
		em_open := em_open + 1;
		emit ("<em>");
	      end;
	    end;
	  else
	    emit ('\'');
	  end;
	elsif c = '<' then		 /* Escape HTML special characters. */
	  emit ("&lt;");
	  c := io.get (wiki_f);
	elsif c = '>' then
	  emit ("&gt;");
	  c := io.get (wiki_f);
	elsif c = '&' then
	  emit ("&amp;");
	  c := io.get (wiki_f);
	else
	  emit (c);
	  c := io.get (wiki_f);
	end;
      end;

      // We found an alphabetic letter.  Parse up to the next non-alphabetic
      // character and decide whether is is a WikiWord or an external link
      // or a normal word.
      //
      if c <> chars.EOF and c <> '\n' then
	while c <> chars.EOF and 
	  (strings.index ("ABCDEFGHIJKLMNOPQRSTUVWXYZ", c) >= 0 or
	   strings.index ("abcdefghijklmnopqrstuvwxyz", c) >= 0) do
	  if strings.index ("ABCDEFGHIJKLMNOPQRSTUVWXYZ", c) >= 0 then
	    caps := caps + 1;
	  else
	    small := small + 1;
	  end;
	  wiki_word := c :: wiki_word;
	  c := io.get (wiki_f);
	end;
	if caps > 1 and small > 1 then
	  if wiki_exists (strings.rimplode (wiki_word)) then
	    emit ("<a href=\"");
	    emit (config.wiki_prefix);
	    emit (wiki_word);
	    emit ("\">");
	    emit (wiki_word);
	    emit ("</a>");
	  else
	    emit (wiki_word);
	    emit ("<a href=\"");
	    emit (config.wiki_prefix);
	    emit (wiki_word);
	    emit ("/new\">[?]</a>");
	  end;
	elsif strings.eq (strings.rimplode (wiki_word), "http") and c = ':' then
	  while c <> chars.EOF and c <> '\n' and c <> '\t' and c <> ' ' 
	    and c <> ',' and c <> '\'' do
	    wiki_word := c :: wiki_word;
	    c := io.get (wiki_f);
	  end;
	  emit ("<a href=\"");
	  emit (wiki_word);
	  emit ("\">");
	  emit (wiki_word);
	  emit ("</a>");
	else
	  emit (wiki_word);
	end;
      end;
    end;
    if c = chars.EOF then
      if ul_open then
	emit ("</ul>\n");
      end;
      if pre_open then
	emit ("</pre>\n");
      end;

      while bold_open > 0 do
	emit ("</strong>");
	bold_open := bold_open - 1;
      end;
      while em_open > 0 do
	emit ("</em>");
	em_open := em_open - 1;
      end;
      emit ("<hr>[ <a href=\"");
      emit (config.wiki_prefix);
      emit ("\">index</a> | <a href=\"");
      emit (config.wiki_prefix);
      emit (wiki_name);
      emit ("/edit\">edit</a> | <a href=\"");
      emit (config.wiki_prefix);
      emit (wiki_name);
      emit ("/fanout\">fanout</A> | <a href=\"");
      emit (config.wiki_prefix);
      emit (wiki_name);
      emit ("/recent\">recent changes</A> | <a href=\"");
      emit (config.wiki_prefix);
      emit (wiki_name);
      emit ("/additions\">new pages</A> | <a href=\"");
      emit (config.wiki_prefix);
      emit (wiki_name);
      emit ("/all\">all</A> ]\n");
      return strings.rimplode (res);
    else
      emit ('\n');
      c := io.get (wiki_f);
    end;
  end;
  return "nix";
end;

//* Read the contents of the Wiki file @var{wiki_f} and return a
//* string containing the HTML for the page.
//
/*fun massage_wiki_file (wiki_name: string, wiki_f: io.file): string
  var ahref: list of char := 
    lists.reverse (strings.explode ("<a href=\"" + config.wiki_prefix));
  var aend: list of char := lists.reverse (strings.explode ("\">"));
  var aclose: list of char := lists.reverse (strings.explode ("</a>"));
  var aedit: list of char := lists.reverse (strings.explode ("/edit"));
  var anew: list of char := lists.reverse (strings.explode ("/new"));
  var result: list of char := [];
  var wiki: list of char := [];
  var c: char := io.get (wiki_f);
  var maybe_wiki: int := 0;
  var small: int := 0;
  var newlines: int := 0;
  var start_of_line: bool := true;
  var in_list: bool := false;
  var in_pre: bool := false;
  var in_strong: bool := false;
  while c <> chars.EOF do
    if strings.index ("ABCDEFGHIJKLMNOPQRSTUVWXYZ", c) > -1 then
      wiki := c :: wiki;
      maybe_wiki := maybe_wiki + 1;
      c := io.get (wiki_f);
      start_of_line := false;
    elsif strings.index ("abcdefghijklmnopqrstuvwxyz0123456789", c) > -1 then
      wiki := c :: wiki;
      small := small + 1;
      c := io.get (wiki_f);
      start_of_line := false;
    elsif c = '*' and start_of_line then
      if not in_list then
	result := lists.append (['\n', '>', 'l', 'u', '<'], result);
	in_list := true;
      end;
      result := lists.append (['>', 'i', 'l', '<'], result);
      c := io.get (wiki_f);
      start_of_line := false;
    elsif (c = ' ' or c = '\t') and start_of_line then
      if not in_pre then
	result := lists.append (['\n', '>', 'e', 'r', 'p', '<'], result);
	in_pre := true;
      end;
      c := io.get (wiki_f);
      start_of_line := false;
    else
      if maybe_wiki > 1 and small > 1 then
	if wiki_exists (strings.rimplode (wiki)) then
	  result := lists.append (aclose,
	  lists.append (wiki, lists.append (lists.append (aend, wiki), 
	  lists.append (ahref, result))));
	else
	  result := lists.append (aclose, lists.append ([']', '?', '['], lists.append (aend, lists.append (anew, lists.append (wiki, lists.append (ahref, lists.append (wiki, result)))))));
	end;
      elsif wiki <> null then
	result := lists.append (wiki, result);
      end;
      wiki := [];
      maybe_wiki := 0;
      small := 0;
      if c = '\n' then
	c := io.get (wiki_f);
	if c = '\n' then
	  while c = '\n' do
	    c := io.get (wiki_f);
	  end;
	  if in_list then
	    result := lists.append (['>', 'l', 'u', '/', '<', '\n'], result);
	    in_list := false;
	  end;
	  if in_pre then
	    result := lists.append (['>', 'e', 'r', 'p', '/', '<', '\n'],
	    result);
	    in_pre := false;
	  end;
	  result := lists.append (['\n', '>', 'p', '<', '\n'], result);
	else
	  result := '\n' :: result;
	end;
	start_of_line := true;
      elsif c = '\'' then
	start_of_line := false;
	c := io.get (wiki_f);
	if c = '\'' then
	  c := io.get (wiki_f);
	  if in_strong then
	    in_strong := false;
	    result := lists.append (lists.reverse (strings.explode ("</strong>")),
	    result);
	  else
	    in_strong := true;
	    result := lists.append (lists.reverse (strings.explode ("<strong>")),
	    result);
	  end;
	else
	  result := '\'' :: result;
	end;
      elsif c = '-' and start_of_line then
	start_of_line := false;
	c := io.get (wiki_f);
	if c = '-' then
	  c := io.get (wiki_f);
	  if c = '-' then
	    c := io.get (wiki_f);
	    if c = '-' then
	      c := io.get (wiki_f);
	      result := lists.append (['>', 'r', 'h', '<'], result);
	    else
	      result := c :: '-' :: '-' :: '-' :: result;
	    end;
	  else
	    result := c :: '-' :: '-' :: result;
	  end;
	else
	  result := c :: '-' :: result;
	end;
      elsif c = '<' then
	result := lists.append ([';', 't', 'l', '&'], result);
	c := io.get (wiki_f);
	start_of_line := false;
      elsif c = '>' then
	result := lists.append ([';', 't', 'g', '&'], result);
	c := io.get (wiki_f);
	start_of_line := false;
      else
	start_of_line := false;
	result := c :: result;
	c := io.get (wiki_f);
      end;
    end;
  end;
  if maybe_wiki > 1 and small > 1 then
    if wiki_exists (strings.rimplode (wiki)) then
      result := lists.append (aclose,
      lists.append (wiki, lists.append (lists.append (aend, wiki), 
      lists.append (ahref, result))));
    else
      result := lists.append (aclose, lists.append ([']', '?', '['], lists.append (aend, lists.append (anew, lists.append (wiki, lists.append (ahref, lists.append (wiki, result)))))));
    end;
  elsif wiki <> null then
    result := lists.append (wiki, result);
  end;
  if in_list then
    result := lists.append (['>', 'l', 'u', '/', '<', '\n'], result);
    in_list := false;
  end;
  if in_pre then
    result := lists.append (['>', 'e', 'r', 'p', '/', '<', '\n'],
    result);
    in_pre := false;
  end;
  result := append_string ("<hr>[ <a href=\"" + config.wiki_prefix +
    "\">index</a> | <a href=\"" + config.wiki_prefix + 
  wiki_name + "/edit\">edit</a> | <a href=\"" + config.wiki_prefix + wiki_name + "/recent\">recent changes</A> | <a href=\"" + config.wiki_prefix + wiki_name + "/additions\">new pages</A> ]\n", result);
  return strings.rimplode (result);
end;
*/

//* Render the given Wiki page to HTML output.  Also include the
//* links for navigation and the edit page to the bottom of the
//* page.
//
fun render_wiki (req: http.request, wiki_name: string, page_name: string):
                 http.response

  var wiki_f: io.file := io.open (page_name);
  var stat: sys.files.stat := sys.files.stat (page_name);

  if wiki_f = null then
    return http.not_found_response (req);
  end;
  if stat = null then
    io.close (wiki_f);
    return http.not_found_response (req);
  end;

  var body: list of string := ["<HTML>\n<HEAD>\n<TITLE>",
    wiki_name,
    "</TITLE>\n</HEAD>\n<BODY bgcolor=\"white\" text=\"black\">\n",
    "<img src=\"",
    config.wiki_logo_prefix,
    "turtle-logo-small.png\"><H1>",
    wiki_name,
    "</H1>\n"];

//  var html: string := massage_wiki_file (wiki_name, wiki_f);
  var html: string := render_wiki_file (wiki_name, wiki_f);

  body := lists.append (body, html :: ["<hr>Modified: ", 

  sys.times.asctime (sys.times.gmtime (sys.files.mtime (stat))),
  "<br>Rendered: ",
  sys.times.asctime (sys.times.gmtime (sys.times.time ())),
  "\n</BODY>\n</HTML>\n"]);
  io.close (wiki_f);
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;


fun lock_file_name (page_name: string): string
  return strings.append (page_name, ".lock");
end;

fun locked (page_name: string): bool
  var lf_name: string := lock_file_name (page_name);
  var stat: sys.files.stat := sys.files.stat (lf_name);
  return stat <> null;
end;

fun page_footer_no_edit (wiki_name: string): list of string
  return  ["\n<HR>[ <A href=\"", config.wiki_prefix,
    "\">index</A> | <A href=\"", config.wiki_prefix, wiki_name, "/recent\">recent changes</A> | <A href=\"", config.wiki_prefix, wiki_name, "/additions\">new pages</A> | <A href=\"", config.wiki_prefix, wiki_name, "/all\">all</A> ]<hr>Rendered: ",
  sys.times.asctime (sys.times.gmtime (sys.times.time ())), 
  "</BODY></HTML>\n"];
end;

public fun wiki_page_link (wiki_name: string): string
  return strings.append ("<a href=\"", config.wiki_prefix,
  strings.append (wiki_name, "\">",
  strings.append (wiki_name, "</a>")));
end;

public fun locked_response (req: http.request, wiki_name: string): http.response
  var body: list of string;
  var stat: sys.files.stat := sys.files.stat (lock_file_name (config.wiki_root + wiki_name));

  if stat <> null then
    var curtime: long :=sys.times.time ();

    body := "<html><head><title>Page Locked</title></head>\n<body bgcolor=\"white\" text=\"black\"><img src=\"" ::
    config.wiki_logo_prefix ::
    "turtle-logo-small.png\">" ::
    "<h1>Page Locked</h1>\nSorry, but the page is currently locked.  That means that someone is already editing the page.<p>The page has been locked at: " :: 
    sys.times.asctime (sys.times.gmtime (sys.files.mtime (stat))) :: 
    " GMT (>=" ::
    strings.to_string ((curtime - sys.files.mtime (stat)) / 3600L) ::
    " hours ago)\n<p>Your options:<ul><li>Back to " :: 
    wiki_page_link (wiki_name) :: 
    "</li><li>If the lock seems old, you can <a href=\"" :: 
    config.wiki_prefix :: 
    wiki_name :: 
    "/steal\">remove the lock</a>, examine the latest state and try again to edit the page.</li></ul>" :: 
    page_footer_no_edit (wiki_name);
  else
    body := "<html><head><title>Page Locked</title></head>\n<body bgcolor=\"white\" text=\"black\"><img src=\"" ::
    config.wiki_logo_prefix ::
    "turtle-logo-small.png\"><h1>Page Locked</h1>\nSorry, but the page is currently locked.  That means that someone is already editing the page.<p>Your options:<ul><li>Back to " :: 
    wiki_page_link (wiki_name) :: 
    "</li><li>You can <a href=\"" :: 
    config.wiki_prefix :: 
    wiki_name :: 
    "/steal\">remove the lock</a>, examine the latest state and try again to edit the page.</li></ul>" :: 
    page_footer_no_edit (wiki_name);
  end;
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;

fun try_lock (page_name: string): bool
  var f: io.file := io.create (lock_file_name (page_name));
  if f = null then
    return false;
  end;
  io.putln (f, sys.procs.getpid ());
  io.close (f);
  return true;
end;

fun unlock (page_name: string)
  var err: int := sys.files.unlink (lock_file_name (page_name));
end;

//* Create the edit page for the given Wiki page.
//
fun edit_wiki (req: http.request, wiki_name: string, page_name: string,
               new: bool): http.response
  var save: string;
  var change_text: string;
  if new then
    save := "create";
    change_text := "Enter the text of the new wiki page into the input field below and press <strong>Save Page</strong> to save it.<p>If you decide <strong>not</strong> to save the new page, press <strong>Discard</strong> instead.";
  else
    save := "save";
    change_text := "Edit the text of the wiki page in the input field below and press <strong>Save Page</strong> to save your changes.<p>If you decide <strong>not</strong> to save your modifications, press <strong>Discard</strong> instead.";
  end;
  if wiki_exists ("PageEditing") then
    change_text := change_text + "<br>See <a href=\"" + config.wiki_prefix + 
    "PageEditing\">PageEditing</a> for instructions on editing pages.";
  end;
  if wiki_exists ("WikiMarkup") then
    change_text := change_text + "<br>See <a href=\"" + config.wiki_prefix + 
    "WikiMarkup\">WikiMarkup</a> for information on text formatting.";
  end;
  if locked (page_name) or not try_lock (page_name) then
    return locked_response (req, wiki_name);
  end;
  var wiki_f: io.file := io.open (page_name);
  var body: list of string := ["<HTML>\n<HEAD>\n<TITLE>Edit ",
    wiki_name,
    "</TITLE>\n</HEAD>\n<BODY bgcolor=\"white\" text=\"black\">\n<H1>Edit ",
    wiki_name,
    "</H1>\n",
    change_text,
    "<form action=\"",
    config.wiki_prefix,
    wiki_name,
    "/",
    save,
    "\" method=\"POST\">\n<textarea name=\"text\" cols=\"72\" rows=\"20\">\n"];
  var res: list of string := [];
  if wiki_f <> null then
    var s: string := io.get (wiki_f);
    while s <> null do
      res := "\n" :: s :: res;
      s := io.get (wiki_f);
    end;
    io.close (wiki_f);
  end;
  body := lists.append (body, lists.append (lists.reverse (res),
  ["</textarea>\n<p><input type=\"SUBMIT\" value=\"Save Page\"> <input type=\"SUBMIT\" value=\"Discard\" name=\"discard\"></form></BODY>\n</HTML>\n"]));
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;

public fun too_many_response (req: http.request, count: int, limit: int,
  wiki_name: string): http.response
  var body: list of string;

  body := "<html><head><title>Too many pages</title></head>\n<body bgcolor=\"white\" text=\"black\"><img src=\"" ::
    config.wiki_logo_prefix ::
    "turtle-logo-small.png\"><h1>Too many pages</h1>\nSorry, but the page count limit on this server is exceeded.<p>Page count: " :: ints.to_string (count) ::"<br>Limit: " :: ints.to_string (limit) :: page_footer_no_edit (wiki_name);
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;


public fun too_large_response (req: http.request, size: int, limit: int,
  wiki_name: string): http.response
  var body: list of string;

  body := "<html><head><title>Page too large</title></head>\n<body bgcolor=\"white\" text=\"black\"><img src=\"" ::
    config.wiki_logo_prefix ::
    "turtle-logo-small.png\"><h1>Page too large</h1>\nSorry, but the page length exceeds the page size limit.<p>Page size: " :: ints.to_string (size) :: "<br>Limit: " :: ints.to_string (limit) :: page_footer_no_edit (wiki_name);
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;


//* Store the new text for the Wiki page.  Create a backup of the
//* old text before overwriting it.  When it is a new page, add it
//* to the file $WIKI_ROOT/new, when it is a changed page, remove
//* it from $WIKI_ROOT/new (if contained) and add it to
//* $WIKI_ROOT/changed.  These two files are used for rendering the
//* list of new and changed pages.
//
fun save_wiki (req: http.request, wiki_name: string, page_name: string,
               new: bool): http.response
  var discard: string := http.header_value ("discard", http.body (req));
  if discard <> null then
    unlock (page_name);
    return render_wiki (req, wiki_name, page_name);
  end;

  var backup: string := page_name + "~";
  var v: string := http.header_value ("TEXT", http.body (req));

  if v <> null then
    if sizeof v > config.wiki_page_size_limit then
      unlock (page_name);
      return too_large_response (req, sizeof v, config.wiki_page_size_limit,
                                 wiki_name);
    end;
  else
    unlock (page_name);
    return http.bad_request_response (req);
  end;

  var input: io.file := io.open (page_name);
  var output: io.file;

  if input <> null then
    output := io.create (backup);
    var c: char := io.get (input);
    while c <> chars.EOF do
      io.put (output, c);
      c := io.get (input);
    end;
    io.close (input);
    io.close (output);
  else
    var cnt: int := wiki_page_count ();
    if cnt >= config.wiki_page_count_limit then
      unlock (page_name);
      return too_many_response (req, cnt, config.wiki_page_count_limit,
                                wiki_name);
    end;
  end;

  output := io.create (page_name);
  var i: int := 0;
  while i < sizeof v do
    if v[i] <> '\r' then
      io.put (output, v[i]);
    end;
    i := i + 1;
  end;
  io.close (output);

  var change_file: string;
  if new then
    change_file := "new";
  else
    change_file := "recent";
  end;
  var recent: list of string := [];
  output := io.open (config.wiki_root + change_file);
  if output <> null then
    var s: string := io.get (output);
    while s <> null do
      if not strings.eq (s, wiki_name) and wiki_exists (s) then
	recent := s :: recent;
      end;
      s := io.get (output);
    end;
    io.close (output);
  end;
  output := io.create (config.wiki_root + change_file);
  recent := wiki_name :: lists.reverse (recent);
  while recent <> null do
    io.put (output, hd recent + "\n");
    recent := tl recent;
  end;
  io.close (output);
  if not new then
    var must_write: bool := false;
    recent := [];
    output := io.open (config.wiki_root + "new");
    if output <> null then
      s := io.get (output);
      while s <> null do
	if not strings.eq (s, wiki_name) and wiki_exists (s) then
	  recent := s :: recent;
	else
	  must_write := true;
	end;
	s := io.get (output);
      end;
      io.close (output);
      if must_write then
	output := io.create (config.wiki_root + "new");
	lists.foreach (fun (s: string) io.put (output, s + "\n"); end,
	               lists.reverse (recent));
	io.close (output);
      end;
    end;
  end;
  unlock (page_name);
  return render_wiki (req, wiki_name, page_name);
end;


//* Remove a possibly existing lock file for the page @var{wiki_name} and
//* redisplay the latest version of the page.
//
fun steal_lock (req: http.request, wiki_name: string, 
                page_name: string): http.response
  unlock (page_name);
  return render_wiki (req, wiki_name, page_name);
end;


//* Check whether the WikiWord @var{wiki_name} is contained in the page
//* called @var{fn}.  Return @code{true} if it is, @code{false} otherwise.
// 
fun file_contains (fn: string, wiki_name: string): bool
  var f: io.file := io.open (config.wiki_root + fn);
  if f = null then
    return false;
  else
    var c: char := io.get (f);
    var small: int;
    var large: int;
    var word: list of char;
    while c <> chars.EOF do
      while c <> chars.EOF and (c < 'A' or c > 'Z') do
	c := io.get (f);
      end;
      small := 0;
      large := 0;
      word := [];
      if c <> chars.EOF then
	while (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') do
	  word := c :: word;
	  if c >= 'a' and c <= 'z' then
	    small := small + 1;
	  else
	    large := large + 1;
	  end;
	  c := io.get (f);
	end;
	if small > 1 and large > 1 then
	  if strings.eq (strings.rimplode (word), wiki_name) then
	    io.close (f);
	    return true;
	  end;
	end;
      end;
    end;
    io.close (f);
    return false;
  end;
end;


//* Create a list of all pages which reference the page called
//* @var{wiki_name}.  The page @var{wiki_name} itself is excluded from the
//* list of pages.
//
fun fanout (req: http.request, wiki_name: string, 
                page_name: string): http.response
  var body: list of string := [];
  var all_files: list of string := [];
  var d: sys.dirs.dir := sys.dirs.opendir (config.wiki_root);
  var stat: sys.files.stat := sys.files.stat (config.wiki_root);
  var mtime: long;
  if d = null or stat = null then
    return http.not_found_response (req);
  end;
  mtime := sys.files.mtime (stat);
  var s: string := sys.dirs.readdir (d);
  while s <> null do
    if not strings.eq (s, wiki_name) and not invalid_wiki_name (s)then
      if file_contains (s, wiki_name) then
	all_files := s :: all_files;
      end;
    end;
    s := sys.dirs.readdir (d);
  end;
  var i: int := sys.dirs.closedir (d);

  body := listmap.map (fun (s: string): string
                         return "<li><a href=\"" + config.wiki_prefix + s +
                           "\">" + s + "</a></li>\n";
                       end,
		       listsort.sort (all_files, compare.cmp));
  body := "<HTML><HEAD><TITLE>Fanout of " ::
  wiki_name :: 
  "</TITLE></HEAD>\n<BODY bgcolor=\"white\" text=\"black\"><img src=\"" ::
    config.wiki_logo_prefix ::
    "turtle-logo-small.png\"><H1>Fanout of " ::
  wiki_name :: 
  "</H1>\nThe page " ::
  wiki_name ::
  " is referenced by the following pages:\n<UL>\n" ::
  lists.append (body, ["</UL>\n<HR>[ <A href=\"", config.wiki_prefix,
    "\">index</A> | <A href=\"", config.wiki_prefix, wiki_name, "/recent\">recent changes</A> | <A href=\"", config.wiki_prefix, wiki_name, "/additions\">new pages</A>  | <A href=\"", config.wiki_prefix, wiki_name, "/all\">all</A> ]<hr>Modified: ", 

  sys.times.asctime (sys.times.gmtime (mtime)), "<br>Rendered: ",
  sys.times.asctime (sys.times.gmtime (sys.times.time ())), "</BODY></HTML>\n"]);
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;

//* Render the list of recently changed Wiki pages from the file
//* @code{$WIKI_ROOT/new}.
//
fun recent_changes (req: http.request, wiki_name: string, page_name: string):
                    http.response
  var recent: list of string := [];
  var body: list of string := [];
  var mtime: long := 0L;

  var input: io.file := io.open (config.wiki_root + "recent");
  var stat: sys.files.stat := sys.files.stat (config.wiki_root + "recent");

  if stat <> null then
    mtime := sys.files.mtime (stat);
  end;
  if input <> null then
    recent := io.get (input);
    io.close (input);
  end;
  body := listmap.map (fun (s: string): string
                         return "<li><a href=\"" + config.wiki_prefix + s +
                           "\">" + s + "</a></li>\n";
                       end,
		       recent);
  body := "<HTML><HEAD><TITLE>Recent Changes</TITLE></HEAD>\n<BODY bgcolor=\"white\" text=\"black\"><img src=\"" ::
    config.wiki_logo_prefix ::
    "turtle-logo-small.png\"><H1>Recent Changes</H1>\nThe following pages have recently been edited:\n<UL>\n" ::
  lists.append (body, ["</UL>\n<HR>[ <A href=\"", config.wiki_prefix,
    "\">index</A> | <A href=\"", config.wiki_prefix, wiki_name, "/recent\">recent changes</A> | <A href=\"", config.wiki_prefix, wiki_name, "/additions\">new pages</A> | <A href=\"", config.wiki_prefix, wiki_name, "/all\">all</A> ]<hr>Modified: ", 

  sys.times.asctime (sys.times.gmtime (mtime)), "<br>Rendered: ",
  sys.times.asctime (sys.times.gmtime (sys.times.time ())), "</BODY></HTML>\n"]);
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;


//* Render the list of new Wiki pages from the file
//* @code{$WIKI_ROOT/new}.
//
fun additions (req: http.request, wiki_name: string, page_name: string):
                    http.response
  var new_files: list of string := [];
  var body: list of string := [];
  var mtime: long := 0L;

  var input: io.file := io.open (config.wiki_root + "new");
  var stat: sys.files.stat := sys.files.stat (config.wiki_root + "new");

  if stat <> null then
    mtime := sys.files.mtime (stat);
  end;
  if input <> null then
    new_files := io.get (input);
    io.close (input);
  end;
  body := listmap.map (fun (s: string): string
                         return "<li><a href=\"" + config.wiki_prefix + s +
                           "\">" + s + "</a></li>\n";
                       end,
		       new_files);
  body := "<HTML><HEAD><TITLE>New Pages</TITLE></HEAD>\n<BODY bgcolor=\"white\" text=\"black\"><img src=\"" ::
    config.wiki_logo_prefix ::
    "turtle-logo-small.png\"><H1>New Pages</H1>\nThe following pages have recently been added:\n<UL>\n" ::
  lists.append (body, ["</UL>\n<HR>[ <A href=\"", config.wiki_prefix,
    "\">index</A> | <A href=\"", config.wiki_prefix, wiki_name, "/recent\">recent changes</A> | <A href=\"", config.wiki_prefix, wiki_name, "/additions\">new pages</A>  | <A href=\"", config.wiki_prefix, wiki_name, "/all\">all</A> ]<hr>Modified: ", 

  sys.times.asctime (sys.times.gmtime (mtime)), "<br>Rendered: ",
  sys.times.asctime (sys.times.gmtime (sys.times.time ())), "</BODY></HTML>\n"]);
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;

fun all_pages (req: http.request, wiki_name: string, page_name: string):
               http.response
  var body: list of string := [];
  var all_files: list of string := [];
  var d: sys.dirs.dir := sys.dirs.opendir (config.wiki_root);
  var stat: sys.files.stat := sys.files.stat (config.wiki_root);
  var mtime: long;
  if d = null or stat = null then
    return http.not_found_response (req);
  end;
  mtime := sys.files.mtime (stat);
  var s: string := sys.dirs.readdir (d);
  while s <> null do
    if not invalid_wiki_name (s) then
      all_files := s :: all_files;
    end;
    s := sys.dirs.readdir (d);
  end;
  var i: int := sys.dirs.closedir (d);
  body := listmap.map (fun (s: string): string
                         return "<li><a href=\"" + config.wiki_prefix + s +
                           "\">" + s + "</a></li>\n";
                       end,
		       listsort.sort (all_files, compare.cmp));
  body := "<HTML><HEAD><TITLE>All Pages</TITLE></HEAD>\n<BODY bgcolor=\"white\" text=\"black\"><img src=\"" ::
    config.wiki_logo_prefix ::
    "turtle-logo-small.png\"><H1>All Pages</H1>\nThe following pages exist:\n<UL>\n" ::
  lists.append (body, ["</UL>\n<HR>[ <A href=\"", config.wiki_prefix,
    "\">index</A> | <A href=\"", config.wiki_prefix, wiki_name, "/recent\">recent changes</A> | <A href=\"", config.wiki_prefix, wiki_name, "/additions\">new pages</A>  | <A href=\"", config.wiki_prefix, wiki_name, "/all\">all</A> ]<hr>Modified: ", 

  sys.times.asctime (sys.times.gmtime (mtime)), "<br>Rendered: ",
  sys.times.asctime (sys.times.gmtime (sys.times.time ())), "</BODY></HTML>\n"]);
  return http.ok (http.version (req), http.std_html_headers (),
                  body, -1L, req, http.file (req), -1, null);
end;


//* Return true if @var{s} is a valid Wiki name according to the
//* Turtle Wiki rules, that is, it must be an alphabetic string
//* with more than one upper case and more than one lower case
//* letter.
//
fun invalid_wiki_name (s: string): bool
  var i: int := 0;
  if sizeof s = 0 then
    return true;
  else
    var lower: int := 0;
    var upper: int := 0;
    while i < sizeof s do
      if not chars.letter? (s[i]) then
	return true;
      end;
      if chars.uppercase? (s[i]) then
	upper := upper + 1;
      elsif chars.lowercase? (s[i]) then
	lower := lower + 1;
      end;
      i := i + 1;
    end;
    if upper < 2 or lower < 2 then
      return true;
    else
      return false;
    end;
  end;
end;


//* Treat @var{req} as a Wiki request.  Extract the name of the
//* Wiki page and the optional action from the ressource.  Then
//* construct the filename corresponding to the Wiki page and
//* dispatch to the function responsible for the given action.
//
public fun handle_wiki_request (req: http.request): http.response
  var resp: http.response;
  var wiki_name: string := strings.substring 
    (http.ressource (req), 5, strings.length (http.ressource (req)));

  if sizeof wiki_name > 0 then
    wiki_name := strings.substring (wiki_name, 1, sizeof wiki_name);
  end;
  if sizeof wiki_name = 0 then
    wiki_name := "IndexPage";
  end;
  var action: string := "show";
  var idx: int := strings.index (wiki_name, '/');
  if idx >= 0 then
    action := strings.substring (wiki_name, idx + 1, sizeof wiki_name);
    wiki_name := strings.substring (wiki_name, 0, idx);
  end;

  if invalid_wiki_name (wiki_name) then
    return http.bad_request_response (req);
  end;

//  io.put ("Action: " + action + "\n");
  var page_name: string := strings.append (config.wiki_root, wiki_name);

  if strings.eq (action, "edit") then
    resp := edit_wiki (req, wiki_name, page_name, false);
    return resp;
  elsif strings.eq (action, "new") then
    resp := edit_wiki (req, wiki_name, page_name, true);
    return resp;
  elsif strings.eq (action, "save") then
    resp := save_wiki (req, wiki_name, page_name, false);
    return resp;
  elsif strings.eq (action, "create") then
    resp := save_wiki (req, wiki_name, page_name, true);
    return resp;
  elsif strings.eq (action, "steal") then
    resp := steal_lock (req, wiki_name, page_name);
    return resp;
  elsif strings.eq (action, "fanout") then
    resp := fanout (req, wiki_name, page_name);
    return resp;
  elsif strings.eq (action, "recent") then
    resp := recent_changes (req, wiki_name, page_name);
    return resp;
  elsif strings.eq (action, "additions") then
    resp := additions (req, wiki_name, page_name);
    return resp;
  elsif strings.eq (action, "all") then
    resp := all_pages (req, wiki_name, page_name);
    return resp;
  elsif strings.eq (action, "show") then
    resp := render_wiki (req, wiki_name, page_name);
    return resp;
  else
    return http.bad_request_response (req);
  end;
end;

// End of wiki.t.
