/* libturtle/error.c - error handling and printing
 
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

#include "error.h"

void
ttl_error_print_location (FILE * f, ttl_ast_node node)
{
#if 1
  fprintf (f, "%s:%d:%d: ",
	   node->filename ? node->filename : "(unknown)",
	   node->start_line + 1, node->start_column + 1);
#else
  fprintf (f, "%s:%d:%d:%d:%d: ",
	   node->filename ? node->filename : "(unknown)",
	   node->start_line + 1, node->start_column + 1,
	   node->end_line + 1, node->end_column + 1);
#endif
}


void
ttl_error_print_string (FILE * f, char * s)
{
  fprintf (f, "%s", s);
}


void
ttl_error_print_node (FILE * f, ttl_ast_node node)
{
  ttl_ast_print (f, node, 0);
}


void
ttl_error_print_nl (FILE * f)
{
  fprintf (f, "\n");
}

/* End of error.c.  */
