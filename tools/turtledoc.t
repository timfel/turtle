// turtledoc.t -- Turtle documentation utility..
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
//* This is a parser for the Turtle language.

module turtledoc;

import io, parser, lists<string>, scanner, ast, lists<ast.ast>;


//* Entry point of the turtledoc program.
//
public fun main (args: list of string): int
  var f: io.file;
  if lists.length (args) >= 2 then
    args := tl args;
    while args <> null do
      f := io.open (hd args);
      if f = null then
	io.put (io.error, "turtledoc: cannot open input file: ");
	io.put (io.error, hd args);
	io.nl (io.error);
      else
//	io.put ("Parsing ");
//	io.put (hd args);
//	io.nl ();
	var s: scanner.state;
	s := scanner.state (hd args, f);
	var mod: ast.ast := parser.parse (s);
	io.close (f);
//	ast.put (io.output, mod, 0);
	var outf: io.file := io.create (hd args + ".html");
	if outf <> null then
	  gendoc (outf, mod);
	  io.close (outf);
	else
	  io.put (io.error, "turtledoc: cannot open output file: ");
	  io.put (io.error, hd args + ".html");
	  io.nl (io.error);
	end;
      end;
      args := tl args;
    end;
  else
    io.put (io.error, "turtledoc: No input files\n");
  end;
  return 0;
end;

fun gendoc (f: io.file, mod: ast.ast)
  io.put (f, "<html><head><title>");
  ast.put (f, ast.name (mod), 0);
  io.put (f, ": module documentation</title></head>\n<body bgcolor=\"white\" text=\"black\">\n<h1>");
  ast.put (f, ast.name (mod), 0);
  io.put (f, ": module documentation</h1>\n");

  io.put (f, "<p>\n");
  io.put (f, ast.comment (mod));
  io.put (f, "</p>\n");
  var d: list of ast.ast := ast.defs (mod);
  while d <> null do
    gendef (f, hd d);
    d := tl d;
  end;
  io.put (f, "</body></html>\n");
end;

fun gendef (f: io.file, d: ast.ast)
  var l: list of ast.ast;
  if ast.pub (d) then
    if ast.fundef? (d) then
      io.put (f, "<dl>\n<dt>Function: <b>");
      ast.put (f, ast.name (d), 0);
      io.put (f, "</b>");
      io.put (f, " (");
      l := ast.params (d);
      if l <> null then
	io.put (f, "<var>");
	ast.put (f, ast.name (hd l), 0);
	io.put (f, "</var>");
	io.put (f, ": ");
	ast.put (f, ast.typ (hd l), 0);
	l := tl l;
	while l <> null do
	  io.put (f, ", <var>");
	  ast.put (f, ast.name (hd l), 0);
	  io.put (f, "</var>");
	  io.put (f, ": ");
	  ast.put (f, ast.typ (hd l), 0);
	  l := tl l;
	end;
      end;
      io.put (f, ")");
      if ast.ret_type (d) <> null and not ast.void_type? (ast.ret_type (d)) 
	then
	io.put (f, ": ");
	ast.put (f, ast.ret_type (d), 0);
      end;
      io.put (f, "\n<dd>\n");
      io.put (f, ast.comment (d));
      io.put (f, "</dd></dl>\n");
    elsif ast.constraintdef? (d) then
      io.put (f, "<dl>\n<dt>Constraint: <b>");
      ast.put (f, ast.name (d), 0);
      io.put (f, "</b>");
      io.put (f, " (");
      l := ast.params (d);
      if l <> null then
	io.put (f, "<var>");
	ast.put (f, ast.name (hd l), 0);
	io.put (f, "</var>");
	io.put (f, ": ");
	ast.put (f, ast.typ (hd l), 0);
	l := tl l;
	while l <> null do
	  io.put (f, ", <var>");
	  ast.put (f, ast.name (hd l), 0);
	  io.put (f, "</var>");
	  io.put (f, ": ");
	  ast.put (f, ast.typ (hd l), 0);
	  l := tl l;
	end;
      end;
      io.put (f, ")");
      io.put (f, "\n<dd>\n");
      io.put (f, ast.comment (d));
      io.put (f, "</dd></dl>\n");
    elsif ast.typedef? (d) then
      io.put (f, "<dl>\n<dt>Type: <b>");
      ast.put (f, ast.name (d), 0);
      io.put (f, "</b>");
      io.put (f, " = ");
      ast.put (f, ast.typ (d), 0);
      io.put (f, "\n<dd>\n");
      io.put (f, ast.comment (d));
      io.put (f, "</dd></dl>\n");
    elsif ast.datatypedef? (d) then
      io.put (f, "<dl>\n<dt>Datatype: <b>");
      ast.put (f, ast.name (d), 0);
      io.put (f, "</b>");
      io.put (f, "\n<dd>\n");
      io.put (f, "Defined as:\n<pre>\n");
      ast.put (f, ast.name (d), 0);
      io.put (f, " = ");
      l := ast.variants (d);
      ast.put (f, hd l, 0);
      l := tl l;
      while l <> null do
	io.put (f, " or\n    ");
	ast.put (f, hd l, 0);
	l := tl l;
      end;
      io.put (f, "\n</pre>\n");
      io.put (f, ast.comment (d));
      io.put (f, "</dd></dl>\n");
    end;
  end;
end;

// End of turtledoc.t.
