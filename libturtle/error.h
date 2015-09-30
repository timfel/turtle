/* libturtle/error.h - error handling and printing
 
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

#ifndef TTL_ERROR_H
#define TTL_ERROR_H

#include <stdio.h>

#include "ast.h"


/* Use these error reporting functions for nicely formatted and
   informative error messages.  First, print the source code location
   with `ttl_error_print_location()', then an error message using
   `ttl_error_print_string()', optionally the textual representation
   of an AST node with `ttl_error_print_node()' and terminate the line
   with `ttl_error_print_nl()'.  */

void ttl_error_print_location (FILE * f, ttl_ast_node node);
void ttl_error_print_string (FILE * f, char * s);
void ttl_error_print_node (FILE * f, ttl_ast_node node);
void ttl_error_print_nl (FILE * f);

#endif /* not TTL_ERROR_H */
