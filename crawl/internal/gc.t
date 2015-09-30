// internal/gc.t -- Interface functions to the garbage collector.
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
//* Low-level module for interfacing to the garbage collector.

module internal.gc;


//* Return the number of heap garbage collections since the program 
//* started.
//
public fun gc_calls (): int = "ttl_gc_calls";


//* Return the number of heap overflow checks since the program 
//* started.
//
public fun gc_checks (): int = "ttl_gc_checks";


//* Force a garbage collection.
//
public fun garbage_collect () = "ttl_do_garbage_collect";

// End of internal/gc.t.
