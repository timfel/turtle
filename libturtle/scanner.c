/* libturtle/scanner.c - Turtle scanner
 
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

#include "scanner.h"

char * ttl_token_names[] =
  {
    "<eof>",
    "<identifier>",
    "<string>",
    "<integer>",
    "<long>",
    "<real>",
    "<character>",
    "module",
    "then",
    "if",
    "else",
    "end",
    "while",
    "do",
    "fun",
    "constraint",
    "return",
    "var",
    "type",
    "array",
    "list",
    "of",
    ";",
    ":",
    "+",
    "-",
    "=",
    "<>",
    "<",
    "<=",
    ">", 
    ">=",
    ":=",
    "and",
    "or",
    "(",
    ")",
    "[",
    "]",
    "{",
    "}",
    ".",
    ",",
    "import",
    "export",
    "*",
    "/",
    "not",
    "false",
    "true",
    "out",
    "require",
    "string",
    "%",
    "datatype",
    "hd", 
    "tl",
    "null",
    "!",
    "sizeof",
    "::",
    "public",
    "elsif",
    "in",
    "const",
    "foreign",
  };

static void ttl_next_char (ttl_scanner state);
static void ttl_unget_char (ttl_scanner state, int c);


/* Create a new scanner state and initialize it according to the given
   parameters (resp. default values).  */
ttl_scanner
ttl_make_scanner (ttl_pool pool, ttl_symbol_table symbol_table,
		  FILE * f, char * filename)
{
  ttl_scanner state = ttl_malloc (pool, sizeof (struct ttl_scanner));
  state->f = f;
  state->token_class = token_eof;
  state->token_value = NULL;
  state->filename = ttl_malloc (pool, strlen (filename) + 1);
  strcpy (state->filename, filename);
  state->pool = pool;
  state->symbol_table = symbol_table;
  state->current_char = '\0';
  state->pushed_back = 0;
  state->pushed_back_char = '\0';
  state->current_line = 0;
  state->current_column = 0;
  state->last_line = 0;
  state->last_column = 0;
  state->buffer = NULL;
  state->buf_len = 0;
  state->buf_size = 0;

  state->scan_errors = 0;
  state->parse_errors = 0;

  state->comment_buffer = NULL;
  state->comment_buf_len = 0;
  state->comment_buf_size = 0;

  ttl_next_char (state);
  ttl_next_token (state);
  return state;
}


/* Resets state's character buffer pointer to zero, to prepare
   collecting a new token.  */
static void
ttl_reset_char_buffer (ttl_scanner state)
{
  state->buf_len = 0;
}


/* Add `c' to the state's character buffer and grows that buffer, if
   necessary.  */
static void
ttl_add_char (ttl_scanner state, int c)
{
  if (state->buf_len >= state->buf_size)
    {
      static int i = 0;
      char * buf = ttl_malloc (state->pool, state->buf_size + 32);
      if (state->buf_len > 0)
	memcpy (buf, state->buffer, state->buf_len);
      state->buf_size += 32;
      state->buffer = buf;
    }
  state->buffer[state->buf_len++] = c;
}


/* Resets state's comment buffer pointer to zero, to prepare
   collecting a documentation comment.  */
static void
reset_comment_buffer (ttl_scanner state)
{
  state->comment_buf_len = 0;
}


/* Add the character `c' to `state's comment buffer.  Similar to
   ttl_add_char, but for the documentation comment buffer.  */
static void
add_comment_char (ttl_scanner state, int c)
{
  if (state->comment_buf_len >= state->comment_buf_size)
    {
      static int i = 0;
      char * buf = ttl_malloc (state->pool, state->comment_buf_size + 32);
      if (state->comment_buf_len > 0)
	memcpy (buf, state->comment_buffer, state->comment_buf_len);
      state->comment_buf_size += 32;
      state->comment_buffer = buf;
    }
  state->comment_buffer[state->comment_buf_len++] = c;
}


/* Return the contents of the documentation comment buffer, and reset
   the buffer.  Return NULL if no comment was read before.  */
char *
ttl_get_comment_buffer (ttl_scanner state)
{
  if (state->comment_buf_len > 0)
    {
      char * buf = ttl_malloc (state->pool, state->comment_buf_len + 1);
      memcpy (buf, state->comment_buffer, state->comment_buf_len);
      buf[state->comment_buf_len] = '\0';
      reset_comment_buffer (state);
      return buf;
    }
  else
    return NULL;
}

/* Print an error message to stderr, including the current source file
   name and position, and the error message `msg'.  Also the scanning
   error counter in the scanner state is increased.  */
static void
report_error (ttl_scanner scanner, const char * msg)
{
  scanner->scan_errors++;
  fprintf (stderr, "%s:%d:%d: %s\n", scanner->filename, scanner->last_line + 1,
	   scanner->last_column + 1, msg);
}


/* Read the next character from the input file and place it in the
   token character buffer.  If the character is escaped, as in string
   and character constants, as much characters as necessary are read
   to determine the character value.  */
static void
ttl_add_string_char (ttl_scanner state)
{
  int c;

  if (state->current_char == '\\')
    {
      ttl_next_char (state);
      if (state->current_char == EOF)
	{
	  report_error (state, "unexpected end of file in constant");
	  /* Assign something to `c', so that we can continue in a
	     known state.  No invalid code will be generated, because
	     report_error() records that an error occurred for later
	     passes.  */
	  c = '!';
	}
      else
	{
	  c = state->current_char;
	  switch (c)
	    {
	    case '"':
	      c = '"';
	      break;
	    case '\'':
	      c = '\'';
	      break;
	    case '\\':
	      c = '\\';
	      break;
	    case 'n':
	      c = '\n';
	      break;
	    case 'r':
	      c = '\r';
	      break;
	    case 't':
	      c = '\t';
	      break;
	    case 'b':
	      c = '\b';
	      break;
	    case 'v':
	      c = '\v';
	      break;
	    case 'a':
	      c = '\a';
	      break;
	    case 'f':
	      c = '\f';
	      break;
	    default:
	      report_error (state, "invalid escape sequence");
	      break;
	    }
	}
    }
  else
    c = state->current_char;
  ttl_next_char (state);
  ttl_add_char (state, c);
}


/* NOTE: Keep this list sorted, it is searched using binary
   search.  */
static struct reserved
{
  char * ident;
  enum ttl_token_type token;
} reserved[] = {
  {"and", token_and},
  {"array", token_array},
  {"const", token_const},
  {"constraint", token_constraint},
  {"datatype", token_datatype},
  {"do", token_do},
  {"else", token_else},
  {"elsif", token_elsif},
  {"end", token_end},
  {"export", token_export},
  {"false", token_false},
  {"foreign", token_foreign},
  {"fun", token_fun},
  {"hd", token_hd},
  {"if", token_if},
  {"import", token_import},
  {"in", token_in},
  {"list", token_list},
  {"module", token_module},
  {"not", token_not},
  {"null", token_null},
  {"of", token_of},
  {"or", token_or},
/*   {"out", token_out}, */
  {"public", token_public},
  {"require", token_require},
  {"return", token_return},
  {"sizeof", token_sizeof},
  {"string", token_string},
  {"then", token_then},
  {"tl", token_tl},
  {"true", token_true},
  {"type", token_type},
  {"var", token_var},
  {"while", token_while}
};

#define NUM_RESERVED (sizeof (reserved) / sizeof (reserved[0]))


/* Read the next token from the input file and place the token class
   (and possibly token value) into the state variable.  */
void
ttl_next_token (ttl_scanner state)
{
  state->token_class = token_eof;
  state->token_value = NULL;

 rescan:			/* Restart scanning after comments.  */

  /* Skip all whitespace separating tokens.  */
  while (state->current_char == ' ' || state->current_char == '\t' ||
	 state->current_char == '\n' || state->current_char == '\r')
    ttl_next_char (state);

  /* After skipping initial whitespace, we can record the current
     position as the start position of the next token.  */
  state->begin_line = state->last_line;
  state->begin_column = state->last_column;
  while (1)
    {
      switch (state->current_char)
	{
	case EOF:
	  return;

	case '.':
	  ttl_next_char (state);
	  state->token_class = token_dot;
	  return;

	case ',':
	  ttl_next_char (state);
	  state->token_class = token_comma;
	  return;

	case '(':
	  ttl_next_char (state);
	  state->token_class = token_lparen;
	  return;

	case ')':
	  ttl_next_char (state);
	  state->token_class = token_rparen;
	  return;

	case '[':
	  ttl_next_char (state);
	  state->token_class = token_lbracket;
	  return;

	case ']':
	  ttl_next_char (state);
	  state->token_class = token_rbracket;
	  return;

	case '{':
	  ttl_next_char (state);
	  state->token_class = token_lbrace;
	  return;

	case '}':
	  ttl_next_char (state);
	  state->token_class = token_rbrace;
	  return;

	case ';':
	  ttl_next_char (state);
	  state->token_class = token_semicolon;
	  return;

	case '!':
	  ttl_next_char (state);
	  state->token_class = token_exclamation;
	  return;

	case '+':
	  ttl_next_char (state);
	  state->token_class = token_plus;
	  return;

	case '-':
	  ttl_next_char (state);
	  state->token_class = token_minus;
	  return;

	case '*':
	  ttl_next_char (state);
	  state->token_class = token_star;
	  return;

	case '/':
	  ttl_next_char (state);

	  /* One-line comment.  Skip until the next newline
	     character.  */
	  if (state->current_char == '/')
	    {
	      ttl_next_char (state);
	      if (state->current_char == '*')
		{
		  /* Documentation comment, collect the string.  */
		  ttl_next_char (state);
		  while (state->current_char == ' ' ||
			 state->current_char == '\t')
		    ttl_next_char (state);
		  while (state->current_char != EOF &&
			 state->current_char != '\n')
		    {
		      add_comment_char (state, state->current_char);
		      ttl_next_char (state);
		    }
		  add_comment_char (state, '\n');
		}
	      else
		{
		  while (state->current_char != EOF &&
			 state->current_char != '\n')
		    ttl_next_char (state);
		}
	      goto rescan;
	    }
	  else if (state->current_char == '*')
	    {
	      ttl_next_char (state);
	      if (state->current_char == '*')
		{
		  ttl_next_char (state);
		  if (state->current_char != '/')
		    {
		      while (state->current_char == ' ' ||
			     state->current_char == '\t')
			ttl_next_char (state);
		      do
			{
			  while (state->current_char != EOF &&
				 state->current_char != '*')
			    {
			      add_comment_char (state, state->current_char);
			      if (state->current_char == '\n')
				{
				  ttl_next_char (state);
				  while (state->current_char == ' ' ||
					 state->current_char == '\t')
				    ttl_next_char (state);
				}
			      else
				ttl_next_char (state);
			    }
			  while (state->current_char == '*')
			    ttl_next_char (state);
			}
		      while (state->current_char != EOF &&
			     state->current_char != '/');
		      add_comment_char (state, '\n');
		    }
		  ttl_next_char (state);
		}
	      else
		{
		  do
		    {
		      while (state->current_char != EOF &&
			     state->current_char != '*')
			ttl_next_char (state);
		      while (state->current_char == '*')
			ttl_next_char (state);
		    }
		  while (state->current_char != EOF &&
			 state->current_char != '/');
		  ttl_next_char (state);
		}
	      goto rescan;
	    }
	  state->token_class = token_slash;
	  return;

	case '%':
	  ttl_next_char (state);
	  state->token_class = token_percent;
	  return;

	case '=':
	  ttl_next_char (state);
	  state->token_class = token_eq;
	  return;

	case '<':
	  ttl_next_char (state);
	  if (state->current_char == '=')
	    {
	      ttl_next_char (state);
	      state->token_class = token_le;
	      return;
	    }
	  else if (state->current_char == '>')
	    {
	      ttl_next_char (state);
	      state->token_class = token_ne;
	      return;
	    }
	  state->token_class = token_lt;
	  return;

	case '>':
	  ttl_next_char (state);
	  if (state->current_char == '=')
	    {
	      ttl_next_char (state);
	      state->token_class = token_ge;
	      return;
	    }
	  state->token_class = token_gt;
	  return;

	case ':':
	  ttl_next_char (state);
	  if (state->current_char == '=')
	    {
	      ttl_next_char (state);
	      state->token_class = token_assign;
	      return;
	    }
	  else if (state->current_char == ':')
	    {
	      ttl_next_char (state);
	      state->token_class = token_cons;
	      return;
	    }
	  state->token_class = token_colon;
	  return;

	case '"':
	  ttl_reset_char_buffer (state);
	  ttl_next_char (state);

	  while (state->current_char != EOF && state->current_char != '\n' &&
		 state->current_char != '"')
	    ttl_add_string_char (state);

	  if (state->current_char == '"')
	    ttl_next_char (state);
	  else
	    report_error(state, "unterminated string constant");

	  state->token_class = token_string_const;
	  state->token_value = ttl_make_ast_string
	    (state->pool, state->buffer, state->buf_len,
	     state->filename,
	     state->begin_line, state->begin_column,
	     state->last_line, state->last_column);
	  return;

	case '\'':
	  ttl_reset_char_buffer (state);
	  ttl_next_char (state);
	  while (state->current_char != EOF && state->current_char != '\n' &&
		 state->current_char != '\'')
	    ttl_add_string_char (state);

	  if (state->current_char == '\'')
	    ttl_next_char (state);
	  else
	    report_error(state, "unterminated character constant");
	  if (state->buf_len < 1)
	    {
	      report_error (state, "empty character constant");
	      ttl_add_char (state, '!');
	    }
	  else if (state->buf_len > 1)
	    report_error (state, "character constant too long");
	  state->token_class = token_char_const;
	  state->token_value = ttl_make_ast_character
	    (state->pool, state->buffer[0],
	     state->filename,
	     state->begin_line, state->begin_column,
	     state->last_line, state->last_column);
	  return;

	case '0': case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
	  {
	    /* FIXME: 1e2 must parse as integer, not real!  */
	    int in_int = 1;
	    int is_long = 0;
	    int exp_seen = 0;
	    unsigned long res;
	    char * end_ptr;

	    ttl_reset_char_buffer (state);
	    while (1)
	      {
		switch (state->current_char)
		  {
		  case '0': case '1': case '2': case '3': case '4': case '5':
		  case '6': case '7': case '8': case '9':
		    ttl_add_char (state, state->current_char);
		    ttl_next_char (state);
		    break;

		  case 'e': case 'E':
		    if (exp_seen)
		      {
			report_error
			  (state, "multiple exponent marker in real constant");
			state->token_class = token_real_const;
			state->token_value = ttl_make_ast_real
			  (state->pool, 0.0,
			   state->filename,
			   state->begin_line, state->begin_column,
			   state->last_line, state->last_column);
			ttl_next_char (state);
			/* 			return; */
		      }
		    else
		      {
			ttl_add_char (state, state->current_char);
			ttl_next_char (state);
			exp_seen = 1;
		      }
		    break;

		  case 'L':
		    ttl_next_char (state);
		    is_long = 1;
		    if (!in_int)
		      {
			report_error
			  (state, "L suffix only allowed on integers");
			state->token_class = token_long_const;
			state->token_value = ttl_make_ast_long
			  (state->pool, 0,
			   state->filename,
			   state->begin_line, state->begin_column,
			   state->last_line, state->last_column);
			ttl_next_char (state);
		      }
		    goto analyze;
		    break;

		  case '.':
		    /* FIXME: Check whether an exponent was already
		       seen.  */
		    if (in_int)
		      {
			ttl_add_char (state, state->current_char);
			ttl_next_char (state);
			in_int = 0;
		      }
		    else
		      {
			report_error
			  (state, "multiple dots in numeric constant");
			state->token_class = token_real_const;
			state->token_value = ttl_make_ast_real
			  (state->pool, 0.0,
			   state->filename,
			   state->begin_line, state->begin_column,
			   state->last_line, state->last_column);
			ttl_next_char (state);
		      }
		    break;

		  default:
		    goto analyze;
		  }
	      }
	  analyze:
	    ttl_add_char (state, '\0');
	    res = strtoul (state->buffer, &end_ptr, 10);
	    if (*end_ptr == '\0')
	      {
		if (is_long)
		  {
		    state->token_class = token_long_const;
		    state->token_value = ttl_make_ast_long
		      (state->pool, (long) res,
		       state->filename,
		       state->begin_line, state->begin_column,
		       state->last_line, state->last_column);
		  }
		else
		  {
		    state->token_class = token_int_const;
		    state->token_value = ttl_make_ast_integer
		      (state->pool, (long) res,
		       state->filename,
		       state->begin_line, state->begin_column,
		       state->last_line, state->last_column);
		  }
	      }
	    else
	      {
		double res = strtod (state->buffer, &end_ptr);
		state->token_class = token_real_const;
		state->token_value = ttl_make_ast_real
		  (state->pool, res, 
		   state->filename,
		   state->begin_line, state->begin_column,
		   state->last_line, state->last_column);
	      }
	    return;
	  }
	  break;

	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	case 'Y': case 'Z':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	case 'y': case 'z':
	case '_':
	  ttl_reset_char_buffer (state);
	  while (1)
	    {
	      switch (state->current_char)
		{
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z':
		case '0': case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
		case '_':
		  ttl_add_char (state, state->current_char);
		  ttl_next_char (state);
		  break;

		case '?':
		case '!':
		  ttl_add_char (state, state->current_char);
		  ttl_next_char (state);
		  break;

		default:
		  /* NUL-terminate the string to enable string
		     comparing using strcmp() below.  */
		  ttl_add_char (state, '\0');
		  {
		    /* Do a binary search over the reserved words.  */
		    int l = 0;
		    int r = NUM_RESERVED;
		    int m;

		    while (l < r)
		      {
			int res;

			m = (l + r) / 2;
			res = strcmp (state->buffer, reserved[m].ident);
			if (res < 0)
			  r = m;
			else if (res > 0)
			  l = m + 1;
			else
			  {
			    state->token_class = reserved[m].token;
			    return;
			  }
		      }
		  }
		  state->token_class = token_identifier;
		  state->token_value = ttl_make_ast_identifier
		    (state->pool, 
		     ttl_symbol_enter (state->symbol_table,
				       state->buffer, state->buf_len - 1),
		     state->filename,
		     state->begin_line, state->begin_column,
		     state->last_line, state->last_column);
		  return;
		}
	    }
	  break;

	default:
	  state->scan_errors++;

	  fprintf (stderr, "%s:%d:%d: invalid character `%c' '%d'\n",
		   state->filename,
		   state->last_line + 1,
		   state->last_column + 1,
		   state->current_char,
		   (unsigned char) state->current_char);
	  ttl_next_char (state);
	  goto rescan;
	}      
    }
}


/* Put the character `c' back into the input stream of scanner state
   `state'.  Maintain the source code location, so that it will be
   correct when the character is read again.  */
static void
ttl_unget_char (ttl_scanner state, int c)
{
  if (state->pushed_back)
    {
      fprintf (stderr, "turtle: multiple unget char\n");
      abort ();
    }
  state->pushed_back = 1;
  state->pushed_back_char = c;
  state->current_line = state->last_line;
  state->current_column = state->last_column;
}


/* Advance the scanner state `state' to the next character in the
   input file, respecting a pushed back character.  Also, maintain the
   current source code location.  */
static void
ttl_next_char (ttl_scanner state)
{
  if (state->pushed_back)
    {
      state->pushed_back = 0;
      state->current_char = state->pushed_back_char;
    }
  else
    state->current_char = fgetc (state->f);
  state->last_line = state->current_line;
  state->last_column = state->current_column;
  switch (state->current_char)
    {
    case EOF:
      break;

    case '\n':
      state->current_line++;
      state->current_column = 0;
      break;

    case '\t':
      state->current_column += 8 - (state->current_column % 8);
      break;

    default:
      state->current_column++;
      break;
    }
}


/* End of scanner.c.  */
