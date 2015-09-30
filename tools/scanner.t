// scanner.t -- Turtle tokenizer.
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
//* This is a tokenizer for the Turtle language.

module scanner;

import core, io, chars, strings, lists<char>, compare, reals, longs;

//* This data type holds the state of the scanner, including line
//* number information and documentation comments.
//
public datatype state = state (name: string, f: io.file, line: int, col: int,
                        last_line: int, last_col: int, start_line: int,
			start_col: int, comment: list of char);

//* Shortcut function for the constructor for the state data type.
//* Initialize the state with the file f called name.
//
public fun state (name: string, f: io.file): state
  return state (name, f, 0, 0, 0, 0, 0, 0, []);
end;

public fun get_comment (s: state): string
  var s: string := strings.rimplode (comment (s));
  comment! (s, []);
  return s;
end;

fun append_comment (s: state, c: char)
  comment! (s, c :: comment (s));
end;

public datatype token = eof (name: string) or
                        identifier (name: string) or
			str_const (name: string, value: string) or
			int_const (name: string, value: int) or
			long_const (name: string, value: long) or
			real_const (name: string, value: real) or
			char_const (name: string, value: char) or
			tmodule (name: string) or
			timport (name: string) or
			tdatatype (name: string) or
			tfun (name: string) or 
			tand (name: string) or 
			tarray (name: string) or 
			tconstraint (name: string) or
			tdo (name: string) or
			telse (name: string) or 
			telsif (name: string) or 
			tend (name: string) or
			texport (name: string) or 
			tfalse (name: string) or
			tforeign (name: string) or
			thd (name: string) or 
			tif (name: string) or
			tin (name: string) or
			tlist (name: string) or 
			tnot (name: string) or
			tnull (name: string) or 
			tof (name: string) or
			tor (name: string) or 
			tout (name: string) or
			tpublic (name: string) or
			trequire (name: string) or 
			treturn (name: string) or 
			tsizeof (name: string) or 
			tstring (name: string) or
			tthen (name: string) or 
			ttl (name: string) or
			ttrue (name: string) or 
			ttype (name: string) or 
			tvar (name: string) or
			tconst (name: string) or
			twhile (name: string) or
			lparen (name: string) or
			rparen (name: string) or
			lbrace (name: string) or
			rbrace (name: string) or
			lbracket (name: string) or
			rbracket (name: string) or
			semicolon (name: string) or
			period (name: string) or
			comma (name: string) or
			plus (name: string) or
			minus (name: string) or
			star (name: string) or
			slash (name: string) or
			percent (name: string) or
			colon (name: string) or
			cons (name: string) or
			assign (name: string) or
			lt (name: string) or
			gt (name: string) or
			le (name: string) or 
			ge (name: string) or 
			eq (name: string) or 
			ne (name: string) or
			backquote (name: string) or
			exclamation (name: string);

fun get (s: state): char
  var c: char := io.get (f (s));
  last_line! (s, line (s));
  last_col! (s, col (s));
  if c = '\n' then
    line! (s, line (s) + 1);
    col! (s, 0);
  elsif c = '\t' then
    col! (s, col (s) + (8 - (col (s) % 8)));
  else
    col! (s, col (s) + 1);
  end;
  return c;
end;

fun unget (s: state, c: char)
  io.unget (f (s), c);
  line! (s, last_line (s));
  col! (s, last_col (s));
end;

public fun error (s: state, msg: string)
  io.put (io.error, name (s));
  io.put (io.error, ":");
  io.put (io.error, line (s) + 1);
  io.put (io.error, ":");
  io.put (io.error, col (s) + 1);
  io.put (io.error, ": ");
  io.put (io.error, msg);
  io.nl (io.error);
end;

fun skip_whitespace (s: state, c: char): char
  while (c = ' ') or (c = '\t') or (c = '\n') or (c = '\r') do
    c := get (s);
  end;
  return c;
end;

var reserved_words: array of (string, fun(string): token) :=
{
  ("and", tand),
  ("array", tarray),
  ("const", tconst),
  ("constraint", tconstraint),
  ("datatype", tdatatype),
  ("do", tdo),
  ("else", telse),
  ("elsif", telsif),
  ("end", tend),
  ("export", texport),
  ("false", tfalse),
  ("foreign", tforeign),
  ("fun", tfun),
  ("hd", thd),
  ("if", tif),
  ("import", timport),
  ("in", tin),
  ("list", tlist),
  ("module", tmodule),
  ("not", tnot),
  ("null", tnull),
  ("of", tof),
  ("or", tor),
  ("out", tout),
  ("public", tpublic),
  ("require", trequire),
  ("return", treturn),
  ("sizeof", tsizeof),
  ("string", tstring),
  ("then", tthen),
  ("tl", ttl),
  ("true", ttrue),
  ("type", ttype),
  ("var", tvar),
  ("while", twhile)
};

fun bsearch(s: string): int
  var l: int, r: int, m: int;
  var tmp: string, dummy: fun(string): token;

  l := 0;
  r := sizeof reserved_words;
  while l < r do
    var c: int;
    m := (l + r) / 2;
    tmp, dummy := reserved_words[m];
    c := compare.cmp (s, tmp);
    if c < 0 then
      r := m;
    else
      if c > 0 then
	l := m + 1;
      else
	return m;
      end;
    end;
  end;
  return -1;
end;

fun scan_identifier (s: state, c: char): token
  var ident: list of char := c :: null;

  c := get (s);
  while ((c >= 'A') and (c <= 'Z')) or
    ((c >= 'a') and (c <= 'z')) or
    ((c >= '0') and (c <= '9')) or
    (c = '_') or (c = '!') or (c = '?') do
    ident := c :: ident;
    c := get (s);
  end;
  unget (s, c);
  var str: string := strings.implode (lists.reverse (ident));
  var idx: int := bsearch (str);
  if idx < 0 then
    return identifier (str);
  else
    var s: string, f: fun(string): token;
    s, f := reserved_words[idx];
    return f (str);
  end;
end;

fun scan_number_const (s: state, c: char): token
  var ival: int := core.ord (c) - core.ord ('0');
  var rval: int := 0, scale: real := 10.0;
  var eval: int := 0;
  c := get (s);
  while (c >= '0') and (c <= '9') do
    ival := ival * 10 + (core.ord (c) - core.ord ('0'));
    c := get (s);
  end;
  if c = '.' then
    c := get (s);
    while (c >= '0') and (c <= '9') do
      scale := scale * 10.0;
      rval := rval * 10 + (core.ord (c) - core.ord ('0'));
      c := get (s);
    end;
    if c = 'e' or c = 'E' then
      c := get (s);
      if c = '-' then
	c := get (s);
      end;
      while (c >= '0') and (c <= '9') do
	eval := eval * 10 + (core.ord (c) - core.ord ('0'));
	c := get (s);
      end;
      unget (s, c);
      return real_const ("<real>", 
                         (reals.from_int (ival) + 
			  reals.from_int (rval) / scale) * 
			 reals.pow (10.0, eval));
    else
      unget (s, c);
      return real_const ("<real>", reals.from_int (ival) + 
                                   reals.from_int (rval) / scale);
    end;
  elsif c = 'L' then
    return long_const ("<long>", longs.from_int (ival));
  else
    unget (s, c);
    return int_const ("<integer>", ival);
  end;
end;
  
fun scan_escaped_char (s: state, c: char): char
  if (c = '\\') then
    c := get (s);
    if c = 'n' then
      c := '\n';
    elsif c = 'r' then
      c := '\r';
    elsif c = 't' then
      c := '\t';
    elsif c = 'b' then
      c := '\b';
    elsif c = 'v' then
      c := '\v';
    elsif c = 'f' then
      c := '\f';
    elsif c = 'a' then
      c := '\a';
    elsif c = '\'' then
      c := '\'';
    elsif c = '\"' then
      c := '\"';
    elsif c = '\\' then
      c := '\\';
    else
      error (s, ": invalid escape sequence");
    end;
  end;
  return c;
end;

fun scan_string_const (s: state): token
  var content: list of char := null;
  var c: char := get (s);
  while (c <> chars.EOF) and (c <> '\"') and (c <> '\n') do
    if (c = '\\') then
      c := scan_escaped_char (s, c);
    end;
    content := c :: content;
    c := get (s);
  end;
  if (c = chars.EOF) or (c = '\n') then
    error (s, "unterminated string constant");
  end;
  return str_const ("<string>", strings.implode (lists.reverse (content)));
end;

fun scan_char_const (s: state): token
  var c: char := get (s);
  if c = '\\' then
    c := scan_escaped_char (s, c);
  end;
  var close: char := get (s);
  if close <> '\'' then
    error (s, "improperly closed character constant");
  end;
  return char_const ("<char>", c);
end;

fun skip_comment (s: state)
  var c: char;
  c := get (s);
  if c = '*' then
    c := get (s);
    while c = ' ' or c = '\t' do
      c := get (s);
    end;
    while (c <> chars.EOF) and (c <> '\n') do
      append_comment (s, c);
      c := get (s);
    end;
    append_comment (s, '\n');
  else
    while (c <> chars.EOF) and (c <> '\n') do
      c := get (s);
    end;
  end;
end;

fun skip_c_comment (s: state)
  var c: char := get (s);
  if c = '*' then
    c := get (s);
    if c = '/' then
      return;
    end;
    while c = ' ' or c = '\t' do
      c := get (s);
    end;
    while true do
      while c <> chars.EOF and c <> '*' do
	append_comment (s, c);
	c := get (s);
      end;
      while c = '*' do
	c := get (s);
      end;
      if c = chars.EOF or c = '/' then
	if c = '/' then
	  c := get (s);
	end;
	return;
      end;
    end;
  else
    while true do
      while c <> chars.EOF and c <> '*' do
	c := get (s);
      end;
      while c = '*' do
	c := get (s);
      end;
      if c = chars.EOF or c = '/' then
	if c = '/' then
	  c := get (s);
	end;
	return;
      end;
    end;
  end;
end;

fun scan_symbol (s: state, c: char): token
  if c = '+' then
    return plus ("+");
  elsif c = '-' then
    return minus ("-");
  elsif c = '*' then
    return star ("*");
  elsif c = '%' then
    return percent ("%");
  elsif c = '(' then
    return lparen ("(");
  elsif c = ')' then
    return rparen (")");
  elsif c = '[' then
    return lbracket ("[");
  elsif c = ']' then
    return rbracket ("]");
  elsif c = '{' then
    return lbrace ("{");
  elsif c = '}' then
    return rbrace ("}");
  elsif c = ':' then
    c := get (s);
    if c = '=' then
      return assign (":=");
    elsif c = ':' then
      return cons ("::");
    else
      unget (s, c);
      return colon (":");
    end;
  elsif c = '.' then
    return period (".");
  elsif c = ',' then
    return comma (",");
  elsif c = ';' then
    return semicolon (";");
  elsif c = '=' then
    return eq ("=");
  elsif c = '<' then
    c := get (s);
    if c = '>' then
      return ne ("<>");
    elsif c = '=' then
      return le (">=");
    else
      unget (s, c);
      return lt ("<");
    end;
  elsif c = '>' then
    c := get (s);
    if c = '=' then
      return ge (">=");
    else
      unget (s, c);
      return gt (">");
    end;
  elsif c = '!' then
    return exclamation ("!");
  elsif c = '`' then
    return backquote ("`");
  else
    return eof ("<eof>");
  end;
end;

public fun scan (s: state): token
  while true do
    var c: char := skip_whitespace (s, ' ');
    start_line! (s, last_line (s));
    start_col! (s, last_col (s));
    if c = chars.EOF then
      return eof ("<eof>");
    elsif (c >= '0') and (c <= '9') then 
      return scan_number_const (s, c); 
    elsif ((c >= 'A') and (c <= 'Z')) or
      ((c >= 'a') and (c <= 'z')) or
      (c = '_') then
      return scan_identifier (s, c);
    elsif (c = '\"') then
      return scan_string_const (s);
    elsif c = '\'' then
      return scan_char_const (s);
    elsif (c = '(') or (c = ')') or
      (c = '[') or (c = ']') or
      (c = '{') or (c = '}') or
      (c = '<') or (c = '>') or
      (c = '.') or (c = ',') or
      (c = ';') or (c = '%') or
      (c = '+') or (c = '-') or
      (c = '*') or (c = '/') or
      (c = ':') or (c = '=') or
      (c = '`') or (c = '!') then
      if (c = '/') then
	c := get (s);
	if (c = '/') then
	  skip_comment (s);
	elsif c = '*' then
	  skip_c_comment (s);
	else
	  unget (s, c);
	  return slash ("/");
	end;
      else
	return scan_symbol (s, c);
      end;
    else
      error (s, "invalid character: " + strings.to_string (c)); 
    end;
  end;
  return eof ("<eof>");
end;

// End of scanner.t.
