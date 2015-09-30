/* libturtle/parser.c - Turtle parser
 
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


/* FIXME: Remember all source code positions where currently only -1
   is recorded in the AST (because of laziness).  */

#include "scanner.h"
#include "parser.h"


/* Return a true value if `node' is an error node, 0 otherwise.  */
static int 
error_node (ttl_ast_node node)
{
  if (!node)
    {
      fprintf (stderr, "turtle: NULL node in error_node()\n");
      abort ();
    }
  return node->kind == ast_error;
}


/* Report an error message including scanner's file name and source
   code position and the error message `msg' to standard error.  Also
   increase the number of parse errors in `scanner'.  */
static void
report_error (ttl_scanner scanner, const char * msg)
{
  scanner->parse_errors++;

  fprintf (stderr, "%s:%d:%d: %s\n",
	   scanner->filename, scanner->begin_line + 1,
	   scanner->begin_column + 1, msg);
}


/* Test whether the current token in `scanner' is in the same token
   class as `tok' and return 1 if it is, 0 otherwise.  In the error
   case, report an error to standard error and increase the number of
   parse errors in `scanner'.  Skip to the next token in any case, to
   guarantee termination.  */
static int
accept_token (ttl_scanner scanner, enum ttl_token_type tok)
{
  if (scanner->token_class == tok)
    {
      ttl_next_token (scanner);
      return 1;
    }
  else
    {
      scanner->parse_errors++;

      fprintf (stderr, "%s:%d:%d: unexpected token %s (expected was %s)\n",
	       scanner->filename, scanner->begin_line + 1,
	       scanner->begin_column + 1,
	       ttl_token_names[scanner->token_class], 
	       ttl_token_names[tok]);
      ttl_next_token (scanner);
      return 0;
    }
}


/* Return true iff the token class `class' may follow a statement.  */
static int
follows_statement (enum ttl_token_type class)
{
  return (class == token_end ||
	  class == token_else ||
	  class == token_elsif ||
	  class == token_var ||
	  class == token_fun ||
	  class == token_constraint ||
	  class == token_if ||
	  class == token_while ||
	  class == token_return ||
	  class == token_require ||
	  class == token_identifier);
}


/* Return true iff the token class `class' may follow a top-level
   declaration.  */
static int
follows_declaration (enum ttl_token_type class)
{
  return (class == token_var ||
	  class == token_fun ||
	  class == token_constraint ||
	  class == token_type ||
	  class == token_datatype ||
	  class == token_import ||
	  class == token_export ||
	  class == token_eof);
}


/* Try to match a semicolon token following a statement and emit an
   error message if not found.  Only skip the next token on success or
   if it can't start a new statement or end a statement sequence.  */
static int
accept_statement_semicolon (ttl_scanner scanner)
{
  if (scanner->token_class == token_semicolon)
    {
      ttl_next_token (scanner);
      return 1;
    }
  else
    {
      if (follows_statement (scanner->token_class))
	{
	  scanner->parse_errors++;

	  fprintf (stderr, "%s:%d:%d: missing semicolon\n",
		   scanner->filename, scanner->begin_line + 1,
		   scanner->begin_column + 1);
	  return 1;
	}
      else
	{
	  accept_token (scanner, token_semicolon);
	  return 0;
	}
    }
}


/* Try to match a semicolon token following a top-level declaration
   and emit an error message if not possible.  Only skip the current
   token on success or if it can't start a new declaration.  */
static int
accept_declaration_semicolon (ttl_scanner scanner)
{
  if (scanner->token_class == token_semicolon)
    {
      ttl_next_token (scanner);
      return 1;
    }
  else
    {
      if (follows_declaration (scanner->token_class))
	{
	  scanner->parse_errors++;

	  fprintf (stderr, "%s:%d:%d: missing semicolon\n",
		   scanner->filename, scanner->begin_line + 1,
		   scanner->begin_column + 1);
	  return 1;
	}
      else
	{
	  accept_token (scanner, token_semicolon);
	  return 0;
	}
    }
}

static ttl_ast_node parse_type (ttl_pool pool, ttl_scanner scanner);
static ttl_ast_node parse_stmt (ttl_pool pool, ttl_scanner scanner);
static ttl_ast_node parse_cons_expr (ttl_pool pool, ttl_scanner scanner);
static ttl_ast_node parse_tuple_expr (ttl_pool pool,
				      ttl_scanner scanner);
static ttl_ast_node parse_add_expr (ttl_pool pool, ttl_scanner scanner);
static ttl_ast_node parse_simple_expr (ttl_pool pool, ttl_scanner scanner);
static ttl_ast_node parse_compare_expr (ttl_pool pool,
					ttl_scanner scanner);
static ttl_ast_node parse_and_expr (ttl_pool pool,
					ttl_scanner scanner);
static ttl_ast_node parse_or_expr (ttl_pool pool,
					ttl_scanner scanner);


/* Parse a (possibly) qualified identifier.  Return either an
   `identifier' node (for unqualified identifiers), a `qualident' node
   (for qualified identifiers) or an `error' node, if a parse error
   occurred.  

   QualIdent ::= Ident {'.' Ident}
*/
static ttl_ast_node
parse_qualident (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node name = scanner->token_value;

  /* Parse the first identifier.  */
  if (!accept_token (scanner, token_identifier))
    return ttl_make_ast_error (pool);

  /* Parse all trailing identifiers, connected with dots.  */
  while (scanner->token_class == token_dot)
    {
      ttl_next_token (scanner);
      name = ttl_make_ast_qualident (pool, name, scanner->token_value, 
				     scanner->filename,
				     scanner->begin_line,
				     scanner->begin_column,
				     scanner->last_line,
				     scanner->last_column);
      if (!accept_token (scanner, token_identifier))
	return ttl_make_ast_error (pool);
    }
  return name;
}


static ttl_ast_node
parse_ident (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node name = scanner->token_value;

  /* Parse the first identifier.  */
  if (!accept_token (scanner, token_identifier))
    return ttl_make_ast_error (pool);
  return name;
}


/* Parse a qualified identifier, possibly followed by module
   parameters in angle brackets.  Returns either an identifier, a
   qualified identifier or an annotated (qualified) identifier.  */
static ttl_ast_node
parse_import_ident (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node ident;
  ttl_ast_node ann_list = NULL, * listp = &ann_list, open_list = NULL;

  ident = parse_qualident (pool, scanner);
  if (error_node (ident))
    return ident;
  if (scanner->token_class == token_lt)
    {
      ttl_ast_node type;

      ttl_next_token (scanner);

      type = parse_type (pool, scanner);
      if (error_node (type))
	return type;

      *listp = ttl_make_ast_pair (pool, type, NULL);
      listp = &((*listp)->d.pair.cdr);
      while (scanner->token_class != token_gt &&
	     scanner->token_class != token_eof)
	{
	  if (!accept_token (scanner, token_comma))
	    break;		/* Declare the list finished... */
	  type = parse_type (pool, scanner);
	  if (error_node (type))
	    return type;
	  
	  *listp = ttl_make_ast_pair (pool, type, NULL);
	  listp = &((*listp)->d.pair.cdr);
	}
      accept_token (scanner, token_gt);	/* Ignore error.  */
    }
  listp = &open_list;
  if (scanner->token_class == token_lparen)
    {
      ttl_ast_node name;

      ttl_next_token (scanner);

      name = parse_ident (pool, scanner);
      if (error_node (name))
	return name;

      *listp = ttl_make_ast_pair (pool, name, NULL);
      listp = &((*listp)->d.pair.cdr);
      while (scanner->token_class != token_rparen &&
	     scanner->token_class != token_eof)
	{
	  if (!accept_token (scanner, token_comma))
	    break;		/* Declare the list finished... */
	  name = parse_ident (pool, scanner);
	  if (error_node (name))
	    return name;
	  
	  *listp = ttl_make_ast_pair (pool, name, NULL);
	  listp = &((*listp)->d.pair.cdr);
	}
      accept_token (scanner, token_rparen);	/* Ignore error.  */
    }
  return ttl_make_ast_annotated_identifier (pool, ident, ann_list,
					    open_list,
					    NULL, -1, -1, -1, -1);
}

/* Parse an `import' declaration.  Return NULL if no import keyword
   was given or if a parse error occurred.  On success, return a list
   of (possibly) qualified identifiers.

   FIXME: Update grammar in documentation to include the import
   identifier annotations.

   Imports    ::= <empty>
               |  'import' ImportIdent {',' ImportIdent} ';'
   ImportIdent ::= QualIdent [ImportAnnotation] [OpenIdentList]
   ImportAnnotation ::= '<' Type {',' Type} '>'
   OpenIdentList ::= '(' Ident {',' Ident} ')'
*/
static ttl_ast_node
parse_module_imports (ttl_pool pool, ttl_scanner scanner)
{
  /* An `import' clause may be missing, so only parse it when the
     token `import' is present.  */
  if (scanner->token_class == token_import)
    {
      ttl_ast_node list = NULL;
      ttl_ast_node name;
      ttl_ast_node * p;

      /* Skip `import' token.  */
      ttl_next_token (scanner);

      /* Parse the name of the first imported module.  */
      name = parse_import_ident (pool, scanner);
      if (error_node (name))
	return NULL;
      list = ttl_make_ast_pair (pool, name, NULL);
      p = &(list->d.pair.cdr);

      /* Parse the remaining module names.  */
      while (scanner->token_class == token_comma)
	{
	  /* Skip `,' token.  */
	  ttl_next_token (scanner);

	  name = parse_import_ident (pool, scanner);
	  if (error_node (name))
	    return NULL;
	  *p = ttl_make_ast_pair (pool, name, NULL);
	  p = &((*p)->d.pair.cdr);
	}
      if (!accept_declaration_semicolon (scanner))
	return NULL;
      return list;
    }
  else
    return NULL;
}


/* Parse an `export' declaration.  Return NULL if no import keyword
   was given or if a parse error occurred.  On success, return a list
   of (possibly) qualified identifiers.

   Exports    ::= <empty>
               |  'export' QualIdent {, QualIdent} ';'
*/
static ttl_ast_node
parse_module_exports (ttl_pool pool, ttl_scanner scanner)
{
  /* Only parse an export clause if the token `export' is present,
     since the clause can be empty.  */
  if (scanner->token_class == token_export)
    {
      ttl_ast_node list = NULL;
      ttl_ast_node name;
      ttl_ast_node * p;

      /* Skip `export' token.  */
      ttl_next_token (scanner);

      /* Parse the name of the first exported identifier.  */
      name = parse_qualident (pool, scanner);
      if (error_node (name))
	return NULL;
      list = ttl_make_ast_pair (pool, name, NULL);
      p = &(list->d.pair.cdr);

      /* Parse the rest of the exported identifier names.  */
      while (scanner->token_class == token_comma)
	{
	  /* Skip `,' token.  */
	  ttl_next_token (scanner);

	  name = parse_qualident (pool, scanner);
	  if (error_node (name))
	    return NULL;
	  *p = ttl_make_ast_pair (pool, name, NULL);
	  p = &((*p)->d.pair.cdr);
	}
      if (!accept_declaration_semicolon (scanner))
	return NULL;
      return list;
    }
  else
    return NULL;
}


static ttl_ast_node parse_variable_list (ttl_pool pool, ttl_scanner scanner);

/* Parse a datatype variant.

   DatatypeVariant ::= Ident ['(' VariableList ')']
*/
static ttl_ast_node
parse_datatype_variant (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node variant;
  ttl_ast_node list;
  ttl_ast_node variant_name;

  variant_name = scanner->token_value;
  if (!accept_token (scanner, token_identifier))
    return ttl_make_ast_error (pool);

  if (scanner->token_class == token_lparen)
    {
      ttl_next_token (scanner);
      
      list = parse_variable_list (pool, scanner);
      
      /* Skip ')' token.  */
      if (!accept_token (scanner, token_rparen))
	return ttl_make_ast_error (pool);
    }
  else
    list = NULL;

  return ttl_make_ast_datatype_variant (pool, variant_name, list);
}

/* Parse a datatype declaration.

   DatatypeDecl ::= ['public'] 'datatype' Ident [TypeParams] '='
                    DatatypeVariant {'or' DatatypeVariant}
   TypeParams   ::= '<' Ident {',' Ident} '>'
*/
static ttl_ast_node
parse_datatype (ttl_pool pool, ttl_scanner scanner, int public)
{
  ttl_ast_node variant;
  ttl_ast_node list = NULL, * p = &list;
  ttl_ast_node types = NULL, * tp = &types;
  ttl_ast_node datatype_name;
  char * documentation;

  /* Skip 'datatype' token.  */
  ttl_next_token (scanner);
  
  documentation = ttl_get_comment_buffer (scanner);

  datatype_name = scanner->token_value;
  if (!accept_token (scanner, token_identifier))
    return ttl_make_ast_error (pool);

  if (scanner->token_class == token_lt)
    {
      ttl_ast_node type;

      ttl_next_token (scanner);
      type = parse_type (pool, scanner);
      /* FIXME: Check that type is a basic_type.  */
      if (error_node (type))
	return type;
      *tp = ttl_make_ast_pair (pool, type, NULL);
      tp = &((*tp)->d.pair.cdr);
      while (scanner->token_class == token_comma)
	{
	  ttl_next_token (scanner);
	  type = parse_type (pool, scanner);
	  /* FIXME: Check that type is a basic_type.  */
	  if (error_node (type))
	    return type;
	  *tp = ttl_make_ast_pair (pool, type, NULL);
	  tp = &((*tp)->d.pair.cdr);
	}
      if (!accept_token (scanner, token_gt))
	return ttl_make_ast_error (pool);
    }

  /* Skip '=' token.  */
  if (!accept_token (scanner, token_eq))
    return ttl_make_ast_error (pool);

  variant = parse_datatype_variant (pool, scanner);
  if (error_node (variant))
    return variant;

  *p = ttl_make_ast_pair (pool, variant, NULL);
  p = &((*p)->d.pair.cdr);
  while (scanner->token_class == token_or)
    {
      /* Skip the ',' token.  */
      ttl_next_token (scanner);

      variant = parse_datatype_variant (pool, scanner);
      if (error_node (variant))
	return variant;

      *p = ttl_make_ast_pair (pool, variant, NULL);
      p = &((*p)->d.pair.cdr);
    }
  return ttl_make_ast_datatype (pool, datatype_name, types, list, public,
				documentation);
}


/* Parse a formal parameter, that is an identifier followed by a `:'
   and a type expression.  An optional `out' modifier is parsed as
   well and the returned variable marked as an out parameter
   accordingly.  Return a `variable' node on success and an error node
   on parse failure.

   Parameter ::= Ident ':' Type 
*/
static ttl_ast_node
parse_parameter (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node name, type;
  int out = 0;
  int beg_line, beg_col;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;

  /* Check for parameter modifier.  */
/*   if (scanner->token_class == token_out) */
/*     { */
/*       ttl_next_token (scanner); */
/*       out = 1; */
/*     } */

  /* Parse the parameter name.  */
  name = scanner->token_value;
  if (!accept_token (scanner, token_identifier))
    return ttl_make_ast_error (pool);

  /* Skip the colon.  */
  if (!accept_token (scanner, token_colon))
    return ttl_make_ast_error (pool);

  /* Parse the parameter type.  */
  type = parse_type (pool, scanner);
  if (error_node (type))
    return type;

  return ttl_make_ast_variable (pool, name, type, NULL, out,
				scanner->filename,
				beg_line, beg_col,
				scanner->last_line, scanner->last_column);
}


/* Parse a formal parameter list.  Return a list of `variable' nodes
   on success and an NULL on failure.  

   ParameterList  ::= '(' [Parameter { ',' Parameter }] ')' 
*/
static ttl_ast_node
parse_param_list (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node param, list = NULL, * p = &list;

  if (!accept_token (scanner, token_lparen))
    return NULL;

  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_rparen)
    {
      param = parse_parameter (pool, scanner);
      if (error_node (param))
	return NULL;

      /* Add the parameter to the result list.  */
      *p = ttl_make_ast_pair (pool, param, NULL);
      p = &((*p)->d.pair.cdr);

      if (scanner->token_class != token_eof &&
	  scanner->token_class != token_rparen)
	{
	  if (!accept_token (scanner, token_comma))
	    return NULL;
	}
    }

  /* XXX: Better use accept_token() here?  */
  ttl_next_token (scanner);	/* Eat right paren.  */
  return list;
}


/* Parse a variable, as it appears in a `var' statement.  Return an
   error node on error and a `variable' node on success.  

   Variable       ::= Ident ':' Type [':= ConsExpression]
*/
static ttl_ast_node
parse_variable (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node name, type, init = NULL;
  int beg_line, beg_col;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;

  /* Parse the variable name.  */
  name = scanner->token_value;
  if (!accept_token (scanner, token_identifier))
    return ttl_make_ast_error (pool);

  /* Skip the colon.  */
  if (!accept_token (scanner, token_colon))
    return ttl_make_ast_error (pool);

  /* Parse the variable type.  */
  type = parse_type (pool, scanner);
  if (error_node (type))
    return type;
  if (scanner->token_class == token_assign)
    {
      ttl_next_token (scanner);
      init = parse_cons_expr (pool, scanner);
    }

  return ttl_make_ast_variable (pool, name, type, init, 0,
				scanner->filename,
				beg_line, beg_col,
				scanner->last_line, scanner->last_column);
}


/* Parse a variable list, as it appears in a `var' statement.  Returns
   a list of `var' nodes, or the empty list on error.  

   VariableList   ::= Variable {',' Variable}
*/
static ttl_ast_node
parse_variable_list (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node var, list = NULL, * p = &list;

  /* Parse the first variable.  (A variable list is always
     non-empty.)  */
  var = parse_variable (pool, scanner);
  if (error_node (var))
    return NULL;

  /* Add the variable to the result list.  */
  *p = ttl_make_ast_pair (pool, var, NULL);
  p = &((*p)->d.pair.cdr);

  /* Parse the remaining variables.  */
  while (scanner->token_class == token_comma)
    {
      ttl_next_token (scanner);	/* Skip the comma.  */

      var = parse_variable (pool, scanner);
      if (error_node (var))
	return NULL;

      /* Add the variable to the result list.  */
      *p = ttl_make_ast_pair (pool, var, NULL);
      p = &((*p)->d.pair.cdr);
    }
  return list;
}


/* Parse the body of a function or constraint.  Return a (possibly
   empty) list of statements.  When an error occurs during parsing a
   statement, this statement is omitted from the resulting list.  

   SubrBody       ::= { Statement } 'end'
*/
static ttl_ast_node
parse_subr_body (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node stmt, list = NULL, * p = &list;

  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_end)
    {
      stmt = parse_stmt (pool, scanner);
      if (error_node (stmt))
	continue;		/* Skip erroneous statements.   */

      *p = ttl_make_ast_pair (pool, stmt, NULL);
      p = &((*p)->d.pair.cdr);
    }

  /* The following will print and record an error, but we ignore the
     return value because there is no reason to propagate the error
     further.  */
  accept_token (scanner, token_end);
  return list;
}



/* Parse a type expression.

   TypeExpr       ::= QualIdent ['<' Type {',' Type} '>']
                   | '!' Type
                   |  '(' ')'
                   |  '(' Type {',' Type} ')'
                   |  'array' 'of' Type
                   |  'list' 'of' Type
                   |  'string'
                   |  'fun' '(' [Type {',' Type}] ')' [':' Type]
*/
static ttl_ast_node
parse_type_expr (ttl_pool pool, ttl_scanner scanner)
{
  switch (scanner->token_class)
    {
    case token_identifier:
      {
	ttl_ast_node types = NULL, * tp = &types;

	/* This is either a predefined or user-defined type.  */
	ttl_ast_node name = parse_qualident (pool, scanner);
	if (error_node (name))
	  return name;		/* Propagate error.  */

	if (scanner->token_class == token_lt)
	  {
	    ttl_ast_node type;

	    ttl_next_token (scanner);
	    type = parse_type (pool, scanner);
	    if (error_node (type))
	      return type;
	    *tp = ttl_make_ast_pair (pool, type, NULL);
	    tp = &((*tp)->d.pair.cdr);
	    while (scanner->token_class == token_comma)
	      {
		ttl_next_token (scanner);
		type = parse_type (pool, scanner);
		if (error_node (type))
		  return type;
		*tp = ttl_make_ast_pair (pool, type, NULL);
		tp = &((*tp)->d.pair.cdr);
	      }
	    if (!accept_token (scanner, token_gt))
	      return ttl_make_ast_error (pool);
	    return ttl_make_ast_user_type (pool, name, types,
					   scanner->filename,
					   name->start_line,
					   name->start_column,
					   name->end_line, name->end_column);
	  }

	return ttl_make_ast_basic_type (pool, name,
					scanner->filename,
					name->start_line, name->start_column,
					name->end_line, name->end_column);
      }

    case token_lparen:
      {
	ttl_ast_node type;
	ttl_ast_node list, * p = &list;
	int beg_line, beg_col, end_line, end_col;

	beg_line = scanner->begin_line;
	beg_col = scanner->begin_column;

	/* Skip opening parenthesis.  */
	ttl_next_token (scanner);

	/* A following closing parenthesis denotes the void type.  */
	if (scanner->token_class == token_rparen)
	  {
	    end_line = scanner->last_line;
	    end_col = scanner->last_column;
	    ttl_next_token (scanner);
	    return ttl_make_ast_void_type (pool, scanner->filename,
					   beg_line, beg_col,
					   end_line, end_col);
	  }

	/* A possible error node will be propagated below.  */
	type = parse_type (pool, scanner);

	if (scanner->token_class == token_rparen)
	  {
	    ttl_next_token (scanner);
	    return type;
	  }

	*p = ttl_make_ast_pair (pool, type, NULL);
	p = &((*p)->d.pair.cdr);
	while (scanner->token_class == token_comma)
	  {
	    ttl_next_token (scanner);
	    type = parse_type (pool, scanner);
	    if (error_node (type))
	      return type;
	    *p = ttl_make_ast_pair (pool, type, NULL);
	    p = &((*p)->d.pair.cdr);
	  }
	/* XXX: Skip the following test if parse_type() above already
	   reported an error?  */
	/* Skip closing parenthesis.  */
	if (!accept_token (scanner, token_rparen))
	  return ttl_make_ast_error (pool);

	end_line = scanner->last_line;
	end_col = scanner->last_column;
	return ttl_make_ast_tuple_type (pool, list, scanner->filename,
					beg_line, beg_col,
					end_line, end_col);
      }
      
    case token_array:
      {
	ttl_ast_node type;
	int beg_line, end_line, beg_col, end_col;

	beg_line = scanner->begin_line;
	beg_col = scanner->begin_column;

	/* Skip the `array' keyword.  */
	ttl_next_token (scanner);

	/* Skip the `of' token.  */
	if (!accept_token (scanner, token_of))
	  return ttl_make_ast_error (pool);

	/* Parse the base type of the array type.  */
	type = parse_type (pool, scanner);
	if (error_node (type))
	  return type;		/* Propagate error.  */

	end_line = scanner->last_line;
	end_col = scanner->last_column;
	return ttl_make_ast_array_type (pool, type, scanner->filename,
					beg_line, beg_col, 
					end_line, end_col);
      }

    case token_string:
      {
	ttl_ast_node type;
	int beg_line, end_line, beg_col, end_col;

	beg_line = scanner->begin_line;
	beg_col = scanner->begin_column;

	/* Skip the `string' keyword.  */
	ttl_next_token (scanner);

	end_line = scanner->last_line;
	end_col = scanner->last_column;
	return ttl_make_ast_string_type (pool, scanner->filename,
					 beg_line, beg_col,
					 end_line, end_col);
      }

    case token_list:
      {
	ttl_ast_node type;
	int beg_line, end_line, beg_col, end_col;

	beg_line = scanner->begin_line;
	beg_col = scanner->begin_column;


	/* Skip the `list' keyword.  */
	ttl_next_token (scanner);

	/* Skip the `of' keyword.  */
	if (!accept_token (scanner, token_of))
	  return ttl_make_ast_error (pool);

	/* Paarse the base type of the list type.  */
	type = parse_type (pool, scanner);
	if (error_node (type))
	  return type;		/* Propagate error.  */

	end_line = scanner->last_line;
	end_col = scanner->last_column;
	return ttl_make_ast_list_type (pool, type, scanner->filename,
				       beg_line, beg_col,
				       end_line, end_col);
      }

    case token_fun:
      {
	ttl_ast_node left, right, list = NULL, * p = &list;
	int beg_line, end_line, beg_col, end_col;

	beg_line = scanner->begin_line;
	beg_col = scanner->begin_column;

	/* Skip the `fun' keyword.  */
	ttl_next_token (scanner);
	/* Skip the opening parenthesis.  */
	if (!accept_token (scanner, token_lparen))
	  return ttl_make_ast_error (pool);

	/* Parse the optional parameter types.  */
	while (scanner->token_class != token_eof &&
	       scanner->token_class != token_rparen)
	  {
	    left = parse_type (pool, scanner);
	    if (error_node (left))
	      return left;	/* Propagate error.  */

	    *p = ttl_make_ast_pair (pool, left, NULL);
	    p = &((*p)->d.pair.cdr);

	    /* Skip the `comma' token.  */
	    if (scanner->token_class != token_eof &&
		scanner->token_class != token_rparen)
	      accept_token (scanner, token_comma);
	  }
	/* Skip the closing parenthesis.  */
	if (!accept_token (scanner, token_rparen))
	  return ttl_make_ast_error (pool);

	if (scanner->token_class == token_colon)
	  {
	    ttl_next_token (scanner);

	    /* Parse the return type of the function type.  */
	    right = parse_type (pool, scanner);
	    if (error_node (right))
	      return right;		/* Propagate error.  */
	  }
	else
	  right = NULL;

	end_line = scanner->last_line;
	end_col = scanner->last_column;
	return ttl_make_ast_function_type (pool, list, right,
					   scanner->filename,
					   beg_line, beg_col,
					   end_line, end_col);
      }

    case token_exclamation:
      {
	ttl_ast_node base;
	int beg_line, end_line, beg_col, end_col;

	beg_line = scanner->begin_line;
	beg_col = scanner->begin_column;

	/* Skip `!' token.  */
	ttl_next_token (scanner);

	base = parse_type (pool, scanner);

	end_line = scanner->last_line;
	end_col = scanner->last_column;
	return ttl_make_ast_constrained_type (pool, base,
					      scanner->filename,
					      beg_line, beg_col,
					      end_line, end_col);
      }

    default:
      /* Skip erroneous token, to guarantee progress.  */
      ttl_next_token (scanner);
      report_error (scanner, "type expression expected");
      return ttl_make_ast_error (pool);
    }
}


/* Parse a tuple type.

   TupleType      ::= Type {'*' Type}
*/
static ttl_ast_node
parse_tuple_type (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node left, right, list = NULL, * p;
  int beg_line, end_line, beg_col, end_col;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;

  /* Parse the first type expression.  If there are no following `*'
     tokens, this will be the resulting type.  */
  left = parse_type_expr (pool, scanner);
  if (error_node (left))
    return left;		/* Propagate error.   */

  /* When a type expression is followed by a `*' token, this is a
     tuple type.  */
  if (scanner->token_class == token_star)
    {
      /* Put the first type expression in the result list.  */
      list = ttl_make_ast_pair (pool, left, NULL);
      p = &(list->d.pair.cdr);

      /* Parse all remainting type expressions.  */
      while (scanner->token_class == token_star)
	{
	  /* Skip the `*' token.  */
	  ttl_next_token (scanner);
	  
	  /* Parse the next type expression.  */
	  right = parse_type_expr (pool, scanner);
	  if (error_node (right))
	    return right;	/* Propagate error.  */

	  *p = ttl_make_ast_pair (pool, right, NULL);
	  p = &((*p)->d.pair.cdr);
	}
      end_line = scanner->last_line;
      end_col = scanner->last_column;
      return ttl_make_ast_tuple_type (pool, list, scanner->filename,
				      beg_line, beg_col,
				      end_line, end_col);
    }
  else
    return left;
}

/* Parse a type.

   Type           ::= TupleType  
*/
static ttl_ast_node
parse_type (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node left;

  left = parse_tuple_type (pool, scanner);
  return left;
}


/* Parse a function declaration.  

   FunDecl        ::= ['public'] 'fun' Ident ParameterList [':' Type] SubrBody
*/
static ttl_ast_node
parse_fundef (ttl_pool pool, ttl_scanner scanner, int public)
{
  int beg_line, beg_col, end_line, end_col;
  ttl_ast_node name, params, type, body;
  unsigned handcoded = 0;
  unsigned mapped = 0;
  ttl_ast_node ast_alias;
  ttl_symbol alias = NULL;
  char * documentation;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;
  documentation = ttl_get_comment_buffer (scanner);

  /* Skip the `fun' token.  */
  ttl_next_token (scanner);

  /* Parse the function name.  */
  name = scanner->token_value;
  if (!accept_token (scanner, token_identifier))
    return ttl_make_ast_error (pool);

  /* Parse the parameter list.  (It will be empty if an error
     occured.)  */
  params = parse_param_list (pool, scanner);

  /* Parse the function's optional return type.  */
  if (scanner->token_class == token_colon)
    {
      if (!accept_token (scanner, token_colon))
	return ttl_make_ast_error (pool);
      type = parse_type (pool, scanner);
      if (error_node (type))
	return type;
    }
  else
    type = NULL;
  if (scanner->token_class == token_semicolon)
    {
      handcoded = 1;
      body = NULL;
    }
  else if (scanner->token_class == token_eq)
    {
      ttl_next_token (scanner);
      body = NULL;
      mapped = 1;
      ast_alias = scanner->token_value;
      if (!accept_token (scanner, token_string_const))
	return ttl_make_ast_error (pool);
      alias = ttl_symbol_enter (scanner->symbol_table,
				ast_alias->d.string.value,
				ast_alias->d.string.length);
    }
  else
    body = parse_subr_body (pool, scanner);

  end_line = scanner->last_line;
  end_col = scanner->last_column;

  {
    ttl_ast_node fun;

    fun = ttl_make_ast_function (pool, name, params, type, body, public,
				 handcoded, 
				 documentation,
				 scanner->filename,
				 beg_line, beg_col, end_line, end_col);
    fun->d.function.mapped = mapped;
    fun->d.function.alias = alias;
    return fun;
  }
}


/* Parse a constraint declaration.  

   ConstraintDecl ::= ['public'] 'constraint' Ident ParameterList SubrBody
*/
static ttl_ast_node
parse_constraintdef (ttl_pool pool, ttl_scanner scanner, int public)
{
  int beg_line, beg_col, end_line, end_col;
  ttl_ast_node name, params, body;
  char * documentation;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;
  documentation = ttl_get_comment_buffer (scanner);

  ttl_next_token (scanner);
  name = scanner->token_value;
  if (!accept_token (scanner, token_identifier))
    return ttl_make_ast_error (pool);
  params = parse_param_list (pool, scanner);
  body = parse_subr_body (pool, scanner);

  end_line = scanner->last_line;
  end_col = scanner->last_column;

  return ttl_make_ast_constraint (pool, name, params, body, public,
				  documentation,
				  scanner->filename, beg_line, beg_col,
				  end_line, end_col);
}


/* Parse a type declaration.  

   TypeDecl       ::= ['public'] 'type' Ident = Type
*/
static ttl_ast_node
parse_typedef (ttl_pool pool, ttl_scanner scanner, int public)
{
  ttl_ast_node name, type;
  char * documentation = ttl_get_comment_buffer (scanner);

  /* Skip the `type' keyword.  */
  ttl_next_token (scanner);

  /* Record the name of the new type.  */
  name = scanner->token_value;
  if (!accept_token (scanner, token_identifier))
    return ttl_make_ast_error (pool);

  /* Skip the `=' token.  */
  if (!accept_token (scanner, token_eq))
    return ttl_make_ast_error (pool);

  /* Parse the type expression.  */
  type = parse_type (pool, scanner);
  return ttl_make_ast_typedef (pool, name, type, public, documentation);
}


/* Parse a variable declaration.  

   VarDecl        ::= ['public'] 'var' VariableList
*/
static ttl_ast_node
parse_vardef (ttl_pool pool, ttl_scanner scanner, int public)
{
  ttl_ast_node list;
  int beg_line, beg_col;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;

  ttl_next_token (scanner);
  list = parse_variable_list (pool, scanner);
  return ttl_make_ast_vardef (pool, list, public,
			      ttl_get_comment_buffer (scanner),
			      scanner->filename, beg_line, beg_col,
			      scanner->last_line, scanner->last_column);
}

/* Parse a constant declaration.  

   ConstDecl        ::= ['public'] 'const' VariableList
*/
static ttl_ast_node
parse_constdef (ttl_pool pool, ttl_scanner scanner, int public)
{
  ttl_ast_node list;
  int beg_line, beg_col;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;

  ttl_next_token (scanner);
  list = parse_variable_list (pool, scanner);
  return ttl_make_ast_constdef (pool, list, public,
				ttl_get_comment_buffer (scanner),
				scanner->filename, beg_line, beg_col,
				scanner->last_line, scanner->last_column);
}

/* Parse a function expression.

   FunExpression  ::= 'fun' ParameterList [':' TypeExpr] SubrBody
*/
static ttl_ast_node
parse_fun_expr (ttl_pool pool, ttl_scanner scanner)
{
  int beg_line, beg_col, end_line, end_col;
  ttl_ast_node name, params, type, body;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;

  /* Skip 'fun' keyword.  */
  ttl_next_token (scanner);

  /* Parse formal parameter list.  */
  params = parse_param_list (pool, scanner);

  if (scanner->token_class == token_colon)
    {
      ttl_next_token (scanner);

      /* Parse return type.  */
      type = parse_type (pool, scanner);
      if (error_node (type))
	return type;
    }
  else
    type = NULL;

  /* Parse the function body.  */
  body = parse_subr_body (pool, scanner);

  end_line = scanner->last_line;
  end_col = scanner->last_column;

  return ttl_make_ast_function (pool, NULL, params, type, body,
				0, /* Not public.  */
				0, /* Not handcoded.  */
				ttl_get_comment_buffer (scanner),
				scanner->filename, beg_line, beg_col,
				end_line, end_col);
}

/* Parse a constraint expression.

   ConstraintExpression ::= 'constraint' ParameterList SubrBody
*/
static ttl_ast_node
parse_constraint_expr (ttl_pool pool, ttl_scanner scanner)
{
  int beg_line, beg_col, end_line, end_col;
  ttl_ast_node name, params, type, body;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;

  /* Skip 'constraint' keyword.  */
  ttl_next_token (scanner);

  /* Parse formal parameter list.  */
  params = parse_param_list (pool, scanner);

  /* Parse the constraint body.  */
  body = parse_subr_body (pool, scanner);

  end_line = scanner->last_line;
  end_col = scanner->last_column;

  return ttl_make_ast_constraint (pool, NULL, params, body,
				  0, /* Not public.  */
				  NULL, /* No documentation.  */
				  scanner->filename, beg_line, beg_col,
				  end_line, end_col);
}


/* Parse an array expression.

   ArrayExpression           ::= '{' [{ConsExpression}
                                 {',' ConsExpression} '}'
*/
static ttl_ast_node
parse_array_expression (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr, list = NULL, * p = &list;

  ttl_next_token (scanner);
  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_rbrace)
    {
      expr = parse_cons_expr (pool, scanner);
      if (error_node (expr))
	return NULL;
      *p = ttl_make_ast_pair (pool, expr, NULL);
      p = &((*p)->d.pair.cdr);
      if (scanner->token_class != token_eof &&
	  scanner->token_class != token_rbrace)
	{
	  if (!accept_token (scanner, token_comma))
	    return NULL;
	}
    }
  ttl_next_token (scanner);	/* Eat right paren.  */
  return list;
}


/* Parse a list expression.

   ListExpression ::= '[' [ConsExpression {',' ConsExpression}] ']'
*/
static ttl_ast_node
parse_list_expression (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr, list = NULL, * p = &list;

  ttl_next_token (scanner);
  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_rbracket)
    {
      expr = parse_cons_expr (pool, scanner);
      if (error_node (expr))
	return NULL;
      *p = ttl_make_ast_pair (pool, expr, NULL);
      p = &((*p)->d.pair.cdr);
      if (scanner->token_class != token_rbracket)
	{
	  if (!accept_token (scanner, token_comma))
	    return NULL;
	}
    }
  ttl_next_token (scanner);	/* Eat right paren.  */
  return list;
}


/* Parse an atomic expression.  An atomic expression is a qualified
   identifier, a literal, a parenthesized expression or a
   function/constraint expression.

   AtomicExpression     ::= QualIdent
                         |  IntConst | RealConst | StringConst | CharConst
                         |  BoolConst | LongConst | ArrayExpr | ListExpr
                         |  FunExpression | ConstraintExpression
                         |  'array' AddExpression 'of' TupleExpression
                         |  'list' AddExpression 'of' TupleExpression
                         |  'string' AddExpression ['of' SimpleExpression]
                         |  '(' TupleExpression ')'
                         |  'null'
                         |  'var' AtomicExpression
                         |  '!' AtomicExpression
                         |  'foreign' StringConst
   BoolConst           ::= 'false' | 'true'
*/
static ttl_ast_node
parse_atomic_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr;
  int beg_line, beg_col;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;
  switch (scanner->token_class)
    {
    case token_foreign:
      ttl_next_token (scanner);
      expr = scanner->token_value;
      if (!accept_token (scanner, token_string_const))
	return ttl_make_ast_error (pool);
      return ttl_make_ast_foreign_expr
	(pool, 
	 ttl_symbol_enter (scanner->symbol_table, expr->d.string.value,
			   expr->d.string.length),
	 scanner->filename, beg_line, beg_col,
	 scanner->last_line, scanner->last_column);
      
    case token_var:
      ttl_next_token (scanner);
      expr = parse_atomic_expr (pool, scanner);
      if (error_node (expr)) 
	return expr;
      return ttl_make_ast_var_expr
	(pool, 
	 expr,
	 scanner->filename, beg_line, beg_col,
	 scanner->last_line, scanner->last_column);
      
    case token_exclamation:
      ttl_next_token (scanner);
      expr = parse_atomic_expr (pool, scanner);
      if (error_node (expr)) 
	return expr;
      return ttl_make_ast_deref_expr
	(pool, 
	 expr,
	 scanner->filename, beg_line, beg_col,
	 scanner->last_line, scanner->last_column);
      
    case token_identifier:
      return parse_qualident (pool, scanner);

    case token_int_const:
    case token_long_const:
    case token_string_const:
    case token_char_const:
    case token_real_const:
      expr = scanner->token_value;
      ttl_next_token (scanner);
      return expr;

    case token_false:
      expr = ttl_make_ast_bool (pool, 0, 
				scanner->filename,
				scanner->begin_line,
				scanner->begin_column,
				scanner->last_line,
				scanner->last_column);
      ttl_next_token (scanner);
      return expr;

    case token_true:
      expr = ttl_make_ast_bool (pool, 1,
				scanner->filename,
				scanner->begin_line,
				scanner->begin_column,
				scanner->last_line,
				scanner->last_column);
      ttl_next_token (scanner);
      return expr;

    case token_null:
      expr = ttl_make_ast_null (pool, 
				scanner->filename,
				scanner->begin_line,
				scanner->begin_column,
				scanner->last_line,
				scanner->last_column);
      ttl_next_token (scanner);
      return expr;

    case token_lparen:

      ttl_next_token (scanner);
      expr = parse_tuple_expr (pool, scanner);
      if (!accept_token (scanner, token_rparen))
	return ttl_make_ast_error (pool);
      return expr;

    case token_lbrace:
      expr = parse_array_expression (pool, scanner);
      return ttl_make_ast_array_expr (pool, expr, 
				      scanner->filename,
				      beg_line, beg_col,
				      scanner->last_line,
				      scanner->last_column);

    case token_lbracket:
      expr = parse_list_expression (pool, scanner);
      return ttl_make_ast_list_expr (pool, expr,
				     scanner->filename,
				     beg_line, beg_col,
				     scanner->last_line, scanner->last_column);

    case token_fun:
      expr = parse_fun_expr (pool, scanner);
      return expr;

    case token_constraint:
      expr = parse_constraint_expr (pool, scanner);
      return expr;

    case token_array:
      {
	ttl_ast_node size, initial;

	ttl_next_token (scanner);
	size = parse_add_expr (pool, scanner);
	if (error_node (size))
	  return size;

	/* Currently, the 'of' part cannot be omitted.  */
	if (!accept_token (scanner, token_of))
	  return ttl_make_ast_error (pool);

	initial = parse_tuple_expr (pool, scanner);
	if (error_node (initial))
	  return initial;

	return ttl_make_ast_array_constructor (pool, size, initial,
					  scanner->filename,
					  beg_line, beg_col,
					  scanner->last_line,
					  scanner->last_column);
      }
      
    case token_list:
      {
	ttl_ast_node size, initial;

	ttl_next_token (scanner);
	size = parse_add_expr (pool, scanner);
	if (error_node (size))
	  return size;

	/* Currently, the 'of' part cannot be omitted.  */
	if (!accept_token (scanner, token_of))
	  return ttl_make_ast_error (pool);

	initial = parse_tuple_expr (pool, scanner);
	if (error_node (initial))
	  return initial;
	return ttl_make_ast_list_constructor (pool, size, initial,
					      scanner->filename,
					      beg_line, beg_col,
					      scanner->last_line,
					      scanner->last_column);
      }
      
    case token_string:
      {
	ttl_ast_node size, initial;

	ttl_next_token (scanner);
	size = parse_add_expr (pool, scanner);
	if (error_node (size))
	  return size;

	if (scanner->token_class == token_of)
	  {
	    ttl_next_token (scanner);

	    initial = parse_simple_expr (pool, scanner);
	    if (error_node (initial))
	      return initial;
	  }
	else
	  initial = NULL;
	return ttl_make_ast_string_constructor (pool, size, initial,
						scanner->filename,
						beg_line, beg_col,
						scanner->last_line,
						scanner->last_column);
      }
      
    default:
      report_error (scanner, "expression expected");
      return ttl_make_ast_error (pool);
    }
}


/* Parse an actual parameter list.

   ActualParameters ::= '(' [ConsExpression {',' ConsExpression}] ')'
*/
static ttl_ast_node
parse_actual_params (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr, list = NULL, * p = &list;

  /* Skip opening parenthesis.  */
  ttl_next_token (scanner);

  /* Parse zero or more cons expressions.  */
  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_rparen)
    {
      expr = parse_cons_expr (pool, scanner);
      if (error_node (expr))
	return NULL;

      *p = ttl_make_ast_pair (pool, expr, NULL);
      p = &((*p)->d.pair.cdr);

      /* Skip `,' token.  */
      if (scanner->token_class != token_eof &&
	  scanner->token_class != token_rparen)
	{
	  if (!accept_token (scanner, token_comma))
	    return NULL;
	}
    }
  ttl_next_token (scanner);	/* Eat right paren.  */
  return list;
}


/* Parse an array index.

   Index    ::= '[' AddExpression ']'
*/
static ttl_ast_node
parse_index (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr;

  /* Skip '['.  */
  ttl_next_token (scanner);

  /* Parse the first expression.  */
  expr = parse_add_expr (pool, scanner);

  /* Skip ']'.  */
  if (!accept_token (scanner, token_rbracket))
    return ttl_make_ast_error (pool);

  return expr;
}


/* Parse a simple expression.  This is an atomic expression followed
   by zero or more function applications and/or an array index.

   SimpleExpression ::= AtomicExpression {ActualParameters | Index}
*/
static ttl_ast_node
parse_simple_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr, params, index;
  expr = parse_atomic_expr (pool, scanner);
  if (error_node (expr))
    return expr;
  while (scanner->token_class == token_lparen ||
	 scanner->token_class == token_lbracket)
    {
      switch (scanner->token_class)
	{
	case token_lparen:
	  params = parse_actual_params (pool, scanner);
	  expr = ttl_make_ast_call (pool, expr, params,
				    scanner->filename,
				    expr->start_line,
				    expr->start_column,
				    scanner->last_line,
				    scanner->last_column);
	  break;
	case token_lbracket:
	  index = parse_index (pool, scanner);
	  expr = ttl_make_ast_index (pool, expr, index,
				     scanner->filename,
				     expr->start_line,
				     expr->start_column,
				     scanner->last_line,
				     scanner->last_column);
	  break;
	default:
	  abort ();
	  break;
	}
    }
  return expr;
}


/* Parse a factor expression.  A factor is a simple expression with an
   optional unary prefix operator.

   Factor   ::= SimpleExpression
             |  '-' Factor
             |  'not' Factor
             | 'hd' Factor
             | 'tl' Factor
             | 'sizeof' Factor
*/
static ttl_ast_node
parse_factor_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr;
  int beg_line = scanner->begin_line;
  int beg_col = scanner->begin_column;
  int end_line = scanner->last_line;
  int end_col = scanner->last_column;

  if (scanner->token_class == token_minus)
    {
      ttl_next_token (scanner);
      expr = parse_factor_expr (pool, scanner);
      if (error_node (expr))
	return expr;
      return ttl_make_ast_unop (pool, unop_neg, expr,
				scanner->filename,
				beg_line, beg_col, end_line, end_col);
    }
  else if (scanner->token_class == token_not)
    {
      ttl_next_token (scanner);
      expr = parse_factor_expr (pool, scanner);
      if (error_node (expr))
	return expr;
      return ttl_make_ast_unop (pool, unop_not, expr,
				scanner->filename,
				beg_line, beg_col, end_line, end_col);
    }
  else if (scanner->token_class == token_hd)
    {
      ttl_next_token (scanner);
      expr = parse_factor_expr (pool, scanner);
      if (error_node (expr))
	return expr;
      return ttl_make_ast_unop (pool, unop_hd, expr,
				scanner->filename,
				beg_line, beg_col, end_line, end_col);
    }
  else if (scanner->token_class == token_tl)
    {
      ttl_next_token (scanner);
      expr = parse_factor_expr (pool, scanner);
      if (error_node (expr))
	return expr;
      return ttl_make_ast_unop (pool, unop_tl, expr,
				scanner->filename,
				beg_line, beg_col, end_line, end_col);
    }
  else if (scanner->token_class == token_sizeof)
    {
      ttl_next_token (scanner);
      expr = parse_factor_expr (pool, scanner);
      if (error_node (expr))
	return expr;
      return ttl_make_ast_unop (pool, unop_sizeof, expr,
				scanner->filename,
				beg_line, beg_col, end_line, end_col);
    }
  else 
    {
      expr = parse_simple_expr (pool, scanner);
      if (error_node (expr))
	return expr;
      return expr;
    }
}


/* Parse a multiplication expression.

   MulExpression    ::= Factor {MulOp Factor}
*/
static ttl_ast_node
parse_mul_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr, * p;

  expr = parse_factor_expr (pool, scanner);
  if (error_node (expr))
    return expr;
  while (scanner->token_class == token_star ||
	 scanner->token_class == token_slash ||
	 scanner->token_class == token_percent)
    {
      int beg_line = scanner->begin_line;
      int beg_col = scanner->begin_column;
      int end_line = scanner->last_line;
      int end_col = scanner->last_column;
      ttl_ast_node expr2;
      enum ttl_token_type type = scanner->token_class;
      enum ttl_binop op;

      ttl_next_token (scanner);
      expr2 = parse_factor_expr (pool, scanner);
      if (error_node (expr2))
	return expr2;
      
      switch (type)
	{
	case token_star: op = binop_mul; break;
	case token_slash: op = binop_div; break;
	case token_percent: op = binop_mod; break;
	default:
	  abort ();
	  break;
	}
      expr = ttl_make_ast_binop (pool, op, expr, expr2, scanner->filename,
				 beg_line, beg_col, end_line, end_col);
    }
  return expr;
}


/* Parse an addition expression.

   AddExpression   ::= MulExpression {AddOp MulExpression}
*/
static ttl_ast_node
parse_add_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr, * p;
  expr = parse_mul_expr (pool, scanner);
  if (error_node (expr))
    return expr;
  while (scanner->token_class == token_plus ||
	 scanner->token_class == token_minus)
    {
      int beg_line = scanner->begin_line;
      int beg_col = scanner->begin_column;
      int end_line = scanner->last_line;
      int end_col = scanner->last_column;
      ttl_ast_node expr2;
      enum ttl_token_type type = scanner->token_class;
      enum ttl_binop op;

      ttl_next_token (scanner);
      expr2 = parse_mul_expr (pool, scanner);
      if (error_node (expr2))
	return expr2;
      
      switch (type)
	{
	case token_plus: op = binop_add; break;
	case token_minus: op = binop_sub; break;
	default:
	  abort ();
	  break;
	}
      expr = ttl_make_ast_binop (pool, op, expr, expr2, scanner->filename,
				 beg_line, beg_col, end_line, end_col);
    }
  return expr;
}


/* Parse a comparison expression.

   CompareExpression ::= AddExpression [CompareOp AddExpression]
*/
static ttl_ast_node
parse_compare_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr;

  expr = parse_add_expr (pool, scanner);
  if (error_node (expr))
    return expr;
  if (scanner->token_class == token_eq ||
	 scanner->token_class == token_ne ||
	 scanner->token_class == token_lt ||
	 scanner->token_class == token_le ||
	 scanner->token_class == token_gt ||
	 scanner->token_class == token_ge)
    {
      int beg_line = scanner->begin_line;
      int beg_col = scanner->begin_column;
      int end_line = scanner->last_line;
      int end_col = scanner->last_column;
      enum ttl_token_type type = scanner->token_class;
      enum ttl_binop op;
      ttl_ast_node expr2;

      ttl_next_token (scanner);
      expr2 = parse_add_expr (pool, scanner);
      if (error_node (expr2))
	return expr2;
      
      switch (type)
	{
	case token_eq: op = binop_eq; break;
	case token_ne: op = binop_ne; break;
	case token_lt: op = binop_lt; break;
	case token_le: op = binop_le; break;
	case token_gt: op = binop_gt; break;
	case token_ge: op = binop_ge; break;
	default: abort ();
	}
      expr = ttl_make_ast_binop (pool, op, expr, expr2,
				 scanner->filename,
				 beg_line, beg_col,
				 end_line, end_col);
    }
  return expr;
}


/* Parse a boolean `and' expression.

   AndExpression   ::= CompareExpression {'and' CompareExpression}
*/
static ttl_ast_node
parse_and_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr, * p;
  expr = parse_compare_expr (pool, scanner);
  if (error_node (expr))
    return expr;
  while (scanner->token_class == token_and)
    {
      int beg_line = scanner->begin_line;
      int beg_col = scanner->begin_column;
      int end_line = scanner->last_line;
      int end_col = scanner->last_column;
      ttl_ast_node expr2;
      enum ttl_token_type type = scanner->token_class;

      ttl_next_token (scanner);
      expr2 = parse_compare_expr (pool, scanner);
      if (error_node (expr2))
	return expr2;
      
      expr = ttl_make_ast_binop (pool, binop_and, expr, expr2,
				 scanner->filename,
				 beg_line, beg_col, end_line, end_col);
    }
  return expr;
}


/* Parse a boolean `or' expression.

   OrExpression   ::= AndExpression {'or' AndExpression}
*/
static ttl_ast_node
parse_or_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr, * p;
  expr = parse_and_expr (pool, scanner);
  if (error_node (expr))
    return expr;
  while (scanner->token_class == token_or)
    {
      int beg_line = scanner->begin_line;
      int beg_col = scanner->begin_column;
      int end_line = scanner->last_line;
      int end_col = scanner->last_column;
      ttl_ast_node expr2;
      enum ttl_token_type type = scanner->token_class;

      ttl_next_token (scanner);
      expr2 = parse_and_expr (pool, scanner);
      if (error_node (expr2))
	return expr2;
      
      expr = ttl_make_ast_binop (pool, binop_or, expr, expr2,
				 scanner->filename,
				 beg_line, beg_col, end_line, end_col);
    }
  return expr;
}


/* Parse a list CONStruction expression.

   ConsExpression ::= OrExpression ['::' ConsExpression]
*/
static ttl_ast_node
parse_cons_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr;

  expr = parse_or_expr (pool, scanner);
  if (error_node (expr))
    return expr;
  if (scanner->token_class == token_cons)
    {
      int beg_line = scanner->begin_line;
      int beg_col = scanner->begin_column;
      int end_line = scanner->last_line;
      int end_col = scanner->last_column;
      ttl_ast_node expr2;

      ttl_next_token (scanner);
      expr2 = parse_cons_expr (pool, scanner);
      if (error_node (expr2))
	return expr2;
      
      expr = ttl_make_ast_binop (pool, binop_cons, expr, expr2,
				 scanner->filename,
				 beg_line, beg_col,
				 end_line, end_col);
    }
  return expr;
}


/* Parse a tuple expression.

   TupleExpression ::= ConsExpression {',' ConsExpression} 
*/
static ttl_ast_node
parse_tuple_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr;
  int beg_line = scanner->begin_line;
  int beg_col = scanner->begin_column;
  int end_line = scanner->last_line;
  int end_col = scanner->last_column;

  expr = parse_cons_expr (pool, scanner);
  if (error_node (expr))
    return expr;
  if (scanner->token_class == token_comma)
    {
      ttl_ast_node list = NULL, * p = &list;

      *p = ttl_make_ast_pair (pool, expr, NULL);
      p = &((*p)->d.pair.cdr);
      while (scanner->token_class == token_comma)
	{
	  ttl_next_token (scanner);

	  expr = parse_cons_expr (pool, scanner);
	  if (error_node (expr))
	    return expr;
	  *p = ttl_make_ast_pair (pool, expr, NULL);
	  p = &((*p)->d.pair.cdr);
	}
      return ttl_make_ast_tuple_expr (pool, list,
				      scanner->filename,
				      beg_line,
				      beg_col,
				      scanner->last_line,
				      scanner->last_column);
    }
  else
    return expr;
}


/* Parse an assignment expression or procedure call.

   AssignExpression ::= TupleExpression [':=' TupleExpression]
*/
static ttl_ast_node
parse_assign_expr (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node left, right;
  left = parse_tuple_expr (pool, scanner);
  if (error_node (left))
    return left;
  if (scanner->token_class == token_assign)
    {
      int beg_line = scanner->begin_line;
      int beg_col = scanner->begin_column;
      int end_line = scanner->last_line;
      int end_col = scanner->last_column;

      /* Skip `:='.  */
      ttl_next_token (scanner);
      right = parse_tuple_expr (pool, scanner);
      if (error_node (right))
	return right;
      return ttl_make_ast_binop (pool, binop_assign, left, right,
				 scanner->filename, beg_line, beg_col,
				 end_line, end_col);
    }
  else
    return left;
}


/* Parse an expression statement.

   ExpressionStatement ::= Expression
*/
static ttl_ast_node
parse_stmt_expr (ttl_pool pool, ttl_scanner scanner)
{
  return parse_assign_expr (pool, scanner);
}


/* Parse a single constraint.

   Constraint ::= CompareExpression
*/
static ttl_ast_node
parse_constraint (ttl_pool pool, ttl_scanner scanner)
{
  return parse_compare_expr (pool, scanner);
}


/* Parse a constraint conjunction, including strength annotations.

   ConstraintConjunction ::= Constraint [':' Strength]
                             {'and' Constraint [':' Strength]}
   Strength              ::= IntConst
*/
static ttl_ast_node
parse_constraint_conjunction (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node list = NULL, * listp = &list;
  ttl_ast_node expr, * p;
  ttl_ast_node strength = NULL;
  expr = parse_constraint (pool, scanner);
  if (error_node (expr))
    return expr;
  if (scanner->token_class == token_colon)
    {
      ttl_next_token (scanner);
      strength = scanner->token_value;
      if (!accept_token (scanner, token_int_const))
	return ttl_make_ast_error (pool);
      expr = ttl_make_ast_ann_expr (pool, expr, strength,
				    scanner->filename, 
				    scanner->last_line, scanner->last_column,
				    scanner->last_line, scanner->last_column);
    }
  *listp = ttl_make_ast_pair (pool, expr, NULL);
  listp = &((*listp)->d.pair.cdr);
  while (scanner->token_class == token_and)
    {
      int beg_line = scanner->begin_line;
      int beg_col = scanner->begin_column;
      int end_line = scanner->last_line;
      int end_col = scanner->last_column;

      ttl_next_token (scanner);
      expr = parse_constraint (pool, scanner);
      if (error_node (expr))
	return expr;

      if (scanner->token_class == token_colon)
	{
	  ttl_next_token (scanner);
	  strength = scanner->token_value;
	  if (!accept_token (scanner, token_int_const))
	    return ttl_make_ast_error (pool);
	  expr = ttl_make_ast_ann_expr (pool, expr, strength,
					scanner->filename, 
					scanner->last_line,
					scanner->last_column,
					scanner->last_line,
					scanner->last_column);
	}
      
      *listp = ttl_make_ast_pair (pool, expr, NULL);
      listp = &((*listp)->d.pair.cdr);
    }
  return list;
}


/* Parse an `if' statement.

   IfStatement    ::= 'if' OrExpression 'then' {Statement} 
                      {'elsif' OrExpression 'then' {Statement}
                      ['else' {Statement}] 'end'
*/
static ttl_ast_node
parse_if (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node stmt, cond, thenlist = NULL, * p = &thenlist, elselist = NULL;
  int beg_line = scanner->begin_line;
  int beg_col = scanner->begin_column;

  ttl_next_token (scanner);
  cond = parse_or_expr (pool, scanner);
  if (error_node (cond))
    return cond;
  if (!accept_token (scanner, token_then))
    return ttl_make_ast_error (pool);
  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_end &&
	 scanner->token_class != token_else &&
	 scanner->token_class != token_elsif)
    {
      stmt = parse_stmt (pool, scanner);
      if (error_node (stmt))
	continue;
      *p = ttl_make_ast_pair (pool, stmt, NULL);
      p = &((*p)->d.pair.cdr);
    }
  if (scanner->token_class == token_else)
    {
      p = &elselist;
      ttl_next_token (scanner);
      while (scanner->token_class != token_eof &&
	     scanner->token_class != token_end)
	{
	  stmt = parse_stmt (pool, scanner);
	  if (error_node (stmt))
	    continue;
	  *p = ttl_make_ast_pair (pool, stmt, NULL);
	  p = &((*p)->d.pair.cdr);
	}
      accept_token (scanner, token_end); /* Ignore error on purpose!  */
      return ttl_make_ast_if (pool, cond, thenlist, elselist,
			      scanner->filename,
			      beg_line, beg_col,
			      scanner->last_line, scanner->last_column);
    }
  else if (scanner->token_class == token_elsif) 
    {
      ttl_ast_node next_if = parse_if (pool, scanner);
      return ttl_make_ast_if (pool, cond, thenlist,
			      ttl_make_ast_pair (pool, next_if, NULL),
			      scanner->filename,
			      beg_line, beg_col,
			      scanner->last_line, scanner->last_column);
    }
  else
    {
      /* Ignore error on purpose!  */
      accept_token (scanner, token_end);
      return ttl_make_ast_if (pool, cond, thenlist, elselist,
			      scanner->filename,
			      beg_line, beg_col,
			      scanner->last_line, scanner->last_column);
    }
}


/* Parse a `while' statement.

   WhileStatement ::= 'while' OrExpression 'do' {Statement} 'end'
*/
static ttl_ast_node
parse_while (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node stmt, cond, dolist = NULL, * p = &dolist;
  int beg_line = scanner->begin_line;
  int beg_col = scanner->begin_column;

  /* Skip `while'.  */
  ttl_next_token (scanner);

  cond = parse_or_expr (pool, scanner);
  if (error_node (cond))
    return cond;

  if (!accept_token (scanner, token_do))
    return ttl_make_ast_error (pool);

  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_end)
    {
      stmt = parse_stmt (pool, scanner);
      if (error_node (stmt))
	continue;
      *p = ttl_make_ast_pair (pool, stmt, NULL);
      p = &((*p)->d.pair.cdr);
    }
  /* Ignore error on purpose!  */
  accept_token (scanner, token_end);
  return ttl_make_ast_while (pool, cond, dolist, scanner->filename, beg_line,
			     beg_col, scanner->last_line,
			     scanner->last_column);
}


/* Parse a `in' statement.

   InStatement ::= 'in' StmtList 'end'
*/
static ttl_ast_node
parse_in (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node stmt, cond, dolist = NULL, * p = &dolist;
  int beg_line = scanner->begin_line;
  int beg_col = scanner->begin_column;

  ttl_next_token (scanner);
  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_end)
    {
      stmt = parse_stmt (pool, scanner);
      if (error_node (stmt))
	continue;
      *p = ttl_make_ast_pair (pool, stmt, NULL);
      p = &((*p)->d.pair.cdr);
    }
  /* Ignore error on purpose!  */
  accept_token (scanner, token_end);
  return ttl_make_ast_in (pool, dolist, scanner->filename, beg_line, beg_col,
			  scanner->last_line, scanner->last_column);
}


/* Parse a `return' statement.

   ReturnStatement ::= 'return' [TupleExpression]
*/
static ttl_ast_node
parse_return (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr = NULL;
  int beg_line = scanner->begin_line;
  int beg_col = scanner->begin_column;

  ttl_next_token (scanner);
  if (scanner->token_class != token_eof &&
      scanner->token_class != token_semicolon)
    {
      expr = parse_tuple_expr (pool, scanner);
      if (error_node (expr))
	return expr;
    }
  return ttl_make_ast_return (pool, expr, scanner->filename, beg_line,
			      beg_line, scanner->last_line,
			      scanner->last_column);
}


/* Parse a `require' statement.

   RequireStatement ::= 'require' ConstraintConjunction 'in' StmtList 'end'
                     |  'require' ConstraintConjunction
*/
static ttl_ast_node
parse_require (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node expr = NULL;
  ttl_ast_node stmt = NULL;
  int beg_line = scanner->begin_line;
  int beg_col = scanner->begin_column;

  /* Skip 'require' keyword.  */
  ttl_next_token (scanner);

  /* Parse constraint conjunction, including strength annotations.  */
  expr = parse_constraint_conjunction (pool, scanner);
  if (error_node (expr))
    return expr;

  /* Parse possible following `in' statement.  */
  if (scanner->token_class == token_in)
    stmt = parse_stmt (pool, scanner);
  else
    {
      accept_token (scanner, token_semicolon);
      stmt = NULL;
    }
  return ttl_make_ast_require (pool, expr, stmt, scanner->filename, beg_line,
			       beg_col, scanner->last_line,
			       scanner->last_column);
}


/* Parse a statement.

   Statement      ::= VarDecl ';' | ConstDecl ';'
                   |  FunDecl ';' | ConstraintDecl ';'
                   |  IfStatement ';' | WhileStatement ';'
                   |  InStatement ';'
                   |  ReturnStatement ';'
                   |  RequireStatement ';'
                   |  ExpressionStatement ';'
*/
static ttl_ast_node
parse_stmt (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node stmt;

  switch (scanner->token_class)
    {
    case token_fun:
      stmt = parse_fundef (pool, scanner, 0);
      break;
    case token_constraint:
      stmt = parse_constraintdef (pool, scanner, 0);
      break;
    case token_var:
      stmt = parse_vardef (pool, scanner, 0);
      break;
    case token_const:
      stmt = parse_constdef (pool, scanner, 0);
      break;
    case token_if:
      stmt = parse_if (pool, scanner);
      break;
    case token_while:
      stmt = parse_while (pool, scanner);
      break;
    case token_in:
      stmt = parse_in (pool, scanner);
      break;
    case token_return:
      stmt = parse_return (pool, scanner);
      break;
    case token_require:
      stmt = parse_require (pool, scanner);
      /* Bail out early, because we don't want to eat any semicolon.
	 The reason is that the statement inside the require statement
	 already has eaten it.  */
      return stmt;
      break;
    case token_identifier:
      stmt = parse_stmt_expr (pool, scanner);
      break;
    default:
      scanner->parse_errors++;
      fprintf (stderr,
	       "%s:%d:%d: unexpected token %s (expected was a statement)\n",
	       scanner->filename, scanner->begin_line + 1,
	       scanner->begin_column + 1, 
	       ttl_token_names[scanner->token_class]);
      ttl_next_token (scanner);
      stmt = ttl_make_ast_error (pool);
    }
  if (!accept_statement_semicolon (scanner))
    stmt = ttl_make_ast_error (pool);
  return stmt;
}


/* Parse all declarations until the end of file is reached.

   Declarations   ::= { Declaration ';' }
   Declaration    ::= TypeDecl | DataTypeDecl | VarDecl | FunDecl 
                   |  ConstraintDecl
*/
static ttl_ast_node
parse_module_declarations (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node list = NULL;
  ttl_ast_node def;
  ttl_ast_node * p = &list;
  int public;

  while (scanner->token_class != token_eof)
    {
      /* Remember a possible `public' keyword, which will be attached
	 to the following declaration.  */
      if (scanner->token_class == token_public)
	{
	  ttl_next_token (scanner);
	  public = 1;
	}
      else
	public = 0;

      switch (scanner->token_class)
	{
	case token_fun:
	  def = parse_fundef (pool, scanner, public);
	  break;
	case token_constraint:
	  def = parse_constraintdef (pool, scanner, public);
	  break;
	case token_type:
	  def = parse_typedef (pool, scanner, public);
	  break;
	case token_datatype:
	  def = parse_datatype (pool, scanner, public);
	  break;
	case token_var:
	  def = parse_vardef (pool, scanner, public);
	  break;
	case token_const:
	  def = parse_constdef (pool, scanner, public);
	  break;
	default:
	  scanner->parse_errors++;
	  fprintf (stderr,
		"%s:%d:%d: unexpected token %s (expected was a definition)\n",
		   scanner->filename, scanner->begin_line+1,
		   scanner->begin_column + 1, 
		   ttl_token_names[scanner->token_class]);

	  /* Eat the erroneous token, to guarantee progress.  */
	  ttl_next_token (scanner);
	  continue;
	}
      if (!accept_declaration_semicolon (scanner))
	break;
      *p = ttl_make_ast_pair (pool, def, NULL);
      p = &((*p)->d.pair.cdr);
    }
  return list;
}


/* Parse a single module parameter.  This is only an identifier.  */
static ttl_ast_node
parse_module_param (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node name;
  ttl_ast_node type = NULL;
  int beg_line, beg_col;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;

  name = scanner->token_value;
  if (!accept_token (scanner, token_identifier))
    return ttl_make_ast_error (pool);
#if 0
  if (!accept_token (scanner, token_colon))
    return ttl_make_ast_error (pool);
  if (scanner->token_class == token_type)
    {
      type = NULL;
      ttl_next_token (scanner);
    }
  else
    {
      type = NULL;
      accept_token (scanner, token_type);
    }
/*   else */
/*     { */
/*       type = parse_type (pool, scanner);       */
/*     } */
#endif
  return ttl_make_ast_variable (pool, name, type, NULL, 0, scanner->filename,
				beg_line, beg_col,
				scanner->last_line, scanner->last_column);
}


/* Parse a list of module parameters, enclosed in `<' and `>'.  */
static ttl_ast_node
parse_module_params (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node list = NULL, * listp = &list;
  ttl_ast_node param;

  /* Skip '<'.  */
  ttl_next_token (scanner);

  /* Parse the first parameter, return NULL on error.  */
  param = parse_module_param (pool, scanner);
  if (error_node (param))
    return NULL;
  *listp = ttl_make_ast_pair (pool, param, NULL);
  listp = &((*listp)->d.pair.cdr);

  while (scanner->token_class != token_gt &&
	 scanner->token_class != token_eof)
    {
      /* Skip `,'.  */
      if (!accept_token (scanner, token_comma))
	return list;

      /* Parse the remaining module parameter names.  */
      param = parse_module_param (pool, scanner);
      if (error_node (param))
	return NULL;
      *listp = ttl_make_ast_pair (pool, param, NULL);
      listp = &((*listp)->d.pair.cdr);
    }
  /* Return parameter list even if closing `>' is missing, to increase
     robustness.  */
  if (!accept_token (scanner, token_gt))
    return list;
  return list;
}


/* Parse a complete compilation unit.

   FIXME: Update grammar documentation with module parameters.

   CompUnit   ::= Module
   Module     ::= 'module' QualIdent [ModuleParams] ';' ModDecls Declarations
   ModuleParams ::= '<' ModuleParam {',' ModuleParam} '>'
   ModuleParam ::= Ident
*/
static ttl_ast_node
parse_module (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node name, params, imports, exports, defs;
  char * documentation;
  int beg_line, beg_col;

  beg_line = scanner->begin_line;
  beg_col = scanner->begin_column;

  /* Store the module documentation, if any.  */
  documentation = ttl_get_comment_buffer (scanner);

  /* Skip the `module' keyword.  */
  if (!accept_token (scanner, token_module))
    return ttl_make_ast_error (pool);

  /* Parse the module name.  */
  name = parse_qualident (pool, scanner);
  if (error_node (name))
    return name;

  /* Parse (possibly empty) list of parameter names.  */
  if (scanner->token_class == token_lt)
    params = parse_module_params (pool, scanner);
  else
    params = NULL;

  /* Skip the `;'.  */
  if (!accept_declaration_semicolon (scanner))
    return ttl_make_ast_error (pool);

  /* Parse the module import and export clauses.  */
  imports = parse_module_imports (pool, scanner);
  exports = parse_module_exports (pool, scanner);

  /* Parse type, variable, function and constraint declarations.  */
  defs = parse_module_declarations (pool, scanner);

  /* Create a module node containing all information from the
     module.  */
  return ttl_make_ast_module (pool, name, params, exports, imports, defs,
			      documentation,
			      scanner->filename, beg_line, beg_col,
			      scanner->last_line, scanner->last_column);
}


/* Parse a module and return it.  If an error occurs during parsing
   which results in an unusable parse, return NULL.  */
ttl_ast_node
ttl_parse_module (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node node;
  
  node = parse_module (pool, scanner);
  if (error_node (node))
    return NULL;
  return node;
}


/* Parse an interface file.  We are not wasting too much work on error
   messages, since interface files are compiler-generated anyway.  On
   errors, NULL will be returned.  */
static ttl_ast_node
parse_interface (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node name, type, entry, list = NULL, * p = &list;
  ttl_ast_node l, * lp;

  /* Parse the list of imported modules.  */
  l = NULL;
  lp = &l;
  if (!accept_token (scanner, token_lparen))
    return NULL;
  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_rparen)
    {
#if 0      
      name = scanner->token_value;
      if (!accept_token (scanner, token_identifier))
	return NULL;
#endif
      name = parse_qualident (pool, scanner);
      if (error_node (name))
	return NULL;
      *lp = ttl_make_ast_pair (pool, name, NULL);
      lp = &((*lp)->d.pair.cdr);

    }
  *p = ttl_make_ast_pair (pool, l, NULL);
  p = &((*p)->d.pair.cdr);
  if (!accept_token (scanner, token_rparen))
    return NULL;

  /* Parse the list of module parameters.  */
  l = NULL;
  lp = &l;
  if (!accept_token (scanner, token_lparen))
    return NULL;
  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_rparen)
    {
      name = scanner->token_value;
      if (!accept_token (scanner, token_identifier))
	return NULL;
      *lp = ttl_make_ast_pair (pool, name, NULL);
      lp = &((*lp)->d.pair.cdr);

    }
  *p = ttl_make_ast_pair (pool, l, NULL);
  p = &((*p)->d.pair.cdr);
  if (!accept_token (scanner, token_rparen))
    return NULL;

  /* Parse the list of opaque types.  */
  l = NULL;
  lp = &l;
  if (!accept_token (scanner, token_lparen))
    return NULL;
  while (scanner->token_class != token_eof &&
	 scanner->token_class != token_rparen)
    {
      name = scanner->token_value;
      if (!accept_token (scanner, token_identifier))
	return NULL;
      *lp = ttl_make_ast_pair (pool, name, NULL);
      lp = &((*lp)->d.pair.cdr);

    }
  *p = ttl_make_ast_pair (pool, l, NULL);
  p = &((*p)->d.pair.cdr);
  if (!accept_token (scanner, token_rparen))
    return NULL;

  while (scanner->token_class != token_eof)
    {
      name = scanner->token_value;
      if (!accept_token (scanner, token_identifier))
	return NULL;

      switch (scanner->token_class)
	{
	case token_type:
	  ttl_next_token (scanner);
	  type = parse_type (pool, scanner);
	  if (error_node (type))
	    return NULL;
	  entry = ttl_make_ast_typedef (pool, name, type, 1 /* Public.  */,
					NULL);
	  break;

	case token_var:
	  ttl_next_token (scanner);
	  type = parse_type (pool, scanner);
	  if (error_node (type))
	    return NULL;
	  entry = ttl_make_ast_variable (pool, name, type, NULL, 0,
					 NULL, -1, -1, -1, -1);
	  break;

	case token_const:
	  ttl_next_token (scanner);
	  type = parse_type (pool, scanner);
	  if (error_node (type))
	    return NULL;
	  {
	    ttl_ast_node init = parse_tuple_expr (pool, scanner);
	    entry = ttl_make_ast_variable (pool, name, type, init, 0,
					   NULL, -1, -1, -1, -1);
	  }
	  break;

	case token_fun:
	  ttl_next_token (scanner);
	  type = parse_type (pool, scanner);
	  if (error_node (type))
	    return NULL;
	  entry = ttl_make_ast_function (pool, name, type, 
					 NULL, NULL,
					 1, /* Public.  */
					 0, /* Not handcoded.  */
					 NULL, /* No docs. */
					 NULL, -1, -1, -1, -1);
	  break;

	case token_constraint:
	  ttl_next_token (scanner);
	  type = parse_type (pool, scanner);
	  if (error_node (type))
	    return NULL;
	  entry = ttl_make_ast_constraint (pool, name, type, NULL,
					   1, /* Public.  */
					   NULL, /* No documentation.  */
					   NULL, -1, -1, -1, -1);
	  break;

	default:
	  return NULL;
	}
      *p = ttl_make_ast_pair (pool, entry, NULL);
      p = &((*p)->d.pair.cdr);
    }
  return list;
}


/* An interface file contains the signature of a module.  Since
   interface files are generated by the compiler, error messages are
   not expected to be very good.  */
ttl_ast_node
ttl_parse_interface (ttl_pool pool, ttl_scanner scanner)
{
  ttl_ast_node node;
  
  node = parse_interface (pool, scanner);
  return node;
}


/* End of parser.c.  */
