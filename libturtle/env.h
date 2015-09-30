/* libturtle/env.h - Compile time environment
 
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

#ifndef TTL_ENV_H
#define TTL_ENV_H

#include <stdio.h>

#include "symbols.h"
#include "types.h"
#include "ast.h"

/* Definitions declare bindings between names (identifiers) and
   program entities like types, variables, functions or modules.
   These are distinguished using the following enumeration type.  */
enum ttl_binding_kind
  {binding_variable,
   binding_type,
   binding_function,
   binding_module};

/* Functions can be of different kinds.  Constructors, accessors and
   discriminators are created with `datatype' declarations.  */
enum ttl_function_kind
  {function_function,
   function_constraint,
   function_constructor,
   function_accessor,
   function_setter,
   function_discriminator};

enum ttl_variable_kind
  {variable_global,
   variable_param,
   variable_local};

typedef struct ttl_function * ttl_function;

/* For each declared global or local variable or parameter one
   instance of the following type is created and linked into its
   defining function or module.  */
typedef struct ttl_variable * ttl_variable;
struct ttl_variable
{
  enum ttl_variable_kind kind;	/* Global, local, parameter...  */
  ttl_type type;		/* The variable's type.  */
  int constant;			/* Is this a constant (read-only) variable?  */
  ttl_ast_node init;
  ttl_symbol name;		/* User-defined name.  */
  ttl_symbol unique_name;	/* Either mangled or uniquified.  */
  ttl_variable next;		/* Next variable in scope.  */
  unsigned exported;		/* Non-zero if mentioned in export clause.  */
  int index;			/* Index in environment.  */
  ttl_function defining;	/* Function defining this variable,
				   NULL for gloabal variable.  */

  char * documentation;		/* Optional embedded documentation.  */
};


typedef struct ttl_field_list * ttl_field_list;
struct ttl_field_list
{
  ttl_field_list next;
  ttl_symbol name;
  ttl_ast_node identifier;
  ttl_type type;
  int variant;
  int offset;
  int constrainable;
  enum ttl_function_kind function_kind;
};


/* Functions represent functions, constraints and induced functional
   entities like constructors, accessors, setters and discriminators.  */
struct ttl_function
{
  enum ttl_function_kind kind;	/* Function, constraint, constructor, etc. */
  ttl_type type;		/* Argument and return types as `fun' type. */
  ttl_symbol name;		/* User-defined name.  */
  ttl_symbol unique_name;	/* Mangled name.  */
  ttl_variable params;		/* Function parameters.  */
  ttl_variable locals;		/* Local variables. */
  ttl_function enclosing;	/* Enclosing function.  */
  ttl_function enclosed;	/* Enclosed function(s).  */
  ttl_function next;		/* Next function in same scope.  */
  ttl_function total_next;	/* Next function in module.  */

  unsigned variant_count;	/* How many variants are there?  */
  union {
    struct {
      unsigned field_count;	/* Number of fields in variant.  */
      unsigned variant;		/* variant number if constr./discrim.  */
      unsigned constraint_mask;	/* Mask of constrainable fields.  */
    } constr_discrim;
    struct {
      ttl_field_list field_list; /* Fields in function.  */
#if 0
      int constrainable;       /* True iff field is constrainable.  */
#endif
    } accessor;
    struct {
      unsigned handcoded;	/* Non-zero if implemented in C.  */
      unsigned mapped;		/* Non-zero if mapped to C function.  */
      ttl_symbol alias;		/* Name of the mapped function.  */

      /* fixme: Make the following type-safe!  */
      void * il_code;		/* Function body in intermediate language.  */
      int nesting_level;
    } function;
  } d;

  void * asm_code;		/* Same in assembly language.  */

  ttl_variable variable;	/* Variable holding this function's value.  */


  unsigned exported;		/* Non-zero if mentioned in export clause.  */

  unsigned param_count;		/* Number of parameters.  */
  unsigned local_count;		/* Number of local variables.  */

  unsigned index;		/* Function index assigned during code
				   generation. */

  char * documentation;		/* Optional embedded documentation.  */
};

typedef struct ttl_environment * ttl_environment;

typedef struct ttl_module_list * ttl_module_list;

typedef struct ttl_module * ttl_module;
struct ttl_module
{
  ttl_environment env;		/* Module's environment. */
  ttl_module_list imported;	/* List of imported modules.  */
  ttl_variable globals;		/* List of module-global variables.  */
  ttl_function functions;	/* List of functions in module.  */
  ttl_function toplevel_functions; /* List of toplevel functions.  */

  ttl_type declared_types;	/* Declared types.  */

  ttl_ast_node module_ast_name; /* Qualified name of the module.  */
  ttl_ast_node imports;		/* Imported modules.  */
  ttl_ast_node params;		/* Module parameters.  */
  ttl_ast_node types;		/* Opaque types.  */
  ttl_ast_node exports;		/* Other exported entities.  */

  char * documentation;		/* Documentation, if any.  */
  unsigned recursing;		/* Marker for recursive module traversal.  */
};

struct ttl_module_list
{
  ttl_module_list next;
  ttl_module module;
};

typedef struct ttl_binding * ttl_binding;
struct ttl_binding
{
  enum ttl_binding_kind kind;	/* Which kind of binding?  */
  ttl_symbol symbol;		/* Name of the binding.  */
  ttl_symbol mangled_name;	/* Mangled binding's name.  */
  ttl_binding next;		/* For chaining in the hash table.  */
  int synthetic;
  union				/* Additional information.  */
  {
    ttl_variable variable;
    ttl_type type;
    ttl_function function;
    ttl_module module;
  } d;
};

/* Binding lists store the result(s) for environment lookups.  This is
   necessary because due to overloading, a single name can bind
   several objects, as long as they are distinguishable by kind or
   type.  */
typedef struct ttl_binding_list * ttl_binding_list;
struct ttl_binding_list
{
  ttl_binding_list next;
  ttl_binding binding;
};

#define TTL_ENVIRONMENT_TABLE_SIZE 71

struct ttl_environment
{
  ttl_pool pool;		/* Pool for allocating entries.  */
  ttl_environment parent;	/* Enclosing environment.  */
  ttl_binding table[TTL_ENVIRONMENT_TABLE_SIZE]; /* Hash table headers.  */
};

/* Create a new environment with the given parent environment.  For
   the toplevel environment, pass NULL as PARENT.  */
ttl_environment ttl_environment_make (ttl_pool pool, ttl_environment parent);

/* Search for the binding of SYMBOL in ENV, but not in the parents of
   ENV.  */
ttl_binding_list ttl_environment_search (ttl_pool pool, ttl_environment env,
					 ttl_symbol symbol);

/* Search for all bindings of SYMBOL in ENV or any of its parents, and
   return them in a list.  */
ttl_binding_list ttl_environment_lookup (ttl_pool pool, ttl_environment env,
					 ttl_symbol symbol);

/* Add the binding BIND to the environment ENV.  Before calling this,
   make sure that no binding for the symbol in BIND exists.  */
void ttl_environment_add (ttl_environment env, ttl_binding bind);

ttl_variable ttl_make_variable (ttl_pool pool, enum ttl_variable_kind kind,
				ttl_type type, int constant);
ttl_function ttl_make_function (ttl_pool pool, enum ttl_function_kind kind,
				ttl_type type);
ttl_module ttl_make_module (ttl_pool pool, ttl_environment env);

ttl_binding ttl_make_variable_binding (ttl_pool pool, ttl_symbol symbol,
				       ttl_symbol mangled_name,
				       ttl_variable variable);
ttl_binding ttl_make_type_binding (ttl_pool pool, ttl_symbol symbol,
				   ttl_symbol mangled_name, ttl_type type);
ttl_binding ttl_make_function_binding (ttl_pool pool, ttl_symbol symbol,
				       ttl_symbol mangled_name,
				       ttl_function function,
				       int synthetic);
ttl_binding ttl_make_module_binding (ttl_pool pool, ttl_symbol symbol,
				     ttl_module module);

/* Cons the binding BINDING onto the head of the binding list NEXT,
   using POOL for allocating the list node.  */
ttl_binding_list
ttl_binding_cons (ttl_pool pool, ttl_binding binding, ttl_binding_list next);

/* Dump a description of the environment `env' to the output file `f',
   for debugging purposes.  Indent tells how much the output is to be
   indented with spaces.  */
void ttl_dump_env (FILE * f, ttl_environment env, int indent);
void ttl_print_env (FILE * f, ttl_environment env, int indent);

ttl_module ttl_module_find (ttl_ast_node mod_name, ttl_module_list list);
ttl_module_list ttl_module_cons (ttl_pool pool, ttl_module mod,
				 ttl_module_list list);

#endif /* not TTL_ENV_H */
