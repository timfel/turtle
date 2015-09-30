/* libturtle/ast.h - abstract syntax
 
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

#ifndef TTL_AST_H
#define TTL_AST_H

#include <stdio.h>

#include "memory.h"
#include "symbols.h"

/* The datatypes and functions declared in this file implement the
   abstract syntax trees for Turtle.  For each kind of AST node, an
   appropriate constructor function is defined.  */

enum ttl_ast_kind
  {ast_error,
   ast_module,
   ast_pair,
   ast_string_const, 
   ast_char_const,
   ast_int_const,
   ast_identifier,
   ast_annotated_identifier,
   ast_qualident,
   ast_long_const,
   ast_real_const,
   ast_bool_const,
   ast_null_const,
   ast_binop,
   ast_unop,
   ast_if,
   ast_while,
   ast_in,
   ast_call,
   ast_return,
   ast_function,
   ast_constraint,
   ast_typedef,
   ast_vardef,
   ast_constdef,
   ast_variable,
   ast_require,
   ast_index,
   ast_array_expr,
   ast_list_expr,
   ast_array_constructor,
   ast_list_constructor,
   ast_string_constructor,
   ast_array_type,
   ast_string_type,
   ast_list_type,
   ast_void_type,
   ast_tuple_type,
   ast_function_type,
   ast_constrained_type,
   ast_basic_type,
   ast_user_type,
   ast_datatype,
   ast_datatype_variant,
   ast_tuple_expr,
   ast_ann_expr,
   ast_foreign_expr,
   ast_var_expr,
   ast_deref_expr};

enum ttl_binop
  {binop_add,
   binop_sub,
   binop_or,
   binop_and,
   binop_mul, 
   binop_div,
   binop_mod,
   binop_assign,
   binop_assign_force,
   binop_eq,
   binop_ne,
   binop_lt,
   binop_le,
   binop_gt,
   binop_ge,
   binop_cons};

enum ttl_unop
  {unop_neg,			/* Arithmetic negation, `-EXPR'.  */
   unop_not,			/* Logical negation, `not EXPR' */
   unop_hd,			/* List operator `hd EXPR' */
   unop_tl,			/* List operator `tl EXPR' */
   unop_sizeof};		/* Size operator `sizeof EXPR'.  */

typedef struct ttl_ast_node * ttl_ast_node;


struct ttl_ast_error
{
  int reported;
};

struct ttl_ast_module
{
  ttl_ast_node name;		/* Name of module as identifier.  */
  ttl_ast_node params;		/* List of module parameters. */
  ttl_ast_node imports;		/* List of imported modules.  */
  ttl_ast_node exports;		/* List of exported identifiers.  */
  ttl_ast_node definitions;	/* List of definitions. */
  char * documentation;
};

struct ttl_ast_pair
{
  ttl_ast_node car;
  ttl_ast_node cdr;
};

struct ttl_ast_string
{
  size_t length;
  char * value;
};

struct ttl_ast_identifier
{
  ttl_symbol symbol;
};

struct ttl_ast_annotated_identifier
{
  ttl_ast_node identifier;
  ttl_ast_node annotation;
  ttl_ast_node open_list;
};

struct ttl_ast_qualident
{
  ttl_ast_node module;
  ttl_ast_node identifier;
};

struct ttl_ast_character
{
  int value;
};

struct ttl_ast_integer
{
  long value;
};

struct ttl_ast_long
{
  long value;
};

struct ttl_ast_real
{
  double value;
};

struct ttl_ast_bool
{
  int value;
};

struct ttl_ast_array_expr
{
  ttl_ast_node elements;
};

struct ttl_ast_list_expr
{
  ttl_ast_node elements;
};

struct ttl_ast_array_constructor
{
  ttl_ast_node size;
  ttl_ast_node initial;
};

struct ttl_ast_list_constructor
{
  ttl_ast_node size;
  ttl_ast_node initial;
};

struct ttl_ast_string_constructor
{
  ttl_ast_node size;
  ttl_ast_node initial;
};

struct ttl_ast_array_type
{
  ttl_ast_node element_type;
};

struct ttl_ast_string_type
{
  unsigned unused;
};

struct ttl_ast_list_type
{
  ttl_ast_node element_type;
};

struct ttl_ast_tuple_type
{
  ttl_ast_node fields;
};

struct ttl_ast_function_type
{
  ttl_ast_node left;
  ttl_ast_node right;
};

struct ttl_ast_constrained_type
{
  ttl_ast_node base_type;
};

struct ttl_ast_basic_type
{
  ttl_ast_node type_name;
};

struct ttl_ast_user_type
{
  ttl_ast_node type_name;
  ttl_ast_node types;
};

struct ttl_ast_datatype_variant
{
  ttl_ast_node name;
  ttl_ast_node fields;
};

struct ttl_ast_datatype
{
  ttl_ast_node name;
  ttl_ast_node types;
  ttl_ast_node variants;
  int public;
  char * documentation;
};

struct ttl_ast_binop
{
  enum ttl_binop op;
  ttl_ast_node op0;		/* Expression.  */
  ttl_ast_node op1;		/* Expression.  */
};

struct ttl_ast_unop
{
  enum ttl_unop op;
  ttl_ast_node op0;		/* Expression.  */
};

struct ttl_ast_if
{
  ttl_ast_node cond;		/* Expression.  */
  ttl_ast_node thenstmt;	/* List of statements.  */
  ttl_ast_node elsestmt;	/* List of statements.  */
};

struct ttl_ast_while
{
  ttl_ast_node cond;		/* Expression.  */
  ttl_ast_node dostmt;		/* List of statements.  */
};

struct ttl_ast_in
{
  ttl_ast_node instmt;		/* List of statements.  */
};

struct ttl_ast_call
{
  ttl_ast_node function;	/* Function expression.  */
  ttl_ast_node args;		/* List of arguments.  */
};

struct ttl_ast_index
{
  ttl_ast_node array;		/* Array expression.  */
  ttl_ast_node index;		/* List of index expressions.  */
};

struct ttl_ast_return
{
  ttl_ast_node expr;		/* Return value.  */
};

struct ttl_ast_require
{
  ttl_ast_node expr;		/* Constraint expression.  */
  ttl_ast_node stmt;
};

struct ttl_ast_function
{
  ttl_ast_node name;
  ttl_ast_node params;
  ttl_ast_node type;
  ttl_ast_node body;
  int public;
  unsigned handcoded;		/* Non-zero if handcoded function.  */
  unsigned mapped;		/* Non-zero if mapped.  */
  ttl_symbol alias;		/* Name of mapped function.  */
  char * documentation;
};

struct ttl_ast_constraint
{
  ttl_ast_node name;
  ttl_ast_node params;
  ttl_ast_node body;
  int public;
  char * documentation;
};

struct ttl_ast_typedef
{
  ttl_ast_node name;
  ttl_ast_node type;
  int public;
  char * documentation;
};

struct ttl_ast_vardef
{
  ttl_ast_node list;
  int public;
  char * documentation;
};

struct ttl_ast_constdef
{
  ttl_ast_node list;
  int public;
  char * documentation;
};

struct ttl_ast_variable
{
  ttl_ast_node name;
  ttl_ast_node type;
  ttl_ast_node init;
  int out_param;
};

struct ttl_ast_tuple_expr
{
  ttl_ast_node fields;
};

struct ttl_ast_ann_expr
{
  ttl_ast_node expr;
  ttl_ast_node strength;
};

struct ttl_ast_foreign_expr
{
  ttl_symbol expr;
};

struct ttl_ast_var_expr
{
  ttl_ast_node expr;
};

struct ttl_ast_deref_expr
{
  ttl_ast_node expr;
};

struct ttl_ast_node
{
  enum ttl_ast_kind kind;
  char * filename;
  int start_line;
  int end_line;
  int start_column;
  int end_column;
  union {
    struct ttl_ast_error error;
    struct ttl_ast_module module;
    struct ttl_ast_pair pair;
    struct ttl_ast_string string;
    struct ttl_ast_identifier identifier;
    struct ttl_ast_annotated_identifier annotated_identifier;
    struct ttl_ast_qualident qualident;
    struct ttl_ast_character character;
    struct ttl_ast_integer integer;
    struct ttl_ast_long longint;
    struct ttl_ast_real real;
    struct ttl_ast_bool bool;
    struct ttl_ast_array_expr array_expr;
    struct ttl_ast_list_expr list_expr;
    struct ttl_ast_array_constructor array_constructor;
    struct ttl_ast_list_constructor list_constructor;
    struct ttl_ast_string_constructor string_constructor;
    struct ttl_ast_array_type array_type;
    struct ttl_ast_string_type string_type;
    struct ttl_ast_list_type list_type;
    struct ttl_ast_tuple_type tuple_type;
    struct ttl_ast_function_type function_type;
    struct ttl_ast_constrained_type constrained_type;
    struct ttl_ast_basic_type basic_type;
    struct ttl_ast_user_type user_type;
    struct ttl_ast_binop binop;
    struct ttl_ast_unop unop;
    struct ttl_ast_if ifstmt;
    struct ttl_ast_while whilestmt;
    struct ttl_ast_in instmt;
    struct ttl_ast_call call;
    struct ttl_ast_index index;
    struct ttl_ast_return returnstmt;
    struct ttl_ast_require require;
    struct ttl_ast_function function;
    struct ttl_ast_constraint constraint;
    struct ttl_ast_typedef typedf;
    struct ttl_ast_vardef vardef;
    struct ttl_ast_constdef constdef;
    struct ttl_ast_variable variable;
    struct ttl_ast_datatype datatype;
    struct ttl_ast_datatype_variant datatype_variant;
    struct ttl_ast_tuple_expr tuple_expr;
    struct ttl_ast_ann_expr ann_expr;
    struct ttl_ast_foreign_expr foreign_expr;
    struct ttl_ast_var_expr var_expr;
    struct ttl_ast_deref_expr deref_expr;
  } d;
};


ttl_ast_node ttl_make_ast_error (ttl_pool pool);
ttl_ast_node ttl_make_ast_module (ttl_pool pool, ttl_ast_node name, 
				  ttl_ast_node params,
				  ttl_ast_node exports, 
				  ttl_ast_node imports,
				  ttl_ast_node definitions,
				  char * documentation,
				  char * filename,
				  int beg_line, int beg_col,
				  int end_line, int end_col);
ttl_ast_node ttl_make_ast_pair (ttl_pool pool, ttl_ast_node car,
				ttl_ast_node cdr);
ttl_ast_node ttl_make_ast_string (ttl_pool pool, char * value, size_t len,
				  char * filename,
				  int beg_line, int beg_col,
				  int end_line, int end_col);
ttl_ast_node ttl_make_ast_identifier (ttl_pool pool, ttl_symbol symbol,
				      char * filename,
				      int beg_line, int beg_col,
				      int end_line, int end_col);
ttl_ast_node ttl_make_ast_annotated_identifier (ttl_pool pool,
						ttl_ast_node identifier,
						ttl_ast_node annotation,
						ttl_ast_node open_list,
						char * filename,
						int beg_line, int beg_col,
						int end_line, int end_col);
ttl_ast_node ttl_make_ast_qualident (ttl_pool pool, ttl_ast_node module,
				     ttl_ast_node identifier,
				     char * filename,
				     int beg_line, int beg_col,
				     int end_line, int end_col);
ttl_ast_node ttl_make_ast_character (ttl_pool pool, int value,
				     char * filenapme,
				     int beg_line, int beg_col,
				     int end_line, int end_col);
ttl_ast_node ttl_make_ast_integer (ttl_pool pool, long value,
				   char * filename,
				   int beg_line, int beg_col,
				   int end_line, int end_col);
ttl_ast_node ttl_make_ast_long (ttl_pool pool, long value,
				char * filename,
				int beg_line, int beg_col,
				int end_line, int end_col);
ttl_ast_node ttl_make_ast_real (ttl_pool pool, double value,
				char * filename,
				int beg_line, int beg_col,
				int end_line, int end_col);
ttl_ast_node ttl_make_ast_bool (ttl_pool pool, int value,
				char * filename,
				int beg_line, int beg_col,
				int end_line, int end_col);
ttl_ast_node ttl_make_ast_null (ttl_pool pool, 
				char * filename,
				int beg_line, int beg_col,
				int end_line, int end_col);
ttl_ast_node ttl_make_ast_array_expr (ttl_pool pool, ttl_ast_node elements,
				      char * filename,
				      int beg_line, int beg_col,
				      int end_line, int end_col);
ttl_ast_node ttl_make_ast_array_constructor (ttl_pool pool, ttl_ast_node size,
					     ttl_ast_node initial,
					     char * filename,
					     int beg_line, int beg_col,
					     int end_line, int end_col);
ttl_ast_node ttl_make_ast_list_constructor (ttl_pool pool, ttl_ast_node size,
					    ttl_ast_node initial,
					    char * filename,
					    int beg_line, int beg_col,
					    int end_line, int end_col);
ttl_ast_node ttl_make_ast_string_constructor (ttl_pool pool, ttl_ast_node size,
					      ttl_ast_node initial,
					      char * filename,
					      int beg_line, int beg_col,
					      int end_line, int end_col);
ttl_ast_node ttl_make_ast_list_expr (ttl_pool pool, ttl_ast_node elements,
				     char * filename,
				     int beg_line, int beg_col,
				     int end_line, int end_col);
ttl_ast_node ttl_make_ast_array_type (ttl_pool pool, 
				      ttl_ast_node element_type,
				      char * filename,
				      int beg_line, int beg_col,
				      int end_line, int end_col);
ttl_ast_node ttl_make_ast_string_type (ttl_pool pool, 
				       char * filename,
				       int beg_line, int beg_col,
				       int end_line, int end_col);
ttl_ast_node ttl_make_ast_list_type (ttl_pool pool,
				     ttl_ast_node element_type,
				     char * filename,
				     int beg_line, int beg_col,
				     int end_line, int end_col);
ttl_ast_node ttl_make_ast_void_type (ttl_pool pool, 
				     char * filename,
				     int beg_line, int beg_col,
				     int end_line, int end_col);
ttl_ast_node ttl_make_ast_tuple_type (ttl_pool pool, ttl_ast_node fields,
				      char * filename,
				      int beg_line, int beg_col,
				      int end_line, int end_col);
ttl_ast_node ttl_make_ast_function_type (ttl_pool pool, ttl_ast_node left,
					 ttl_ast_node right,
					 char * filename,
					 int beg_line, int beg_col,
					 int end_line, int end_col);
ttl_ast_node ttl_make_ast_constrained_type (ttl_pool pool,
					    ttl_ast_node base_type,
					    char * filename,
					    int beg_line, int beg_col,
					    int end_line, int end_col);
ttl_ast_node ttl_make_ast_basic_type (ttl_pool pool,
				      ttl_ast_node type_name,
				      char * filename,
				      int beg_line, int beg_col,
				      int end_line, int end_col);
ttl_ast_node ttl_make_ast_user_type (ttl_pool pool,
				     ttl_ast_node type_name,
				     ttl_ast_node types,
				     char * filename,
				     int beg_line, int beg_col,
				     int end_line, int end_col);
ttl_ast_node ttl_make_ast_binop (ttl_pool pool, enum ttl_binop op,
				 ttl_ast_node op0, ttl_ast_node op1,
				 char * filename,
				 int beg_line, int beg_col,
				 int end_line, int end_col);
ttl_ast_node ttl_make_ast_unop (ttl_pool pool, enum ttl_unop op,
				ttl_ast_node op0,
				char * filename,
				int beg_line, int beg_col,
				int end_line, int end_col);
ttl_ast_node ttl_make_ast_if (ttl_pool pool, ttl_ast_node cond,
			      ttl_ast_node thenstmt, ttl_ast_node elsestmt,
			      char * filename,
			      int beg_line, int beg_col,
			      int end_line, int end_col);
ttl_ast_node ttl_make_ast_while (ttl_pool pool, ttl_ast_node cond,
				 ttl_ast_node dostmt,
				 char * filename,
				 int beg_line, int beg_col,
				 int end_line, int end_col);
ttl_ast_node ttl_make_ast_in (ttl_pool pool, ttl_ast_node instmt,
			      char * filename,
			      int beg_line, int beg_col,
			      int end_line, int end_col);
ttl_ast_node ttl_make_ast_call (ttl_pool pool, ttl_ast_node func,
				ttl_ast_node args,
				char * filename,
				int beg_line, int beg_col,
				int end_line, int end_col);
ttl_ast_node ttl_make_ast_index (ttl_pool pool, ttl_ast_node array,
				 ttl_ast_node index,
				 char * filename,
				 int beg_line, int beg_col,
				 int end_line, int end_col);
ttl_ast_node ttl_make_ast_return (ttl_pool pool, ttl_ast_node expr,
				  char * filename,
				  int beg_line, int beg_col,
				  int end_line, int end_col);
ttl_ast_node ttl_make_ast_require (ttl_pool pool, ttl_ast_node expr,
				   ttl_ast_node stmt,
				   char * filename,
				   int beg_line, int beg_col,
				   int end_line, int end_col);

ttl_ast_node ttl_make_ast_function (ttl_pool pool,
				    ttl_ast_node name,
				    ttl_ast_node params,
				    ttl_ast_node type,
				    ttl_ast_node body,
				    int public,
				    unsigned handcoded,
				    char * documentation,
				    char * filename,
				    int beg_line, int beg_col,
				    int end_line, int end_col);
ttl_ast_node ttl_make_ast_constraint (ttl_pool pool,
				      ttl_ast_node name,
				      ttl_ast_node params,
				      ttl_ast_node body,
				      int public,
				      char * documentation,
				      char * filename,
				      int beg_line, int beg_col,
				      int end_line, int end_col);
ttl_ast_node ttl_make_ast_typedef (ttl_pool pool,
				   ttl_ast_node name,
				   ttl_ast_node type,
				   int public, char * documentation);
ttl_ast_node ttl_make_ast_vardef (ttl_pool pool, ttl_ast_node list,
				  int public,
				  char * documentation,
				  char * filename,
				  int beg_line, int beg_col,
				  int end_line, int end_col);
ttl_ast_node ttl_make_ast_constdef (ttl_pool pool, ttl_ast_node list,
				    int public,
				    char * documentation,
				    char * filename,
				    int beg_line, int beg_col,
				    int end_line, int end_col);
ttl_ast_node ttl_make_ast_variable (ttl_pool pool,
				    ttl_ast_node name,
				    ttl_ast_node type,
				    ttl_ast_node init,
				    int out_param,
				    char * filename,
				    int beg_line, int beg_col,
				    int end_line, int end_col);
ttl_ast_node ttl_make_ast_datatype_variant (ttl_pool pool, ttl_ast_node name,
					    ttl_ast_node fields);
ttl_ast_node ttl_make_ast_datatype (ttl_pool pool, ttl_ast_node name,
				    ttl_ast_node types,
				    ttl_ast_node variants,
				    int public,
				    char * documentation);
ttl_ast_node ttl_make_ast_tuple_expr (ttl_pool pool, ttl_ast_node fields,
				      char * filename,
				      int beg_line, int beg_col,
				      int end_line, int end_col);
ttl_ast_node ttl_make_ast_ann_expr (ttl_pool pool, ttl_ast_node expr,
				    ttl_ast_node strength,
				    char * filename,
				    int beg_line, int beg_col,
				    int end_line, int end_col);
ttl_ast_node ttl_make_ast_foreign_expr (ttl_pool pool, ttl_symbol expr,
					char * filename,
					int beg_line, int beg_col,
					int end_line, int end_col);
ttl_ast_node ttl_make_ast_var_expr (ttl_pool pool, ttl_ast_node expr,
				    char * filename,
				    int beg_line, int beg_col,
				    int end_line, int end_col);
ttl_ast_node ttl_make_ast_deref_expr (ttl_pool pool, ttl_ast_node expr,
				      char * filename,
				      int beg_line, int beg_col,
				      int end_line, int end_col);

int ttl_ast_print (FILE * f, ttl_ast_node node, int indent);
void ttl_ast_print_stmtlist (FILE * f, ttl_ast_node l, int indent);

size_t ttl_ast_length (ttl_ast_node l);
int ttl_compare_identifiers (ttl_ast_node name1, ttl_ast_node name2);
ttl_ast_node ttl_strip_annotation (ttl_ast_node name);

#endif /* not TTL_AST_H */
