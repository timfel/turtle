// internal/stats.t -- Interface functions to the runtime system statistics.
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
//* Low-level module for interfacing to the runtime system
//* statistics.

module internal.stats;


//* These functions deliver some statistics gathered by the runtime
//* system.
//
public fun dispatch_call_count (): int;
//* ""
public fun direct_call_count (): int;
//* ""
public fun local_call_count (): int;
//* ""
public fun closure_call_count (): int;
//* ""
public fun gc_checks (): int;
//* ""
public fun gc_calls (): int;
//* ""
public fun allocations (): int;
//* ""
public fun alloced_words (): int;
//* ""
public fun forwarded_words (): int;
//* ""
public fun save_cont_count (): int;
//* ""
public fun restore_cont_count (): int;
//* ""
public fun total_gc_time (): int;
//* ""
public fun min_gc_time (): int;
//* ""
public fun max_gc_time (): int;

//* - This does not work yet.
/*public*/ fun total_run_time (): int;

// End of internal/stats.t.
