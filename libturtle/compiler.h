/* libturtle/compiler.h - Turtle compiler
 
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

#ifndef TTL_COMPILER_H
#define TTL_COMPILER_H

#include "memory.h"
#include "symbols.h"
#include "env.h"
#include "types.h"

typedef struct ttl_compile_state * ttl_compile_state;
struct ttl_compile_state
{
  unsigned errors;		/* Total number of compilation errors. */
  char * filename;		/* Input file name.  */
  ttl_pool pool;		/* Pool for memory allocation.  */
  ttl_symbol_table symbol_table; /* For storing symbols.  */
  ttl_environment builtin_env;	/* Builtin types.  */
  ttl_environment env;		/* Current compilation environment.  */

  ttl_ast_node module_name;	/* Name of the currently compiled module.  */
  ttl_ast_node module_exports;	/* List of exported names.  */

  ttl_module_list all_modules;	/* List of all modules.  */
  ttl_module current_module;	/* Pointer to current module.  */
  ttl_function current_function; /* Pointer to current function.  */
  ttl_ast_node current_function_ast; /* AST of current function.  */
  int tail_position;		/* Non-zero when compiling in tail
				   position.  */

  /* The following are the predefined types.  */
  ttl_type error_type;
  ttl_type void_type;
  ttl_type any_type;
  ttl_type int_type;
  ttl_type long_type;
  ttl_type char_type;
  ttl_type string_type;
  ttl_type bool_type;
  ttl_type real_type;
  ttl_type nil_type;

  ttl_type current_return_type; /* Return type of current function.  */

  ttl_binding_list type_bindings;

  char * module_path;		/* Colon-seperated path of directory
				   names, which will be searched when
				   looking for imported modules.  */
  char * include_path;		/* Colon-seperated path of directory
				   names, which will be searched when
				   looking for the runtime headers.  */

  unsigned pragma_handcoded;	/* Non-zero if handcoded functions are
				   allowed.  */

  int nesting_level;		/* Nesting level of current function.  */

  int has_init_stmts;		/* True if init function exists. */
  void * init_stmts;		/* Initialization statements.  */

  void * mapping;
  int label_count;
  int next_label;

  /* HACK-ALERT: This is used to supress `unbound identifier' error
     messages during the parsing of interface files.  */
  unsigned complain_unbound_types;

  struct ttl_compile_options * compile_options;
};

struct ttl_compile_options
{
  unsigned debug_dump_env:1;
  unsigned debug_dump_ast:1;
  unsigned debug_dump_il_file:1;
  unsigned debug_dump_bytecode:1;
  unsigned pragma_handcoded:1;
  unsigned pragma_foreign:1;
  unsigned pragma_compile_only:1;
  unsigned pragma_turtledoc:1;
  unsigned pragma_printdeps:1;
  unsigned pragma_printdepsstdout:1;
  unsigned main:1;
  unsigned opt_local_calls:1;
  unsigned opt_local_jumps:1;
  unsigned opt_merge_gc_checks:1;
  unsigned opt_inline_constructors:1;
  unsigned opt_gcc_level;
  unsigned link_static:1;
  unsigned verbose;
  char * program_name;
  char * module_path;
  char * include_path;
};

void ttl_init_compile_options (struct ttl_compile_options * options);
int ttl_compile (char * filename, struct ttl_compile_options * options);

#endif /* not TTL_COMPILER_H */
