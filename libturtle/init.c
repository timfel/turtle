/* libturtle/init.c - initialize the runtime
 
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
#include "init.h"

ttl_state ttl_initialize (int argc, void * argv)
{
  ttl_state state = malloc (sizeof (struct ttl_state));
  if (!state)
    {
      fprintf (stderr, "turtle: virtual memory exhausted.\n");
      exit (1);
    }
  state->not_yet_used = 1;
  return state;
}


int ttl_finalize (ttl_state state)
{
  if (!state)
    {
      fprintf (stderr, "turtle: NULL state in ttl_finalize().\n");
      abort ();
    }
  free (state);
  return 0;
}

/* End of init.c.  */
