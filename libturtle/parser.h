/* libturtle/parser.h - Turtle parser
 
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

#ifndef TTL_PARSER_H
#define TTL_PARSER_H

#include <stdio.h>

#include "ast.h"
#include "scanner.h"


/* Parse the module file for which the scanner state `scanner' was
   created and return it as a parse tree.  `pool' is used for
   allocating memory for the abstract syntax tree.  Note that the
   memory for literals and identifiers is allocated in the scanner, so
   possibly not in the same pool as `pool'.  

   If parsing gets seriously stuck, NULL is returned instead of an AST
   node.  Less serious errors are recorded in the parser error count
   in `scanner', so it is recommended to check this variable before
   processing the AST further.  */
ttl_ast_node ttl_parse_module (ttl_pool pool, ttl_scanner scanner);


/* Parse the interface file for which the scanner state `scanner' was
   created and return it as an AST list.  `pool' is used for
   allocating memory for the abstract syntax tree.  Note that the
   memory for literals and identifiers is allocated in the scanner, so
   possibly not in the same pool as `pool'.

   If parsing gets seriously stuck, NULL is returned instead of an AST
   node.  But note that an empty interface file also results in a NULL
   return value, so better check the parse error counter in `scanner'
   before processing the AST list further.  */
ttl_ast_node ttl_parse_interface (ttl_pool pool, ttl_scanner scanner);


#endif /* not TTL_PARSER_H */
