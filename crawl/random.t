// random.t -- Random numbers.
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
//* The exported functions @code{rand} delivers a random number in the range
//* 0..@code{ints.max}.  The current implementation uses the C library
//* function @code{rand()} for obtaining the random numbers and the
//* C library function @code{srand()} for seeding the generator.

module random;

import internal.random;


//* Seed the random number generator with the given seed value 
//* @var{seed}.
//
public fun seed (seed: int)
  internal.random.srand (seed);
end;


//* Return a random number in the range 0..@code{ints.max}.
//
public fun rand (): int
  return internal.random.rand ();
end;

// End of random.t.
