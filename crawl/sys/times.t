// sys/times.t -- File handling.
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
//* Module for handling times.

module sys.times;

import sys.errno;

// NOTE: Do not change the order of the fields in this data type,
// because the C code which fills in the fields depends on the current
// order.
//
//* This data type represents date and time information.
//
public datatype tm = tm (sec: int, min: int, hour: int, mday: int,
                         mon: int, year: int, wday: int, yday: int,
			 isdst: int);


//* Return a @code{tm} structure representing the current time in
//* Universal Coordinated Time (UTC).
//
public fun gmtime (t: long): tm
  var tm: tm := tm (0, 0, 0, 0, 0, 0, 0, 0, 0);
  return igmtime (tm, t);
end;
//* - Internal helper function for gmtime ().
fun igmtime (tm: tm, t: long): tm;


//* Return a @code{tm} structure representing the current time in
//* local time.
//
public fun localtime (t: long): tm
  var tm: tm := tm (0, 0, 0, 0, 0, 0, 0, 0, 0);
  return ilocaltime (tm, t);
end;
//* - Internal helper function for localtime ().
fun ilocaltime (tm: tm, t: long): tm;


//* Return the current time in seconds since the Epoch (00:00:00
//* UTC, January 1, 1970), measured in seconds.
public fun time (): long
  var t: long := 0L;
  return itime (t);
end;
//* - Internal helper function for time ().
fun itime (t: long): long;

//* Return processor clock time in milliseconds
public fun clock (): real
  var t: real := 0.0;
  return iclock (t);
end;
//* - Internal helper function for time ().
fun iclock (t: real): real;


//* Return a string representation of the time @var{tm} or @var{t},
//* respectively.
//
public fun asctime (tm: tm): string;
public fun ctime (t: long): string;

// End of sys/times.t.
