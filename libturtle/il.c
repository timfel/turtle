/* libturtle/il.c - intermediate language
 
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

#include "util.h"
#include "il.h"

ttl_il_node_list
ttl_il_cons (ttl_compile_state state, ttl_il_node node, ttl_il_node_list next)
{
  ttl_il_node_list ls = ttl_malloc (state->pool,
				    sizeof (struct ttl_il_node_list));
  ls->elem = node;
  ls->next = next;
  return ls;
}

static ttl_il_node
ttl_make_il_node (ttl_compile_state state, enum ttl_il_kind kind,
		  ttl_type type, char * filename,
		  int beg_line, int beg_col,
		  int end_line, int end_col)
{
  ttl_il_node node = ttl_malloc (state->pool, sizeof (struct ttl_il_node));

  node->kind = kind;
  node->filename = filename;
  node->start_line = beg_line;
  node->start_column = beg_col;
  node->end_line = end_line;
  node->end_column = end_col;
  node->type = type;
  node->ambiguous = 0;
  node->lvalue = 0;
  return node;
}


ttl_il_node
ttl_make_il_error (ttl_compile_state state)
{
  ttl_il_node node = ttl_make_il_node (state, il_error, state->error_type,
				       NULL, -1, -1, -1, -1);
  node->d.error.reported = 0;
  return node;
}

ttl_il_node
ttl_make_il_module (ttl_compile_state state, ttl_symbol name, 
		     ttl_il_node exports, ttl_il_node imports,
		     ttl_il_node definitions, char * filename,
		     int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_module, state->void_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.module.name = name;
  node->d.module.exports = exports;
  node->d.module.imports = imports;
  node->d.module.definitions = definitions;
  return node;
}

ttl_il_node
ttl_make_il_pair (ttl_compile_state state, ttl_il_node car, ttl_il_node cdr)
{
  ttl_il_node node = ttl_make_il_node (state, il_pair, state->void_type,
				       NULL, -1, -1, -1, -1);
  node->d.pair.car = car;
  node->d.pair.cdr = cdr;
  return node;
}

ttl_il_node
ttl_make_il_variable (ttl_compile_state state, ttl_ast_node name,
		      ttl_symbol mangled_name, ttl_type type,
		      unsigned lvalue, char * filename,
		      int beg_line, int beg_col,
		      int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_variable, type, 
				       filename,
				       beg_line, beg_col, 
				       end_line, end_col);
  node->d.variable.name = name;
  node->d.variable.mangled_name = mangled_name;
  node->d.variable.variable = NULL;
  node->lvalue = lvalue;
  return node;
}

ttl_il_node
ttl_make_il_function (ttl_compile_state state, ttl_ast_node name,
		      ttl_symbol mangled_name, ttl_type type,
		      ttl_function function)
{
  ttl_il_node node = ttl_make_il_node (state, il_function, type, NULL,
				       -1, -1, -1, -1);
  node->d.function.name = name;
  node->d.function.mangled_name = mangled_name;
  node->d.function.function = function;
  return node;
}

/* XXX: Correct for wide char usage.  */
ttl_il_node
ttl_make_il_string (ttl_compile_state state, char * value, size_t len,
		     char * filename,
		     int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_string_const,
				       state->string_type, filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.string.text = ttl_malloc (state->pool, len);
  memmove (node->d.string.text, value, len);
  node->d.string.length = len;
  return node;
}

ttl_il_node
ttl_make_il_character (ttl_compile_state state, int value, char * filename,
		       int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_char_const, state->char_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.character.value = value;
  return node;
}

ttl_il_node
ttl_make_il_integer (ttl_compile_state state, long value, char * filename,
		     int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_int_const, state->int_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.integer.value = value;
  return node;
}

ttl_il_node
ttl_make_il_long (ttl_compile_state state, long value, char * filename,
		  int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_long_const, state->long_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.longint.value = value;
  return node;
}

ttl_il_node
ttl_make_il_real (ttl_compile_state state, double value, char * filename,
		  int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_real_const, state->real_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.real.value = value;
  return node;
}

ttl_il_node
ttl_make_il_bool (ttl_compile_state state, int value, char * filename,
		  int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_bool_const, state->bool_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.bool.value = value;
  return node;
}

ttl_il_node
ttl_make_il_null (ttl_compile_state state, char * filename,
		  int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_null_const, state->nil_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  return node;
}

ttl_il_node
ttl_make_il_array_expr (ttl_compile_state state, ttl_il_node elements,
			ttl_type type, char * filename,
			int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_array_expr,
				       type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.array_expr.elements = elements;
  return node;
}

ttl_il_node
ttl_make_il_list_expr (ttl_compile_state state, ttl_il_node elements,
		       ttl_type type, char * filename,
		       int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_list_expr, type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.list_expr.elements = elements;
  return node;
}

ttl_il_node
ttl_make_il_tuple_expr (ttl_compile_state state, ttl_il_node elements,
			ttl_type type, char * filename,
			int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_tuple_expr, type, filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.tuple_expr.elements = elements;
  return node;
}

ttl_il_node
ttl_make_il_array_constructor (ttl_compile_state state, ttl_il_node size,
			       ttl_il_node initial, ttl_type type,
			       char * filename, int beg_line, int beg_col,
			       int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_array_constructor, type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.array_constructor.size = size;
  node->d.array_constructor.initial = initial;
  return node;
}

ttl_il_node
ttl_make_il_list_constructor (ttl_compile_state state, ttl_il_node size,
			      ttl_il_node initial, ttl_type type,
			      char * filename, int beg_line, int beg_col,
			      int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_list_constructor, type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.list_constructor.size = size;
  node->d.list_constructor.initial = initial;
  return node;
}

ttl_il_node
ttl_make_il_string_constructor (ttl_compile_state state, ttl_il_node size,
				ttl_il_node initial, ttl_type type,
				char * filename, int beg_line, int beg_col,
				int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_string_constructor, type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.string_constructor.size = size;
  node->d.string_constructor.initial = initial;
  return node;
}

ttl_il_node
ttl_make_il_seq (ttl_compile_state state, ttl_il_node stmts)
{
  ttl_il_node node = ttl_make_il_node (state, il_seq, state->void_type, NULL,
				       -1, -1, -1, -1);
  node->d.seq.stmts = stmts;
  if (stmts)
    {
      node->filename = stmts->d.pair.car->filename;
      node->start_line = stmts->d.pair.car->start_line;
      node->start_column = stmts->d.pair.car->start_column;
      node->end_line = stmts->d.pair.car->end_line;
      node->end_column = stmts->d.pair.car->end_column;
    }
  return node;
}

ttl_il_node
ttl_make_il_binop (ttl_compile_state state, enum ttl_il_binop op,
		   ttl_il_node op0, ttl_il_node op1, ttl_type type,
		   char * filename,
		   int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_binop, type, filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.binop.op = op;
  node->d.binop.op0 = op0;
  node->d.binop.op1 = op1;
  return node;
}

ttl_il_node
ttl_make_il_unop (ttl_compile_state state, enum ttl_il_unop op,
		  ttl_il_node op0, ttl_type type, char * filename,
		  int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_unop, type, filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.unop.op = op;
  node->d.unop.op0 = op0;
  return node;
}

ttl_il_node
ttl_make_il_if (ttl_compile_state state, ttl_il_node cond,
		ttl_il_node thenstmt, ttl_il_node elsestmt, char * filename,
		int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_if, state->void_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.ifstmt.cond = cond;
  node->d.ifstmt.thenstmt = thenstmt;
  node->d.ifstmt.elsestmt = elsestmt;
  return node;
}

ttl_il_node
ttl_make_il_while (ttl_compile_state state, ttl_il_node cond,
		   ttl_il_node dostmt, char * filename,
		   int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_while, state->void_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.whilestmt.cond = cond;
  node->d.whilestmt.dostmt = dostmt;
  return node;
}

ttl_il_node
ttl_make_il_in (ttl_compile_state state, ttl_il_node instmt, char * filename,
		int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_in, state->void_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.instmt.instmt = instmt;
  return node;
}

ttl_il_node
ttl_make_il_call (ttl_compile_state state, ttl_il_node func, ttl_il_node args,
		  ttl_type type, char * filename,
		  int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_call, type, filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.call.function = func;
  node->d.call.args = args;
  return node;
}

ttl_il_node
ttl_make_il_index (ttl_compile_state state, ttl_il_node array,
		   ttl_il_node index, ttl_type type, char * filename,
		   int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_index, type, filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.index.array = array;
  node->d.index.index = index;
  return node;
}

ttl_il_node
ttl_make_il_return (ttl_compile_state state, ttl_il_node expr, char * filename,
		    int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_return, state->void_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.returnstmt.expr = expr;
  return node;
}

ttl_il_node
ttl_make_il_require (ttl_compile_state state, ttl_il_node expr,
		     ttl_il_node stmt, char * filename,
		     int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_require, state->void_type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.require.expr = expr;
  node->d.require.stmt = stmt;
  return node;
}

ttl_il_node
ttl_make_il_ann_expr (ttl_compile_state state, ttl_il_node expr,
		      int strength, char * filename,
		      int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_ann_expr, expr->type,
				       filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.ann_expr.expr = expr;
  node->d.ann_expr.strength = strength;
  return node;
}

ttl_il_node
ttl_make_il_foreign_expr (ttl_compile_state state, ttl_symbol expr,
			  char * filename, int beg_line, int beg_col,
			  int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_foreign_expr, 
				       state->any_type, filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.foreign_expr.expr = expr;
  return node;
}

ttl_il_node
ttl_make_il_var_expr (ttl_compile_state state, ttl_il_node expr,
		      ttl_type type,
		      char * filename, int beg_line, int beg_col,
		      int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_var_expr, 
				       type, filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.var_expr.expr = expr;
  return node;
}

ttl_il_node
ttl_make_il_deref_expr (ttl_compile_state state, ttl_il_node expr,
			ttl_type type,
			char * filename, int beg_line, int beg_col,
			int end_line, int end_col)
{
  ttl_il_node node = ttl_make_il_node (state, il_deref_expr, 
				       type, filename,
				       beg_line, beg_col, end_line, end_col);
  node->d.deref_expr.expr = expr;
  return node;
}

static void
il_print_list (FILE * f, ttl_il_node l, char * sep, int indent)
{
  if (l)
    {
      ttl_il_print (f, l->d.pair.car, indent);
      l = l->d.pair.cdr;
      while (l)
	{
	  fprintf (f, sep);
	  ttl_il_print (f, l->d.pair.car, indent);
	  l = l->d.pair.cdr;
	}
    }
}

static void
spaces (FILE * f, int indent)
{
  int i;

  for (i = 0; i < indent; i++)
    fprintf (f, "  ");
}

void
ttl_il_print_stmtlist (FILE * f, ttl_il_node l, int indent)
{
  if (l)
    {
      while (l)
	{
	  if (l->d.pair.car)
	    {
	      spaces (f, indent);
	      ttl_il_print (f, l->d.pair.car, indent);
	      l = l->d.pair.cdr;
	      fprintf (f, "\n");
	    }
	  else
	    l = l->d.pair.cdr;
	}
    }
}

int
ttl_il_print (FILE * f, ttl_il_node node, int indent)
{
  if (!node)
    return 0;
/*   if (node->type->kind != type_void) */
/*     ttl_print_type (f, node->type); */
  switch (node->kind)
    {
    case il_error:
      fprintf (f, "<<error node>>");
      break;
    case il_module:
      fprintf (f, "(module ");
      ttl_symbol_print (f, node->d.module.name);
      fprintf (f, "\n");
      spaces (f, indent);
      fprintf (f, "(import ");
      il_print_list (f, node->d.module.imports, ", ", indent);
      fprintf (f, ")\n");
      spaces (f, indent);
      fprintf (f, "(export ");
      il_print_list (f, node->d.module.exports, ", ", indent);
      fprintf (f, ")\n");
      ttl_il_print_stmtlist (f, node->d.module.definitions, indent);
      fprintf (f, ")\n");
      break;

    case il_pair:
      ttl_il_print (f, node->d.pair.car, indent);
      node = node->d.pair.cdr;
      while (node)
	{
	  fprintf (f, ", ");
	  ttl_il_print (f, node->d.pair.car, indent);
	  node = node->d.pair.cdr;
	}
      break;

    case il_string_const:
      {
	size_t i = 0;
	fprintf (f, "\"");
	while (i < node->d.string.length)
	  {
	    ttl_print_escaped_char (f, node->d.string.text[i]);
	    i++;
	  }
	fprintf (f, "\"");
	break;
      }

    case il_variable:
      ttl_symbol_print (f, node->d.variable.mangled_name);
      break;

    case il_function:
      ttl_symbol_print (f, node->d.function.mangled_name);
      break;

    case il_char_const:
      fprintf (f, "'");
      ttl_print_escaped_char (f, node->d.character.value);
      fprintf (f, "'");
      break;
      
    case il_int_const:
      fprintf (f, "%ld", node->d.integer.value);
      break;

    case il_long_const:
      fprintf (f, "%ldL", node->d.longint.value);
      break;

    case il_real_const:
      fprintf (f, "%g", node->d.real.value);
      break;

    case il_bool_const:
      fprintf (f, "%s", node->d.bool.value ? "true" : "false");
      break;

    case il_null_const:
      fprintf (f, "null");
      break;

    case il_binop:
      if (node->d.binop.op0->kind == il_binop)
	fprintf (f, "(");
      ttl_il_print (f, node->d.binop.op0, indent);
      if (node->d.binop.op0->kind == il_binop)
	fprintf (f, ")");
      switch (node->d.binop.op)
	{
	case il_binop_add:
	  fprintf (f, " + ");
	  break;
	case il_binop_sub:
	  fprintf (f, " - ");
	  break;
	case il_binop_mul:
	  fprintf (f, " * ");
	  break;
	case il_binop_div:
	  fprintf (f, " / ");
	  break;
	case il_binop_mod:
	  fprintf (f, " %% ");
	  break;
	case il_binop_and:
	  fprintf (f, " and ");
	  break;
	case il_binop_or:
	  fprintf (f, " or ");
	  break;
	case il_binop_assign:
	  fprintf (f, " := ");
	  break;
	case il_binop_eq:
	  fprintf (f, " = ");
	  break;
	case il_binop_ne:
	  fprintf (f, " <> ");
	  break;
	case il_binop_lt:
	  fprintf (f, " < ");
	  break;
	case il_binop_le:
	  fprintf (f, " <= ");
	  break;
	case il_binop_gt:
	  fprintf (f, " > ");
	  break;
	case il_binop_ge:
	  fprintf (f, " >= ");
	  break;
	case il_binop_cons:
	  fprintf (f, " :: ");
	  break;
	}
      if (node->d.binop.op1->kind == il_binop)
	fprintf (f, "(");
      ttl_il_print (f, node->d.binop.op1, indent);
      if (node->d.binop.op1->kind == il_binop)
	fprintf (f, ")");
      break;

    case il_unop:
      switch (node->d.unop.op)
	{
	case il_unop_neg:
	  fprintf (f, "-");
	  break;
	case il_unop_not:
	  fprintf (f, "not ");
	  break;
	case il_unop_hd:
	  fprintf (f, "hd ");
	  break;
	case il_unop_tl:
	  fprintf (f, "tl ");
	  break;
	case il_unop_sizeof:
	  fprintf (f, "sizeof ");
	  break;
	}
      ttl_il_print (f, node->d.unop.op0, indent);
      break;
  
    case il_if:
      fprintf (f, "if ");
      ttl_il_print (f, node->d.ifstmt.cond, indent);
      fprintf (f, "\n");
      ttl_il_print_stmtlist (f, node->d.ifstmt.thenstmt, indent + 1);
      if (node->d.ifstmt.elsestmt)
	{
	  spaces (f, indent);
	  fprintf (f, "else\n");
	  ttl_il_print_stmtlist (f, node->d.ifstmt.elsestmt, indent + 1);
	}
      spaces (f, indent);
/*       fprintf (f, "end"); */
      break;

    case il_while:
      fprintf (f, "while ");
      ttl_il_print (f, node->d.whilestmt.cond, indent);
      fprintf (f, "\n");
      ttl_il_print_stmtlist (f, node->d.whilestmt.dostmt, indent + 1);
      spaces (f, indent);
/*       fprintf (f, "end"); */
      break;

    case il_in:
      fprintf (f, "in\n");
      ttl_il_print_stmtlist (f, node->d.instmt.instmt, indent + 1);
      spaces (f, indent);
/*       fprintf (f, "end"); */
      break;

    case il_call:
      ttl_il_print (f, node->d.call.function, indent);
      fprintf (f, "(");
      if (node->d.call.args)
	{
	  il_print_list (f, node->d.call.args, ", ", indent);
	}
      fprintf (f, ")");
      break;

    case il_index:
      ttl_il_print (f, node->d.index.array, indent);
      fprintf (f, "[");
      ttl_il_print (f, node->d.index.index, indent);
      fprintf (f, "]");
      break;

    case il_return:
      fprintf (f, "return");
      if (node->d.returnstmt.expr)
	{
	  fprintf (f, " ");
	  ttl_il_print (f, node->d.returnstmt.expr, indent);
	}
      break;

    case il_require:
      fprintf (f, "require ");
      ttl_il_print (f, node->d.require.expr, indent);
      fprintf (f, "\n");
      ttl_il_print_stmtlist (f, node->d.require.stmt, indent + 1);
      spaces (f, indent);
      break;

    case il_array_expr:
      fprintf (f, "{");
      ttl_il_print (f, node->d.array_expr.elements, indent);
      fprintf (f, "}");
      break;

    case il_list_expr:
      fprintf (f, "[");
      ttl_il_print (f, node->d.list_expr.elements, indent);
      fprintf (f, "]");
      break;

    case il_tuple_expr:
      fprintf (f, "(");
      ttl_il_print (f, node->d.tuple_expr.elements, indent);
      fprintf (f, ")");
      break;

    case il_array_constructor:
      fprintf (f, "array ");
      ttl_il_print (f, node->d.array_constructor.size, indent);
      fprintf (f, " of ");
      ttl_il_print (f, node->d.array_constructor.initial, indent);
      break;

    case il_list_constructor:
      fprintf (f, "list ");
      ttl_il_print (f, node->d.list_constructor.size, indent);
      fprintf (f, " of ");
      ttl_il_print (f, node->d.list_constructor.initial, indent);
      break;

    case il_string_constructor:
      fprintf (f, "string ");
      ttl_il_print (f, node->d.string_constructor.size, indent);
      fprintf (f, " of ");
      ttl_il_print (f, node->d.string_constructor.initial, indent);
      break;

    case il_seq:
      ttl_il_print_stmtlist (f, node->d.seq.stmts, indent);
      break;

    case il_ann_expr:
      ttl_il_print (f, node->d.ann_expr.expr, indent);
      fprintf (f, " : %d", node->d.ann_expr.strength);
      break;

    case il_foreign_expr:
      fprintf (f, "foreign ");
      ttl_symbol_print (f, node->d.foreign_expr.expr);
      break;

    case il_var_expr:
      fprintf (f, "var ");
      ttl_il_print (f, node->d.var_expr.expr, indent);
      break;

    case il_deref_expr:
      fprintf (f, "!");
      ttl_il_print (f, node->d.deref_expr.expr, indent);
      break;
    }
  return 0;
}

ttl_il_node
ttl_il_reverse (ttl_compile_state state, ttl_il_node list)
{
  ttl_il_node res = NULL;

  while (list)
    {
      res = ttl_make_il_pair (state, list->d.pair.car, res);
      list = list->d.pair.cdr;
    }
  return res;
}

static void
dump_il_function (FILE * f, ttl_compile_state state, ttl_function function)
{
  ttl_variable var;

  fprintf (f, "; Function ");
  ttl_symbol_print (f, function->name);
  fprintf (f, " ");
  ttl_print_type (f, function->type);
  fprintf (f, "\n");

  if (function->exported)
    {
      fprintf (f, ".export ");
      ttl_symbol_print (f, function->unique_name);
      fprintf (f, "\n");
    }
  fprintf (f, ".fun ");
  ttl_symbol_print (f, function->unique_name);
  fprintf (f, "\n");
  var = function->params;
  while (var)
    {
      fprintf (f, "\t.param ");
      ttl_symbol_print (f, var->unique_name);
      fprintf (f, " # ");
      ttl_print_type (f, var->type);
      fprintf (f, "\n");
      var = var->next;
    }
  var = function->locals;
  while (var)
    {
      fprintf (f, "\t.local ");
      ttl_symbol_print (f, var->unique_name);
      fprintf (f, " # ");
      ttl_print_type (f, var->type);
      fprintf (f, "\n");
      var = var->next;
    }
  fprintf (f, "\n");
  if (function->kind == function_function && function->d.function.il_code)
    ttl_il_print_stmtlist (f, (ttl_il_node)(function->d.function.il_code), 4);
  fprintf (f, "\n");
}

static void
dump_il_variable (FILE * f, ttl_compile_state state,
			  ttl_variable variable)
{
  fprintf (f, "; Variable ");
  ttl_symbol_print (f, variable->name);
  fprintf (f, " ");
  ttl_print_type (f, variable->type);
  fprintf (f, "\n");

  if (variable->exported)
    {
      fprintf (f, ".export ");
      ttl_symbol_print (f, variable->unique_name);
      fprintf (f, "\n");
    }
  fprintf (f, ".var ");
  ttl_symbol_print (f, variable->unique_name);
  fprintf (f, "\n\n");
}

static ttl_module_list
dump_il_init_sequence (FILE * f, ttl_compile_state state, ttl_module module,
		    ttl_module_list done)
{
  ttl_module_list list = module->imported;

  while (list)
    {
      if (!list->module->recursing &&
	  !ttl_module_find (list->module->module_ast_name, done))
	{
	  list->module->recursing = 1;
	  done = dump_il_init_sequence (f, state, list->module,
				     ttl_module_cons (state->pool,
						      list->module, done));
	  fprintf (f, "          _init_");
	  ttl_ast_print (f,
			 ttl_strip_annotation (list->module->module_ast_name),
			 0);
	  fprintf (f, "()\n");
	  list->module->recursing = 0;
	}
      list = list->next;
    }
  return done;
}

void
ttl_dump_il_module (FILE * f, ttl_compile_state state, ttl_module module)
{
  {
    ttl_function function = module->functions;
    ttl_variable variable = module->globals;
    while (variable)
      {
	dump_il_variable (f, state, variable);
	variable = variable->next;
      }
    while (function)
      {
	dump_il_function (f, state, function);
	function = function->total_next;
      }
    fprintf (f, ".var _initialized = false;\n\n.export _init_");
    ttl_ast_print (f,
		   ttl_strip_annotation (module->module_ast_name), 0);
    fprintf (f, "\n.fun _init_");
    ttl_ast_print (f,
		   ttl_strip_annotation (module->module_ast_name), 0);
    fprintf (f, "\n");
    fprintf (f, "\n        if not _initialized then\n");
    fprintf (f, "          _initialized = true\n");
    dump_il_init_sequence (f, state, module, NULL);
/*     ttl_il_print_stmtlist (f, (ttl_il_node)(state->init_stmts), 1); */
    fprintf (f, "\n");
  }
}

void
ttl_il_print_list (FILE * f, ttl_il_node_list l)
{
  while (l)
    {
      ttl_il_print (f, l->elem, 0);
      l = l->next;
      if (l)
	fprintf (f, ", ");
    }
}

/* End of il.c.  */
