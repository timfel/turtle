/* libturtle/memory.h - memory management
 
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

#ifndef TTL_MEMORY_H
#define TTL_MEMORY_H

#include <stdlib.h>

#define TTL_POOL_CHUNK_SIZE (4 * 1024)

typedef struct ttl_pool_chunk * ttl_pool_chunk;
struct ttl_pool_chunk
{
  ttl_pool_chunk next;
  size_t data_length;
  void * data;
};


typedef struct ttl_pool * ttl_pool;
struct ttl_pool
{
  ttl_pool_chunk chunks;
};

/* Create a fresh memory pool from which memory can be allocated via
   ttl_malloc().  Aborts the process if not enough memory is available
   for satisfying the request.  */
ttl_pool ttl_create_pool (void);

/* Destroy a memory pool previously created with ttl_create_pool().
   Free all memory allocated for the pool.  Return 0 on success, a
   non-zero value on failure.  */
int ttl_destroy_pool (ttl_pool pool);

/* Allocate BYTES bytes of memory from the memory pool POOL and return
a pointer to the beginning of the allocated chunk.  Abort the process
if not enough memory is available for satisfying the request.  Note
that no free() function is available, the memory will be freed when
the memory pool is destroyed.  */
void * ttl_malloc (ttl_pool pool, size_t bytes);

extern size_t ttl_allocated_bytes;

void ttl_check_mem (void * data_area, size_t data_length,
		    char * filename, int lineno);

#endif /* not TTL_MEMORY_H */
