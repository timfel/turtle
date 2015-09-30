// internal/timeout.t -- Timeout function installation handling.
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
//* Module for installing a timeout function, that is a function
//* which gets called repeatedly in some interval.

module internal.timeout;

//* Register @var{handler} as the timeout function for the Turtle
//* runtime.  The timeout function gets called repeatedly, but
//* there is no guarantee on how often it will be called.
//* 
//* On a 800Mhz AMD Duron(tm) processor, the function is called
//* about every 0.1 seconds.
//
public fun set (handler: fun());

//* Remove the current timeout function, so that no function will
//* be called until another handler is registered with the function
//* @code{set} above.
//
public fun clear ();

// End of internal/timeout.t.
