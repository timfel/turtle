// filenames.t -- Turtle module for filename handling.
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
//* Library module for filename manipulation functions.

module filenames;

import strings;


//* The path name element seperator used by the module.
//
public var path_seperator: char := '/';


//* Return the filename @var{s} without any directory component.
//* If the parameter @var{ext} is specified and matches the file
//* name extension of @var{s}, this is removed also.
//
public fun basename (s: string): string
  var idx: int := sizeof s;
  while idx > 0 do
    idx := idx - 1;
    if s[idx] = path_seperator then
      return strings.substring (s, idx + 1, sizeof s);
    end;
  end;
  return s;
end;
//* ""
public fun basename (s: string, ext: string): string
  s := basename (s);
  if sizeof s >= sizeof ext then
    var idx_s: int := sizeof s - sizeof ext;
    var idx_e: int := 0;
    while idx_e < sizeof ext do
      if s[idx_s] <> ext[idx_e] then
	return s;
      end;
      idx_s := idx_s + 1;
      idx_e := idx_e + 1;
    end;
    return strings.substring (s, 0, sizeof s - sizeof ext);
  else
    return s;
  end;
end;


//* Return the directory component of the filename @var{s}, without
//* any trailing path seperator.
//
public fun dirname (s: string): string
  var idx: int := sizeof s;

  while idx > 0 do
    idx := idx - 1;
    if s[idx] = path_seperator then
      if idx = 0 then
	return s;
      else
	return strings.substring (s, 0, idx);
      end;
    end;
  end;
  return s;
end;

// End of filenames.t.
