/* libturtle/symbols.h - Symbol tables
 
  Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
 
  This is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.
  
  This software is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this package; see the file COPYING.  If not, write to the
  Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA 02111-1307, USA.  */

#ifndef TTL_SYMBOLS_H
#define TTL_SYMBOLS_H

#include <stdio.h>

#include "memory.h"

/*  Should be prime.  */
#define TTL_HASH_TABLE_SIZE 131

typedef struct ttl_symbol * ttl_symbol;
struct ttl_symbol
{
  ttl_symbol next;		/* Next symbol in open hash chain. */
  unsigned raw_hash_code;	/* Hash code before modulo operation.  */
  char * text;			/* Text contents of the symbol.  */
  size_t length;		/* Number of valid characters in `text'.  */
};

typedef struct ttl_symbol_table * ttl_symbol_table;
struct ttl_symbol_table
{
  ttl_pool pool;		/* Memory pool for allocation.  */
  ttl_symbol table[TTL_HASH_TABLE_SIZE]; /* Array of hash chains.  */
};

/* Create a new symbol table which will use the memory pool `pool' for
   allocation.  The symbol table and all symbols entered will be
   allocated with this pool.  */
ttl_symbol_table ttl_make_symbol_table (ttl_pool pool);

/* Enter the symbol with contents `text' of `length' characters into
   the symbol table `table'.  If a symbol with the same text already
   exists, the existing symbol will be returned, otherwise a new
   symbol is created, entered into the table and returned.  */
ttl_symbol ttl_symbol_enter (ttl_symbol_table table, char * text,
			     size_t length);

/* Print the textual representation of the symbol `sym' to the open
   output file `f'.  */
void ttl_symbol_print (FILE * f, ttl_symbol sym);

#endif /* not TTL_SYMBOLS_H */
