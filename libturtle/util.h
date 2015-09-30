/* libturtle/util.h - Utility routines.
 
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

#ifndef TTL_UTIL_H
#define TTL_UTIL_H

#include "memory.h"
#include "ast.h"
#include "compiler.h"

/* Return a freshly allocated copy of `filename's basename, that is,
   all directory components are removed.  */
char * ttl_basename (ttl_pool pool, char * filename);

/* Return a freshly allocated copy of `filename', with the file
   extension replaced by the string `ext'.  If `filename' has no file
   extension, `ext' is appended to `filename'.  */
char * ttl_replace_file_ext (ttl_pool pool, char * filename, char * ext);

/* Return the length of a (possibly qualified) identifier, including
   the one character separating the parts.  (This can be used for
   creating filenames and mangled symbols.)  */
size_t ttl_qualident_length (ttl_ast_node ident);

/* Write the (possibly qualified) identifier `ident' to the string
   `s', separating the parts of the name with `sep'.  (This can be
   used for creating filenames and mangled symbols.)  `s' is required
   to be large enough to take the string representation of `ident',
   which can be computed using `ttl_qualident_length()' above.  The
   return value is the number of characters placed in `s' by the
   call.  */
size_t ttl_qualident_to_string (char * s, ttl_ast_node ident, char sep);

/* Convert the (possibly qualified) identifier `ident' to a newly
   allocated string.  The string `suffix' is appended to the string to
   form a proper filename.  `pool' is used for allocating the memory
   of the result string.  */
char * ttl_qualident_to_filename (ttl_pool pool, ttl_ast_node ident,
				  char * suffix);

/* Convert a (possibly qualified) identifier to a string which is
   a valid identifier in C.  */
char * ttl_qualident_to_c_ident (ttl_pool pool, ttl_ast_node ident);

/* Create a symbol representing the mangled identifier consisting of
   the module name `mod_name', the identifier `name' and the data type
   `type'.  The name prefix `prefix' is currently ignored.  */
ttl_symbol ttl_mangle_name (ttl_compile_state state, ttl_ast_node mod_name,
			    ttl_ast_node name, ttl_type type);

/* Create a unique name using the name `name' and appending a unique
   integer to the name.  Return it as a symbol.  */
ttl_symbol ttl_uniquify_name (ttl_compile_state state, ttl_ast_node name);

/* Print the textual representation of the character `c' to the file
   `f'.  Escape the character in a suitable way (e.g. in Turtle
   syntax) if it is not printable.  */
void ttl_print_escaped_char (FILE * f, int c);

/* Try to find the file called `fname'.  If it exists, return `fname'.
   Otherwise, prepend each element of `search_path' (a colon-delimited
   list of directory names) to `fname', and check whether a file with
   the resulting name is readable.  If yes, return the resulting file
   name.  If none of the elements in `search_path' results in a
   readable file, return NULL.  */
char * ttl_find_file (ttl_pool pool, char * fname, char * search_path);

/* Return a freshly allocated string which contains the concatenation
   of `s1' and `s2'.  */
char * ttl_string_append (ttl_pool pool, char * s1, char * s2);

#endif /* not TTL_UTIL_H */
