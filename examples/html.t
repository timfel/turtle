// html.t -- HTML definitions for the Turtle Webserver.
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

module html;

import io, lists<string>;

public datatype tag = t (name: string, attr: list of (string, string),
                         content: list of tag) or
                      s (content: string);

public fun t (name: string, content: list of tag): tag
  return t (name, [], content);
end;

public fun t (name: string, content: string): tag
  return t (name, [], [html.s (content)]);
end;

public fun t (name: string, attr: list of (string, string), content: string): tag
  return t (name, attr, [html.s (content)]);
end;

fun flatten_list (f: io.file, ts: list of tag)
  while ts <> null do
    flatten (f, hd ts);
    ts := tl ts;
  end;
end;

fun flatten_attrs (f: io.file, attr: list of (string, string))
  io.put (f, ' ');
  while attr <> null do
    var name: string, val: string;
    name, val := hd attr;
    io.put (f, name);
    io.put (f, "=\"");
    io.put (f, val);
    io.put (f, '\"');
    attr := tl attr;
  end;
end;

public fun flatten (f: io.file, t: tag)
  if s? (t) then
    var s: string := content (t);
    io.put (f, s);
  elsif t? (t) then
    io.put (f, '<');
    io.put (f, name (t));
    if attr (t) <> null then
      flatten_attrs (f, attr (t));
    end;
    io.put (f, '>');
    flatten_list (f, content (t));
    io.put (f, '<');
    io.put (f, '/');
    io.put (f, name (t));
    io.put (f, '>');
    io.put (f, '\n');
  end;
end;


fun flatten_list (ts: list of tag): list of string
  var l: list of string := [];
  while ts <> null do
    l := lists.append (l, flatten (hd ts));
    ts := tl ts;
  end;
  return l;
end;

fun flatten_attrs (attr: list of (string, string)): list of string
  var l: list of string := [];
  while attr <> null do
    var name: string, val: string;
    name, val := hd attr;
    l := lists.append (l, [" ", name, "=\"", val, "\""]);
    attr := tl attr;
  end;
  return l;
end;

public fun flatten (t: tag): list of string
  if s? (t) then
    var s: string := content (t);
    return [s];
  elsif t? (t) then
    var l: list of string := [];
    l := lists.append (["<", name (t)], flatten_attrs (attr (t)));
    l := lists.append (l, [">"]);
    l := lists.append (l, flatten_list (content (t)));
    l := lists.append (l, ["</", name (t), ">\n"]);
    return l;
  else
    return [];
  end;
end;


public fun page (title: string, body: list of tag): tag
  return html.t ("html",
                 [html.t ("head", [html.t ("title", title)]),
                  html.t ("body", [("bgcolor", "white"), ("text", "black")],
                          html.t ("h1", title) :: body)]);
end;

public fun ul (items: list of tag): tag
  var l: list of tag := [];
  while items <> null do
    l := html.t ("li", [hd items]) :: l;
    items := tl items;
  end;
  while l <> null do
    items := hd l :: items;
    l := tl l;
  end;
  return html.t ("ul", items);
end;
                 
// End of html.t.
