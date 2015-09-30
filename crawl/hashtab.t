// hashtab.t -- Hash table implementation
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
//* This is a generic implementation for hash table.  The range and
//* domain types for the partial mapping implemented by the hash table are
//* supplied as module parameters and the hash function for mapping the
//* keys to integers must be given when constructing a hash table.
//*
//* The module expects two module parameters, @var{range} and @var{domain},
//* where @var{range} is the type of the keys in the hash table and
//* @var{domain} is the type of the associated values.

module hashtab<range, domain>;

import option<domain>, io;

export hashtable;


//* -
//* The hashtable implementation uses an open hashing scheme, where
//* the hash table is an array of buckets, each bucket holding a
//* key/value pair.  All keys which hash to the same array index are
//* chained together via the buckets.
//
datatype bucket = nil or
                  bucket (key: range, val: domain, next: bucket);


//* This is the hastable data type.  It stores the hashing and comparison
//* functions to be used with the keys, so that these functions must
//* not be passed to the insertion/search functions each time they
//* are invoked.
//
datatype hashtable = tab (size: int, data: array of bucket, 
                          hashfn: fun (range): int,
			  cmpfn: fun (range, range): bool);


//* Create a new hash table which maps keys of type @var{range} to
//* values of type @var{domain}.  @var{hashfn} is a function which
//* determines the hash value of a key and @var{cmdfn} is a function
//* which determines whether two keys are actually the same.
//
public fun make (hashfn: fun (range): int, cmpfn: fun (range, range): bool): 
                 hashtable
  var size: int := 631;
  var tab: hashtable := tab (size, (array size of (nil())), hashfn, cmpfn);
  return tab;
end;


//* Insert the @var{key}/@var{value} pair into the hash table @var{tab}.
//* If @var{key} is already in the table, its value is overwritten.
//
public fun insert (tab: hashtable, key: range, val: domain)
  var h: int := hashfn(tab)(key) % size(tab);
  var b: bucket := data (tab)[h];

  while not nil? (b) and not cmpfn (tab) (key, key (b)) do
    b := next (b);
  end;
  if nil? (b) then
    data (tab)[h] := bucket (key, val, data (tab)[h]);
  else
    val! (b, val);
  end;
end;


//* Remove the entry with key @var{key} from the hash table @var{tab}.
//* Do nothing, if the key is not present in the table.
//
public fun delete (tab: hashtable, key: range)
  var h: int := hashfn(tab)(key) % size(tab);
  var b: bucket := data (tab)[h];
  if not nil? (b) then
    if cmpfn (tab) (key, key (b)) then
      data (tab)[h] := next (b);
      return;
    end;
    while not nil? (next (b)) do
      if cmpfn (tab) (key, key (next (b))) then
	next! (b, next (next (b)));
	return;
      end;
      b := next (b);
    end;
  end;
end;


//* Search the hash table @var{tab} for an entry with key @var{key} and
//* return @code{some(value)} if the key was found, and @code{none()}
//* if not found in the table.
//
public fun lookup (tab: hashtable, key: range): option.option
  var h: int := hashfn(tab)(key) % size(tab);
  var b: bucket := data(tab)[h];

  while not nil?(b) and not cmpfn(tab)(key, key(b)) do
    b := next(b);
  end;
  if nil?(b) then
    return option.none();
  else
    return option.some(val(b));
  end;
end;

// End of hashtab.t.
