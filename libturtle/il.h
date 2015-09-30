/* libturtle/il.h - intermediate language
 
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

#ifndef TTL_IL_H
#define TTL_IL_H

#include <stdio.h>

#include "symbols.h"
#include "types.h"
#include "ast.h"
#include "compiler.h"

enum ttl_il_kind
  {il_error,
   il_module,
   il_pair,
   il_string_const,
   il_variable,
   il_function,
   il_char_const,
   il_int_const,
   il_long_const,
   il_real_const,
   il_bool_const,
   il_null_const,
   il_binop,
   il_unop,
   il_if,
   il_while,
   il_in,
   il_call,
   il_index,
   il_return,
   il_require,
   il_array_expr,
   il_list_expr,
   il_array_constructor,
   il_list_constructor,
   il_string_constructor,
   il_tuple_expr,
   il_seq,
   il_ann_expr,
   il_foreign_expr,
   il_var_expr,
   il_deref_expr};

enum ttl_il_binop
  {il_binop_add,
   il_binop_sub,
   il_binop_or,
   il_binop_and,
   il_binop_mul,
   il_binop_div,
   il_binop_mod,
   il_binop_assign,
   il_binop_eq,
   il_binop_ne,
   il_binop_lt,
   il_binop_le,
   il_binop_gt,
   il_binop_ge,
   il_binop_cons};

enum ttl_il_unop
  {il_unop_neg,
   il_unop_not,
   il_unop_hd,
   il_unop_tl,
   il_unop_sizeof};

typedef struct ttl_il_node * ttl_il_node;


struct ttl_il_error
{
  int reported;
};

struct ttl_il_module
{
  ttl_symbol name;		/* Name of module as identifier.  */
  ttl_il_node imports;		/* List of imported modules.  */
  ttl_il_node exports;		/* List of exported identifiers.  */
  ttl_il_node definitions;	/* List of definitions. */
};

struct ttl_il_pair
{
  ttl_il_node car;
  ttl_il_node cdr;
};

struct ttl_il_variable
{
  ttl_ast_node name;
  ttl_symbol mangled_name;
  ttl_variable variable;
};

struct ttl_il_function
{
  ttl_ast_node name;
  ttl_symbol mangled_name;
  ttl_function function;
};

struct ttl_il_string
{
  char * text;
  unsigned length;
};

struct ttl_il_integer
{
  long value;
};

struct ttl_il_long
{
  long value;
};

struct ttl_il_real
{
  double value;
};

struct ttl_il_bool
{
  int value;
};

struct ttl_il_char
{
  int value;
};

struct ttl_il_array_expr
{
  ttl_il_node elements;
};

struct ttl_il_list_expr
{
  ttl_il_node elements;
};

struct ttl_il_array_constructor
{
  ttl_il_node size;
  ttl_il_node initial;
};

struct ttl_il_list_constructor
{
  ttl_il_node size;
  ttl_il_node initial;
};

struct ttl_il_string_constructor
{
  ttl_il_node size;
  ttl_il_node initial;
};

struct ttl_il_tuple_expr
{
  ttl_il_node elements;
};

struct ttl_il_seq
{
  ttl_il_node stmts;
};

struct ttl_binop_il
{
  enum ttl_il_binop op;
  ttl_il_node op0;		/* Expression.  */
  ttl_il_node op1;		/* Expression.  */
};

struct ttl_unop_il
{
  enum ttl_il_unop op;
  ttl_il_node op0;		/* Expression.  */
};

struct ttl_il_if
{
  ttl_il_node cond;		/* Expression.  */
  ttl_il_node thenstmt;		/* List of statements.  */
  ttl_il_node elsestmt;		/* List of statement.  */
};

struct ttl_il_while
{
  ttl_il_node cond;		/* Expression.  */
  ttl_il_node dostmt;		/* List of statement.  */
};

struct ttl_il_in
{
  ttl_il_node instmt;		/* List of statement.  */
};

struct ttl_il_call
{
  ttl_il_node function;		/* Function expression.  */
  ttl_il_node args;		/* List of arguments.  */
};

struct ttl_il_index
{
  ttl_il_node array;
  ttl_il_node index;
};

struct ttl_il_return
{
  ttl_il_node expr;		/* Return value.  */
};

struct ttl_il_require
{
  ttl_il_node expr;		/* Constraint expression.  */
  ttl_il_node stmt;
};

struct ttl_il_ann_expr
{
  ttl_il_node expr;
  int strength;
};

struct ttl_il_foreign_expr
{
  ttl_symbol expr;
};

struct ttl_il_var_expr
{
  ttl_il_node expr;
};

struct ttl_il_deref_expr
{
  ttl_il_node expr;
};

struct ttl_il_node
{
  enum ttl_il_kind kind;
  char * filename;
  int start_line;
  int end_line;
  int start_column;
  int end_column;
  ttl_type type;
  unsigned ambiguous;
  unsigned lvalue;
  union {
    struct ttl_il_module module;
    struct ttl_il_error error;
    struct ttl_il_pair pair;
    struct ttl_il_string string;
    struct ttl_il_variable variable;
    struct ttl_il_function function;
    struct ttl_il_char character;
    struct ttl_il_integer integer;
    struct ttl_il_long longint;
    struct ttl_il_real real;
    struct ttl_il_bool bool;
    struct ttl_il_array_expr array_expr;
    struct ttl_il_list_expr list_expr;
    struct ttl_il_array_constructor array_constructor;
    struct ttl_il_list_constructor list_constructor;
    struct ttl_il_string_constructor string_constructor;
    struct ttl_il_tuple_expr tuple_expr;
    struct ttl_il_seq seq;
    struct ttl_binop_il binop;
    struct ttl_unop_il unop;
    struct ttl_il_if ifstmt;
    struct ttl_il_while whilestmt;
    struct ttl_il_in instmt;
    struct ttl_il_call call;
    struct ttl_il_index index;
    struct ttl_il_return returnstmt;
    struct ttl_il_require require;
    struct ttl_il_ann_expr ann_expr;
    struct ttl_il_foreign_expr foreign_expr;
    struct ttl_il_var_expr var_expr;
    struct ttl_il_deref_expr deref_expr;
  } d;
};

typedef struct ttl_il_node_list * ttl_il_node_list;
struct ttl_il_node_list
{
  ttl_il_node_list next;
  ttl_il_node elem;
};


ttl_il_node_list ttl_il_cons (ttl_compile_state state,
			      ttl_il_node node,
			      ttl_il_node_list next);
ttl_il_node ttl_make_il_error (ttl_compile_state state);
ttl_il_node ttl_make_il_module (ttl_compile_state state, ttl_symbol name, 
				ttl_il_node exports, ttl_il_node imports,
				ttl_il_node definitions,
				char * filename,
				int beg_line, int beg_col,
				int end_line, int end_col);
ttl_il_node ttl_make_il_pair (ttl_compile_state state, ttl_il_node car,
			      ttl_il_node cdr);
ttl_il_node ttl_make_il_variable (ttl_compile_state state, ttl_ast_node name,
				  ttl_symbol mangled_name, ttl_type type,
				  unsigned lvalue,
				  char * filename,
				  int beg_line, int beg_col,
				  int end_line, int end_col);
ttl_il_node ttl_make_il_function (ttl_compile_state state, ttl_ast_node name,
				  ttl_symbol mangled_name, ttl_type type,
				  ttl_function function);
ttl_il_node ttl_make_il_string (ttl_compile_state state, char * value,
				size_t len,
				char * filename,
				int beg_line, int beg_col,
				int end_line, int end_col);
ttl_il_node ttl_make_il_character (ttl_compile_state state, int value,
				   char * filename,
				   int beg_line, int beg_col,
				   int end_line, int end_col);
ttl_il_node ttl_make_il_integer (ttl_compile_state state, long value,
				 char * filename,
				 int beg_line, int beg_col,
				 int end_line, int end_col);
ttl_il_node ttl_make_il_long (ttl_compile_state state, long value,
			      char * filename,
			      int beg_line, int beg_col,
			      int end_line, int end_col);
ttl_il_node ttl_make_il_real (ttl_compile_state state, double value,
			      char * filename,
			      int beg_line, int beg_col,
			      int end_line, int end_col);
ttl_il_node ttl_make_il_bool (ttl_compile_state state, int value,
			      char * filename,
			      int beg_line, int beg_col,
			      int end_line, int end_col);
ttl_il_node ttl_make_il_null (ttl_compile_state state,
			      char * filename,
			      int beg_line, int beg_col,
			      int end_line, int end_col);
ttl_il_node ttl_make_il_array_expr (ttl_compile_state state,
				    ttl_il_node elements,
				    ttl_type type,
				    char * filename,
				    int beg_line, int beg_col,
				    int end_line, int end_col);
ttl_il_node ttl_make_il_list_expr (ttl_compile_state state,
				   ttl_il_node elements,
				   ttl_type type,
				   char * filename,
				   int beg_line, int beg_col,
				   int end_line, int end_col);
ttl_il_node ttl_make_il_array_constructor (ttl_compile_state state,
					   ttl_il_node size,
					   ttl_il_node initial,
					   ttl_type type,
					   char * filename,
					   int beg_line, int beg_col,
					   int end_line, int end_col);
ttl_il_node ttl_make_il_list_constructor (ttl_compile_state state,
					  ttl_il_node size,
					  ttl_il_node initial,
					  ttl_type type,
					  char * filename,
					  int beg_line, int beg_col,
					  int end_line, int end_col);
ttl_il_node ttl_make_il_string_constructor (ttl_compile_state state,
					    ttl_il_node size,
					    ttl_il_node initial,
					    ttl_type type,
					    char * filename,
					    int beg_line, int beg_col,
					    int end_line, int end_col);
ttl_il_node ttl_make_il_tuple_expr (ttl_compile_state state,
				    ttl_il_node elements,
				    ttl_type type,
				    char * filename,
				    int beg_line, int beg_col,
				    int end_line, int end_col);
ttl_il_node ttl_make_il_seq (ttl_compile_state state,
			     ttl_il_node stmts);
ttl_il_node ttl_make_il_binop (ttl_compile_state state,
			       enum ttl_il_binop op, ttl_il_node op0,
			       ttl_il_node op1,
			       ttl_type type,
			       char * filename,
			       int beg_line, int beg_col,
			       int end_line, int end_col);
ttl_il_node ttl_make_il_unop (ttl_compile_state state, enum ttl_il_unop op,
			      ttl_il_node op0, ttl_type type,
			      char * filename,
			      int beg_line, int beg_col,
			      int end_line, int end_col);
ttl_il_node ttl_make_il_if (ttl_compile_state state, ttl_il_node cond,
			    ttl_il_node thenstmt,
			    ttl_il_node elsestmt,
			    char * filename,
			    int beg_line, int beg_col,
			    int end_line, int end_col);
ttl_il_node ttl_make_il_while (ttl_compile_state state,
			       ttl_il_node cond, ttl_il_node dostmt,
			       char * filename,
			       int beg_line, int beg_col,
			       int end_line, int end_col);
ttl_il_node ttl_make_il_in (ttl_compile_state state, ttl_il_node instmt,
			    char * filename,
			    int beg_line, int beg_col,
			    int end_line, int end_col);
ttl_il_node ttl_make_il_call (ttl_compile_state state, ttl_il_node func,
			      ttl_il_node args,
			      ttl_type type,
			      char * filename,
			      int beg_line, int beg_col,
			      int end_line, int end_col);
ttl_il_node ttl_make_il_index (ttl_compile_state state, ttl_il_node array,
			       ttl_il_node index,
			       ttl_type type,
			       char * filename,
			       int beg_line, int beg_col,
			       int end_line, int end_col);
ttl_il_node ttl_make_il_return (ttl_compile_state state, ttl_il_node expr,
				char * filename,
				int beg_line, int beg_col,
				int end_line, int end_col);
ttl_il_node ttl_make_il_require (ttl_compile_state state, ttl_il_node expr,
				 ttl_il_node stmt,
				 char * filename,
				 int beg_line, int beg_col,
				 int end_line, int end_col);
ttl_il_node ttl_make_il_ann_expr (ttl_compile_state state, ttl_il_node expr,
				  int strength,
				  char * filename,
				  int beg_line, int beg_col,
				  int end_line, int end_col);
ttl_il_node ttl_make_il_foreign_expr (ttl_compile_state state,
				      ttl_symbol expr,
				      char * filename,
				      int beg_line, int beg_col,
				      int end_line, int end_col);
ttl_il_node ttl_make_il_var_expr (ttl_compile_state state,
				  ttl_il_node expr, ttl_type type,
				  char * filename,
				  int beg_line, int beg_col,
				  int end_line, int end_col);
ttl_il_node ttl_make_il_deref_expr (ttl_compile_state state,
				    ttl_il_node expr, ttl_type type,
				    char * filename,
				    int beg_line, int beg_col,
				    int end_line, int end_col);

void ttl_il_print_stmtlist (FILE * f, ttl_il_node l, int indent);
int ttl_il_print (FILE * f, ttl_il_node node, int indent);

ttl_il_node ttl_il_reverse (ttl_compile_state state, ttl_il_node list);

void ttl_dump_il_module (FILE * f, ttl_compile_state state, ttl_module module);

void ttl_il_print_list (FILE * f, ttl_il_node_list l);

#endif /* not TTL_AST_H */
