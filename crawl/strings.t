// strings.t -- Turtle module for string handling.
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
//* Library module for string utilities.

module strings;

import core, bools, chars, ints, longs, reals, lists<char>;


//* Return the number of characters in @var{s}.
//
public fun length (s: string): int
  return sizeof s;
end;


//* Return a freshly allocated string with the same length and 
//* contents as the given string @var{s}.
//
public fun copy(s: string): string
  var temp: string, idx: int;

  temp := string length(s) of ' ';
  idx := 0;
  while idx < length(s) do
    temp[idx] := s[idx];
    idx := idx + 1;
  end;
  return temp;
end;


//* Return a string containing the characters from @var{s} starting at
//* index @var{from} (inclusive), up to @var{to} (exclusive).
//
public fun substring (s: string, from: int, to: int): string
  var res: string := string to - from of ' ';
  var src: int := from, dst: int := 0;
  while src < to do
    res[dst] := s[src];
    dst := dst + 1;
    src := src + 1;
  end;
  return res;
end;
//* Similar to @code{substring(string, int, int)}, where the last
//* argument defaults to the length of the string.
//
public fun substring (s: string, from: int): string
  return substring (s, from, sizeof s);
end;


//* Return a string which is the concatenation of the argument strings.
//
public fun append (s1: string, s2: string): string
  var temp: string;
  var src: int, dst: int;
  var len1: int, len2: int;

  len1 := length (s1);
  len2 := length (s2);
  temp := string len1 + len2 of ' ';
  dst := 0;
  src := 0;
  while src < len1 do
    temp[dst] := s1[src];
    dst := dst + 1;
    src := src + 1;
  end;
  src := 0;
  while src < len2 do
    temp[dst] := s2[src];
    dst := dst + 1;
    src := src + 1;
  end;
  return temp;
end;
//* ""
public fun append (s1: string, s2: string, s3: string): string
  return append (append (s1, s2), s3);
end;


//* Return the string representation of the argument.
//
public fun to_string (b: bool): string
  return bools.to_string (b);
end;
//* ""
public fun to_string (i: int): string
  return ints.to_string (i);
end;
//* ""
public fun to_string (l: long): string
  return longs.to_string (l);
end;
//* ""
public fun to_string (r: real): string
  return reals.to_string (r);
end;
//* ""
public fun to_string (c: char): string
  return chars.to_string (c);
end;


//* Return the smallest index of any appearence of character @var{c} in
//* the string @var{s}.  Return -1 if not found.
//
public fun index (s: string, c: char): int
  var i: int := 0;
  while i < sizeof s do
    if s[i] = c then
      return i;
    end;
    i := i + 1;
  end;
  return -1;
end;


//* Return the largest index of any appearence of character @var{c} in
//* the string @var{s}.  Return -1 if not found.
//
public fun rindex (s: string, c: char): int
  var i: int := sizeof s;
  while i > 0 do
    i := i - 1;
    if s[i] = c then
      return i;
    end;
  end;
  return -1;
end;


//* Return a list containing all indices of the appearences of
//* character @var{c} in the string @var{s}.  An empty list is
//* returned if @var{c} does not appear in @var{s}.
//
public fun indices (s: string, c: char): list of int
  var result: list of int := [];
  var index: int := sizeof s;
  while index > 0 do
    index := index - 1;
    if s[index] = c then
      result := index :: result;
    end;
  end;
  return result;
end;


//* Return true if the two strings have the same length and contents,
//* false otherwise.
//
public fun eq (s1: string, s2: string): bool
  if sizeof s1 <> sizeof s2 then
    return false;
  else
    var idx: int := 0;
    while idx < sizeof s1 do
      if s1[idx] <> s2[idx] then
	return false;
      end;
      idx := idx + 1;
    end;
    return true;
  end;
end;


//* Return a list of characters containing the characters of string 
//* @var{s}, in the same order.
//
public fun explode (s: string): list of char
  var idx: int := sizeof s;
  var res: list of char := null;
  while idx > 0 do
    idx := idx - 1;
    res := s[idx] :: res;
  end;
  return res;
end;


//* Return a list of characters containing the characters of string
//* @var{s}, in reversed order.  This function is often convenient
//* when constructing strings from character lists.
//
public fun rexplode (s: string): list of char
  var idx: int := 0;
  var res: list of char := null;
  while idx < sizeof s do
    idx := idx + 1;
    res := s[idx] :: res;
  end;
  return res;
end;


//* Return a string with the same length as the list @var{l}, and with the
//* elements of @var{l} as string contents, in the same order.
//
public fun implode (l: list of char): string
  var res: string := string lists.length (l) of ' ';
  var idx: int := 0;
  while l <> null do
    res[idx] := hd l;
    l := tl l;
    idx := idx + 1;
  end;
  return res;
end;


//* Return a string with the same length as the list @var{l} and
//* with the elements of @var{l} as string contents, but in reverse
//* order.  The same result would be obtained by calling
//*
//* @example
//* strings.implode (lists.reverse (l));
//* @end example
//*
//* but calling @code{rimplode} is more efficient.
//
public fun rimplode (l: list of char): string
  var res: string := string lists.length (l) of ' ';
  var idx: int := sizeof res;
  while l <> null do
    idx := idx - 1;
    res[idx] := hd l;
    l := tl l;
  end;
  return res;
end;


//* Split up the string @var{s} at all occurences of the character
//* @var{c} and return a list of the strings between these
//* occurences.  Note that occurences of @var{c} without any
//* characters between will result in empty strings in the
//* resulting list.
//* 
//* @example
//* strings.split ("root:x:0:0:root:/root:/bin/bash", ':')
//* @result{}
//* ["root", "x", "0", "0", "root", "/root", "/bin/bash"]
//* 
//* strings.split (":x:0:0:root::/bin/bash", ':')
//* @result{}
//* ["", "x", "0", "0", "root", "", "/bin/bash"]
//* @end example
//
public fun split (s: string, c: char): list of string
  var idx: int := sizeof s;
  var last: int;
  var res: list of string := [];

  last := idx;
  while idx > 0 do
    idx := idx - 1;
    if s[idx] = c then
      res := substring (s, idx + 1, last) :: res;
      last := idx;
    end;
  end;
  res := substring (s, 0, last) :: res;
  return res;
end;


//* Return a string of length @var{len}, consisting of the
//* character @var{elem}.
//
public fun replicate (elem: char, len: int): string
  return string len of elem;
end;


//* Return a string of at least @var{places} characters which
//* contains @var{s}, padded on the right (for @code{rpad}) or left
//* (for @code{lpad}) with character @var{ch}.
public fun rpad (s: string, places: int, ch: char): string
  if sizeof s < places then
    return s + replicate (ch, places - sizeof s);
  else
    return s;
  end;
end;
//* ""
public fun lpad (s: string, places: int, ch: char): string
  if sizeof s < places then
    return replicate (ch, places - sizeof s) + s;
  else
    return s;
  end;
end;


//* Return a newly allocated string containing the characters from
//* @var{s} converted to upper case or lower case, respectively.
//
public fun upcase (s: string): string
  var ret: string := string sizeof s of ' ';
  var i: int := 0;
  while i < sizeof s do
    ret[i] := chars.upcase (s[i]);
    i := i + 1;
  end;
  return ret;
end;
//* ""
public fun downcase (s: string): string
  var ret: string := string sizeof s of ' ';
  var i: int := 0;
  while i < sizeof s do
    ret[i] := chars.downcase (s[i]);
    i := i + 1;
  end;
  return ret;
end;

// End of strings.t.
