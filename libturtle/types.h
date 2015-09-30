/* libturtle/types.h - data type representations
 
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

#ifndef TTL_TYPES_H
#define TTL_TYPES_H

#include <stdio.h>

#include "memory.h"
#include "symbols.h"
#include "ast.h"

/* FIXME: 
   - Remove `any' type kind, if we find a solution for higher-order
     function typing
   - Fix up the handling of forward references and `alias' types.
   - Fix the handling of `sum' data types, especially preserve the
     variants of the type.
*/

/* The following datatype and function declarations implement the
   internal representation of Turtle data types.  

   When reading in datatype definitions from a source file, they are
   translated to type descriptors, which are then used internally for
   typing variables and functions and for performing type checks. 

   The basic types (void, int, long, real, bool, char and string) are
   built-in and automatically defined, tuples, arrays, list, function
   and sum types are created by datatype declarations and type
   expressions.  */

enum ttl_type_kind
  {type_error, 			/* Used for typing erroneous nodes.  */
   type_void,			/* Return type of procedures, e.g. ().  */
   type_integer,		/* Integral numbers, e.g 23 or 42.  */
   type_long,			/* Long integers.  */
   type_real,			/* Approx. real numbers, like 3.14159.  */
   type_bool,			/* The constants `true' and `false'.  */
   type_char,			/* Characters, e.g. 'a', '6' or '!'.  */
   type_string,			/* Strings of characters, e.g. "Hello." */
   type_tuple,			/* Tuples, like (3, 'a', 2.71). */
   type_array,			/* Arrays, [3, 4, 5].  */
   type_list,			/* Lists, {'a', 'b', 'c'}.  */
   type_function,		/* Functions, fun(int, bool): string.  */
   type_sum,			/* User-defined data types.  */
   type_record,			/* Record types.  */
   type_alias,			/* Temporarily used for forward references.  */
#if 0
   type_variable,		/* Type variable for polymorphism.  */
#endif
   type_nil,			/* Type of `null'. */
   type_any,			/* Compatible with all others except void. */
   type_constrained};		/* Constrained types, !<TYPE>.  */

typedef struct ttl_type * ttl_type;

struct ttl_type_tuple
{
  size_t elem_type_count;	/* Number of tuple elements.  */
  ttl_type * elem_types;	/* Types of elements.  */
};

struct ttl_type_array
{
  ttl_type element;		/* Base type of array type.  */
};

struct ttl_type_list
{
  ttl_type element;		/* Base type of list type.  */
};

struct ttl_type_function
{
  size_t param_type_count;	/* Number of parameter types.  */
  ttl_type * param_types;	/* types of parameters.  */
  ttl_type return_type;		/* Return type, `void' for procedures.  */
};

struct ttl_type_sum
{
  ttl_symbol name;		/* Name of the type.  */
  ttl_ast_node full_name;	/* Fully qualified name.  */
  size_t type_count;
  ttl_type * types;
};

typedef struct ttl_field * ttl_field;
struct ttl_field
{
  ttl_symbol name;		/* Name of the record field.  */
  ttl_type type;		/* Type of the record field.  */
};

typedef struct ttl_variant * ttl_variant;
struct ttl_variant
{
  ttl_symbol name;		/* Name of the record variant.  */
  size_t field_count;		/* Number of fields of the variant.  */
  ttl_field * fields;		/* Fields.  */
};

struct ttl_type_record
{
  size_t variant_count;		/* Number of record variants.  */
  ttl_variant * variants;	/* Variants.  */
};

struct ttl_type_alias
{
  ttl_symbol name;		/* Name referenced.  */
  ttl_type type;		/* NULL if not yet known, aliased type
				   otherwise.  */
};

#if 0
struct ttl_type_variable
{
  ttl_symbol name;		/* Variable name.  */
};
#endif

struct ttl_type_constrained
{
  ttl_type base;		/* Base of constrained type.  */
};

struct ttl_type
{
  enum ttl_type_kind kind;
  ttl_symbol name;		/* Only valid if declared type.  */
  int exported;
  ttl_type next;		/* In list of declared types.  */
  union {
    struct ttl_type_array array;
    struct ttl_type_tuple tuple;
    struct ttl_type_function function;
    struct ttl_type_sum sum;
    struct ttl_type_record record;
    struct ttl_type_list list;
    struct ttl_type_alias alias;
#if 0
    struct ttl_type_variable variable;
#endif
    struct ttl_type_constrained constrained;
  } d;
};

/* Create a new type descriptor.  */
ttl_type ttl_make_type (ttl_pool pool, enum ttl_type_kind kind);

/* Specialized type descriptor constructors.  Allocate and initialize
   the respective type descriptors.  

   Arrays which are passed in by the caller (e.g for typle or function
   types) must be caller-allocated and may not be freed
   afterwards.  */
ttl_type ttl_make_basic_type (ttl_pool pool, enum ttl_type_kind kind);
ttl_type ttl_make_tuple_type (ttl_pool pool, size_t elem_type_count,
			      ttl_type * elem_types);
ttl_type ttl_make_array_type (ttl_pool pool, ttl_type element);
ttl_type ttl_make_list_type (ttl_pool pool, ttl_type element);
ttl_type ttl_make_function_type (ttl_pool pool, size_t param_type_count,
				 ttl_type * param_types,
				 ttl_type return_type);
ttl_type ttl_make_sum_type (ttl_pool pool, ttl_symbol name,
			    ttl_ast_node full_name,
			    size_t type_count,
			    ttl_type * types);
ttl_field ttl_make_record_field (ttl_pool pool, ttl_symbol name,
				 ttl_type type);
ttl_variant ttl_make_record_variant (ttl_pool pool, ttl_symbol name,
				     size_t field_count,
				     ttl_field * fields);
ttl_type ttl_make_record_type (ttl_pool pool, size_t variant_count,
			       ttl_variant *variants);
ttl_type ttl_make_alias_type (ttl_pool pool, ttl_symbol name,
			      ttl_type base_type);
#if 0
ttl_type ttl_make_variable_type (ttl_pool pool, ttl_symbol name);
#endif
ttl_type ttl_make_constrained_type (ttl_pool pool, ttl_type base);

/* Print a textual representation of `type' to the file `f'.  The
   representation is suitable for reading it back in with the
   parser.  */
void ttl_print_type (FILE * f, ttl_type type);

/* Print the textual representation of `type' to the file `f', but use
   the closest matching `C' datatype.  */
void ttl_print_c_type (FILE * f, ttl_type type);

/* Return non-zero if `type0' and `type1' are equal according to the
   language definition (that is structural equality except for
   user-defined types.  */
int ttl_types_equal (ttl_type type0, ttl_type type1);

/* Return non-zero if a value with type `type1' can be assigned to a
   location with type `type0', zero otherwise.  */
int ttl_types_assignable (ttl_type type0, ttl_type type1);

/* Return the number of characters required to store the string
   representation (as used for name mangling).  */
size_t ttl_type_string_length (ttl_type type);
/* Put the string representation of `type' into the string `s', which
   must be large enough for the representation if the given type.  The
   function `ttl_type_string_length()' can be used to compute the
   required number of characters.  */
void ttl_type_to_string (char * s, ttl_type type);

/* NOTE: Experimental.  Return the deepest nested base type, looking
   through constrained, array and list types.  */
ttl_type ttl_constrained_type_base (ttl_type type);

extern int ttl_permissive_checking;

#endif /* not TTL_TYPES_H */
