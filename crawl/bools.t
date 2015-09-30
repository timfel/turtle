// bools.t -- Turtle module for boolean value handling.
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
//* Library module for utility functions for boolean values.

module bools;


//* Convert the boolean value @var{b} to a string.  The result will
//* be either the string @code{"true"} or the string @code{"false"}.
//
public fun to_string (b: bool): string
  if b then
    return "true";
  else
    return "false";
  end;
end;


//* Convert a string to a boolean value.  The two recognized strings
//* are @code{"true"} and @code{"false"}.  Any other string will
//* cause an unspecified value to be returned.
//
public fun from_string (s: string): bool
  if (sizeof s = 4) and
    (s[0] = 't') and
    (s[1] = 'r') and
    (s[2] = 'u') and
    (s[3] = 'e') then
    return true;
  elsif (sizeof s = 5) and
    (s[0] = 'f') and
    (s[1] = 'a') and
    (s[2] = 'l') and
    (s[3] = 's') and
    (s[4] = 'e') then
    return false;
  end;
  return false;
end;

// End of bools.t.
