/* libturtle/memory.c - memory management
 
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

#include <stdio.h>

#include "memory.h"

#define MAGIC_1 0x5a
#define MAGIC_2 0x5a

size_t ttl_allocated_bytes = 0;

ttl_pool ttl_create_pool (void)
{
  ttl_pool pool = malloc (sizeof (struct ttl_pool));
  if (!pool)
    {
      fprintf (stderr, "turtle: virtual memory exhausted.\n");
      exit (1);
    }
  ttl_allocated_bytes += sizeof (struct ttl_pool);
  pool->chunks = NULL;
  return pool;
}


int ttl_destroy_pool (ttl_pool pool)
{
  ttl_pool_chunk chunk, c;

  if (!pool)
    {
      fprintf (stderr, "turtle: NULL pool in ttl_destroy_pool().\n");
      abort ();
    }

  chunk = pool->chunks;
  while (chunk)
    {
      c = chunk;
      chunk = chunk->next;

      ttl_check_mem (c->data, c->data_length, __FILE__, __LINE__);
      free (c->data);
      ttl_allocated_bytes -= c->data_length;
      free (c);
      ttl_allocated_bytes -= sizeof (struct ttl_pool_chunk);
    }

  free (pool);
  ttl_allocated_bytes -= sizeof (struct ttl_pool);
  return 0;
}


void * ttl_malloc (ttl_pool pool, size_t bytes)
{
  void * p;
  ttl_pool_chunk chunk;

  chunk = malloc (sizeof (struct ttl_pool_chunk));
  if (!chunk)
    {
      fprintf (stderr, "turtle: virtual memory exhausted.\n");
      exit (1);
    }
  ttl_allocated_bytes += sizeof (struct ttl_pool_chunk);
  chunk->data = malloc (bytes + 2);
  if (!chunk->data)
    {
      fprintf (stderr, "turtle: virtual memory exhausted.\n");
      exit (1);
    }
  ((char *)chunk->data)[bytes] = (char) MAGIC_1;
  ((char *)chunk->data)[bytes + 1] = (char) MAGIC_2;
  ttl_allocated_bytes += bytes;
  chunk->next = pool->chunks;
  chunk->data_length = bytes;
  pool->chunks = chunk;
  p = chunk->data;
  return p;
}

void
ttl_check_mem (void * data_area, size_t data_length,
	       char * filename, int lineno)
{
  char * data;
  data = data_area;
  if (data[data_length] != (char) MAGIC_1)
    {
      unsigned i;
      fprintf (stderr,
	       "turtle:%s:%d: buffer overflow (1): expected: %d, got: %d\n",
	       filename, lineno, MAGIC_1, data[data_length]);
      fprintf (stderr, "dump: \"");
      for (i = 0; i < data_length && i < 100; i++)
	if (data[i] >= ' ' && data[i] < 127)
	  fprintf (stderr, "%c", data[i]);
	else
	  fprintf (stderr, "\\%d", data[i]);
      fprintf (stderr, "\"\n");
      abort ();
    }
  if (data[data_length + 1] != (char) MAGIC_2)
    {
      unsigned i;
      fprintf (stderr,
	       "turtle:%s:%d: buffer overflow (2): expected: %d, got: %d\n",
	       filename, lineno, MAGIC_2, data[data_length + 1]);
      fprintf (stderr, "dump: \"");
      for (i = 0; i < data_length && i < 100; i++)
	if (data[i] >= ' ' && data[i] < 127)
	  fprintf (stderr, "%c", data[i]);
	else
	  fprintf (stderr, "\\%d", data[i]);
      fprintf (stderr, "\"\n");
      abort ();
    }
}

/* End of memory.c.  */
