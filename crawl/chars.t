// chars.t -- Turtle module for character handling.
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
//* Library module for character utilities.

module chars;

import core;

public const min: char := chr (0);
public const max: char := chr (65535);

//* @code{EOF} is the character which is returned by input functions
//* when the end of an input file is reached.
//
public const EOF: char := chr (65535);

//* Convenience variable containing the character @code{'\t'}.
public const tab: char := '\t';
//* Convenience variable containing the character @code{'\n'}.
public const newline: char := '\n';
//* Convenience variable containing the character @code{'\r'}.
public const carriage_return: char := '\r';
//* Convenience variable containing the character @code{' '}.
public const blank: char := ' ';
//* Convenience variable containing the character @code{' '}.
public const space: char := ' ';
//* Convenience variable containing the character @code{'\v'}.
public const vtab: char := '\v';
//* Convenience variable containing the character @code{'\b'}.
public const backspace: char := '\b';
//* Convenience variable containing the character @code{'\f'}.
public const formfeed: char := '\f';
//* Convenience variable containing the character @code{'\a'}.
public const bell: char := '\a';


//* Return the character code of character @var{c}.
//
public fun ord (c: char): int
  return core.ord (c);
end;


//* Return the character which has character code @var{i}.
//
public fun chr (i: int): char
  return core.chr (i);
end;


//* Convert the character @var{c} to a string of length one which
//* only contains @var{c}.
//
public fun to_string (c: char): string
  var s: string;
  s := string 1 of c;
  return s;
end;


//* Convert a string to a character value by simply extracting the
//* first character.  An exception will be raised if the string is
//* empty.
//
public fun from_string (s: string): char
  return s[0];
end;


//* Return @code{true}, if @var{c} is a digit, a letter, an
//* uppercase letter or a lowercase letter, respectively.
//
public fun digit? (c: char): bool
  return (c >= '0') and (c <= '9');
end;
//* ""
public fun letter? (c: char): bool
  return ((c >= 'a') and (c <= 'z')) or ((c >= 'A') and (c <= 'Z'));
end;
//* ""
public fun uppercase? (c: char): bool
  return ((c >= 'A') and (c <= 'Z'));
end;
//* ""
public fun lowercase? (c: char): bool
  return ((c >= 'a') and (c <= 'z'));
end;
//* ""
public fun control? (c: char): bool
  return core.ord (c) < core.ord (' ');
end;
//* ""
public fun punctuation? (c: char): bool
  const puncts: string := "!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~";
  var i: int := 0;
  while i < sizeof puncts do
    if puncts[i] = c then
      return true;
    end;
    i := i + 1;
  end;
  return false;
end;
//* ""
public fun letgit? (c: char): bool
  return letter? (c) or digit? (c);
end;
//* ""
public fun space? (c: char): bool
  return c = ' ' or c = '\t' or c = '\n';
end;
//* ""
public fun whitespace? (c: char): bool
  return space? (c) or c = '\f' or c = '\r' or c = '\v';
end;
//* ""
public fun printable? (c: char): bool
  return c = ' ' or digit? (c) or letter? (c) or punctuation? (c);
end;

//* If @var{c} is a lowercase letter, return its uppercase
//* equivalent, otherwise, return @var{c}.
//
public fun upcase (c: char): char
  if lowercase? (c) then
    return chr (ord (c) - (ord ('a') - ord ('A')));
  else
    return c;
  end;
end;


//* If @var{c} is an uppercase letter, return its lowercase
//* equivalent, otherwise, return @var{c}.
//
public fun downcase (c: char): char
  if uppercase? (c) then
    return chr (ord (c) + (ord ('a') - ord ('A')));
  else
    return c;
  end;
end;

//* Return the predecessor or successor of @var{c}, respectively.
public fun pred (c: char): char
  return core.chr (core.ord (c) - 1);
end;
//* ""
public fun succ (c: char): char
  return core.chr (core.ord (c) + 1);
end;

// End of chars.t.
