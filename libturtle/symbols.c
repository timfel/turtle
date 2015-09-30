/* libturtle/symbols.c - Symbol tables
 
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


#include <string.h>

#include "symbols.h"
#include "util.h"

ttl_symbol_table
ttl_make_symbol_table (ttl_pool pool)
{
  unsigned i;
  ttl_symbol_table table = ttl_malloc (pool, sizeof (struct ttl_symbol_table));

  table->pool = pool;
  for (i = 0; i < TTL_HASH_TABLE_SIZE; i++)
    table->table[i] = NULL;

  return table;
}

static unsigned
calc_hash (char * text, size_t length)
{
  unsigned hash = 0xdeadbeef;

  while (length-- > 0)
    {
      hash = (hash << 1) ^ *text++ ^ (hash >> 1);
    }
  return hash;
}

ttl_symbol
ttl_symbol_enter (ttl_symbol_table table, char * text, size_t length)
{
  unsigned raw_hash_code = calc_hash (text, length);
  unsigned hash_code = raw_hash_code % TTL_HASH_TABLE_SIZE;
  ttl_symbol sym = table->table[hash_code];

  while (sym)
    {
      if (sym->length == length && !memcmp (sym->text, text, length))
	break;
      sym = sym->next;
    }
  if (!sym)
    {
      sym = ttl_malloc (table->pool, sizeof (struct ttl_symbol));
      sym->raw_hash_code = raw_hash_code;
      sym->text = ttl_malloc (table->pool, (length + 1) * sizeof (char));
      memcpy (sym->text, text, length);
      sym->text[length] = '\0';
      sym->length = length;
      sym->next = table->table[hash_code];
      table->table[hash_code] = sym;
    }
  return sym;
}

void
ttl_symbol_print (FILE * f, ttl_symbol sym)
{
  char * text = sym->text;
  unsigned length = sym->length;
  size_t i;

  for (i = 0; i < length; i++)
    ttl_print_escaped_char (f, text[i]);
/*     fprintf (f, "%c", text[i]); */
}

/* End of symbols.c.  */
