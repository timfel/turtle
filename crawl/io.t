// io.t -- Basic input/output.
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
//* This module provides basic input and output functions for some of
//* the builtin data types.  Also some functions for handling files
//* are defined.

module io;

import core, sys.files, chars, strings, lists<string>;


/** The @code{file} data type represents input and output streams.
    Values of this type are either found in one of the pre-defined
    variables @var{input}, @var{output} or @var{error}, or are
    obtained by calling functions like @code{open}. */
public datatype file = file (fd: int, buf: string, pos: int, fill: int,
                             writable: bool);

/** Standard input file.  */
public var input: file := file (sys.files.stdin, string 128 of (' '), 0, 0, 
                                false);
/** Standard output file.  */
public var output: file := file (sys.files.stdout, string 128 of (' '), 0, 0, 
                                 true);
/** Standard output file for error messages.  */
public var error: file := file (sys.files.stderr, string 128 of (' '), 0, 0, 
                                true);

//* -
fun flush_buf (f: file)
  var idx: int := 0;
  var limit: int := fill (f);
  while idx < limit do
    core.write_char (fd (f), buf (f)[idx]);
    idx := idx + 1;
  end;
  pos! (f, 0);
  fill! (f, 0);
end;

//* -
fun fill_buf (f: file): bool
  var fl: int := fill (f);
  var limit: int := sizeof buf (f);
  var c: char;
  if fl < limit then
    put ("filling\n");
    c := core.read_char (fd (f));
    while (c <> chars.EOF) and (fl < limit) do
      buf (f)[fl] := c;
      c := core.read_char (fd (f));
    end;
    fill! (f, fl);
  end;
  pos! (f, 0);
  return fl > 0;
end;

//* -
fun add_char (f: file, c: char)
  if fill (f) >= sizeof buf (f) then
    flush_buf (f);
  end;
  buf (f)[fill (f)] := c;
  fill! (f, fill (f) + 1);
end;


//* Write the string representation of the argument to the file @var{f}
//* or the standard output file, respectively.
//
public fun put (f: file, s: string)
  var x: int := 0;
  while x < sizeof s do
    add_char (f, s[x]);
    x := x + 1;
  end;
  flush_buf (f);
end;
//* ""
public fun put (s: string)
  put (output, s);
end;


//* ""
public fun put (f: file, i: int)
  put (f, strings.to_string (i));
end;
//* ""
public fun put (i: int)
  put (strings.to_string (i));
end;

//* ""
public fun put (f: file, l: long)
  put (f, strings.to_string (l));
end;
//* ""
public fun put (l: long)
  put (strings.to_string (l));
end;

//* ""
public fun put (f: file, r: real)
  put (f, strings.to_string (r));
end;
//* ""
public fun put (r: real)
  put (strings.to_string (r));
end;

//* ""
public fun put (f: file, b: bool)
  put (f, strings.to_string (b));
end;
//* ""
public fun put (b: bool)
  put (strings.to_string (b));
end;

//* ""
public fun put (f: file, c: char)
  put (f, strings.to_string (c));
end;
//* ""
public fun put (c: char)
  put (strings.to_string (c));
end;


//* Write all strings in the argument list to the given file, or
//* standard output, respectively.  The strings are separated by
//* newline characters in the output.
//
public fun put (f: file, ls: list of string)
  lists.foreach (fun (s: string)
                   put (f, s); nl (f);
                 end,
                 ls);
end;
//* ""
public fun put (ls: list of string)
  put (output, ls);
end;


//* Terminate the current output line by writing a newline character
//* to the given file or standard output, respectively.
//
public fun nl (f: file)
  put (f, "\n");
end;
//* ""
public fun nl ()
  put ("\n");
end;


//* Write the textual representation of the argument to standard
//* output or the given file, respectively.  Then terminate the
//* output with a newline character.
//
public fun putln (s: string)
  put (s);
  nl ();
end;
//* ""
public fun putln (f: file, s: string)
  put (f, s);
  nl (f);
end;
//* ""
public fun putln (c: char)
  put (c);
  nl ();
end;
//* ""
public fun putln (f: file, c: char)
  put (f, c);
  nl (f);
end;
//* ""
public fun putln (i: int)
  put (i);
  nl ();
end;
//* ""
public fun putln (f: file, i: int)
  put (f, i);
  nl (f);
end;
//* ""
public fun putln (l: long)
  put (l);
  nl ();
end;
//* ""
public fun putln (f: file, l: long)
  put (f, l);
  nl (f);
end;
//* ""
public fun putln (r: real)
  put (r);
  nl ();
end;
//* ""
public fun putln (f: file, r: real)
  put (f, r);
  nl (f);
end;
//* ""
public fun putln (b: bool)
  put (b);
  nl ();
end;
//* ""
public fun putln (f: file, b: bool)
  put (f, b);
  nl (f);
end;

//* Read the string representation of a value matching the return type,
//* convert it, and return that value.  If the string cannot be
//* converted, return an unspecified value.
//* 
//* The string reading functions returns @code{null} when the
//* end-of-file is reached.
//
// FIXME: Raise an exception if parsing fails.
//
public fun get (f: file): string
  var res: string := string 0 of (' ');
  var pos: int := 0;
  var c: char;
  c := get (f);
  if c = chars.EOF then
    return null;
  end;
  while (c <> '\n') and (c <> chars.EOF) do
    var r: string := string sizeof res + 1 of (' ');
    var x: int := 0;
    while x < sizeof res do
      r[x] := res[x];
      x := x + 1;
    end;
    res := r;
    res[pos] := c;
    pos := pos + 1;
    c := get (f);
  end;
  return res;
end;
//* ""
public fun get (): string
  return get (input);
end;

//* ""
public fun get (f: file): int
  var res: int := 0;
  var c: char := get (f);
  var negate: bool := false;
  if c = '-' then
    negate := true;
    c := get (f);
  end;
  while (core.ord (c) >= core.ord ('0')) and 
        (core.ord (c) <= core.ord ('9')) do
    res := res * 10 + (core.ord (c) - core.ord ('0'));
    c := get (f);
  end;
//  unget (f, c);
  return res;
end;
//* ""
public fun get (): int
  return get (input);
end;

//* ""
public fun get (f: file): bool
  var c: char := get (f);
  if c = 't' then
    c := get (f);
    if c = 'r' then
      c := get (f);
      if c = 'u' then
	c := get (f);
	if c = 'e' then
	  return true;
	end;
      end;
    end;
    return false;
  elsif c = 'f' then
    c := get (f);
    if c = 'a' then
      c := get (f);
      if c = 'l' then
	c := get (f);
	if c = 's' then
	  c := get (f);
	  if c = 'e' then
	    return true;
	  end;
	end;
      end;
    end;
    return false;
  else
    return false;
  end;
end;
//* ""
public fun get (): bool
  return get (input);
end;

//* ""
public fun get (f: file): char
  var c: char;
  if pos (f) < fill (f) then
    c := buf (f)[pos (f)];
    pos! (f, pos (f) + 1);
  else
    fill! (f, 0);
    pos! (f, 0);
    c := core.read_char (fd (f));
  end;
  return c;
end;
//* ""
public fun get (): char
  return get (input);
end;

//* ""
public fun get (f: file): real
  return 0.0;
end;
//* ""
public fun get (): real
  return get (input);
end;

//* Read all lines from the file @var{f} and return them as a list
//* of strings.  For an empty file, return @code{null}.
//
public fun get (f: file): list of string
  var res: list of string := null;
  var s: string;
  s := get (f);
  while s <> null do
    res := s :: res;
    s := get (f);
  end;
  return lists.reverse (res);
end;
//* ""
public fun get (): list of string
  return get (input);
end;

//* Put the character @var{c} back into the input file @var{f}
//* or standard input, respectively.
//
public fun unget (f: file, c: char)
  buf (f)[fill (f)] := c;
  fill! (f, fill (f) + 1);
end;
//* ""
public fun unget (c: char)
  unget (input, c);
end;


//* Open the file called @var{name} for reading.  Return a
//* @code{file} object for reading from the opened file,
//* or return @code{null} if the file can't be opened.
//
public fun open (name: string): file
  var fd: int;
  fd := sys.files.open (name);
  if fd < 0 then
    return null;
  else
    return file (fd, string 128 of ' ', 0, 0, false);
  end;
end;


//* Create a new file called @var{name} and open if for writing.  
//* Return a @code{file} object for writing to the opened file,
//* or return @code{null} if the file can't be opened.
//
public fun create (name: string): file
  var fd: int;
  fd := sys.files.create (name);
  if fd < 0 then
    return null;
  else
    return file (fd, string 128 of ' ', 0, 0, true);
  end;
end;


//* Close the file object @var{f}.  After calling this function,
//* @var{f} may not be used for file operations anymore.
//
public fun close (f: file)
  sys.files.close (fd (f));
  fd! (f, -1);
end;


//* Flush all buffered output to the underlying operating system
//* file descriptor.
//
public fun flush (f: file)
  flush_buf (f);
end;

// End of io.t.
