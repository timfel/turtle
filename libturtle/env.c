/* libturtle/env.c - Compile time environments
 
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


#include "env.h"
#include "ast.h"

ttl_environment
ttl_environment_make (ttl_pool pool, ttl_environment parent)
{
  unsigned i;
  ttl_environment env = ttl_malloc (pool, sizeof (struct ttl_environment));

  memset (env, 0, sizeof (struct ttl_environment));

  env->pool = pool;
  env->parent = parent;
  for (i = 0; i < TTL_ENVIRONMENT_TABLE_SIZE; i++)
    env->table[i] = NULL;
  return env;
}

static int
contained (ttl_binding_list ls, ttl_binding b)
{
  ttl_type type;
  
  switch (b->kind)
    {
    case binding_variable:
      type = b->d.variable->type;
      break;
    case binding_function:
      type = b->d.function->type;
      break;
    default:
      /* Other binding kinds can be distinguished by kind, so it is
	 assumed not to be in the list.  */
      return 0;
    }
  while (ls)
    {
      ttl_binding bb = ls->binding;
      switch (bb->kind)
	{
	case binding_variable:
	  if (ttl_types_equal (bb->d.variable->type, type))
	    return 1;
	  break;
	case binding_function:
	  if (!bb->synthetic && ttl_types_equal (bb->d.function->type, type))
	    return 1;
	  break;
	default:
	  break;
	}
      ls = ls->next;
    }
  return 0;
}

ttl_binding_list
ttl_environment_search (ttl_pool pool, ttl_environment env, ttl_symbol symbol)
{
  ttl_binding_list list = NULL;
  unsigned hash_code = symbol->raw_hash_code % TTL_ENVIRONMENT_TABLE_SIZE;
  ttl_binding bind = env->table[hash_code];

  while (bind != NULL)
    {
      if (bind->symbol == symbol)
	{
	  if (!contained (list, bind))
	    list = ttl_binding_cons (pool, bind, list);
	}
      bind = bind->next;
    }
  return list;
}

ttl_binding_list
ttl_environment_lookup (ttl_pool pool, ttl_environment env, ttl_symbol symbol)
{
  ttl_binding_list list = NULL;
  unsigned hash_code = symbol->raw_hash_code % TTL_ENVIRONMENT_TABLE_SIZE;
  ttl_binding bind;

  while (env != NULL)
    {
      bind = env->table[hash_code];
      while (bind != NULL)
	{
	  if (bind->symbol == symbol)
	    {
/* 	      ttl_symbol_print (stderr, bind->symbol); */
/* 	      fprintf (stderr, " %d\n", bind->kind); */
	      if (!contained (list, bind))
		list = ttl_binding_cons (pool, bind, list);

	    }
	  bind = bind->next;
	}
      env = env->parent;
    }
  return list;
}

void
ttl_environment_add (ttl_environment env, ttl_binding bind)
{
  unsigned hash_code = bind->symbol->raw_hash_code %
    TTL_ENVIRONMENT_TABLE_SIZE;

  bind->next = env->table[hash_code];
  env->table[hash_code] = bind;
}

static ttl_binding
ttl_make_binding (ttl_pool pool, ttl_symbol symbol, ttl_symbol mangled_name,
		  enum ttl_binding_kind kind)
{
  ttl_binding bind = ttl_malloc (pool, sizeof (struct ttl_binding));
  memset (bind, 0, sizeof (struct ttl_binding));
  bind->next = NULL;
  bind->symbol = symbol;
  bind->mangled_name = mangled_name;
  bind->kind = kind;
  bind->synthetic = 0;
  return bind;
}

ttl_binding
ttl_make_type_binding (ttl_pool pool, ttl_symbol symbol, 
		       ttl_symbol mangled_name, ttl_type type)
{
  ttl_binding bind = ttl_make_binding (pool, symbol, mangled_name,
					    binding_type);

  bind->d.type = type;
  return bind;
}

ttl_binding
ttl_make_variable_binding (ttl_pool pool, ttl_symbol symbol, 
			   ttl_symbol mangled_name, ttl_variable variable)
{
  ttl_binding bind = ttl_make_binding (pool, symbol, mangled_name,
				       binding_variable);

  bind->d.variable = variable;
  return bind;
}

ttl_binding
ttl_make_function_binding (ttl_pool pool, ttl_symbol symbol, 
			   ttl_symbol mangled_name, ttl_function function,
			   int synthetic)
{
  ttl_binding bind = ttl_make_binding (pool, symbol, mangled_name,
				       binding_function);

  bind->d.function = function;
  bind->synthetic = synthetic;
  return bind;
}

ttl_binding
ttl_make_module_binding (ttl_pool pool, ttl_symbol symbol, ttl_module module)
{
  ttl_binding bind = ttl_make_binding (pool, symbol, symbol, binding_module);

  bind->d.module = module;
  return bind;
}

ttl_variable
ttl_make_variable (ttl_pool pool, enum ttl_variable_kind kind, ttl_type type,
		   int constant)
{
  ttl_variable var = ttl_malloc (pool, sizeof (struct ttl_variable));
  memset (var, 0, sizeof (struct ttl_variable));
  var->kind = kind;
  var->next = NULL;
  var->type = type;
  var->constant = constant;
  var->init = NULL;
  var->name = NULL;
  var->unique_name = NULL;
  var->exported = 0;
  var->index = 0;
  var->defining = NULL;
  var->documentation = NULL;
  return var;
}

ttl_function
ttl_make_function (ttl_pool pool, enum ttl_function_kind kind,
		   ttl_type type)
{
  ttl_function fun = ttl_malloc (pool, sizeof (struct ttl_function));
  memset (fun, 0, sizeof (struct ttl_function));
  fun->kind = kind;
  fun->type = type;
  fun->name = NULL;
  fun->unique_name = NULL;
  fun->params = NULL;
  fun->locals = NULL;
  fun->enclosing = NULL;
  fun->enclosed = NULL;
  fun->next = NULL;
  fun->total_next = NULL;
/*   fun->variant = 0; */
/*   fun->variant_count = 0; */
/*   fun->field_count = 0; */
/*   fun->field_list = NULL; */
/*   fun->constraint_mask = 0; */
/*   fun->constrainable = 0; */
  fun->variable = NULL;
/*   fun->handcoded = 0; */
/*   fun->mapped = 0; */
/*   fun->alias = NULL; */
  fun->exported = 0;
  fun->param_count = 0;
  fun->local_count = 0;
  fun->index = 0;
/*   fun->nesting_level = 0; */
/*   fun->il_code = NULL; */
  fun->asm_code = NULL;
  fun->documentation = NULL;
  return fun;
}

ttl_module
ttl_make_module (ttl_pool pool, ttl_environment env)
{
  ttl_module mod = ttl_malloc (pool, sizeof (struct ttl_module));
  memset (mod, 0, sizeof (struct ttl_module));
  mod->env = env;
  mod->imported = NULL;
  mod->globals = NULL;
  mod->functions = NULL;
  mod->toplevel_functions = NULL;
  mod->declared_types = NULL;
  mod->module_ast_name = NULL;
  mod->imports = NULL;
  mod->params = NULL;
  mod->types = NULL;
  mod->exports = NULL;
  mod->recursing = 0;
  return mod;
}

ttl_binding_list
ttl_binding_cons (ttl_pool pool, ttl_binding binding, ttl_binding_list next)
{
  ttl_binding_list list = ttl_malloc (pool, sizeof (struct ttl_binding_list));

  list->next = next;
  list->binding = binding;
  return list;
}

static void
spaces (FILE * f, int i)
{
  while (i > 0)
    {
      fprintf (f, " ");
      i--;
    }
}

void
ttl_dump_env (FILE * f, ttl_environment env, int indent)
{
  unsigned i, j;
  unsigned env_no = 0;
  ttl_binding bind;

  while (env != NULL && env->parent != NULL)
    {
      spaces (f, indent);
      fprintf (f, "* Environment %d\n", env_no);
      for (i = 0; i < TTL_ENVIRONMENT_TABLE_SIZE; i++)
	{
	  bind = env->table[i];
	  while (bind != NULL)
	    {
	      ttl_symbol sym = bind->symbol;
	      j = 0;

	      spaces(f, indent + 2);
	      while (j < sym->length)
		{
		  fprintf (f, "%c", sym->text[j]);
		  j++;
		}
	      while (j < 20)
		{
		  fprintf (f, " ");
		  j++;
		}
/* 	      fprintf (f, "%10u  %8x  ", sym->raw_hash_code, (unsigned) sym); */
	      switch (bind->kind)
		{
		case binding_variable:
		  fprintf (f, "variable      ");
		  ttl_print_type (f, bind->d.variable->type);
		  break;
		case binding_type:
		  fprintf (f, "type          ");
		  ttl_print_type (f, bind->d.type);
		  break;
		case binding_function:
		  switch (bind->d.function->kind)
		    {
		    case function_function:
		      fprintf (f, "function      ");
		      break;
		    case function_constraint:
		      fprintf (f, "constraint    ");
		      break;
		    case function_constructor:
		      fprintf (f, "constructor   ");
		      break;
		    case function_accessor:
		      fprintf (f, "accessor      ");
		      break;
		    case function_setter:
		      fprintf (f, "setter        ");
		      break;
		    case function_discriminator:
		      fprintf (f, "discriminator ");
		      break;
		    }
		  ttl_print_type (f, bind->d.function->type);
		  break;
		case binding_module:
		  fprintf (f, "module        ");
		  /* Only re-enable the recursive environment printing
		     if there are no circularities.  Currently there
		     are, because a module is added to its own
		     environment to enable qualified access to
		     module-local names.  */
		  if (!bind->d.module->recursing)
		    {
		      bind->d.module->recursing++;
		      fprintf (f, "\n");
		      ttl_dump_env (f, bind->d.module->env, indent + 4);
		      bind->d.module->recursing--;
		    }
		  break;
		}
	      fprintf (f, "\n");
	      bind = bind->next;
	    }
	}
      env = env->parent;
      env_no++;
    }
}

void
ttl_print_env (FILE * f, ttl_environment env, int indent)
{
  unsigned i, j;
  unsigned env_no = 0;
  ttl_binding bind;

  while (env != NULL && env->parent != NULL)
    {
      spaces (f, indent);
      fprintf (f, "* Environment %d\n", env_no);
      for (i = 0; i < TTL_ENVIRONMENT_TABLE_SIZE; i++)
	{
	  bind = env->table[i];
	  while (bind != NULL)
	    {
	      ttl_symbol sym = bind->symbol;
	      j = 0;

	      spaces(f, indent + 2);
	      while (j < sym->length)
		{
		  fprintf (f, "%c", sym->text[j]);
		  j++;
		}
	      while (j < 20)
		{
		  fprintf (f, " ");
		  j++;
		}
/* 	      fprintf (f, "%10u  %8x  ", sym->raw_hash_code, (unsigned) sym); */
	      switch (bind->kind)
		{
		case binding_variable:
		  fprintf (f, "variable      ");
		  ttl_print_type (f, bind->d.variable->type);
		  break;
		case binding_type:
		  fprintf (f, "type          ");
		  ttl_print_type (f, bind->d.type);
		  break;
		case binding_function:
		  switch (bind->d.function->kind)
		    {
		    case function_function:
		      fprintf (f, "function      ");
		      break;
		    case function_constraint:
		      fprintf (f, "constraint    ");
		      break;
		    case function_constructor:
		      fprintf (f, "constructor   ");
		      break;
		    case function_accessor:
		      fprintf (f, "accessor      ");
		      break;
		    case function_setter:
		      fprintf (f, "setter        ");
		      break;
		    case function_discriminator:
		      fprintf (f, "discriminator ");
		      break;
		    }
		  ttl_print_type (f, bind->d.function->type);
		  break;
		case binding_module:
		  fprintf (f, "module        ");
		  if (bind->d.module->recursing < 1)
		    {
		      bind->d.module->recursing++;
		      fprintf (f, "\n");
		      ttl_print_env (f, bind->d.module->env, indent + 2);
		      bind->d.module->recursing--;
		    }
		  break;
		}
	      fprintf (f, "\n");
	      bind = bind->next;
	    }
	}
      env = env->parent;
      env_no++;
    }
}

ttl_module
ttl_module_find (ttl_ast_node mod_name, ttl_module_list list)
{
  while (list)
    {
      if (list->module->module_ast_name)
	{
	  if (ttl_compare_identifiers (mod_name,
				       list->module->module_ast_name))
	    return list->module;
	}
      list = list->next;
    }
  return NULL;
}

ttl_module_list
ttl_module_cons (ttl_pool pool, ttl_module mod, ttl_module_list list)
{
  ttl_module_list l = ttl_malloc (pool, sizeof (struct ttl_module_list));
  l->next = list;
  l->module = mod;
  return l;
}

/* End of env.c.  */
