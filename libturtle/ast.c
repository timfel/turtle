/* libturtle/ast.c - abstract syntax
 
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

#include "ast.h"
#include "util.h"

/* Please note that the filename passed to this function must be
   allocated in the same pool as passed in as `pool'.  */
ttl_ast_node
ttl_make_ast_node (ttl_pool pool, enum ttl_ast_kind kind, char * filename,
		   int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_malloc (pool, sizeof (struct ttl_ast_node));

  node->kind = kind;
  node->filename = filename;
  node->start_line = beg_line;
  node->start_column = beg_col;
  node->end_line = end_line;
  node->end_column = end_col;
  return node;
}


ttl_ast_node
ttl_make_ast_error (ttl_pool pool)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_error, NULL,
					 -1, -1, -1, -1);
  node->d.error.reported = 0;
  return node;
}

ttl_ast_node
ttl_make_ast_module (ttl_pool pool, ttl_ast_node name, ttl_ast_node params,
		     ttl_ast_node exports, ttl_ast_node imports,
		     ttl_ast_node definitions, char * documentation,
		     char * filename,
		     int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_module, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.module.name = name;
  node->d.module.params = params;
  node->d.module.exports = exports;
  node->d.module.imports = imports;
  node->d.module.definitions = definitions;
  node->d.module.documentation = documentation;
  return node;
}

ttl_ast_node
ttl_make_ast_pair (ttl_pool pool, ttl_ast_node car, ttl_ast_node cdr)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_pair, NULL, -1, -1, -1, -1);
  node->d.pair.car = car;
  node->d.pair.cdr = cdr;
  return node;
}

/* XXX: Correct for wide char usage.  */
ttl_ast_node
ttl_make_ast_string (ttl_pool pool, char * value, size_t len, char * filename,
		     int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_string_const,
					 filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.string.value = ttl_malloc (pool, len);
  memmove (node->d.string.value, value, len);
  node->d.string.length = len;
  return node;
}

/* XXX: Correct for wide char usage.  */
ttl_ast_node
ttl_make_ast_identifier (ttl_pool pool, ttl_symbol symbol, char * filename,
			 int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_identifier, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.identifier.symbol = symbol;
  return node;
}

/* XXX: Correct for wide char usage.  */
ttl_ast_node
ttl_make_ast_annotated_identifier (ttl_pool pool, ttl_ast_node identifier,
				   ttl_ast_node annotation,
				   ttl_ast_node open_list,
				   char * filename,
				   int beg_line, int beg_col,
				   int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_annotated_identifier,
					 filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.annotated_identifier.identifier = identifier;
  node->d.annotated_identifier.annotation = annotation;
  node->d.annotated_identifier.open_list = open_list;
  return node;
}

ttl_ast_node
ttl_make_ast_qualident (ttl_pool pool, ttl_ast_node module,
			ttl_ast_node identifier, char * filename,
			int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_qualident, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.qualident.module = module;
  node->d.qualident.identifier = identifier;
  return node;
}

ttl_ast_node
ttl_make_ast_character (ttl_pool pool, int value, char * filename,
			int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_char_const, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.character.value = value;
  return node;
}

ttl_ast_node
ttl_make_ast_integer (ttl_pool pool, long value, char * filename,
		      int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_int_const, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.integer.value = value;
  return node;
}

ttl_ast_node
ttl_make_ast_long (ttl_pool pool, long value, char * filename,
		   int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_long_const, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.longint.value = value;
  return node;
}

ttl_ast_node
ttl_make_ast_real (ttl_pool pool, double value, char * filename,
		   int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_real_const, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.real.value = value;
  return node;
}

ttl_ast_node
ttl_make_ast_bool (ttl_pool pool, int value, char * filename,
		   int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_bool_const, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.bool.value = value;
  return node;
}

ttl_ast_node
ttl_make_ast_null (ttl_pool pool, char * filename,
		   int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_null_const, filename,
					 beg_line, beg_col, end_line, end_col);
  return node;
}

ttl_ast_node
ttl_make_ast_array_expr (ttl_pool pool, ttl_ast_node elements, char * filename,
			 int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_array_expr, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.array_expr.elements = elements;
  return node;
}

ttl_ast_node
ttl_make_ast_list_expr (ttl_pool pool, ttl_ast_node elements, char * filename,
			int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_list_expr, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.list_expr.elements = elements;
  return node;
}

ttl_ast_node
ttl_make_ast_array_constructor (ttl_pool pool, ttl_ast_node size,
				ttl_ast_node initial, char * filename,
				int beg_line, int beg_col,
				int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_array_constructor, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.array_constructor.size = size;
  node->d.array_constructor.initial = initial;
  return node;
}

ttl_ast_node
ttl_make_ast_list_constructor (ttl_pool pool, ttl_ast_node size,
			       ttl_ast_node initial, char * filename,
			       int beg_line, int beg_col,
			       int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_list_constructor, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.list_constructor.size = size;
  node->d.list_constructor.initial = initial;
  return node;
}

ttl_ast_node
ttl_make_ast_string_constructor (ttl_pool pool, ttl_ast_node size,
				 ttl_ast_node initial, char * filename,
				 int beg_line, int beg_col,
				 int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_string_constructor,
					 filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.string_constructor.size = size;
  node->d.string_constructor.initial = initial;
  return node;
}

ttl_ast_node
ttl_make_ast_array_type (ttl_pool pool, ttl_ast_node element_type,
			 char * filename,
			 int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_array_type, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.array_type.element_type = element_type;
  return node;
}

ttl_ast_node
ttl_make_ast_string_type (ttl_pool pool, char * filename,
			  int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_string_type, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.string_type.unused = 0;
  return node;
}

ttl_ast_node
ttl_make_ast_list_type (ttl_pool pool, ttl_ast_node element_type,
			char * filename,
			int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_list_type, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.list_type.element_type = element_type;
  return node;
}

ttl_ast_node
ttl_make_ast_void_type (ttl_pool pool, char * filename,
			int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_void_type, filename,
					 beg_line, beg_col, end_line, end_col);
  return node;
}

ttl_ast_node
ttl_make_ast_tuple_type (ttl_pool pool, ttl_ast_node fields, char * filename,
			 int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_tuple_type, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.tuple_type.fields = fields;
  return node;
}

ttl_ast_node
ttl_make_ast_function_type (ttl_pool pool, ttl_ast_node left,
			    ttl_ast_node right, char * filename,
			    int beg_line, int beg_col,
			    int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_function_type, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.function_type.left = left;
  node->d.function_type.right = right;
  return node;
}

ttl_ast_node
ttl_make_ast_constrained_type (ttl_pool pool, ttl_ast_node base_type,
			       char * filename,
			       int beg_line, int beg_col,
			       int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_constrained_type, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.constrained_type.base_type = base_type;
  return node;
}

ttl_ast_node
ttl_make_ast_basic_type (ttl_pool pool, ttl_ast_node type_name,
			 char * filename,
			 int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_basic_type, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.basic_type.type_name = type_name;
  return node;
}

ttl_ast_node
ttl_make_ast_user_type (ttl_pool pool, ttl_ast_node type_name,
			ttl_ast_node types, char * filename,
			int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_user_type, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.user_type.type_name = type_name;
  node->d.user_type.types = types;
  return node;
}

ttl_ast_node
ttl_make_ast_binop (ttl_pool pool, enum ttl_binop op, ttl_ast_node op0,
		    ttl_ast_node op1, char * filename,
		    int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_binop, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.binop.op = op;
  node->d.binop.op0 = op0;
  node->d.binop.op1 = op1;
  return node;
}

ttl_ast_node
ttl_make_ast_unop (ttl_pool pool, enum ttl_unop op, ttl_ast_node op0,
		   char * filename,
		   int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_unop, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.unop.op = op;
  node->d.unop.op0 = op0;
  return node;
}

ttl_ast_node
ttl_make_ast_if (ttl_pool pool, ttl_ast_node cond, ttl_ast_node thenstmt,
		 ttl_ast_node elsestmt, char * filename,
		 int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_if, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.ifstmt.cond = cond;
  node->d.ifstmt.thenstmt = thenstmt;
  node->d.ifstmt.elsestmt = elsestmt;
  return node;
}

ttl_ast_node
ttl_make_ast_while (ttl_pool pool, ttl_ast_node cond, ttl_ast_node dostmt,
		    char * filename,
		    int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_while, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.whilestmt.cond = cond;
  node->d.whilestmt.dostmt = dostmt;
  return node;
}

ttl_ast_node
ttl_make_ast_in (ttl_pool pool, ttl_ast_node instmt, char * filename,
		    int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_in, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.instmt.instmt = instmt;
  return node;
}

ttl_ast_node
ttl_make_ast_call (ttl_pool pool, ttl_ast_node func, ttl_ast_node args,
		   char * filename,
		   int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_call, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.call.function = func;
  node->d.call.args = args;
  return node;
}

ttl_ast_node
ttl_make_ast_index (ttl_pool pool, ttl_ast_node array, ttl_ast_node index,
		    char * filename,
		    int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_index, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.index.array = array;
  node->d.index.index = index;
  return node;
}

ttl_ast_node
ttl_make_ast_return (ttl_pool pool, ttl_ast_node expr, char * filename,
		     int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_return, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.returnstmt.expr = expr;
  return node;
}

ttl_ast_node
ttl_make_ast_require (ttl_pool pool, ttl_ast_node expr, 
		      ttl_ast_node stmt, char * filename,
		      int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_require, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.require.expr = expr;
  node->d.require.stmt = stmt;
  return node;
}

ttl_ast_node
ttl_make_ast_function (ttl_pool pool, ttl_ast_node name, ttl_ast_node params,
		       ttl_ast_node type, ttl_ast_node body,
		       int public,
		       unsigned handcoded, char * documentation,
		       char * filename,
		       int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_function, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.function.name = name;
  node->d.function.params = params;
  node->d.function.type = type;
  node->d.function.body = body;
  node->d.function.public = public;
  node->d.function.handcoded = handcoded;
  node->d.function.mapped = 0;
  node->d.function.alias = NULL;
  node->d.function.documentation = documentation;
  return node;
}

ttl_ast_node
ttl_make_ast_constraint (ttl_pool pool, ttl_ast_node name, ttl_ast_node params,
			 ttl_ast_node body, int public, 
			 char * documentation, char * filename,
			 int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_constraint, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.constraint.name = name;
  node->d.constraint.params = params;
  node->d.constraint.body = body;
  node->d.constraint.public = public;
  node->d.constraint.documentation = documentation;
  return node;
}

/* FIXME: Provide source code position.  */
ttl_ast_node
ttl_make_ast_typedef (ttl_pool pool, ttl_ast_node name, ttl_ast_node type,
		      int public, char * documentation)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_typedef, NULL,
					 -1, -1, -1, -1);
  node->d.typedf.name = name;
  node->d.typedf.type = type;
  node->d.typedf.public = public;
  node->d.typedf.documentation = documentation;
  return node;
}


ttl_ast_node
ttl_make_ast_vardef (ttl_pool pool, ttl_ast_node list, int public,
		     char * documentation,
		     char * filename,
		     int beg_line, int beg_col,
		     int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_vardef, filename,
					 beg_line, beg_col,
					 end_line, end_col);
  node->d.vardef.list = list;
  node->d.vardef.public = public;
  node->d.vardef.documentation = documentation;
  return node;
}


ttl_ast_node
ttl_make_ast_constdef (ttl_pool pool, ttl_ast_node list, int public,
		       char * documentation, char * filename,
		       int beg_line, int beg_col,
		       int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_constdef, filename,
					 beg_line, beg_col,
					 end_line, end_col);
  node->d.constdef.list = list;
  node->d.constdef.public = public;
  node->d.constdef.documentation = documentation;
  return node;
}


ttl_ast_node
ttl_make_ast_variable (ttl_pool pool, ttl_ast_node name, ttl_ast_node type,
		       ttl_ast_node init, int out_param, char * filename,
		       int beg_line, int beg_col,
		       int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_variable, filename,
					 beg_line, beg_col,
					 end_line, end_col);
  node->d.variable.name = name;
  node->d.variable.type = type;
  node->d.variable.init = init;
  node->d.variable.out_param = out_param;
  return node;
}

/* FIXME: Provide source code position.  */
ttl_ast_node
ttl_make_ast_datatype_variant (ttl_pool pool, ttl_ast_node name,
			       ttl_ast_node fields)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_datatype_variant, NULL,
					 -1, -1, -1, -1);
  node->d.datatype_variant.name = name;
  node->d.datatype_variant.fields = fields;
  return node;
}

/* FIXME: Provide source code position.  */
ttl_ast_node
ttl_make_ast_datatype (ttl_pool pool, ttl_ast_node name, ttl_ast_node types,
		       ttl_ast_node variants, int public,
		       char * documentation)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_datatype,
					 NULL, -1, -1, -1, -1);
  node->d.datatype.name = name;
  node->d.datatype.types = types;
  node->d.datatype.variants = variants;
  node->d.datatype.public = public;
  node->d.datatype.documentation = documentation;
  return node;
}

ttl_ast_node
ttl_make_ast_tuple_expr (ttl_pool pool, ttl_ast_node fields, char * filename,
			 int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_tuple_expr, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.tuple_expr.fields = fields;
  return node;
}

ttl_ast_node
ttl_make_ast_ann_expr (ttl_pool pool, ttl_ast_node expr,
		       ttl_ast_node strength, char * filename,
		       int beg_line, int beg_col, int end_line, int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_ann_expr, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.ann_expr.expr = expr;
  node->d.ann_expr.strength = strength;
  return node;
}

ttl_ast_node
ttl_make_ast_foreign_expr (ttl_pool pool, ttl_symbol expr, char * filename,
			   int beg_line, int beg_col, int end_line,
			   int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_foreign_expr, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.foreign_expr.expr = expr;
  return node;
}

ttl_ast_node
ttl_make_ast_var_expr (ttl_pool pool, ttl_ast_node expr, char * filename,
		       int beg_line, int beg_col, int end_line,
		       int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_var_expr, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.var_expr.expr = expr;
  return node;
}

ttl_ast_node
ttl_make_ast_deref_expr (ttl_pool pool, ttl_ast_node expr, char * filename,
			 int beg_line, int beg_col, int end_line,
			 int end_col)
{
  ttl_ast_node node = ttl_make_ast_node (pool, ast_deref_expr, filename,
					 beg_line, beg_col, end_line, end_col);
  node->d.deref_expr.expr = expr;
  return node;
}

/* Print the AST list `l' with indentation `indent', where all
   elements of `l' are separated by the string `sep'.  */
static void
ast_print_list (FILE * f, ttl_ast_node l, char * sep, int indent)
{
  if (l)
    {
      ttl_ast_print (f, l->d.pair.car, indent);
      l = l->d.pair.cdr;
      while (l)
	{
	  fprintf (f, sep);
	  ttl_ast_print (f, l->d.pair.car, indent);
	  l = l->d.pair.cdr;
	}
    }
}

/* Print `indent' * 2 spaces to the output file `f'.  */
static void
spaces (FILE * f, int indent)
{
  int i;

  for (i = 0; i < indent; i++)
    fprintf (f, "  ");
}

/* Print a list of statements, one statement per line with proper
   indentation.  */
void
ttl_ast_print_stmtlist (FILE * f, ttl_ast_node l, int indent)
{
  while (l)
    {
      spaces (f, indent);
      ttl_ast_print (f, l->d.pair.car, indent);
      fprintf (f, ";\n");
      l = l->d.pair.cdr;
    }
}

/* Pretty-print the abstract syntax tree rooted at `node' to the
   output file `f'.  Indent it by `indent' * 2 spaces.  */
int
ttl_ast_print (FILE * f, ttl_ast_node node, int indent)
{
  switch (node->kind)
    {
    case ast_error:
      fprintf (f, "<<error node>>");
      break;
    case ast_module:
      fprintf (f, "module ");
      ttl_ast_print (f, node->d.module.name, indent);
      if (node->d.module.params)
	{
	  fprintf (f, "<");
	  ast_print_list (f, node->d.module.params, ", ", indent);
	  fprintf (f, ">");
	}
      fprintf (f, ";\n");
      if (node->d.module.imports)
	{
	  spaces (f, indent);
	  fprintf (f, "import ");
	  ast_print_list (f, node->d.module.imports, ", ", indent);
	  fprintf (f, ";\n");
	}
      if (node->d.module.exports)
	{
	  spaces (f, indent);
	  fprintf (f, "export ");
	  ast_print_list (f, node->d.module.exports, ", ", indent);
	  fprintf (f, ";\n");
	}
      ttl_ast_print_stmtlist (f, node->d.module.definitions, indent);
      break;

    case ast_pair:
      ttl_ast_print (f, node->d.pair.car, indent);
      node = node->d.pair.cdr;
      while (node)
	{
	  fprintf (f, ", ");
	  ttl_ast_print (f, node->d.pair.car, indent);
	  node = node->d.pair.cdr;
	}
      break;

    case ast_string_const:
      {
	size_t i = 0;
	fprintf (f, "\"");
	while (i < node->d.string.length)
	  {
/* 	    fprintf (f, "%c", node->d.string.value[i]); */
	    ttl_print_escaped_char (f, node->d.string.value[i]);
	    i++;
	  }
	fprintf (f, "\"");
	break;
      }

    case ast_identifier:
      {
	ttl_symbol_print (f, node->d.identifier.symbol);
/* 	fprintf (f, ":%x:%u", (unsigned) node->d.identifier.symbol, */
/* 		 node->d.identifier.symbol->raw_hash_code); */
	break;
      }

    case ast_annotated_identifier:
      {
	ttl_ast_print (f, node->d.annotated_identifier.identifier, indent);
	if (node->d.annotated_identifier.annotation)
	  {
	    fprintf (f, "<");
	    ast_print_list (f, node->d.annotated_identifier.annotation,
			    ", ", indent);
	    fprintf (f, ">");
	  }
	if (node->d.annotated_identifier.open_list)
	  {
	    fprintf (f, "(");
	    ast_print_list (f, node->d.annotated_identifier.open_list,
			    ", ", indent);
	    fprintf (f, ")");
	  }
	break;
      }

    case ast_qualident:
      if (node->d.qualident.module)
	{
	  ttl_ast_print (f, node->d.qualident.module, indent);
	  fprintf (f, ".");
	  ttl_ast_print (f, node->d.qualident.identifier, indent);
	}
      break;

    case ast_char_const:
      fprintf (f, "'");
      ttl_print_escaped_char (f, node->d.character.value);
      fprintf (f, "'");
/*       fprintf (f, "'%c'", node->d.character.value); */
      break;
      
    case ast_int_const:
      fprintf (f, "%ld", node->d.integer.value);
      break;

    case ast_long_const:
      fprintf (f, "%ldL", node->d.longint.value);
      break;

    case ast_real_const:
      fprintf (f, "%g", node->d.real.value);
      break;

    case ast_bool_const:
      fprintf (f, "%s", node->d.bool.value ? "true" : "false");
      break;

    case ast_null_const:
      fprintf (f, "null");
      break;

    case ast_binop:
      if (node->d.binop.op0->kind == ast_binop)
	fprintf (f, "(");
      ttl_ast_print (f, node->d.binop.op0, indent);
      if (node->d.binop.op0->kind == ast_binop)
	fprintf (f, ")");
      switch (node->d.binop.op)
	{
	case binop_add:
	  fprintf (f, " + ");
	  break;
	case binop_sub:
	  fprintf (f, " - ");
	  break;
	case binop_mul:
	  fprintf (f, " * ");
	  break;
	case binop_div:
	  fprintf (f, " / ");
	  break;
	case binop_mod:
	  fprintf (f, " %% ");
	  break;
	case binop_and:
	  fprintf (f, " and ");
	  break;
	case binop_or:
	  fprintf (f, " or ");
	  break;
	case binop_assign:
	case binop_assign_force:
	  fprintf (f, " := ");
	  break;
	case binop_eq:
	  fprintf (f, " = ");
	  break;
	case binop_ne:
	  fprintf (f, " <> ");
	  break;
	case binop_lt:
	  fprintf (f, " < ");
	  break;
	case binop_le:
	  fprintf (f, " <= ");
	  break;
	case binop_gt:
	  fprintf (f, " > ");
	  break;
	case binop_ge:
	  fprintf (f, " >= ");
	  break;
	case binop_cons:
	  fprintf (f, " :: ");
	  break;
	}
      if (node->d.binop.op1->kind == ast_binop)
	fprintf (f, "(");
      ttl_ast_print (f, node->d.binop.op1, indent);
      if (node->d.binop.op1->kind == ast_binop)
	fprintf (f, ")");
      break;

    case ast_unop:
      switch (node->d.unop.op)
	{
	case unop_neg:
	  fprintf (f, "-");
	  break;
	case unop_not:
	  fprintf (f, "not ");
	  break;
	case unop_hd:
	  fprintf (f, "hd ");
	  break;
	case unop_tl:
	  fprintf (f, "tl ");
	  break;
	case unop_sizeof:
	  fprintf (f, "sizeof ");
	  break;
	}
      ttl_ast_print (f, node->d.unop.op0, indent);
      break;
  
    case ast_if:
      fprintf (f, "if ");
      ttl_ast_print (f, node->d.ifstmt.cond, indent);
      fprintf (f, " then\n");
      ttl_ast_print_stmtlist (f, node->d.ifstmt.thenstmt, indent + 1);
      if (node->d.ifstmt.elsestmt)
	{
	  spaces (f, indent);
	  fprintf (f, "else\n");
	  ttl_ast_print_stmtlist (f, node->d.ifstmt.elsestmt, indent + 1);
	}
      spaces (f, indent);
      fprintf (f, "end");
      break;

    case ast_while:
      fprintf (f, "while ");
      ttl_ast_print (f, node->d.whilestmt.cond, indent);
      fprintf (f, " do\n");
      ttl_ast_print_stmtlist (f, node->d.whilestmt.dostmt, indent + 1);
      spaces (f, indent);
      fprintf (f," end");
      break;

    case ast_in:
      fprintf (f, "in\n");
      ttl_ast_print_stmtlist (f, node->d.instmt.instmt, indent + 1);
      spaces (f, indent);
      fprintf (f," end");
      break;

    case ast_call:
      ttl_ast_print (f, node->d.call.function, indent);
      fprintf (f, "(");
      ast_print_list (f, node->d.call.args, ", ", indent);
      fprintf (f, ")");
      break;

    case ast_index:
      ttl_ast_print (f, node->d.index.array, indent);
      fprintf (f, "[");
      ttl_ast_print (f, node->d.index.index, indent);
      fprintf (f, "]");
      break;

    case ast_return:
      fprintf (f, "return");
      if (node->d.returnstmt.expr)
	{
	  fprintf (f, " ");
	  ttl_ast_print (f, node->d.returnstmt.expr, indent);
	}
      break;

    case ast_require:
      {
	ttl_ast_node l = node->d.require.expr;
	fprintf (f, "require ");
	while (l)
	  {
	    ttl_ast_print (f, l->d.pair.car, indent);
	    l = l->d.pair.cdr;
	    if (l)
	      fprintf (f, " and ");
	  }
	if (node->d.require.stmt)
	  {
	    fprintf (f, "\n");
	    ttl_ast_print (f, node->d.require.stmt, indent + 1);
	  }
      }
      break;

    case ast_function:
      fprintf (f, "fun ");
      if (node->d.function.name)
	ttl_ast_print (f, node->d.function.name, indent);
      fprintf (f, "(");
      ast_print_list (f, node->d.function.params, ", ", indent);
      fprintf (f, ")");
      if (node->d.function.type)
	{
	  fprintf (f, ": ");
	  ttl_ast_print (f, node->d.function.type, indent);
	}
      if (node->d.function.mapped)
	{
	  fprintf (f, " = ");
	  ttl_symbol_print (f, node->d.function.alias);
	}
      else if (!node->d.function.handcoded)
	{
	  fprintf (f, "\n");
	  ttl_ast_print_stmtlist (f, node->d.function.body, indent + 1);
	  spaces (f, indent);
	  fprintf (f, "end");
	}
      break;

    case ast_constraint:
      fprintf (f, "constraint ");
      if (node->d.constraint.name)
	ttl_ast_print (f, node->d.constraint.name, indent);
      fprintf (f, "(");
      ast_print_list (f, node->d.constraint.params, ", ", indent);
      fprintf (f, ")\n");
      ttl_ast_print_stmtlist (f, node->d.constraint.body, indent + 1);
      spaces (f, indent);
      fprintf (f, "end");
      break;

    case ast_vardef:
      fprintf (f, "var ");
      ast_print_list (f, node->d.vardef.list, ", ", indent);
      break;

    case ast_constdef:
      fprintf (f, "const ");
      ast_print_list (f, node->d.vardef.list, ", ", indent);
      break;

    case ast_typedef:
      fprintf (f, "type ");
      ttl_ast_print (f, node->d.typedf.name, indent);
      fprintf (f, " = ");
      ttl_ast_print (f, node->d.typedf.type, indent);
      break;

    case ast_variable:
      if (node->d.variable.out_param)
	fprintf (f, "out ");
      ttl_ast_print (f, node->d.variable.name, indent);
      fprintf (f, ": ");
      if (node->d.variable.type)
	ttl_ast_print (f, node->d.variable.type, indent);
      else
	fprintf (f, "type");
      if (node->d.variable.init)
	{
	  fprintf (f, " := ");
	  ttl_ast_print (f, node->d.variable.init, indent);
	}
      break;

    case ast_array_expr:
      fprintf (f, "{");
      if (node->d.array_expr.elements)
	ttl_ast_print (f, node->d.array_expr.elements, indent);
      fprintf (f, "}");
      break;

    case ast_list_expr:
      fprintf (f, "[");
      if (node->d.list_expr.elements)
	ttl_ast_print (f, node->d.list_expr.elements, indent);
      fprintf (f, "]");
      break;

    case ast_array_constructor:
      fprintf (f, "array ");
      ttl_ast_print (f, node->d.array_constructor.size, indent);
      if (node->d.array_constructor.initial)
	{
	  fprintf (f, " of ");
	  ttl_ast_print (f, node->d.array_constructor.initial, indent);
	}
      break;

    case ast_list_constructor:
      fprintf (f, "list ");
      ttl_ast_print (f, node->d.list_constructor.size, indent);
      if (node->d.list_constructor.initial)
	{
	  fprintf (f, " of ");
	  ttl_ast_print (f, node->d.list_constructor.initial, indent);
	}
      break;

    case ast_string_constructor:
      fprintf (f, "string ");
      ttl_ast_print (f, node->d.string_constructor.size, indent);
      if (node->d.string_constructor.initial)
	{
	  fprintf (f, " of ");
	  ttl_ast_print (f, node->d.string_constructor.initial, indent);
	}
      break;

    case ast_array_type:
      fprintf (f, "array of ");
      ttl_ast_print (f, node->d.array_type.element_type, indent);
      break;

    case ast_string_type:
      fprintf (f, "string");
      break;

    case ast_list_type:
      fprintf (f, "list of ");
      ttl_ast_print (f, node->d.list_type.element_type, indent);
      break;

    case ast_void_type:
      fprintf (f, "()");
      break;

    case ast_tuple_type:
      {
	ttl_ast_node l = node->d.tuple_type.fields;

	fprintf (f, "(");
	ttl_ast_print (f, l->d.pair.car, indent);
	l = l->d.pair.cdr;
	while (l)
	  {
	    fprintf (f, ", ");
	    ttl_ast_print (f, l->d.pair.car, indent);
	    l = l->d.pair.cdr;
	  }
	fprintf (f, ")");
      }
      break;

    case ast_function_type:
      fprintf (f, "fun(");
      ast_print_list (f, node->d.function_type.left, ", ", 0);
      fprintf (f, ")");
      if (node->d.function_type.right)
	{
	  fprintf (f, ": ");
	  ttl_ast_print (f, node->d.function_type.right, indent);
	}
      break;

    case ast_constrained_type:
      fprintf (f, "! ");
      ttl_ast_print (f, node->d.constrained_type.base_type, indent);
      break;

    case ast_basic_type:
      ttl_ast_print (f, node->d.basic_type.type_name, indent);
      break;

    case ast_user_type:
      ttl_ast_print (f, node->d.user_type.type_name, indent);
      if (node->d.user_type.types)
	{
	  fprintf (f, "<");
	  ast_print_list (f, node->d.user_type.types, ", ", indent);
	  fprintf (f, ">");
	}
      break;

    case ast_datatype_variant:
      ttl_ast_print (f, node->d.datatype_variant.name, indent);
      if (node->d.datatype_variant.fields)
	{
	  fprintf (f, "(");
	  ast_print_list (f, node->d.datatype_variant.fields, ", ", indent);
	  fprintf (f, ")");
	}
      break;

    case ast_datatype:
      {
	ttl_ast_node l;

	fprintf (f, "datatype ");
	ttl_ast_print (f, node->d.datatype.name, indent);
	if (node->d.datatype.types)
	  {
	    fprintf (f, "<");
	    ast_print_list (f, node->d.datatype.types, ", ", indent);
	    fprintf (f, ">");
	  }
	fprintf (f, " =\n");
	l = node->d.datatype.variants;
	if (l)
	  {
	    spaces (f, indent + 1);
	    ttl_ast_print (f, l->d.pair.car, indent);
	    l = l->d.pair.cdr;
	    while (l)
	      {
		fprintf (f, " or\n");
		spaces (f, indent + 1);
		ttl_ast_print (f, l->d.pair.car, indent);
		l = l->d.pair.cdr;
	      }
	  }
	break;
      }

    case ast_tuple_expr:
      {
	ttl_ast_node l = node->d.tuple_expr.fields;

	fprintf (f, "(");
	ttl_ast_print (f, l->d.pair.car, indent);
	l = l->d.pair.cdr;
	while (l)
	  {
	    fprintf (f, ", ");
	    ttl_ast_print (f, l->d.pair.car, indent);
	    l = l->d.pair.cdr;
	  }
	fprintf (f, ")");
      }
      break;

    case ast_ann_expr:
      {
	ttl_ast_print (f, node->d.ann_expr.expr, indent);
	fprintf (f, " : ");
	ttl_ast_print (f, node->d.ann_expr.strength, indent);
      }
      break;

    case ast_foreign_expr:
      {
	fprintf (f, "foreign \"");
	ttl_symbol_print (f, node->d.foreign_expr.expr);
	fprintf (f, "\"");
      }
      break;

    case ast_var_expr:
      {
	fprintf (f, "var ");
	ttl_ast_print (f, node->d.var_expr.expr, indent);
      }
      break;

    case ast_deref_expr:
      {
	fprintf (f, "!");
	ttl_ast_print (f, node->d.deref_expr.expr, indent);
      }
      break;

    }
  return 0;
}

size_t
ttl_ast_length (ttl_ast_node l)
{
  size_t len = 0;
  while (l)
    {
      l = l->d.pair.cdr;
      len++;
    }
  return len;
}

int
ttl_compare_identifiers (ttl_ast_node name1, ttl_ast_node name2)
{
  name1 = ttl_strip_annotation (name1);
  name2 = ttl_strip_annotation (name2);
  if (name1->kind == ast_identifier && name2->kind == ast_identifier)
    {
      return name1->d.identifier.symbol == name2->d.identifier.symbol;
    }
  else if (name1->kind == ast_qualident && name2->kind == ast_qualident)
    {
      return ttl_compare_identifiers (name1->d.qualident.module,
				      name2->d.qualident.module) &&
	ttl_compare_identifiers (name1->d.qualident.identifier,
				 name2->d.qualident.identifier);
    }
  else
    return 0;
}

ttl_ast_node
ttl_strip_annotation (ttl_ast_node name)
{
  while (name->kind == ast_annotated_identifier)
    name = name->d.annotated_identifier.identifier;
  return name;
}

/* End of ast.c.  */
