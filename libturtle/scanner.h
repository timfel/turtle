/* libturtle/scanner.h - Turtle scanner
 
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

#ifndef TTL_SCANNER_H
#define TTL_SCANNER_H

#include <stdio.h>

#include "ast.h"


/* This array maps elements of `enum ttl_token_type' to strings.  Used
   for error messages etc.  Use the token class as the index, as
   in: 

   printf ("token class: %s\n", ttl_token_names[(int) class]); 
*/
extern char * ttl_token_names[];


/* The various token classes recognized by the Turtle scanner.
   Reserved words are each members of one-element token classes
   representing themselves.  Literals (integer, real, character,
   string constants) and identifiers carry additional information,
   which will be saved in the scanner state when delivering
   tokens. 

   NOTE: The ordering of this type must correspond to the entries in
   the string array `ttl_token_names' defined in scanner.c. */
enum ttl_token_type
  {token_eof,
   token_identifier,
   token_string_const,
   token_int_const,
   token_long_const,
   token_real_const,
   token_char_const,
   token_module,
   token_then,
   token_if,
   token_else,
   token_end,
   token_while,
   token_do,
   token_fun,
   token_constraint,
   token_return,
   token_var,
   token_type,
   token_array,
   token_list,
   token_of,
   token_semicolon,
   token_colon,
   token_plus,
   token_minus,
   token_eq,
   token_ne,
   token_lt,
   token_le,
   token_gt,
   token_ge,
   token_assign,
   token_and,
   token_or,
   token_lparen,
   token_rparen,
   token_lbracket,
   token_rbracket,
   token_lbrace,
   token_rbrace,
   token_dot,
   token_comma,
   token_import,
   token_export,
   token_star,
   token_slash,
   token_not,
   token_false,
   token_true,
/*    token_out, */
   token_require,
   token_string,
   token_percent,
   token_datatype,
   token_hd,
   token_tl,
   token_null,
   token_exclamation,
   token_sizeof,
   token_cons,
   token_public,
   token_elsif,
   token_in,
   token_const,
   token_foreign
  };


/* A scanner state remembers all the state a scanner needs for
   scanning the next token, maintain the source file location,
   allocate storage for buffers and literals and for entering
   identifiers into a symbol table.  */
typedef struct ttl_scanner * ttl_scanner;
struct ttl_scanner
{
  FILE * f;			/* Input file to read from. */
  char * filename;		/* For source file location.  */

  enum ttl_token_type token_class; /* Current token class.  */
  ttl_ast_node token_value;	/* Associated data.  */

  ttl_pool pool;		/* For allocating memory.  */
  ttl_symbol_table symbol_table; /* For maintaining identifiers.  */
  
  int current_char;		/* Buffer for lookahead character.  */
  int pushed_back;		/* Non-zero if next field is valid.  */
  int pushed_back_char;		/* Pushback buffer.  */

  int current_line;		/* Source code position.  */
  int current_column;
  int begin_line;		/* Start position of current token.  */
  int begin_column;
  int last_line;		/* End position of current token.  */
  int last_column;

  char * buffer;		/* Internal buffer for accumulating
				   characters.  */
  int buf_len;			/* # of characters used in the above.  */
  int buf_size;			/* # of characters allocated for the
                                   buffer.  */

  int scan_errors;		/* For recording scanning errors.  */
  int parse_errors;		/* Used by the parser for recording
				   errors. */

  char * comment_buffer;	/* Buffer for accumulating
				   documentation comments.  */
  int comment_buf_len;		/* # of characters used in the above.  */
  int comment_buf_size;		/* # of characters allocated for the
                                     buffer.  */
};


/* Create a new scanner state.  It will allocate memory from the
   memory pool `pool', enter scanned identifiers into the symbol table
   `symbol_table' and read from the file `f'.  `filename' is used for
   maintaining source file locations.  */
ttl_scanner ttl_make_scanner (ttl_pool pool, ttl_symbol_table symbol_table,
			      FILE * f, char * filename);

/* Read the next token from `state's input file, update the source
   file position and make its token class available in
   state->token_class.  When the new token is a literal or identifier,
   its associated data is stored in state->token_value.  */
void ttl_next_token (ttl_scanner state);

/* Return the contents of the comment buffer and reset the buffer.
   The returned string is allocated from the state's allocation
   pool.  Return NULL if no comment was read before.  */
char * ttl_get_comment_buffer (ttl_scanner state);

#endif /* not TTL_SCANNER_H */
