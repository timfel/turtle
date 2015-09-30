/* libturtle/compiler.c - Turtle compiler
 
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <version.h>

#include <libturtle/turtle-path.h>

#include "error.h"
#include "parser.h"
#include "env.h"
#include "compiler.h"
#include "util.h"
#include "il.h"
#include "codegen.h"
#include "emit-c.h"


static int in_lvalue_position = 0;

/* FIXME:
   - Clean up constraint handling, e.g. translate constraints to
     functions of kind constraint.
*/

static int
already_defined (ttl_compile_state state, ttl_environment env, 
		 ttl_symbol name, ttl_type type, ttl_ast_node ast_name);

static ttl_type translate_type (ttl_compile_state state, ttl_ast_node type);
static ttl_il_node translate_stmt (ttl_compile_state state, ttl_ast_node stmt);
static ttl_binding_list lookup_qualident (ttl_compile_state state,
					  ttl_module module,
					  ttl_environment env,
					  ttl_ast_node ident);


/* Find the (possibly nested) module called `mod_name' in the
   environment `env'.  Create a new one and bind it in `env', if it
   does not yet exist.  `base_env' is used as the parent of the new
   module's environment, if one is created.  */
ttl_binding
get_module (ttl_compile_state state, ttl_ast_node mod_name,
	    ttl_environment env, ttl_environment base_env)
{
  if (mod_name->kind == ast_identifier)
    {
      ttl_binding module_binding;
      ttl_binding_list bind_list =
	ttl_environment_search (state->pool, env,
				mod_name->d.identifier.symbol);

      /* FIXME: Make sure the binding we return is actually a
	 module!  */

      if (!bind_list)
	{
	  ttl_environment new_env = ttl_environment_make (state->pool, 
							  base_env);
	  ttl_module module = ttl_make_module (state->pool, new_env);

	  module->module_ast_name = mod_name;

	  module_binding = ttl_make_module_binding
	    (state->pool, mod_name->d.identifier.symbol, module);
	  ttl_environment_add (env, module_binding);
	}
      else if (bind_list->next)
	{
	  fprintf (stderr, "More than one module!\n");
	  abort ();
	}
      else
	module_binding = bind_list->binding;
      return module_binding;
    }
  else if (mod_name->kind == ast_annotated_identifier)
    {
      return get_module (state, mod_name->d.annotated_identifier.identifier,
			 env, base_env);
    }
  else if (mod_name->kind == ast_qualident)
    {
      ttl_binding parent_module = get_module (state,
					      mod_name->d.qualident.module,
					      env, state->builtin_env);
      return get_module (state,
			 mod_name->d.qualident.identifier,
			 parent_module->d.module->env, base_env);
    }
  else
    {
      fprintf (stderr, "Invalid AST in get_module()\n");
      abort ();
    }
}


/* Substitute the elements from the list `actuals' for the
   corresponding elements of the list `formals' in the abstract syntax
   tree `type'.  The substitution returns a new AST tree and does not
   side-effect `type'. */
static ttl_ast_node
substitute (ttl_compile_state state, ttl_ast_node formals,
	    ttl_ast_node actuals, ttl_ast_node type)
{
  switch (type->kind)
    {
    case ast_void_type:
      break;

    case ast_tuple_type:
      {
	ttl_ast_node l = type->d.tuple_type.fields;
	ttl_ast_node ll = NULL, * lp = &ll;
	while (l)
	  {
	    *lp = ttl_make_ast_pair
	      (state->pool, substitute (state, formals, actuals,
					l->d.pair.car), NULL);
	    lp = &((*lp)->d.pair.cdr);
	    l = l->d.pair.cdr;
	  }
	return ttl_make_ast_tuple_type (state->pool,
					ll, type->filename,
					type->start_line,
					type->start_column,
					type->end_line,
					type->end_column);
	break;
      }
    case ast_array_type:
      {
	ttl_ast_node element = substitute (state, formals, actuals,
					   type->d.array_type.element_type);
	return ttl_make_ast_array_type (state->pool,
					element,
					type->filename,
					type->start_line,
					type->start_column,
					type->end_line,
					type->end_column);

	break;
      }
    case ast_list_type:
      {
	ttl_ast_node element = substitute (state, formals, actuals,
					   type->d.list_type.element_type);
	return ttl_make_ast_list_type (state->pool,
				       element,
				       type->filename,
				       type->start_line,
				       type->start_column,
				       type->end_line,
				       type->end_column);

	break;
      }
    case ast_function_type:
      {
	ttl_ast_node l = type->d.function_type.left;
	ttl_ast_node ll = NULL, * lp = &ll;
	ttl_ast_node r;
	while (l)
	  {
	    *lp = ttl_make_ast_pair
	      (state->pool, substitute (state, formals, actuals,
					l->d.pair.car), NULL);
	    lp = &((*lp)->d.pair.cdr);
	    l = l->d.pair.cdr;
	  }
	if (type->d.function_type.right)
	  r = substitute (state, formals, actuals,
			  type->d.function_type.right);
	else
	  r = NULL;
	return ttl_make_ast_function_type (state->pool, ll, r,
					   type->filename,
					   type->start_line,
					   type->start_column,
					   type->end_line,
					   type->end_column);
	break;
      }

    case ast_basic_type:
      {
	ttl_ast_node name = type->d.basic_type.type_name;

	if (name->kind == ast_identifier)
	  {
	    while (actuals)
	      {
		if (formals->d.pair.car->d.identifier.symbol ==
		    name->d.identifier.symbol)
		  return actuals->d.pair.car;

		actuals = actuals->d.pair.cdr;
		formals = formals->d.pair.cdr;
	      }
	  }
	else if (name->kind == ast_qualident)
	  {
	    /* - */
	  }
	else
	  {
	    fprintf (stderr,
		     "only (un)qualified names allowed as type variables\n");
	    abort ();
	  }
	break;
      }

    case ast_user_type:
      {
	if (type->d.user_type.types)
	  {
	    ttl_ast_node l = type->d.user_type.types;
	    ttl_ast_node ll = NULL, * lp = &ll;
	    while (l)
	      {
		*lp = ttl_make_ast_pair
		  (state->pool, substitute (state, formals, actuals,
					    l->d.pair.car), NULL);
		lp = &((*lp)->d.pair.cdr);
		l = l->d.pair.cdr;
	      }
	    return ttl_make_ast_user_type (state->pool,
					   type->d.user_type.type_name,
					   ll,
					   type->filename,
					   type->start_line,
					   type->start_column,
					   type->end_line,
					   type->end_column);
	  }
	break;
      }

    default:
      break;
    }
  return type;
}

/* Instantiate the module `imp_module' with the parameters `actuals'
   and install the resulting module in the environment of
   `main_module'.  `type_bindings' holds a binding list of all types
   defined in `main_module', so that these types may be used as actual
   parameters to the imported module. `open_list is a list of
   identifiers which should be imported directly into `main_module's
   environment, so that they can be used unqualified.  */
static void
import_module (ttl_compile_state state, ttl_module imp_module, 
	       ttl_module main_module,
	       ttl_ast_node actuals,
	       ttl_ast_node open_list,
	       int do_subst,
	       ttl_binding_list type_bindings)
{
#if 0
  fprintf (stderr, "  [");
  ttl_ast_print (stderr, ttl_strip_annotation (imp_module->module_ast_name),
		 0);
  if (imp_module->params)
    {
      fprintf (stderr, " ");
      ttl_ast_print (stderr, imp_module->params, 0);
    }
  fprintf (stderr, " / <");
  if (actuals)
    ttl_ast_print (stderr, actuals, 0);
  fprintf (stderr, "> -> ");
  ttl_ast_print (stderr, ttl_strip_annotation (main_module->module_ast_name),
		 0);
  fprintf (stderr, "]\n");
#if 0
  {
    ttl_binding_list l = type_bindings;
    while (l)
      {
	ttl_symbol_print (stderr, l->binding->symbol);
	fprintf (stderr, "\n");
	l = l->next;
      }
  }
#endif
#endif

  {
    ttl_environment old_state_env;
    ttl_environment env;

    ttl_binding mod_bind;

    ttl_ast_node mod_name = ttl_strip_annotation (imp_module->module_ast_name);
    ttl_ast_node modules = imp_module->imports;
    ttl_ast_node formals = imp_module->params;
    ttl_ast_node types = imp_module->types;
    ttl_ast_node list = imp_module->exports;

    if (do_subst && ttl_ast_length (actuals) != ttl_ast_length (formals))
      {
	state->errors++;
	ttl_error_print_location (stderr, main_module->module_ast_name); 
	ttl_error_print_string (stderr, "wrong number of module parameters: ");
	ttl_error_print_node (stderr, imp_module->module_ast_name); 
	ttl_error_print_nl (stderr); 
	return;
      }
    
    mod_bind = get_module (state, mod_name, main_module->env, imp_module->env);
/*     mod_bind = get_module (state, mod_name, main_module->env, main_module->env); */

    /*     if (!do_subst) */
    /*       return; */

    env = mod_bind->d.module->env;
    old_state_env = state->env;
    state->env = ttl_environment_make (state->pool, env);

    {
      ttl_ast_node act = actuals;
      while (act)
	{
	  ttl_binding_list l;

	  ttl_ast_node t = act->d.pair.car;
	  act = act->d.pair.cdr;

#if 0
	  fprintf (stderr, "t: ");
	  ttl_ast_print (stderr, t, 0);
	  fprintf (stderr, "\n");
#endif
	  if (t->kind == ast_user_type || t->kind == ast_basic_type)
	    {
	      ttl_ast_node tt;

	      if (t->kind == ast_user_type)
		tt = t->d.user_type.type_name;
	      else
		tt = t->d.basic_type.type_name;
#if 0
	      fprintf (stderr, "tt: ");
	      ttl_ast_print (stderr, tt, 0);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "kind: %d\n", tt->kind);
#endif
	      l = lookup_qualident (state, state->current_module,
				    old_state_env, tt);
/* 	      ttl_print_env (stderr, main_module->env, 0); */
	      if (!l)
		{
		  ttl_error_print_location (stderr, tt);
		  ttl_error_print_string (stderr, "unbound identifier: ");
		  ttl_error_print_node (stderr, tt);
		  ttl_error_print_nl (stderr);
		}
	      while (l)
		{
		  if (l->binding->kind == binding_type)
		    {
		      ttl_binding bind;
		      bind = ttl_make_type_binding (state->pool, 
						    l->binding->symbol,
						    l->binding->mangled_name,
						    l->binding->d.type);
		      ttl_environment_add (state->env, bind);
#if 0
		      fprintf (stderr, "d.type: ");
		      ttl_print_type (stderr, l->binding->d.type);
		      fprintf (stderr, "\n");
#endif
		    }
		  l = l->next;
		}
	    }
	}
    }

/*     ttl_dump_env (stderr, state->env, 0); */
    {
      ttl_binding_list l = type_bindings;
      while (l)
	{
	  ttl_binding bind;
	  bind = ttl_make_type_binding (state->pool, 
					l->binding->symbol,
					l->binding->mangled_name,
					l->binding->d.type);
	  ttl_environment_add (state->env, bind);
	  l = l->next;
	}
    }

#if 0
    while (types)
      {
	ttl_ast_node name = types->d.pair.car;

	ttl_type tp = ttl_make_sum_type
	  (state->pool, name->d.identifier.symbol,
	   ttl_make_ast_qualident (state->pool, mod_name, name,
				   NULL, -1, -1, -1, -1),
	   0, NULL);
#if 0
	fprintf (stderr, "Importing type ");
	ttl_ast_print (stderr, name, 0);
	fprintf (stderr, "\n");
#endif
	ttl_environment_add
	  (env,
	   ttl_make_type_binding
	   (state->pool,
	    name->d.identifier.symbol, 
	    ttl_mangle_name (state, mod_name, name, tp), tp));
	types = types->d.pair.cdr;
      }
#endif
    while (list)
      {
	ttl_ast_node def = list->d.pair.car;
	ttl_ast_node name, type;
	ttl_type tp, tp1;

/* 	fprintf (stderr, "importing "); */
/* 	ttl_ast_print (stderr, def, 0); */
/* 	fprintf (stderr, "\n"); */
	switch (def->kind)
	  {
	  case ast_typedef:
	    name = def->d.typedf.name;
	    type = def->d.typedf.type;
	    state->complain_unbound_types = 0;
	    tp1 = translate_type (state, type);
	    state->complain_unbound_types = do_subst;
	    tp = translate_type (state, substitute (state, formals, actuals,
						    type));
	    state->complain_unbound_types = 1;
	    if (tp->kind != type_error)
	      {
		if (tp1->kind == type_error)
		  tp1 = state->void_type;
		ttl_environment_add
		  (env,
		   ttl_make_type_binding
		   (state->pool,
		    name->d.identifier.symbol, 
		    ttl_mangle_name (state, mod_name, name, tp1), tp));
	      }
	    break;
	  case ast_variable:
	    name = def->d.variable.name;
	    type = def->d.variable.type;
	    state->complain_unbound_types = 0;
	    tp1 = translate_type (state, type);
	    state->complain_unbound_types = do_subst;
	    tp = translate_type (state, substitute (state, formals, actuals,
						    type));
	    state->complain_unbound_types = 1;
	    if (tp->kind != type_error)
	      {
		ttl_variable var = ttl_make_variable
		  (state->pool, variable_global, tp,
		   def->d.variable.init != NULL);
		var->init = def->d.variable.init;
		if (tp1->kind == type_error)
		  tp1 = state->void_type;
		ttl_environment_add
		  (env,
		   ttl_make_variable_binding
		   (state->pool,
		    name->d.identifier.symbol,
		    ttl_mangle_name (state, mod_name, name, tp1), 
		    var));
		{
		  ttl_ast_node l = open_list;

		  while (l &&
			 !ttl_compare_identifiers (l->d.pair.car, name))
		    l = l->d.pair.cdr;
		  if (l)
		    {   
#if 0
		      fprintf (stderr, "Adding ");
		      ttl_ast_print (stderr, name, 0);
		      fprintf (stderr, "\n");
#endif
		      if (!already_defined (state, main_module->env,
					    name->d.identifier.symbol, tp,
					    l->d.pair.car))
			ttl_environment_add
			  (main_module->env,
			   ttl_make_variable_binding
			   (state->pool,
			    name->d.identifier.symbol,
			    ttl_mangle_name (state, mod_name, name, tp1), 
			    var));
		    }
		}
	      }
	    break;
	  case ast_function:
	    name = def->d.function.name;
	    type = def->d.function.params;
	    if (type)
	      {
		ttl_ast_node subst;

		state->complain_unbound_types = 0;
		tp1 = translate_type (state, type);
		state->complain_unbound_types = do_subst;

		subst = substitute (state, formals, actuals, type);
		tp = translate_type (state, subst);
		state->complain_unbound_types = 1;
	      }
	    else
	      {
		tp1 = state->void_type;
		tp = state->void_type;
	      }
	    if (tp->kind != type_error)
	      {
		if (tp1->kind == type_error)
		  tp1 = state->void_type;
		ttl_environment_add
		  (env,
		   ttl_make_function_binding
		   (state->pool,
		    name->d.identifier.symbol,
		    ttl_mangle_name (state, mod_name, name, tp1), 
		    ttl_make_function (state->pool, function_function, tp),
		    1));
		{
		  ttl_ast_node l = open_list;

		  while (l &&
			 !ttl_compare_identifiers (l->d.pair.car, name))
		    l = l->d.pair.cdr;
		  if (l)
		    {   
		      if (!already_defined (state, main_module->env,
					    name->d.identifier.symbol, tp,
					    l->d.pair.car))
			ttl_environment_add
			  (main_module->env,
			   ttl_make_function_binding
			   (state->pool,
			    name->d.identifier.symbol,
			    ttl_mangle_name (state, mod_name, name, tp1),
			    ttl_make_function (state->pool, function_function,
					       tp), 1));
		    }
		}
	      }
	    break;
	  default:
	    ttl_ast_print (stderr, def, 0);
	    fprintf (stderr, "\n");
	    abort ();
	  }
	list = list->d.pair.cdr;
      }
    state->env = old_state_env;
  }
}

/* Load the interface for the module named `mod_name', create a module
   object for the module and add it to the list of loaded modules.  If
   the module already was loaded, the old module object is returned.
   The modules imprted by the module `mod_name' are also loaded
   (transitively), until all required modules are in memory.  */
static ttl_module
load_module (ttl_compile_state state, ttl_ast_node mod_name)
{
  char * fname, * p;
  FILE * f;
  ttl_scanner scanner;
  ttl_ast_node list;
  ttl_environment env;
  ttl_module module;

  ttl_ast_node modules;
  ttl_ast_node types;

  module = ttl_module_find (mod_name, state->all_modules);
  if (module)
    {
#if 0
      fprintf (stderr, "[");
      ttl_ast_print (stderr, ttl_strip_annotation (mod_name), 0);
      fprintf (stderr, "] (loaded)\n");
#endif
      return module;
    }
#if 0
  fprintf (stderr, "[");
  ttl_ast_print (stderr, ttl_strip_annotation (mod_name), 0);
  fprintf (stderr, "] (new)\n");
#endif

  fname = ttl_qualident_to_filename (state->pool, mod_name, ".ifc");

/*   fprintf (stderr, "module file: %s\n", fname); */
  /* Try to find the interface file in the current and in all
     directories in the module path.  */
  p = ttl_find_file (state->pool, fname, state->module_path);
  if (!p)
    {
      state->errors++;
      ttl_error_print_location (stderr, ttl_strip_annotation (mod_name));
      ttl_error_print_string (stderr, "cannot find module: ");
      ttl_error_print_node (stderr, ttl_strip_annotation (mod_name));
      ttl_error_print_nl (stderr);
      return NULL;
    }
  f = fopen (p, "r");
  if (!f)
    {
      state->errors++;
      fprintf (stderr, "turtle: cannot open file: %s\n", fname);
      return NULL;
    }

  scanner = ttl_make_scanner (state->pool, state->symbol_table, f, fname);

  /* XXX: Check for scan/parse errors in `scanner'.  */
  list = ttl_parse_interface (state->pool, scanner);
  fclose (f);

  if (!list)
    {
      fprintf (stderr, "%s: error in interface file\n", fname);
      return NULL;
    }

  env = ttl_environment_make (state->pool, state->builtin_env);

  {
    ttl_ast_node m_name = ttl_strip_annotation (mod_name);

    while (m_name->kind == ast_qualident)
      {
	module = ttl_make_module (state->pool, env);
	module->module_ast_name = m_name;
	ttl_environment_add (env,
			     ttl_make_module_binding
			     (state->pool,
			      m_name->d.qualident.identifier->d.identifier.symbol,
			      module));
	m_name = m_name->d.qualident.module;
      }
    module = ttl_make_module (state->pool, env);
    module->module_ast_name = m_name;
    ttl_environment_add (env,
			 ttl_make_module_binding
			 (state->pool,
			  ttl_strip_annotation (m_name)->d.identifier.symbol,
			  module));
    module = ttl_make_module (state->pool, env);
    module->module_ast_name = mod_name;
    ttl_environment_add (env,
			 ttl_make_module_binding
			 (state->pool,
			  ttl_strip_annotation (mod_name)->d.identifier.symbol,
			  module));
  }

#if 0
  {
    ttl_binding b = get_module (state, mod_name, env, env);
    module = b->d.module;
  }
#endif
  
  state->all_modules = ttl_module_cons (state->pool, module,
					state->all_modules);
  
  module->imports = list->d.pair.car;
  list = list->d.pair.cdr;

  module->params = list->d.pair.car;
  list = list->d.pair.cdr;

  module->types = list->d.pair.car;
  list = list->d.pair.cdr;

  module->exports = list;

/*   ttl_dump_env (stderr, module->env, 0); */

  types = module->types;

  /* HACK-ALERT begin  */
  mod_name = ttl_strip_annotation (mod_name);
  /* HACK-ALERT end  */

  while (types)
    {
      ttl_ast_node name = types->d.pair.car;
      ttl_type tp = ttl_make_sum_type
	(state->pool, name->d.identifier.symbol,
	 ttl_make_ast_qualident (state->pool, mod_name, name, NULL,
				 -1, -1, -1, -1),
	 0, NULL);
#if 0
	fprintf (stderr, "Importing type ");
	ttl_ast_print (stderr, name, 0);
	fprintf (stderr, "\n");
#endif
      ttl_environment_add
	(env,
	 ttl_make_type_binding
	 (state->pool,
	  name->d.identifier.symbol, 
	  ttl_mangle_name (state, mod_name, name, tp), tp));
      types = types->d.pair.cdr;
    }

  list = module->imports;
  while (list)
    {
      ttl_module imp = ttl_module_find (list->d.pair.car, module->imported);
      if (!imp)
	{
	  imp = load_module (state, list->d.pair.car);
	  if (imp)
	    {
	      module->imported = ttl_module_cons (state->pool,
						  imp, module->imported);
	      /* Note that we import the module only if it was not
		 already in the list of imported modules.  This is
		 because there is no need to instantiate modules in
		 interfaces.  */
	      import_module (state, imp, module, NULL, NULL, 0, NULL);
	    }
	}
      list = list->d.pair.cdr;
    }
  return module;
}


static void
write_interface (ttl_compile_state state, ttl_ast_node module)
{
  ttl_ast_node imports = module->d.module.imports;
  ttl_ast_node exports = module->d.module.exports;
  ttl_ast_node params = module->d.module.params;
  FILE * ifc_f;
  char * ifc_name;

  ifc_name = ttl_basename (state->pool, state->filename);
  ifc_name = ttl_replace_file_ext (state->pool, ifc_name, ".ifc");
  ifc_f = fopen (ifc_name, "w");
  if (!ifc_f)
    {
      state->errors++;
      ttl_error_print_string (stderr,
			      "turtle: cannot create interface file: ");
      ttl_error_print_string (stderr, ifc_name);
      ttl_error_print_nl (stderr);
      return;
    }
  fprintf (ifc_f,
	   "// Created by Turtle %s -- DO NOT EDIT -- -*-turtle-*-\n",
	   __turtle_version);
  fprintf (ifc_f, "\n// Used modules:\n");
  fprintf (ifc_f, "( ");
  while (imports)
    {
      if (!ttl_compare_identifiers
	  (ttl_strip_annotation (state->current_module->module_ast_name),
	   ttl_strip_annotation (imports->d.pair.car)))
	{
	  ttl_ast_print (ifc_f,
			 imports->d.pair.car->d.annotated_identifier.identifier,
			 0);
	  fprintf (ifc_f, " ");
	}
      imports = imports->d.pair.cdr;
    }
  fprintf (ifc_f, ")\n");

  fprintf (ifc_f, "\n// Module parameter names:\n");
  fprintf (ifc_f, "( ");
  while (params)
    {
      ttl_ast_print (ifc_f, params->d.pair.car->d.variable.name, 0);
      fprintf (ifc_f, " ");
      params = params->d.pair.cdr;
    }
  fprintf (ifc_f, ")\n\n");

  exports = module->d.module.exports;
  while (exports)
    {
      ttl_ast_node name;
      ttl_binding_list bind_list;
      ttl_binding bind;

      name = exports->d.pair.car;

      if (name->kind == ast_identifier)
	{
	  bind_list = ttl_environment_search (state->pool, state->env,
					      name->d.identifier.symbol);
	  while (bind_list)
	    {
	      bind = bind_list->binding;
	      bind_list = bind_list->next;
	      switch (bind->kind)
		{
		case binding_type:
		  bind->d.type->exported = 1;
		  break;
		case binding_variable:
		  bind->d.variable->exported = 1;
		  break;
		case binding_function:
		  bind->d.function->exported = 1;
		  break;
		case binding_module:
		  break;
		}
	    }
	}
      else
	{
	  state->errors++;
	  fprintf (stderr, "** Re-exporting not yet implemented.\n");
	}
      exports = exports->d.pair.cdr;
    }


  fprintf (ifc_f, "// Opaque types:\n");
  fprintf (ifc_f, "(");
  {
    int first = 1;
    ttl_type t = state->current_module->declared_types;

    while (t)
      {
	if (t->exported)
	  {
	    if (first)
	      first = 0;
	    else
	      fprintf (ifc_f, " ");
	    ttl_symbol_print (ifc_f, t->name);
	  }
	t = t->next;
      }
  }
  fprintf (ifc_f, ")\n");

  fprintf (ifc_f, "\n// Public functions and variables:\n");

  {
    ttl_function f = state->current_module->toplevel_functions;
    ttl_variable v = state->current_module->globals;
    ttl_type t = state->current_module->declared_types;

    while (t)
      {
	if (t->exported)
	  {
	    ttl_symbol_print (ifc_f, t->name);
	    fprintf (ifc_f, " type ");
	    ttl_print_type (ifc_f, t);
	    fprintf (ifc_f, "\n");
	  }
	t = t->next;
      }
    while (v)
      {
	if (v->exported)
	  {
	    ttl_symbol_print (ifc_f, v->name);
	    if (v->constant)
	      fprintf (ifc_f, " const ");
	    else
	      fprintf (ifc_f, " var ");
	    ttl_print_type (ifc_f, v->type);
	    if (v->constant)
	      {
		fprintf (ifc_f, " ");
		ttl_ast_print (ifc_f, v->init, 0);
	      }
	    fprintf (ifc_f, "\n");
	  }
	v = v->next;
      }
    while (f)
      {
	if (f->exported)
	  {
	    ttl_symbol_print (ifc_f, f->name);
	    fprintf (ifc_f, " fun ");
	    ttl_print_type (ifc_f, f->type);
	    fprintf (ifc_f, "\n");
	  }
	f = f->next;
      }
  }
  fprintf (ifc_f, "\n// End of file.\n");
  fclose (ifc_f);
}

static ttl_environment
make_default_environment (ttl_compile_state state, ttl_symbol_table table)
{
  ttl_environment env = ttl_environment_make (state->pool, NULL);
  ttl_binding bind;

  state->error_type = ttl_make_basic_type (state->pool, type_error);
  state->void_type = ttl_make_basic_type (state->pool, type_void);

  state->any_type = ttl_make_basic_type (state->pool, type_any);
  bind = ttl_make_type_binding (state->pool,
				ttl_symbol_enter (table, "any", 3),
				ttl_symbol_enter (table, "any", 3),
				state->any_type);
  ttl_environment_add (env, bind);

  state->int_type = ttl_make_basic_type (state->pool, type_integer);
  bind = ttl_make_type_binding (state->pool,
				ttl_symbol_enter (table, "int", 3),
				ttl_symbol_enter (table, "int", 3),
				state->int_type);
  ttl_environment_add (env, bind);

  state->char_type = ttl_make_basic_type (state->pool, type_char);
  bind = ttl_make_type_binding (state->pool,
				ttl_symbol_enter (table, "char", 4),
				ttl_symbol_enter (table, "char", 4),
				state->char_type);
  ttl_environment_add (env, bind);

  state->bool_type = ttl_make_basic_type (state->pool, type_bool);
  bind = ttl_make_type_binding (state->pool,
				ttl_symbol_enter (table, "bool", 4),
				ttl_symbol_enter (table, "bool", 4),
				state->bool_type);
  ttl_environment_add (env, bind);

  state->long_type = ttl_make_basic_type (state->pool, type_long);
  bind = ttl_make_type_binding (state->pool,
				ttl_symbol_enter (table, "long", 4),
				ttl_symbol_enter (table, "long", 4),
				state->long_type);
  ttl_environment_add (env, bind);

  state->real_type = ttl_make_basic_type (state->pool, type_real);
  bind = ttl_make_type_binding (state->pool,
				ttl_symbol_enter (table, "real", 4),
				ttl_symbol_enter (table, "real", 4),
				state->real_type);
  ttl_environment_add (env, bind);

  state->string_type = ttl_make_basic_type (state->pool, type_string);
  bind = ttl_make_type_binding (state->pool,
				ttl_symbol_enter (table, "string", 6),
				ttl_symbol_enter (table, "string", 6),
				state->string_type);
  ttl_environment_add (env, bind);

  state->nil_type = ttl_make_basic_type (state->pool, type_nil);

  return env;
}

static int
constrainable_type_p (ttl_type t)
{
  return t->kind == type_integer ||
    t->kind == type_bool ||
    t->kind == type_long ||
    t->kind == type_real ||
    t->kind == type_char;
}

typedef struct ttl_alias_list * ttl_alias_list;
struct ttl_alias_list
{
  ttl_type head;
  ttl_alias_list tail;
};

ttl_alias_list
alias_cons (ttl_pool pool, ttl_type t, ttl_alias_list list)
{
  ttl_alias_list tl = malloc (sizeof (struct ttl_alias_list));
  tl->head = t;
  tl->tail = list;
  return tl;
}

static ttl_alias_list aliases = NULL;

/* Convert a type description in abstract syntax to an internal type
   representation.  */
static ttl_type
translate_type (ttl_compile_state state, ttl_ast_node type)
{
  ttl_binding_list bind_list;
  ttl_binding bind;

  switch (type->kind)
    {
    case ast_user_type:
      {
	ttl_ast_node type_name = type->d.user_type.type_name;
	ttl_type * types;
	size_t type_count = ttl_ast_length (type->d.user_type.types);
	ttl_ast_node l = type->d.user_type.types;

	types = ttl_malloc (state->pool, sizeof (ttl_type) * type_count);
	type_count = 0;
	while (l)
	  {
	    types[type_count++] = translate_type (state, l->d.pair.car);
	    l = l->d.pair.cdr;
	  }
	while (type_name->kind == ast_qualident)
	  type_name = type_name->d.qualident.identifier;
	return ttl_make_sum_type
	  (state->pool, type_name->d.identifier.symbol,
	   type->d.user_type.type_name, type_count, types);
      }
      break;

    case ast_basic_type:
      bind_list = lookup_qualident (state, state->current_module,
				    state->env,
				    type->d.basic_type.type_name);
      while (bind_list)
	{
	  if (bind_list->binding->kind == binding_type)
	    break;
	  bind_list = bind_list->next;
	}
      if (bind_list == NULL)
	{
	  ttl_ast_node type_name = type->d.basic_type.type_name;

	  if (state->complain_unbound_types &&
	      type_name->kind != ast_qualident)
	    {
	      state->errors++;
	      ttl_error_print_location (stderr, type_name);
	      ttl_error_print_string (stderr, "unbound identifier: ");
	      ttl_error_print_node (stderr, type_name);
#if 0
	      ttl_error_print_string (stderr, " in module ");
	      ttl_error_print_node (stderr,
				    state->current_module->module_ast_name);
#endif
	      ttl_error_print_nl (stderr);
	    }

	  while (type_name->kind == ast_qualident)
	    type_name = type_name->d.qualident.identifier;
	  /* 	  ttl_symbol_print (stderr, type_name->d.identifier.symbol); */
	  /* 	  fprintf (stderr, "\n"); */
	  return ttl_make_sum_type
	    (state->pool, type_name->d.identifier.symbol,
	     type->d.basic_type.type_name, 0, NULL);

	  /* 	  state->errors++; */
	  /* 	  ttl_error_print_location (stderr, type_name); */
	  /* 	  ttl_error_print_string (stderr, "unbound identifier: "); */
	  /* 	  ttl_error_print_node (stderr, type_name); */
	  /* 	  ttl_error_print_nl (stderr); */
	  /* 	  return state->error_type; */
	}
      bind = bind_list->binding;
      if (bind->d.type)
	return bind->d.type;
      else
	{
	  ttl_type alias = ttl_make_alias_type
	    (state->pool, type->d.basic_type.type_name->d.identifier.symbol,
	     NULL);

	  aliases = alias_cons (state->pool, alias, aliases);
	  return alias;
	}

    case ast_void_type:
      return state->void_type;

    case ast_list_type:
      {
	ttl_type t = translate_type (state, type->d.list_type.element_type);
	if (t->kind == type_error)
	  return t;
	return ttl_make_list_type (state->pool, t);
      }

    case ast_function_type:
      {
	size_t param_count = 0;
	ttl_type * param_types;
	ttl_ast_node params = type->d.function_type.left;
	ttl_type r;

	while (params)
	  {
	    param_count++;
	    params = params->d.pair.cdr;
	  }
	param_types = ttl_malloc (state->pool,
				  param_count * sizeof (ttl_type));
	params = type->d.function_type.left;
	param_count = 0;
	while (params)
	  {
	    param_types[param_count] = translate_type (state,
						       params->d.pair.car);
	    if (param_types[param_count]->kind == type_error)
	      return param_types[param_count];
	    param_count++;
	    params = params->d.pair.cdr;
	  }
	if (type->d.function_type.right)
	  {
	    r = translate_type (state, type->d.function_type.right);
	    if (r->kind == type_error)
	      return r;
	  }
	else
	  r = state->void_type;
	return ttl_make_function_type (state->pool, param_count, param_types,
				       r);
      }

    case ast_array_type:
      {
	ttl_type t;
	t = translate_type (state, type->d.array_type.element_type);
	if (t->kind == type_error)
	  return t;
	return ttl_make_array_type (state->pool, t);
      }

    case ast_constrained_type:
      {
	ttl_type t = translate_type (state,
				     type->d.constrained_type.base_type);
	if (t->kind == type_error)
	  return t;
	if (!constrainable_type_p (t))
	  {
	    state->errors++;
	    ttl_error_print_location (stderr, type);
	    ttl_error_print_string (stderr, "type not constrainable: ");
	    ttl_error_print_node (stderr, type->d.constrained_type.base_type);
	    ttl_error_print_nl (stderr);
	    return state->error_type;
	  }
	return ttl_make_constrained_type (state->pool, t);
      }

    case ast_string_type:
      return state->string_type;

    case ast_tuple_type:
      {
	int subtypes = 0;
	ttl_type * subtype;
	ttl_ast_node l = type->d.tuple_type.fields;
	while (l)
	  {
	    subtypes++;
	    l = l->d.pair.cdr;
	  }
	subtype = ttl_malloc (state->pool, subtypes * sizeof (ttl_type));
	subtypes = 0;
	l = type->d.tuple_type.fields;
	while (l)
	  {
	    subtype[subtypes] = translate_type (state, l->d.pair.car);
	    if (subtype[subtypes]->kind == type_error)
	      return subtype[subtypes];
	    subtypes++;
	    l = l->d.pair.cdr;
	  }
	
	return ttl_make_tuple_type (state->pool, subtypes, subtype);
      }

    default:
      fprintf (stderr, "invalid AST node in translate_type\n");
      abort ();
      return state->error_type;
    }
}


/* First pass of type declaration: check whether a type is already
   defined, and report an error if it is.  Otherwise, the type
   identifier is added to the current environment with an incomplete
   type description.  */
static void
declare_typedef0 (ttl_compile_state state, ttl_ast_node typedf)
{
  ttl_binding_list bind_list;
  ttl_binding bind;

  /* Get all bindings with the new type name in the current
     environment, and report an error if there is a type declared with
     the same name.  */
  bind_list = ttl_environment_search
    (state->pool, state->env, typedf->d.typedf.name->d.identifier.symbol);
  while (bind_list)
    {
      ttl_binding bind = bind_list->binding;
      bind_list = bind_list->next;
      if (bind->kind == binding_type)
	{
	  state->errors++;
	  ttl_error_print_location (stderr, typedf->d.typedf.name);
	  ttl_error_print_string (stderr,
				  "type already defined in this scope: ");
	  ttl_error_print_node (stderr, typedf->d.typedf.name);
	  ttl_error_print_nl (stderr);
	  return;
	}
    }
  /* Add a type binding with an incomplete type description for
     forward references.  */
  bind = ttl_make_type_binding (state->pool,
				typedf->d.typedf.name->d.identifier.symbol, 
				ttl_mangle_name (state,
						 state->module_name,
						 typedf->d.typedf.name,
						 state->void_type), NULL);
  ttl_environment_add (state->env, bind);
  state->type_bindings = ttl_binding_cons (state->pool,
					   bind, state->type_bindings);
}


/* Second pass of type declaration: Since all declared types are
   already in the environment down, we can translate the type
   description now and update the type description.  */
static void
declare_typedef1 (ttl_compile_state state, ttl_ast_node typedf)
{
  ttl_binding_list bind_list;
  ttl_binding bind;
  ttl_type * typep;
  ttl_type type = translate_type (state, typedf->d.typedf.type);

  type->name = typedf->d.typedf.name->d.identifier.symbol;

  if (typedf->d.typedf.public)
    type->exported = 1;

  typep = &(state->current_module->declared_types);
  while (*typep)
    typep = &((*typep)->next);
  *typep = type;

  bind_list = ttl_environment_search
    (state->pool, state->env, typedf->d.typedf.name->d.identifier.symbol);
  while (bind_list)
    {
      bind = bind_list->binding;
      bind_list = bind_list->next;
      if (bind->kind == binding_type)
	{
	  bind->d.type = type;
	  return;
	}
    }
}


/* First pass of datatype declaration: check whether a type is already
   defined, and report an error if it is.  Otherwise, the type
   identifier is added to the current environment with an incomplete
   type description.  */
static void
declare_datatype (ttl_compile_state state, ttl_ast_node datatype)
{
  ttl_binding_list bind_list;

  /* Get all bindings with the new type name in the current
     environment, and report an error if there is a type declared with
     the same name.  */
  bind_list = ttl_environment_search
    (state->pool, state->env, datatype->d.datatype.name->d.identifier.symbol);
  while (bind_list)
    {
      ttl_binding bind = bind_list->binding;
      bind_list = bind_list->next;
      if (bind->kind == binding_type)
	{
	  state->errors++;
	  ttl_error_print_location (stderr, datatype->d.datatype.name);
	  ttl_error_print_string (stderr,
				  "type already defined in this scope: ");
	  ttl_error_print_node (stderr, datatype->d.datatype.name);
	  ttl_error_print_nl (stderr);
	  return;
	}
    }
  /* Add a type binding with an incomplete type description for
     forward references.  */
  ttl_environment_add
    (state->env,
     ttl_make_type_binding (state->pool,
			    datatype->d.datatype.name->d.identifier.symbol, 
			    ttl_mangle_name (state,
					     state->module_name,
					     datatype->d.datatype.name,
					     state->void_type),
			    NULL));
}


static ttl_type param_list_to_type (ttl_compile_state state,
				    ttl_ast_node params,
				    ttl_ast_node ret);

#if 0
static void
maybe_mark_exported (ttl_compile_state state, ttl_function func)
{
  ttl_ast_node l = state->module_exports;
  while (l)
    {
      if (l->d.pair.car->kind == ast_identifier)
	{
	  if (l->d.pair.car->d.identifier.symbol == func->name)
	    {
	      func->exported = 1;
	      break;
	    }
	}
      else
	{
	  fprintf (stderr, "qualified export NYI\n");
	  abort ();
	}
      l = l->d.pair.cdr;
    }
}
#endif


/* Cons a field description with given name, type, variant
   etc. information onto the list `next'.  */
static ttl_field_list
field_cons (ttl_pool pool, ttl_symbol name, ttl_ast_node identifier,
	    ttl_type type, int variant, int offset, int constrainable,
	    enum ttl_function_kind function_kind, ttl_field_list next)
{
  ttl_field_list d = ttl_malloc (pool, sizeof (struct ttl_field_list));
  d->name = name;
  d->identifier = identifier;
  d->type = type;
  d->variant = variant;
  d->offset = offset;
  d->constrainable = constrainable;
  d->function_kind = function_kind;
  d->next = next;
  return d;
}


/* Partition the list `field_list' into field lists which are to be
   implemented in the same accessor/setter function, because they all
   have the same name and type, but are from different variants.  For
   fields for which the resulting list contains more than one element,
   the function must determine the correct offset in the data object
   by examining the current variant of the object. */
static void
partition_field_list (ttl_field_list * field_list, ttl_field_list * list)
{
  ttl_field_list orig = *field_list;
  ttl_field_list result = NULL;
  ttl_field_list leftovers = NULL;

  /* No fields are left.  */
  if (!orig)
    {
      *list = NULL;		/* XXX: Redundant, maybe remove.  */
      return;
    }

  /* Take the first element on the field list as the representative.
     All of the same kind will be on the result list.  */
  result = orig;
  orig = orig->next;
  result->next = NULL;
  while (orig)
    {
      /* Now chain each element on the field list onto either the
	 result list (if the name and type matches the first field
	 found above) or onto the list of leftovers, which will be
	 handled on the next call to this function.  */
      ttl_field_list next = orig->next;
      if (orig->name == result->name &&
	  ttl_types_equal (orig->type, result->type))
	{
	  orig->next = result;
	  result = orig;
	}
      else
	{
	  orig->next = leftovers;
	  leftovers = orig;
	}
      orig = next;
    }
  *field_list = leftovers;
  *list = result;
}


/* Second pass of datatype declaration: Since all declared types are
   already in the environment, we can translate the type description
   now and update the type description in the environment.  */
static void
define_datatype (ttl_compile_state state, ttl_ast_node datatype)
{
  ttl_binding_list bind_list;
  ttl_binding bind;
  ttl_type * typep;
  ttl_type * types;
  size_t type_count;
  ttl_ast_node l;
  ttl_type sum_type;
  int exported = datatype->d.datatype.public;

  type_count = ttl_ast_length (datatype->d.datatype.types);
  types = ttl_malloc (state->pool, sizeof (ttl_type) * type_count);
  l = datatype->d.datatype.types;
  type_count = 0;
  while (l)
    {
      types[type_count++] = translate_type (state, l->d.pair.car); 
      l = l->d.pair.cdr;
    }
  sum_type = ttl_make_sum_type
    (state->pool, datatype->d.datatype.name->d.identifier.symbol,
     ttl_make_ast_qualident (state->pool, state->module_name,
			     datatype->d.datatype.name,
			     NULL, -1, -1, -1, -1),
     type_count, types);
  
  sum_type->name = datatype->d.datatype.name->d.identifier.symbol;

  sum_type->exported = exported;

  typep = &(state->current_module->declared_types);
  while (*typep)
    typep = &((*typep)->next);
  *typep = sum_type;

  {
    /* This is a bit hackish at the moment, but something like this is
       needed: when there are forward references from one datatype
       definition to another, in the first pass a list of these
       forward references is built.  In the second pass, when the
       datatype definitions are actually worked out, we need to fix up
       thos references by inserting the newly created datatype for the
       forward referencing field. */
    ttl_alias_list ls = aliases;
    while (ls)
      {
	if (ls->head->d.alias.name == sum_type->name)
	  {
	    ls->head->d.alias.type = sum_type;
	  }
	ls = ls->tail;
      }
  }

  bind_list = ttl_environment_search
    (state->pool, state->env, datatype->d.datatype.name->d.identifier.symbol);
  while (bind_list)
    {
      bind = bind_list->binding;
      bind_list = bind_list->next;
      if (bind->kind == binding_type)
	{
	  bind->d.type = sum_type;
	  break;
	}
    }
  {
    ttl_ast_node variants = datatype->d.datatype.variants;
    size_t variant_number = 0;
    size_t variant_count = 0;
    ttl_field_list field_list = NULL;

    while (variants)
      {
	variant_count++;
	variants = variants->d.pair.cdr;
      }
    variants = datatype->d.datatype.variants;
    while (variants)
      {
	ttl_type type;
	ttl_binding binding;
	ttl_variable variables;
	ttl_ast_node variant = variants->d.pair.car;
	ttl_ast_node variant_name = variant->d.datatype_variant.name;

	unsigned variant_mask = 0;
	unsigned variant_bit = 1;

	ttl_function func;


	type = param_list_to_type
	  (state,
	   variant->d.datatype_variant.fields,
	   ttl_make_ast_user_type (state->pool,
				   ttl_make_ast_qualident
				   (state->pool, state->module_name,
				    datatype->d.datatype.name,
				    NULL, -1, -1, -1, -1),
				   datatype->d.datatype.types,
				   datatype->d.datatype.name->filename,
				   datatype->d.datatype.name->start_line,
				   datatype->d.datatype.name->start_column,
				   datatype->d.datatype.name->end_line,
				   datatype->d.datatype.name->end_column));
	{
	  int param_count = type->d.function.param_type_count;
	  ttl_type * t = type->d.function.param_types;
	  int i = 0;
	  while (i < param_count)
	    {
	      if (t[i]->kind == type_constrained)
		variant_mask = variant_mask | variant_bit;
	      variant_bit = variant_bit << 1;
	      i++;
	    }
	}
	func = ttl_make_function (state->pool, function_constructor, type);

	func->name = variant_name->d.identifier.symbol;
	func->unique_name = ttl_mangle_name (state, state->module_name,
					     variant_name, type);
	func->d.constr_discrim.variant = variant_number;
	func->variant_count = variant_count;
	func->d.constr_discrim.field_count = ttl_ast_length
	  (variant->d.datatype_variant.fields);
	func->d.constr_discrim.constraint_mask = variant_mask;
	func->exported = exported;
	{
	  ttl_function * fp = &(state->current_module->toplevel_functions);
	  while (*fp)
	    fp = &((*fp)->next);
	  *fp = func;
	}
	{
	  ttl_function * fp = &(state->current_module->functions);
	  while (*fp)
	    fp = &((*fp)->total_next);
	  *fp = func;
	}
#if 0
	maybe_mark_exported (state, func);
#endif
	binding = ttl_make_function_binding
	  (state->pool, variant_name->d.identifier.symbol, 
	   ttl_mangle_name (state, state->module_name, variant_name, type),
	   func, 0);
	ttl_environment_add (state->env, binding);

	{	
	  char * name = ttl_malloc
	    (state->pool, sizeof (char) *
	     (variant_name->d.identifier.symbol->length + 2));
	  ttl_type type;
	  ttl_symbol sym;
	  ttl_type * param = ttl_malloc (state->pool, sizeof (ttl_type));

	  memmove (name, variant_name->d.identifier.symbol->text,
		   variant_name->d.identifier.symbol->length);
	  name[variant_name->d.identifier.symbol->length] = '?';
	  name[variant_name->d.identifier.symbol->length + 1] = '\0';
	  param[0] = sum_type;
	  type = ttl_make_function_type (state->pool, 1, param,
					 state->bool_type);
	  sym = ttl_symbol_enter (state->symbol_table, name,
				  variant_name->d.identifier.symbol->length +
				  1);

	  func = ttl_make_function (state->pool, function_discriminator, type);

	  func->name = sym;
	  func->unique_name = ttl_mangle_name
	    (state, state->module_name, 
	     ttl_make_ast_identifier (state->pool, sym,
				      NULL, -1, -1, -1, -1),
	     type);
	  func->d.constr_discrim.variant = variant_number;
	  func->variant_count = variant_count;
	  func->d.constr_discrim.field_count = ttl_ast_length
	    (variant->d.datatype_variant.fields);
	  func->exported = exported;
	  {
	    ttl_function * fp = &(state->current_module->toplevel_functions);
	    while (*fp)
	      fp = &((*fp)->next);
	    *fp = func;
	  }
	  {
	    ttl_function * fp = &(state->current_module->functions);
	    while (*fp)
	      fp = &((*fp)->total_next);
	    *fp = func;
	  }
#if 0
	  maybe_mark_exported (state, func);
#endif

	  binding = ttl_make_function_binding
	    (state->pool, sym,
	     ttl_mangle_name (state, state->module_name, 
			      ttl_make_ast_identifier (state->pool, sym,
						       NULL, -1, -1, -1, -1),
			      type), func, 0);
	  ttl_environment_add (state->env, binding);
	}

	{
	  ttl_ast_node fields = variant->d.datatype_variant.fields;
	  size_t field_number = 0;
	  while (fields)
	    {
	      ttl_function func;
	      ttl_type * param = ttl_malloc (state->pool, sizeof (ttl_type));
	      ttl_ast_node field = fields->d.pair.car;
	      type = translate_type (state, field->d.variable.type);
	      param[0] = sum_type;
	      type = ttl_make_function_type (state->pool, 1, param, type);

	      field_list = field_cons
		(state->pool,
		 field->d.variable.name->d.identifier.symbol,
		 field->d.variable.name,
		 type, variant_number, field_number, 
		 (variant_mask & (1 << field_number)) != 0,
		 function_accessor,
		 field_list);
	      {	
		char * name = ttl_malloc
		  (state->pool, sizeof (char) *
		   (field->d.variable.name->d.identifier.symbol->length + 2));
		ttl_symbol sym;
		ttl_type * param = ttl_malloc (state->pool,
					       2 * sizeof (ttl_type));

		memmove (name,
			 field->d.variable.name->d.identifier.symbol->text,
			 field->d.variable.name->d.identifier.symbol->length);
		name[field->d.variable.name->d.identifier.symbol->length] =
		  '!';
		name[field->d.variable.name->d.identifier.symbol->length + 1] =
		  '\0';
		param[0] = sum_type;
		type = translate_type (state, field->d.variable.type);
		param[1] = type;
		type = ttl_make_function_type (state->pool, 2, param,
					       state->void_type);
		sym = ttl_symbol_enter
		  (state->symbol_table, name,
		   field->d.variable.name->d.identifier.symbol->length + 1);

		field_list = field_cons
		  (state->pool,
		   sym,
		   ttl_make_ast_identifier (state->pool, sym,
					    NULL, -1, -1, -1, -1),
		   type, variant_number, field_number, 
		   (variant_mask & (1 << field_number)) != 0,
		   function_setter, field_list);

	      }

	      fields = fields->d.pair.cdr;
	      field_number++;
	    }
	}
	variants = variants->d.pair.cdr;
	variant_number++;
      }

    while (field_list)
      {
	ttl_field_list list = NULL;
	partition_field_list (&field_list, &list);
	{
	  ttl_function func;
	  ttl_binding binding;

	  func = ttl_make_function (state->pool, list->function_kind,
				    list->type);
#if 0
	  ttl_symbol_print (stderr, list->name);
	  fprintf (stderr, ": ");
	  ttl_print_type (stderr, list->type);
	  fprintf (stderr, "\n");
#endif
	  func->name = list->name;
	  func->unique_name = ttl_mangle_name (state, state->module_name, 
					       list->identifier, list->type);
	  func->exported = exported;
	  /* 	  func->d.accessor.variant = list->variant; */
	  /* 	  func->variant_count = variant_count; */
	  func->d.accessor.field_list = list;
	  {
	    ttl_function * fp =
	      &(state->current_module->toplevel_functions);
	    while (*fp)
	      fp = &((*fp)->next);
	    *fp = func;
	  }
	  {
	    ttl_function * fp = &(state->current_module->functions);
	    while (*fp)
	      fp = &((*fp)->total_next);
	    *fp = func;
	  }
#if 0
	  maybe_mark_exported (state, func);
#endif
	    
	  binding = ttl_make_function_binding
	    (state->pool, func->name, func->unique_name, func, 1);
	  ttl_environment_add (state->env, binding);

	}
      }
  }
}


/* Return 1 and emit an error message if the symbol `name' is already
   defined as a variable or function with type `type' in the current
   compilation environment, otherwise, return 0.  */
static int
already_defined (ttl_compile_state state, ttl_environment env, ttl_symbol name,
		 ttl_type type, ttl_ast_node ast_name)
{
  ttl_binding_list bind_list;
  ttl_type t;

  bind_list = ttl_environment_search (state->pool, env, name);
  while (bind_list)
    {
      ttl_binding bind = bind_list->binding;
      switch (bind->kind)
	{
	case binding_function:
	  t = bind->d.function->type;
	  break;
	case binding_variable:
	  t = bind->d.function->type;
	  break;
	default:
	  bind_list = bind_list->next;
	  continue;
	}
      if (ttl_types_equal (type, t))
	{
	  state->errors++;
	  ttl_error_print_location (stderr, ast_name);
	  ttl_error_print_string (stderr, "duplicate identifier: ");
	  ttl_error_print_node (stderr, ast_name);
	  ttl_error_print_nl (stderr);
	  return 1;
	}
      bind_list = bind_list->next;
    }
  return 0;
}

/* Compile a variable declaration, adding an entry for each declared
   variable in the list to the current environment.  */
static void
declare_vardef (ttl_compile_state state, ttl_ast_node vardef, int constant)
{
  ttl_ast_node list = vardef->d.vardef.list;

  while (list)
    {
      ttl_variable variable;
      ttl_type type;
      ttl_symbol mangled;
      ttl_ast_node var = list->d.pair.car;

      type = translate_type (state, var->d.variable.type);
      if (state->current_function)
	mangled = ttl_uniquify_name (state, var->d.variable.name);
      else
	mangled = ttl_mangle_name (state, state->module_name,
			       var->d.variable.name, type);
      variable = ttl_make_variable
	(state->pool,
	 state->current_function ? variable_local : variable_global, type,
	 constant);
      variable->exported = vardef->d.vardef.public;
      variable->name = var->d.variable.name->d.identifier.symbol;
      variable->unique_name = mangled;
      variable->documentation = vardef->d.vardef.documentation;
      variable->init = var->d.variable.init;
#if 0
      {
	ttl_ast_node l = state->module_exports;
	while (l)
	  {
	    if (l->d.pair.car->kind == ast_identifier)
	      {
		if (l->d.pair.car->d.identifier.symbol == variable->name)
		  {
		    variable->exported = 1;
		    break;
		  }
	      }
	    else
	      {
		fprintf (stderr, "qualified export NYI\n");
		abort ();
	      }
	    l = l->d.pair.cdr;
	  }
      }
#endif
      if (!already_defined (state, state->env,
			    var->d.variable.name->d.identifier.symbol,
			    type, var->d.variable.name))
	{
	  ttl_environment_add (state->env,
			       ttl_make_variable_binding
			       (state->pool,
				variable->name, 
				mangled,
				variable));
	  if (!state->current_function)
	    {
	      /* Add this variable to the list of global variables of the
		 current module.  */
	      ttl_variable * v = &state->current_module->globals;
	      while (*v) 
		{
		  v = &((*v)->next);
		}
	      *v = variable;

	    }
	  else
	    {
	      /* Add this variable to the list of local variables of the
		 current function.  */
	      ttl_variable * v = &state->current_function->locals;
	      while (*v) 
		{
		  v = &((*v)->next);
		}
	      *v = variable;
	    }
	}
      list = list->d.pair.cdr;
    }
  
}


/* Translate a list of AST statements to a list of IL statements.  */
static ttl_il_node
translate_stmt_list (ttl_compile_state state, ttl_ast_node stmts)
{
  ttl_il_node list = NULL, * p = &list;
  if (stmts)
    {
      ttl_il_node s;
      ttl_ast_node stmt;
      int old_tail = state->tail_position;
      
      state->tail_position = 0;

      while (stmts->d.pair.cdr)
	{
	  stmt = stmts->d.pair.car;
	  s = translate_stmt (state, stmt);

	  if (s)
	    {
	      *p = ttl_make_il_pair (state, s, NULL);
	      p = &((*p)->d.pair.cdr);
	    }

	  stmts = stmts->d.pair.cdr;
	}

      state->tail_position = old_tail;

      stmt = stmts->d.pair.car;
      s = translate_stmt (state, stmt);
      *p = ttl_make_il_pair (state, s, NULL);
      
    }
  else
    {
      if (state->tail_position &&
	  state->current_return_type != state->void_type)
	{
	  state->errors++;
	  ttl_error_print_location (stderr, state->current_function_ast);
	  ttl_error_print_string (stderr,
				  "missing return statement in function: ");
	  ttl_error_print_node (stderr,
				state->current_function_ast->d.function.name);
	  ttl_error_print_nl (stderr);
	}
    }
  return list;
}


/* Take a formal parameter list in abstract syntax `params' and the
   abstract syntax of the return type `ret' and construct a type
   description for the corresponding function type.  */
static ttl_type
param_list_to_type (ttl_compile_state state, ttl_ast_node params,
		    ttl_ast_node ret)
{
  ttl_ast_node l = params;
  ttl_type * param_types;
  unsigned param_count = 0;
  size_t i;
  ttl_type result;

  while (l)
    {
      param_count++;
      l = l->d.pair.cdr;
    }

  param_types = ttl_malloc (state->pool, param_count * sizeof (ttl_type));
  i = 0;
  l = params;
  while (l)
    {
      param_types[i] = translate_type (state, l->d.pair.car->d.variable.type);
      i++;
      l = l->d.pair.cdr;
    }

  if (ret)
    result = translate_type (state, ret);
  else
    result = state->void_type;

  return ttl_make_function_type (state->pool, param_count, param_types,
				 result);
}


static ttl_type
constraint_param_list_to_type (ttl_compile_state state, ttl_ast_node params)
{
  ttl_ast_node l = params;
  ttl_type * param_types;
  unsigned param_count = 0;
  size_t i;
  ttl_type result;

  while (l)
    {
      param_count++;
      l = l->d.pair.cdr;
    }

  param_types = ttl_malloc (state->pool, param_count * sizeof (ttl_type));
  i = 0;
  l = params;
  while (l)
    {
      param_types[i] = translate_type (state, l->d.pair.car->d.variable.type);
      i++;
      l = l->d.pair.cdr;
    }

  result = state->bool_type;

  return ttl_make_function_type (state->pool, param_count, param_types,
				 result);
}


static ttl_variable
parameter_variables (ttl_compile_state state, ttl_ast_node params)
{
  ttl_ast_node l = params;
  ttl_type type;
  unsigned param_count = 0;
  size_t i;
  ttl_variable variable = NULL, * variablep = &variable;

  while (l)
    {
      param_count++;
      l = l->d.pair.cdr;
    }

  i = 0;
  l = params;
  while (l)
    {
      ttl_symbol unique_name;

      type = translate_type (state, l->d.pair.car->d.variable.type);

      unique_name = ttl_uniquify_name (state, l->d.pair.car->d.variable.name);
      *variablep = ttl_make_variable (state->pool, variable_param,
				      type, 0 /* FIXME: Not constant.  */);
      (*variablep)->name = l->d.pair.car->d.variable.name->d.identifier.symbol;
      (*variablep)->unique_name = unique_name;
      variablep = &((*variablep)->next);

      i++;
      l = l->d.pair.cdr;
    }
  return variable;
}


/* First pass of function definition: declare a function with its
   type.  */
static void
declare_fundef (ttl_compile_state state, ttl_ast_node fun)
{
  ttl_type type;
  ttl_binding binding;
  ttl_function function, * f;
  ttl_variable param_vars;

  param_vars = parameter_variables (state, fun->d.function.params);
  type = param_list_to_type (state, fun->d.function.params,
			     fun->d.function.type);
  function = ttl_make_function (state->pool, function_function, type);
  function->exported = fun->d.function.public;
  function->params = param_vars;
  function->name = fun->d.function.name->d.identifier.symbol;
  function->documentation = fun->d.function.documentation;
  if (state->current_function)
    function->unique_name = ttl_uniquify_name (state, fun->d.function.name);
  else
    function->unique_name = ttl_mangle_name (state, state->module_name,
					     fun->d.function.name, type);
  function->d.function.nesting_level = state->nesting_level;
#if 0
  {
    ttl_ast_node l = state->module_exports;
    while (l)
      {
	if (l->d.pair.car->kind == ast_identifier)
	  {
	    if (l->d.pair.car->d.identifier.symbol == function->name)
	      {
		function->exported = 1;
		break;
	      }
	  }
	else
	  {
	    fprintf (stderr, "qualified export NYI\n");
	    abort ();
	  }
	l = l->d.pair.cdr;
      }
  }
#endif
  if (!already_defined (state, state->env, function->name, type,
			fun->d.function.name))
    {
      if (state->current_function)
	{
	  /* For nested function definitions, we have to create an
	     artificial variable which holds the closure for the local
	     function.  */
	  ttl_variable variable;

	  variable = ttl_make_variable (state->pool, variable_local, type,
					0 /* FIXME: Not constant.  */);
	  variable->name = function->name;
	  variable->unique_name = function->unique_name;
	  {
	    /* Add this variable to the list of local variables of the
	       current function.  */
	    ttl_variable * v = &state->current_function->locals;
	    while (*v) 
	      {
		v = &((*v)->next);
	      }
	    *v = variable;
	  }
	  function->variable = variable;
	}

      binding = ttl_make_function_binding
	(state->pool, function->name, function->unique_name, function, 0);

      ttl_environment_add (state->env, binding);

      f = &(state->current_module->functions);
      while (*f)
	f = &((*f)->total_next);
      *f = function;
      if (state->current_function)
	{
	  function->enclosing = state->current_function;
	  f = &(state->current_function->enclosed);
	  while (*f)
	    f = &((*f)->next);
	  *f = function;
	}
      else
	{
	  f = &(state->current_module->toplevel_functions);
	  while (*f)
	    f = &((*f)->next);
	  *f = function;
	}
    }
}

/* Second pass of function definition: compile a function
   definition.  */
static ttl_il_node
define_fundef (ttl_compile_state state, ttl_ast_node function)
{
  ttl_il_node b;
  ttl_ast_node l;
  ttl_environment old_env, env;
  ttl_type old_return_type;
  ttl_function old_current_function;
  ttl_ast_node old_function_ast;
  ttl_binding_list bind_list;
  ttl_function current_function = NULL;
  ttl_type type;
  int old_tail;
  ttl_variable curpar;

  /* Translate the current function's type.  We need it to find the
     binding for the function which was created during the earlier
     pass for normal functions; or for creating that binding for
     function expressions.  */
  type = param_list_to_type (state, function->d.function.params,
			     function->d.function.type);

  /* If this is a normal function definition (that is, if it has a
     name), we search for the function binding created during the
     earlier pass.  Otherwise, we create a new function descriptor for
     the function.  */
  if (function->d.function.name)
    {
      bind_list = ttl_environment_search
	(state->pool, state->env,
	 function->d.function.name->d.identifier.symbol);
      while (bind_list)
	{
	  if (bind_list->binding->kind == binding_function &&
	      !bind_list->binding->synthetic)
	    {
	      if (ttl_types_equal (type, bind_list->binding->d.function->type))
		{
		  current_function = bind_list->binding->d.function;
		  break;
		}
	    }
	  bind_list = bind_list->next;
	}

      /* This can happen if an error occured during function declaration,
	 pass 1.  */
      if (!current_function)
	{
	  fprintf (stderr, "err\n");
	  return NULL;
	}
    }
  else
    {
      ttl_function * f;
      ttl_variable param_vars;

      param_vars = parameter_variables (state, function->d.function.params);

      current_function = ttl_make_function (state->pool, function_function,
					    type);

      current_function->params = param_vars;

      current_function->name = ttl_uniquify_name (state, state->module_name);
      current_function->unique_name = current_function->name;
      current_function->d.function.nesting_level = state->nesting_level;

      f = &(state->current_module->functions);
      while (*f)
	f = &((*f)->total_next);
      *f = current_function;
      if (state->current_function)
	{
	  current_function->enclosing = state->current_function;
	  f = &(state->current_function->enclosed);
	  while (*f)
	    f = &((*f)->next);
	  *f = current_function;
	}
      else
	{
	  f = &(state->current_module->toplevel_functions);
	  while (*f)
	    f = &((*f)->next);
	  *f = current_function;
	}
    }

  old_tail = state->tail_position;
  old_current_function = state->current_function;
  old_function_ast = state->current_function_ast;
  old_env = state->env;
  old_return_type = state->current_return_type;

  state->tail_position = 1;
  state->current_function = current_function;
  state->current_function_ast = function;

  env = ttl_environment_make (state->pool, state->env);
  l = function->d.function.params;
  curpar = state->current_function->params;
  while (l)
    {
      ttl_environment_add
	(env,
	 ttl_make_variable_binding
	 (state->pool, curpar->name, curpar->unique_name, curpar));
      curpar = curpar->next;
      l = l->d.pair.cdr;
    }
  env = ttl_environment_make (state->pool, env);

  state->env = env;
  
  if (function->d.function.type)
    state->current_return_type = translate_type (state,
						 function->d.function.type);
  else
    state->current_return_type = state->void_type;

  if (!function->d.function.handcoded && !function->d.function.mapped)
    {
      state->nesting_level++;
      b = translate_stmt_list (state, function->d.function.body);
      state->nesting_level--;
    }
  else
    {
      if (!state->pragma_handcoded)
	{
	  ttl_error_print_location (stderr, function);
	  ttl_error_print_string
	    (stderr,
	     "handcoded functions only allowed with --pragma=handcoded");
	  ttl_error_print_nl (stderr);
	  state->errors++;
	  state->pragma_handcoded = 1;
	}
      if (state->current_function->enclosing)
	{
	  ttl_error_print_location (stderr, function);
	  ttl_error_print_string
	    (stderr,
	     "only toplevel functions may be handcoded");
	  ttl_error_print_nl (stderr);
	  state->errors++;
	}
      if (function->d.function.handcoded)
	state->current_function->d.function.handcoded = 1;
      else
	{
	  state->current_function->d.function.mapped = 1;
	  state->current_function->d.function.alias =
	    function->d.function.alias;
	}
      b = NULL;
    }

  state->current_function->d.function.il_code = b;

  {
    int i = 0;
    int locals = 0;
    int params = 0;
    ttl_variable var = state->current_function->params;
    while (var)
      {
	var->defining = state->current_function;
	var->index = i++;
	params++;
	var = var->next;
      }
    var = state->current_function->locals;
    while (var)
      {
	var->defining = state->current_function;
	var->index = i++;
	locals++;
	var = var->next;
      }
    state->current_function->param_count = params;
    state->current_function->local_count = locals;
  }
  /* Restore state for surrounding function.  */
  state->current_return_type = old_return_type;
  state->env = old_env;
  state->current_function = old_current_function;
  state->current_function_ast = old_function_ast;
  state->tail_position = old_tail;

  return ttl_make_il_function (state, function->d.function.name,
			       current_function->unique_name,
			       current_function->type,
			       current_function);
}

static void
declare_constraintdef (ttl_compile_state state, ttl_ast_node cstr)
{
  ttl_type type;
  ttl_binding binding;
  ttl_function function, * f;
  ttl_variable param_vars;

  param_vars = parameter_variables (state, cstr->d.constraint.params);
  type = constraint_param_list_to_type (state, cstr->d.constraint.params);
  function = ttl_make_function (state->pool, function_constraint, type);
  function->exported = cstr->d.constraint.public;
  function->params = param_vars;
  function->name = cstr->d.constraint.name->d.identifier.symbol;
  function->documentation = cstr->d.constraint.documentation;
  if (state->current_function)
    function->unique_name = ttl_uniquify_name (state, cstr->d.constraint.name);
  else
    function->unique_name = ttl_mangle_name (state, state->module_name,
					     cstr->d.constraint.name, type);
  function->d.function.nesting_level = state->nesting_level;
  if (!already_defined (state, state->env, function->name, type,
			cstr->d.constraint.name))
    {
      if (state->current_function)
	{
	  /* For nested function definitions, we have to create an
	     artificial variable which holds the closure for the local
	     function.  */
	  ttl_variable variable;

	  fprintf (stderr, "no nested constraint allowed!\n");
	  exit (1);

	  variable = ttl_make_variable (state->pool, variable_local, type,
					0 /* FIXME: Not constant.  */);
	  variable->name = function->name;
	  variable->unique_name = function->unique_name;
	  {
	    /* Add this variable to the list of local variables of the
	       current function.  */
	    ttl_variable * v = &state->current_function->locals;
	    while (*v) 
	      {
		v = &((*v)->next);
	      }
	    *v = variable;
	  }
	  function->variable = variable;
	}

      binding = ttl_make_function_binding
	(state->pool, function->name, function->unique_name, function, 0);

      ttl_environment_add (state->env, binding);

      f = &(state->current_module->functions);
      while (*f)
	f = &((*f)->total_next);
      *f = function;
      if (state->current_function)
	{
	  function->enclosing = state->current_function;
	  f = &(state->current_function->enclosed);
	  while (*f)
	    f = &((*f)->next);
	  *f = function;
	}
      else
	{
	  f = &(state->current_module->toplevel_functions);
	  while (*f)
	    f = &((*f)->next);
	  *f = function;
	}
    }
}

/* Second pass of function definition: compile a function
   definition.  */
static ttl_il_node
define_constraintdef (ttl_compile_state state, ttl_ast_node constraint)
{
  ttl_il_node b;
  ttl_ast_node l;
  ttl_environment old_env, env;
  ttl_type old_return_type;
  ttl_function old_current_function;
  ttl_ast_node old_function_ast;
  ttl_binding_list bind_list;
  ttl_function current_function = NULL;
  ttl_type type;
  int old_tail;
  ttl_variable curpar;

  /* Translate the current function's type.  We need it to find the
     binding for the function which was created during the earlier
     pass for normal functions; or for creating that binding for
     function expressions.  */
  type = constraint_param_list_to_type (state,
					constraint->d.constraint.params);

  /* If this is a normal function definition (that is, if it has a
     name), we search for the function binding created during the
     earlier pass.  Otherwise, we create a new function descriptor for
     the function.  */
  if (constraint->d.constraint.name)
    {
      bind_list = ttl_environment_search
	(state->pool, state->env,
	 constraint->d.constraint.name->d.identifier.symbol);
      while (bind_list)
	{
	  if (bind_list->binding->kind == binding_function &&
	      !bind_list->binding->synthetic)
	    {
	      if (ttl_types_equal (type, bind_list->binding->d.function->type))
		{
		  current_function = bind_list->binding->d.function;
		  break;
		}
	    }
	  bind_list = bind_list->next;
	}

      /* This can happen if an error occured during function declaration,
	 pass 1.  */
      if (!current_function)
	{
	  fprintf (stderr, "err\n");
	  return NULL;
	}
    }
  else
    {
      ttl_function * f;
      ttl_variable param_vars;

      param_vars = parameter_variables (state, constraint->d.constraint.params);

      current_function = ttl_make_function (state->pool, function_constraint,
					    type);

      current_function->params = param_vars;

      current_function->name = ttl_uniquify_name (state, state->module_name);
      current_function->unique_name = current_function->name;
      current_function->d.function.nesting_level = state->nesting_level;

      f = &(state->current_module->functions);
      while (*f)
	f = &((*f)->total_next);
      *f = current_function;
      if (state->current_function)
	{
	  current_function->enclosing = state->current_function;
	  f = &(state->current_function->enclosed);
	  while (*f)
	    f = &((*f)->next);
	  *f = current_function;
	}
      else
	{
	  f = &(state->current_module->toplevel_functions);
	  while (*f)
	    f = &((*f)->next);
	  *f = current_function;
	}
    }

  old_tail = state->tail_position;
  old_current_function = state->current_function;
  old_function_ast = state->current_function_ast;
  old_env = state->env;
  old_return_type = state->current_return_type;

  state->tail_position = 1;
  state->current_function = current_function;
  state->current_function_ast = constraint;

  env = ttl_environment_make (state->pool, state->env);
  l = constraint->d.constraint.params;
  curpar = state->current_function->params;
  while (l)
    {
      ttl_environment_add
	(env,
	 ttl_make_variable_binding
	 (state->pool, curpar->name, curpar->unique_name, curpar));
      curpar = curpar->next;
      l = l->d.pair.cdr;
    }
  env = ttl_environment_make (state->pool, env);

  state->env = env;
  
  state->current_return_type = state->void_type;

  state->nesting_level++;
  b = translate_stmt_list (state, constraint->d.constraint.body);
  state->nesting_level--;

  state->current_function->d.function.il_code = b;

  {
    int i = 0;
    int locals = 0;
    int params = 0;
    ttl_variable var = state->current_function->params;
    while (var)
      {
	var->defining = state->current_function;
	var->index = i++;
	params++;
	var = var->next;
      }
    var = state->current_function->locals;
    while (var)
      {
	var->defining = state->current_function;
	var->index = i++;
	locals++;
	var = var->next;
      }
    state->current_function->param_count = params;
    state->current_function->local_count = locals;
  }
  /* Restore state for surrounding function.  */
  state->current_return_type = old_return_type;
  state->env = old_env;
  state->current_function = old_current_function;
  state->current_function_ast = old_function_ast;
  state->tail_position = old_tail;

  return ttl_make_il_function (state, constraint->d.constraint.name,
			       current_function->unique_name,
			       current_function->type,
			       current_function);
}

#if 0
/* FIXME: Merge all improvements from function definitions above.  */
static void
declare_constraintdef (ttl_compile_state state, ttl_ast_node con)
{
  ttl_type type;
  ttl_constraint constraint;

  type = param_list_to_type (state, con->d.constraint.params, NULL);
  constraint = ttl_make_constraint (state->pool, type);
  constraint->params = param_types (state, con->d.constraint.params);
  ttl_environment_add
    (state->env,
     ttl_make_constraint_binding
     (state->pool, con->d.constraint.name->d.identifier.symbol, 
      mangle_name (state, 'C', state->module_name,
		   con->d.constraint.name, type),
      constraint));
}

/* FIXME: Merge all improvements from function definitions above.  */
static ttl_il_node
define_constraintdef (ttl_compile_state state, ttl_ast_node constraint)
{
  ttl_il_node b;
  ttl_ast_node l;
  ttl_environment old_env, env;
  ttl_type old_return_type;
  ttl_variable vars;

  env = ttl_environment_make (state->pool, state->env);
  l = constraint->d.constraint.params;
  vars = state->current
  while (l)
    {
      ttl_environment_add
	(env,
	 ttl_make_variable_binding
	 (state->pool,
	  l->d.pair.car->d.variable.name->d.identifier.symbol,
	  l->d.pair.car->d.variable.name->d.identifier.symbol,
	  ttl_make_variable (state->pool, variable_param,
			     l->d.pair.car->d.variable.atype)));
      l = l->d.pair.cdr;
    }
  env = ttl_environment_make (state->pool, env);
  old_env = state->env;
  state->env = env;
  
  old_return_type = state->current_return_type;
  state->current_return_type = state->void_type;

  b = translate_stmt_list (state, constraint->d.constraint.body);

  state->current_return_type = old_return_type;
  state->env = old_env;

  /* FIXME: What about the constraint name?  */
  return ttl_make_il_constraint (state, NULL, NULL, NULL, b);

}
#endif 


/* Look up a (possibly qualified) identifier.  Return a list of all
   matching bindings.  */
static ttl_binding_list
lookup_qualident (ttl_compile_state state, 
		  ttl_module current_module,
		  ttl_environment env,
		  ttl_ast_node ident)
{
  ttl_binding_list bind_list = NULL;

#if 0
  fprintf (stderr, "Looking up: ");
  ttl_ast_print (stderr, ident, 0);
  fprintf (stderr, " in module ");
  ttl_ast_print (stderr, current_module->module_ast_name, 0);
  fprintf (stderr, "\n");
#endif
  if (ident->kind == ast_identifier)
    {
/*       bind_list = ttl_environment_lookup (state->pool, state->env, */
      bind_list = ttl_environment_lookup (state->pool, env,
					  ident->d.identifier.symbol);
    }
  else if (ident->kind == ast_qualident)
    {
      ttl_binding_list b_list = lookup_qualident (state,
						  current_module,
						  env,
/* 						  state->env, */
						  ident->d.qualident.module);
      if (!b_list)
	{
#if 0
	  fprintf (stderr, "not found: ");
	  ttl_ast_print (stderr, ident->d.qualident.module, 0);
	  fprintf (stderr, "\n");
#endif
	  return NULL;
	}
      while (b_list)
	{
	  ttl_binding bind = b_list->binding;
	  switch (bind->kind)
	    {
	    case binding_module:
	      bind_list = ttl_environment_search
		(state->pool,
		 bind->d.module->env,
		 ident->d.qualident.identifier->d.identifier.symbol);
	      return bind_list;

	    case binding_function:
	    case binding_type:
	    case binding_variable:
	      break;
	    }
	  b_list = b_list->next;
	}
    }
  else
    {
      fprintf (stderr, "Invalid AST node in lookup_qualident()\n");
      abort ();
    }
  return bind_list;
}


/* Return non-zero.  */
static int
is_any_type (ttl_compile_state state, ttl_type type)
{
  return 1;
}


/* Return non-zero if `type' is an aggregate type, 0 otherwise.  */
static int
is_aggregate_type (ttl_compile_state state, ttl_type type)
{
  if (type->kind == type_string ||
      type->kind == type_array ||
      type->kind == type_tuple)
    return 1;
  else
    return 0;
}


/* Return zero if `type' is not a numeric type, non-zero otherwise.  */
static int
is_numeric_type (ttl_compile_state state, ttl_type type)
{
  if (type == state->int_type || type == state->long_type ||
      type == state->real_type ||
      (type->kind == type_constrained &&
       is_numeric_type (state, type->d.constrained.base)))
    return 1;
  else
    return 0;
}


/* Return zero if `type' is not a type whose values can be added,
   non-zero otherwise.  */
static int
is_addable_type (ttl_compile_state state, ttl_type type)
{
  if (type == state->string_type ||
      type == state->int_type || type == state->long_type ||
      type == state->real_type ||
      (type->kind == type_constrained &&
       is_numeric_type (state, type->d.constrained.base)))
    return 1;
  else
    return 0;
}


/* Return zero if `type' is not an integer type, non-zero otherwise.  */
static int
is_integer_type (ttl_compile_state state, ttl_type type)
{
  if (type == state->int_type)
    return 1;
  else
    return 0;
}


/* Return zero if `type' is not a character type, non-zero otherwise.  */
static int
is_char_type (ttl_compile_state state, ttl_type type)
{
  if (type == state->char_type)
    return 1;
  else
    return 0;
}


/* Return zero if `type' is not a boolean type, non-zero otherwise.  */
static int
is_boolean_type (ttl_compile_state state, ttl_type type)
{
  if (type == state->bool_type)
    return 1;
  else
    return 0;
}


/* Return zero if `type' is not a basic type, non-zero otherwise.  */
static int
is_basic_type (ttl_compile_state state, ttl_type type)
{
  if (type == state->int_type || type == state->long_type ||
      type == state->real_type || type == state->char_type ||
      type == state->nil_type || type == state->bool_type)
    return 1;
  else
    return 0;
}


/* Return zero if `type' is not a void type, non-zero otherwise.  */
static int
is_void_type (ttl_compile_state state, ttl_type type)
{
  if (type->kind == type_void)
    return 1;
  else
    return 0;
}


/* Return zero if `type' is not a comparable type, non-zero otherwise.  */
static int
is_compare_type (ttl_compile_state state, ttl_type type)
{
  if (type == state->bool_type || type == state->long_type ||
      type == state->int_type || type == state->real_type ||
      type == state->char_type || type == state->string_type ||
      type == state->nil_type ||
      type->kind == type_sum ||
      type->kind == type_list ||
      (type->kind == type_constrained &&
       is_compare_type (state, type->d.constrained.base)))
    return 1;
  else
    return 0;
}


/* Return zero if `type' is not a ordered type, non-zero otherwise.  */
static int
is_ordered_type (ttl_compile_state state, ttl_type type)
{
  if (type == state->bool_type || type == state->int_type ||
      type == state->long_type ||
      type == state->real_type || type == state->char_type ||
      (type->kind == type_constrained &&
       is_ordered_type (state, type->d.constrained.base)))
    return 1;
  else
    return 0;
}


/* Return zero if `type' is not the current return type, non-zero
   otherwise.  */
static int
is_current_return_type (ttl_compile_state state, ttl_type type)
{
#if 0
  ttl_print_type (stderr, state->current_return_type);
  fprintf (stderr, " ");
  ttl_print_type (stderr, type);
  fprintf (stderr, "\n");
#endif
  return ttl_types_equal (state->current_return_type, type);
}


static ttl_il_node_list translate_expr (ttl_compile_state state,
					ttl_ast_node expr);


/* Compile the expression `expr' to intermediate code, restricting it
   to the interpretations typed in a way such that the type predicate
   `type_pred()' returns true.  Then checks that exactly one
   interpretation survives pruning, otherwise emits error
   messages.  */
static ttl_il_node
translate_singleton_expr (ttl_compile_state state,
			  ttl_ast_node expr,
			  int (*type_pred)(ttl_compile_state, ttl_type))
{
  ttl_il_node_list ls, result = NULL;
  int was_empty;

  /* Find all interpretations.  */
  ls = translate_expr (state, expr);
  was_empty = ls == NULL;
  while (ls)
    {
#if 0
      ttl_print_type (stderr, ls->elem->type);
      fprintf (stderr, "\n");
#endif
      /* Skip those which do not pass the type test.  */
      if (type_pred (state, ls->elem->type))
	result = ttl_il_cons (state, ls->elem, result);
      ls = ls->next;
    }
  /* Check for no surviving interpretation.  */
  if (!result)
    {
      state->errors++;
      if (!was_empty)
	{
	  ttl_error_print_location (stderr, expr);
	  ttl_error_print_string (stderr, "type error: ");
	  ttl_error_print_node (stderr, expr);
	  ttl_error_print_nl (stderr);
	}
      return ttl_make_il_error (state);
    }
  /* Check that at most one interpretation survives.  */
  if (result->next)
    {
      state->errors++;
      ttl_error_print_location (stderr, expr);
      ttl_error_print_string (stderr, "ambiguous expression: ");
      ttl_error_print_node (stderr, expr);
      ttl_error_print_nl (stderr);
      return ttl_make_il_error (state);
    }
  return result->elem;
}


/* Translate the numeric binary AST operation `expr' to intermediate
   language.  `op' it the intermediate language binary operator for
   the expression.  Return a list of all possible interpretations.  */
static ttl_il_node_list
translate_binop (ttl_compile_state state,
		 enum ttl_il_binop op,
		 ttl_ast_node expr,
		 int (*pred)(ttl_compile_state, ttl_type))
{
  ttl_il_node_list op0, op1, result = NULL;
  ttl_il_node_list o0, o1;
  int no_complain = 0;

  op0 = translate_expr (state, expr->d.binop.op0);
  op1 = translate_expr (state, expr->d.binop.op1);

  if (!op0 || !op1)
    no_complain = 1;

  o0 = op0;
  while (o0)
    {
      if (pred (state, o0->elem->type))
	{
	  o1 = op1;
	  while (o1)
	    {
	      if (ttl_types_equal (o0->elem->type, o1->elem->type))
		result = ttl_il_cons
		  (state,
		   ttl_make_il_binop (state, op, o0->elem, o1->elem,
				      o0->elem->type,
				      expr->filename,
				      expr->start_line,
				      expr->start_column,
				      expr->end_line,
				      expr->end_column),
		   result);
	      o1 = o1->next;
	    }
	}
      o0 = o0->next;
    }
  if (!result)
    {
      state->errors++;
      if (!no_complain)
	{
	  ttl_error_print_location (stderr, expr);
	  ttl_error_print_string (stderr, "cannot type binary expression: ");
	  ttl_error_print_node (stderr, expr);
	  ttl_error_print_nl (stderr);
	}
    }
  return result;
}


/* This Overloading Resolution Algorithm was mostly taken from:

   Baker, T. "A one-pass algorithm for overload resolution in Ada"
   ACM Transactions on Programming Languages adn Systems 4(4): 601-14,
   1982.

   which arrived from the Cforall web site of Richard C. Bilson,
*/
static ttl_il_node_list
translate_call (ttl_compile_state state, ttl_ast_node expr)
{
  unsigned formal_count = 0;
  unsigned actual_count = 0;
  ttl_il_node_list * actual_lists;
  ttl_il_node_list fun_list;
  ttl_il_node_list result_list = NULL;

  ttl_ast_node l;
  unsigned i;

  /* Count the number of arguments.  */
  l = expr->d.call.args;
  while (l)
    {
      actual_count++;
      l = l->d.pair.cdr;
    }

  /* Allocate storage for all possible interpretations of the
     arguments.  */
  actual_lists = ttl_malloc (state->pool,
			     actual_count * sizeof (ttl_il_node_list));

  /* Now compile all possible interpretations of all actual
     arguments.  */
  l = expr->d.call.args;
  i = 0;
  while (l)
    {
      actual_lists[i] = translate_expr (state, l->d.pair.car);
      i++;
      l = l->d.pair.cdr;
    }

  i = 0;
  while (i < actual_count)
    {
      /* If there is no interpretations for one of the arguments,
	 there is no chance that we find an interpretation for the
	 call, so return early, to avoid unnecessary error
	 messages.  */
      if (!actual_lists[i])
	return NULL;
      i++;
    }

  /* Compile the possible interpretations of the function
     expression.  */
  fun_list = translate_expr (state, expr->d.call.function);

  if (!fun_list)
    {
      /* Same for the function expression... */
      return NULL;
    }

  /* For each interpretation in fun_list... */
  while (fun_list)
    {
      ttl_il_node fun = fun_list->elem;
      ttl_type fun_type = fun->type;
      
      /* Only take the interpretations which are function values.  */
      if (fun_type->kind != type_function)
	goto next_interpretation;
      
/*       fprintf (stderr, "Trying to type "); */
/*       ttl_ast_print (stderr, expr, 0); */
/*       fprintf (stderr, ", with type "); */
/*       ttl_print_type (stderr, fun_type); */
/*       fprintf (stderr, "\n"); */

      formal_count = fun_type->d.function.param_type_count;

/*       fprintf (stderr, "formal count: %d, actual count: %d\n", */
/* 	       formal_count, actual_count); */
/*       fflush (stderr); */

      /* Only take those interpretations which have the right arity.  */
      if (actual_count == formal_count)
	{
	  unsigned i;
	  ttl_il_node param_list = NULL, * p = &param_list;
	  ttl_il_node il_fun;
		  

	  /* Go through all formal/actual parameter pairs.  */
	  for (i = 0; i < actual_count; i++)
	    {
	      ttl_type formal_type;
	      /* Get the list of interpretations for actual I.  */
	      ttl_il_node_list ls = actual_lists[i];

	      /* Get the type of the formal parameter I.  */
	      formal_type = fun_type->d.function.param_types[i];

	      /* FIXME: is it necessary to check for duplicates here?  */
	      while (ls)
		{
/* 		  ttl_print_type (stderr, ls->elem->type); */
/* 		  fprintf (stderr, " <=> "); */
/* 		  ttl_print_type (stderr, formal_type); */
/* 		  fprintf (stderr, "\n"); */
		  if (ttl_types_equal (ls->elem->type, formal_type))
		    {
		      *p = ttl_make_il_pair (state,
					     ls->elem,
					     NULL);
		      p = &((*p)->d.pair.cdr);
		      break;
		    }
		  ls = ls->next;
		}
	      if (!ls)
		goto next_interpretation;
	    }
	  {
	    /* Mark all ambiguous in the result list.  */
	    ttl_il_node_list ls = result_list;
	    while (ls)
	      {
		if (ttl_types_equal (fun_type->d.function.return_type,
				     ls->elem->type))
		  {
		    ls->elem->ambiguous = 1;
		    break;
		  }
		ls = ls->next;
	      }
	    /* If there were no ambiguous results, add the current
	       interpretation to the result set.  */
	    if (!ls)
	      {
		ttl_il_node call = ttl_make_il_call
		  (state, fun_list->elem, param_list,
		   fun_type->d.function.return_type,
		   expr->filename,
		   expr->start_line, expr->start_column,
		   expr->end_line, expr->end_column);
		if (fun->lvalue)
		  call->lvalue = 1;
		result_list = ttl_il_cons (state, call, result_list);
	      }
	  }
	}
    next_interpretation:
      fun_list = fun_list->next;
    }

  /* Finally, we remove all ambiguous interpretations from the
     result set.  */
  {
    ttl_il_node_list result = NULL;
    int ambiguous = 0;
    while (result_list)
      {
	if (!result_list->elem->ambiguous)
	  {
	    result = ttl_il_cons (state, result_list->elem, result);
	  }
	else
	  ambiguous = 1;
	result_list = result_list->next;
      }
    if (!result)
      {
	state->errors++;
	ttl_error_print_location (stderr, expr);
	if (ambiguous)
	  ttl_error_print_string (stderr, "function call is ambiguous: ");
	else
	  ttl_error_print_string (stderr, "cannot type function call: ");
	ttl_error_print_node (stderr, expr);
	ttl_error_print_nl (stderr);
      }
    return result;
  }
}


/* Make the types of all nodes in the IL list the boolean type.  */
static void
make_boolean (ttl_compile_state state, ttl_il_node_list ls)
{
  while (ls)
    {
      ls->elem->type = state->bool_type;
      ls = ls->next;
    }
}


static int
is_constant_expression (ttl_ast_node expr)
{
  switch (expr->kind)
    {
    case ast_int_const:
    case ast_long_const:
    case ast_bool_const:
    case ast_char_const:
    case ast_null_const:
    case ast_real_const:
      return 1;
    default:
      return 0;
    }
}

/* Translate an AST expression to intermediate language.  Because of
   overloading, an expression can have multiple interpretations (and
   in the error case, zero interpretations).  That is why this
   function returns a list of IL nodes.  */
static ttl_il_node_list
translate_expr (ttl_compile_state state, ttl_ast_node expr)
{
  ttl_il_node_list op0, op1;

  switch (expr->kind)
    {
    case ast_binop:
      switch (expr->d.binop.op)
	{
	case binop_assign:
	case binop_assign_force:
	  {
	    ttl_il_node_list result = NULL;
	    ttl_il_node_list o0, o1;
	    int forced = expr->d.binop.op == binop_assign_force;
	    int was_empty;

	    in_lvalue_position++;
	    op0 = translate_expr (state, expr->d.binop.op0);
	    in_lvalue_position--;
	    op1 = translate_expr (state, expr->d.binop.op1);
	    was_empty = !op0 || !op1;
	    o0 = op0;
	    while (o0)
	      {
		o1 = op1;
		while (o1)
		  {
		    if (ttl_types_equal (o0->elem->type, o1->elem->type))
		      {
			result = ttl_il_cons
			  (state, ttl_make_il_binop (state,
						     il_binop_assign,
						     o0->elem,
						     o1->elem,
						     state->void_type,
						     expr->filename,
						     expr->start_line,
						     expr->start_column,
						     expr->end_line,
						     expr->end_column),
			   result);
			if (!forced && !o0->elem->lvalue)
			  {
			    state->errors++;
			    ttl_error_print_location (stderr,
						      expr->d.binop.op0);
			    ttl_error_print_string (stderr,
						    "not an l-value: ");
			    ttl_error_print_node (stderr,
						  expr->d.binop.op0);
			    ttl_error_print_nl (stderr);
			  }
		      }
		    o1 = o1->next;
		  }
		o0 = o0->next;
	      }
	    if (!result)
	      {
		state->errors++;
		if (!was_empty)
		  {
		    ttl_error_print_location (stderr, expr);
		    ttl_error_print_string
		      (stderr, "cannot type assignment: ");
		    ttl_error_print_node (stderr, expr);
		    ttl_error_print_nl (stderr);
		  }
	      }
	    return result;
	  }
	case binop_add:
	  return translate_binop (state, il_binop_add, expr, is_addable_type);
	case binop_sub:
	  return translate_binop (state, il_binop_sub, expr, is_numeric_type);
	case binop_mul:
	  return translate_binop (state, il_binop_mul, expr, is_numeric_type);
	case binop_div:
	  return translate_binop (state, il_binop_div, expr, is_numeric_type);
	case binop_mod:
	  return translate_binop (state, il_binop_mod, expr, is_numeric_type);
	case binop_and:
	  return translate_binop (state, il_binop_and, expr, is_boolean_type);
	case binop_or:
	  return translate_binop (state, il_binop_or, expr, is_boolean_type);
	case binop_eq:
	  op0 = translate_binop (state, il_binop_eq, expr, is_compare_type);
	  make_boolean (state, op0);
	  return op0;
	case binop_ne:
	  op0 = translate_binop (state, il_binop_ne, expr, is_compare_type);
	  make_boolean (state, op0);
	  return op0;
	case binop_lt:
	  op0 = translate_binop (state, il_binop_lt, expr, is_ordered_type);
	  make_boolean (state, op0);
	  return op0;
	case binop_le:
	  op0 = translate_binop (state, il_binop_le, expr, is_ordered_type);
	  make_boolean (state, op0);
	  return op0;
	case binop_gt:
	  op0 = translate_binop (state, il_binop_gt, expr, is_ordered_type);
	  make_boolean (state, op0);
	  return op0;
	case binop_ge:
	  op0 = translate_binop (state, il_binop_ge, expr, is_ordered_type);
	  make_boolean (state, op0);
	  return op0;
	case binop_cons:
	  {
	    ttl_il_node_list result = NULL;
	    ttl_il_node_list o0, o1;
	    int was_empty = 0;

	    op0 = translate_expr (state, expr->d.binop.op0);
	    op1 = translate_expr (state, expr->d.binop.op1);

	    if (!op0 || !op1)
	      was_empty = 1;

	    o0 = op0;
	    while (o0)
	      {
		o1 = op1;
		while (o1)
		  {
		    if ((o1->elem->type == state->nil_type) ||
			(o1->elem->type->kind == type_list &&
			 ttl_types_equal (o0->elem->type,
					  o1->elem->type->d.list.element)))
		      {
			result = ttl_il_cons
			  (state,
			   ttl_make_il_binop
			   (state,
			    il_binop_cons,
			    o0->elem,
			    o1->elem,
			    ttl_make_list_type (state->pool, o0->elem->type),
			    expr->filename,
			    expr->start_line,
			    expr->start_column,
			    expr->end_line,
			    expr->end_column),
			   result);
		      }
		    o1 = o1->next;
		  }
		o0 = o0->next;
	      }
	    if (!result && !was_empty)
	      {
		state->errors++;
		ttl_error_print_location (stderr, expr);
		ttl_error_print_string (stderr, "cannot type expression: ");
		ttl_error_print_node (stderr, expr);
		ttl_error_print_nl (stderr);
	      }
	    return result;
	  }
	}

    case ast_unop:
      switch (expr->d.unop.op)
	{
	case unop_neg:
	  /* Negation.  */
	  {
	    ttl_il_node_list result = NULL;
	    int was_empty = 0;

	    op0 = translate_expr (state, expr->d.unop.op0);
	    if (!op0)
	      was_empty = 1;
	    while (op0)
	      {
		if (is_numeric_type (state, op0->elem->type))
		  result = ttl_il_cons (state,
					ttl_make_il_unop (state,
							  il_unop_neg,
							  op0->elem, 
							  op0->elem->type,
							  expr->filename,
							  expr->start_line,
							  expr->start_column,
							  expr->end_line,
							  expr->end_column),
					result);
		op0 = op0->next;
	      }
	    if (!result && !was_empty)
	      {
		state->errors++;
		ttl_error_print_location (stderr, expr);
		ttl_error_print_string (stderr, "cannot type expression: ");
		ttl_error_print_node (stderr, expr);
		ttl_error_print_nl (stderr);
	      }
	    return result;
	  }

	case unop_not:
	  /* Boolean negation.  */
	  {
	    ttl_il_node_list result = NULL;
	    int was_empty = 0;

	    op0 = translate_expr (state, expr->d.unop.op0);
	    if (!op0)
	      was_empty = 1;
	    while (op0)
	      {
		if (is_boolean_type (state, op0->elem->type))
		  result = ttl_il_cons (state,
					ttl_make_il_unop (state,
							  il_unop_not,
							  op0->elem, 
							  op0->elem->type,
							  expr->filename,
							  expr->start_line,
							  expr->start_column,
							  expr->end_line,
							  expr->end_column),
					result);
		op0 = op0->next;
	      }
	    if (!result && !was_empty)
	      {
		state->errors++;
		ttl_error_print_location (stderr, expr);
		ttl_error_print_string (stderr, "cannot type expression: ");
		ttl_error_print_node (stderr, expr);
		ttl_error_print_nl (stderr);
	      }
	    return result;
	  }
	case unop_hd:
	  /* List operator `hd'.  */
	  {
	    ttl_il_node_list result = NULL;
	    int was_empty = 0;

	    op0 = translate_expr (state, expr->d.unop.op0);
	    if (!op0)
	      was_empty = 1;
	    while (op0)
	      {
		if (op0->elem->type->kind == type_list)
		  result = ttl_il_cons
		    (state,
		     ttl_make_il_unop (state,
				       il_unop_hd,
				       op0->elem, 
				       op0->elem->type->d.list.element,
				       expr->filename,
				       expr->start_line,
				       expr->start_column,
				       expr->end_line,
				       expr->end_column),
		     result);
		op0 = op0->next;
	      }
	    if (!result && !was_empty)
	      {
		state->errors++;
		ttl_error_print_location (stderr, expr);
		ttl_error_print_string (stderr, "cannot type expression: ");
		ttl_error_print_node (stderr, expr);
		ttl_error_print_nl (stderr);
	      }
	    return result;
	  }
	case unop_tl:
	  /* list operator `tl'.  */
	  {
	    ttl_il_node_list result = NULL;
	    int was_empty = 0;

	    op0 = translate_expr (state, expr->d.unop.op0);
	    if (!op0)
	      was_empty = 1;
	    while (op0)
	      {
		if (op0->elem->type->kind == type_list)
		  result = ttl_il_cons
		    (state,
		     ttl_make_il_unop (state,
				       il_unop_tl,
				       op0->elem, 
				       op0->elem->type,
				       expr->filename,
				       expr->start_line,
				       expr->start_column,
				       expr->end_line,
				       expr->end_column),
		     result);
		op0 = op0->next;
	      }
	    if (!result && !was_empty)
	      {
		state->errors++;
		ttl_error_print_location (stderr, expr);
		ttl_error_print_string (stderr, "cannot type expression: ");
		ttl_error_print_node (stderr, expr);
		ttl_error_print_nl (stderr);
	      }
	    return result;
	  }
	case unop_sizeof:
	  /* Size operator `sizeof'.  */
	  {
	    ttl_il_node arg;
	    arg = translate_singleton_expr (state, expr->d.unop.op0,
					    is_aggregate_type);
	    if (arg->kind == il_error)
	      return NULL;
	    else
	      return ttl_il_cons (state,
				  ttl_make_il_unop (state,
						    il_unop_sizeof,
						    arg, 
						    state->int_type,
						    expr->filename,
						    expr->start_line,
						    expr->start_column,
						    expr->end_line,
						    expr->end_column),
				  NULL);
	  }
	}

    case ast_qualident:
    case ast_identifier:
      {
	ttl_binding_list bind_list;
	ttl_il_node_list result_list = NULL;
	ttl_binding bind;

	bind_list = lookup_qualident (state, state->current_module,
				      state->env, expr);
	if (bind_list)
	  {
	    while (bind_list)
	      {
		bind = bind_list->binding;
		switch (bind->kind)
		  {
		  case binding_variable:
		    if (bind->d.variable->constant &&
			bind->d.variable->init &&
			is_basic_type (state, bind->d.variable->type) &&
			is_constant_expression (bind->d.variable->init) &&
			!in_lvalue_position)
		      {
			ttl_il_node_list vallist =
			  translate_expr (state, bind->d.variable->init);
			ttl_il_node val = vallist->elem;
			result_list = ttl_il_cons (state, val, result_list);
		      }
		    else
		      {
			ttl_il_node var = ttl_make_il_variable
			  (state, expr, bind->mangled_name,
			   bind->d.variable->type, !bind->d.variable->constant,
			   expr->filename, expr->start_line,
			   expr->start_column,
			   expr->end_line, expr->end_column);
			var->d.variable.variable = bind->d.variable;
			result_list = ttl_il_cons (state, var, result_list);
		      }
		    break;
		  case binding_type:
		    break;
		  case binding_function:
		    {
		      unsigned lvalue = 0;

		      /* FIXME: Handle accessors!  */
/* 		      if (bind->d.function->kind == function_accessor) */
/* 			lvalue = 1; */

		      if (bind->d.function->variable)
			{
			  ttl_il_node var = ttl_make_il_variable
			    (state, expr, bind->mangled_name,
			     bind->d.variable->type, 1,
			     expr->filename,
			     expr->start_line, expr->start_column,
			     expr->end_line, expr->end_column);
			  var->d.variable.variable =
			    bind->d.function->variable;
			  result_list = ttl_il_cons (state, var, result_list);
			}
		      else
			result_list = ttl_il_cons
			  (state,ttl_make_il_function (state, expr,
						       bind->mangled_name,
						       bind->d.function->type,
						       bind->d.function),
			   result_list);
		      break;
		    }
		  case binding_module:
		    break;
		  }
		bind_list = bind_list->next;
	      }
	    if (!result_list)
	      {
		state->errors++;
		ttl_error_print_location (stderr, expr);
		ttl_error_print_string
		  (stderr, "variable or function name expected: ");
		ttl_error_print_node (stderr, expr);
		ttl_error_print_nl (stderr);
		return NULL;
	      }
	    return result_list;
	  }
	else
	  {
	    state->errors++;
	    ttl_error_print_location (stderr, expr);
	    ttl_error_print_string (stderr, "undefined identifier: ");
	    ttl_error_print_node (stderr, expr);
	    ttl_error_print_nl (stderr);
	    if (expr->kind == ast_qualident)
	      {
		ttl_error_print_location (stderr, expr);
		ttl_error_print_string
		  (stderr, "warning: maybe need to import module: ");
		ttl_error_print_node (stderr, expr->d.qualident.module);
		ttl_error_print_nl (stderr);
	      }
	    return NULL;
	  }
      }

    case ast_bool_const:
      return ttl_il_cons (state,
			  ttl_make_il_bool (state, expr->d.bool.value,
					    expr->filename,
					    expr->start_line,
					    expr->start_column,
					    expr->end_line,
					    expr->end_column),
			  NULL);
    case ast_null_const:
      return ttl_il_cons (state,
			  ttl_make_il_null (state,
					    expr->filename,
					    expr->start_line,
					    expr->start_column,
					    expr->end_line,
					    expr->end_column),
			  NULL);
    case ast_int_const:
      return ttl_il_cons (state,
			  ttl_make_il_integer (state,
					       expr->d.integer.value,
					       expr->filename,
					       expr->start_line,
					       expr->start_column,
					       expr->end_line,
					       expr->end_column),
			  NULL);

    case ast_long_const:
      return ttl_il_cons (state,
			  ttl_make_il_long (state,
					    expr->d.longint.value,
					    expr->filename,
					    expr->start_line,
					    expr->start_column,
					    expr->end_line,
					    expr->end_column),
			  NULL);

    case ast_real_const:
      return ttl_il_cons (state,
			  ttl_make_il_real (state, expr->d.real.value,
					    expr->filename,
					    expr->start_line,
					    expr->start_column,
					    expr->end_line,
					    expr->end_column),
			  NULL);

    case ast_char_const:
      return ttl_il_cons (state,
			  ttl_make_il_character (state,
						 expr->d.character.value,
						 expr->filename,
						 expr->start_line,
						 expr->start_column,
						 expr->end_line,
						 expr->end_column),
			  NULL);

    case ast_string_const:
      return ttl_il_cons (state,
			  ttl_make_il_string (state,
					      expr->d.string.value,
					      expr->d.string.length,
					      expr->filename,
					      expr->start_line,
					      expr->start_column,
					      expr->end_line,
					      expr->end_column),
			  NULL);

    case ast_index:
      {
	ttl_il_node_list left;
	ttl_il_node index;
	ttl_il_node_list result = NULL;

	left = translate_expr (state, expr->d.index.array);
	index = translate_singleton_expr (state, expr->d.index.index,
					  is_integer_type);

	while (left)
	  {
	    ttl_il_node arr = left->elem;
	    if (arr->type->kind == type_array)
	      {
		ttl_il_node e = ttl_make_il_index (state, arr, index,
						   arr->type->d.array.element,
						   expr->filename,
						   expr->start_line,
						   expr->start_column,
						   expr->end_line,
						   expr->end_column);
		e->lvalue = arr->lvalue;
		/* FIXME: Is every index operation an l-value?  */
		e->lvalue = 1;
		result = ttl_il_cons (state, e, result);
	      }
	    else if (arr->type->kind == type_string)
	      {
		ttl_il_node e = ttl_make_il_index (state, arr, index,
						   state->char_type,
						   expr->filename,
						   expr->start_line,
						   expr->start_column,
						   expr->end_line,
						   expr->end_column);
		e->lvalue = arr->lvalue;
		/* FIXME: Is every index operation an l-value?  */
		e->lvalue = 1;
		result = ttl_il_cons (state, e, result);
	      }
	    left = left->next;
	  }
	return result;
      }

    case ast_call:
      return translate_call (state, expr);

    case ast_array_constructor:
      {
	ttl_ast_node size = expr->d.array_constructor.size;
	ttl_ast_node initial = expr->d.array_constructor.initial;
	ttl_il_node_list initials;
	ttl_il_node il_size;
	ttl_il_node_list result = NULL;

	il_size = translate_singleton_expr (state, size, is_integer_type);
	initials = translate_expr (state, initial);
	while (initials)
	  {
	    result = ttl_il_cons (state,
				  ttl_make_il_array_constructor
				  (state, il_size,
				   initials->elem,
				   ttl_make_array_type (state->pool,
							initials->elem->type),
				   expr->filename,
				   expr->start_line,
				   expr->start_column,
				   expr->end_line,
				   expr->end_column),
				  result);
	    initials = initials->next;
	  }
	return result;
      }

    case ast_list_constructor:
      {
	ttl_ast_node size = expr->d.list_constructor.size;
	ttl_ast_node initial = expr->d.list_constructor.initial;
	ttl_il_node_list initials;
	ttl_il_node il_size;
	ttl_il_node_list result = NULL;

	il_size = translate_singleton_expr (state, size, is_integer_type);
	initials = translate_expr (state, initial);
	while (initials)
	  {
	    result = ttl_il_cons (state,
				  ttl_make_il_list_constructor
				  (state, il_size,
				   initials->elem,
				   ttl_make_list_type (state->pool,
						       initials->elem->type),
				   expr->filename,
				   expr->start_line,
				   expr->start_column,
				   expr->end_line,
				   expr->end_column),
				  result);
	    initials = initials->next;
	  }
	return result;
      }

    case ast_string_constructor:
      {
	ttl_ast_node size = expr->d.string_constructor.size;
	ttl_ast_node initial = expr->d.string_constructor.initial;
	ttl_il_node il_initial;
	ttl_il_node il_size;
	ttl_il_node_list result;

	il_size = translate_singleton_expr (state, size, is_integer_type);
	if (initial)
	  il_initial = translate_singleton_expr (state, initial, is_char_type);
	else
	  il_initial = ttl_make_il_character (state, ' ',
					      expr->filename,
					      expr->start_line,
					      expr->start_column,
					      expr->end_line,
					      expr->end_column);
					      
	result = ttl_il_cons (state,
			      ttl_make_il_string_constructor
			      (state, il_size, il_initial,
			       state->string_type,
			       expr->filename,
			       expr->start_line,
			       expr->start_column,
			       expr->end_line,
			       expr->end_column),
			      NULL);
	return result;
      }

    case ast_array_expr:
      {
	ttl_ast_node elements = expr->d.array_expr.elements;
	ttl_il_node_list * elem_lists;
	unsigned elem_count = 0;
	unsigned i;
	ttl_il_node_list result = NULL;
	ttl_il_node_list temp;

	while (elements)
	  {
	    elem_count++;
	    elements = elements->d.pair.cdr;
	  }
	if (elem_count == 0)
	  {
	    result = ttl_il_cons
	      (state,
	       ttl_make_il_array_expr (state, NULL, state->any_type,
				       expr->filename,
				       expr->start_line,
				       expr->start_column,
				       expr->end_line,
				       expr->end_column),
	       NULL);
	    return result;
	  }
	elements = expr->d.array_expr.elements;
	elem_lists = ttl_malloc (state->pool,
				 elem_count * sizeof (ttl_il_node_list));
	elem_count = 0;
	while (elements)
	  {
	    elem_lists[elem_count] = translate_expr (state,
						     elements->d.pair.car);
	    elem_count++;
	    elements = elements->d.pair.cdr;
	  }
	i = 0;
	while (i < elem_count)
	  {
	    /* No need to do any more type checking if one of the list
	       elements was not typeable.  */
	    if (!elem_lists[i])
	      return NULL;
	    i++;
	  }
	temp = elem_lists[0];
	while (temp)
	  {
	    ttl_il_node node = temp->elem;
	    ttl_il_node_list l;
	    ttl_il_node res = ttl_make_il_pair (state, node, NULL);
	    ttl_il_node * resp = &(res->d.pair.cdr);
	    unsigned i;

	    for (i = 1; i < elem_count; i++)
	      {
		l = elem_lists[i];
		while (l)
		  {
		    if (ttl_types_equal (node->type, l->elem->type))
		      break;
		    l = l->next;
		  }
		if (!l)
		  goto next_array_temp;
		*resp = ttl_make_il_pair (state, l->elem, NULL);
		resp = &((*resp)->d.pair.cdr);
	      }
	    result = ttl_il_cons (state,
				  ttl_make_il_array_expr
				  (state,
				   res,
				   ttl_make_array_type (state->pool,
							node->type),
				   expr->filename,
				   expr->start_line,
				   expr->start_column,
				   expr->end_line,
				   expr->end_column),
				  result);
	  next_array_temp:
	    temp = temp->next;
	  }
	return result;
      }

    case ast_list_expr:
      {
	ttl_ast_node elements = expr->d.list_expr.elements;
	ttl_il_node_list * elem_lists;
	unsigned elem_count = 0;
	unsigned i;
	ttl_il_node_list result = NULL;
	ttl_il_node_list temp;

	while (elements)
	  {
	    elem_count++;
	    elements = elements->d.pair.cdr;
	  }
	if (elem_count == 0)
	  {
	    /* Empty list expressions are the same as `null'.  */
	    result = ttl_il_cons
	      (state, ttl_make_il_null (state,
					expr->filename,
					expr->start_line,
					expr->start_column,
					expr->end_line,
					expr->end_column),
	       NULL);
	    return result;
	  }
	elements = expr->d.list_expr.elements;
	elem_lists = ttl_malloc (state->pool,
				 elem_count * sizeof (ttl_il_node_list));
	elem_count = 0;
	while (elements)
	  {
	    elem_lists[elem_count] = translate_expr (state,
						     elements->d.pair.car);
	    elem_count++;
	    elements = elements->d.pair.cdr;
	  }
	i = 0;
	while (i < elem_count)
	  {
	    /* No need to do any more type checking if one of the list
	       elements was not typeable.  */
	    if (!elem_lists[i])
	      return NULL;
	    i++;
	  }
	temp = elem_lists[0];
	while (temp)
	  {
	    ttl_il_node node = temp->elem;
	    ttl_il_node_list l;
	    ttl_il_node res = ttl_make_il_pair (state, node, NULL);
	    ttl_il_node * resp = &(res->d.pair.cdr);
	    unsigned i;

	    for (i = 1; i < elem_count; i++)
	      {
		l = elem_lists[i];
		while (l)
		  {
		    if (ttl_types_equal (node->type, l->elem->type))
		      break;
		    l = l->next;
		  }
		if (!l)
		  goto next_list_temp;
		*resp = ttl_make_il_pair (state, l->elem, NULL);
		resp = &((*resp)->d.pair.cdr);
	      }
	    result = ttl_il_cons (state,
				  ttl_make_il_list_expr
				  (state,
				   res,
				   ttl_make_list_type (state->pool,
						       node->type),
				   expr->filename,
				   expr->start_line,
				   expr->start_column,
				   expr->end_line,
				   expr->end_column),
				  result);
	  next_list_temp:
	    temp = temp->next;
	  }
	if (!result)
	  {
	    state->errors++;
	    ttl_error_print_location (stderr, expr);
	    ttl_error_print_string (stderr, "cannot type list expression: ");
	    ttl_error_print_node (stderr, expr);
	    ttl_error_print_nl (stderr);
	  }
	return result;
      }

    case ast_tuple_expr:
      {
	unsigned len = 0, i;
	ttl_ast_node l;
	ttl_il_node_list * elem_lists;
	ttl_il_node_list * lists;
	ttl_il_node elem;
	ttl_type * types, type;
	ttl_il_node res = NULL, * resp = &res;
	unsigned lvalue = 1;
	ttl_il_node tuple_expr;

	for (l = expr->d.tuple_expr.fields; l; l = l->d.pair.cdr)
	  len++;
	elem_lists = ttl_malloc (state->pool, len * sizeof (ttl_il_node_list));
	types = ttl_malloc (state->pool, len * sizeof (ttl_type));
	len = 0;
	for (l = expr->d.tuple_expr.fields; l; l = l->d.pair.cdr)
	  {
	    elem_lists[len] = translate_expr (state, l->d.pair.car);
	    /* FIXME: We need to build the cross product of all
	       interpretations here!  */
	    if (elem_lists[len])
	      {
		types[len] = elem_lists[len]->elem->type;
		lvalue = lvalue && elem_lists[len]->elem->lvalue;
		*resp = ttl_make_il_pair (state, elem_lists[len]->elem, NULL);
		resp = &((*resp)->d.pair.cdr);
	      }
	    else
	      return NULL;
	    len++;
	  }
	type = ttl_make_tuple_type (state->pool, len, types);
	tuple_expr = ttl_make_il_tuple_expr (state, 
					     res, type,
					     expr->filename,
					     expr->start_line,
					     expr->start_column,
					     expr->end_line,
					     expr->end_column);
	tuple_expr->lvalue = lvalue;
	return ttl_il_cons (state, tuple_expr, NULL);


	{
	  unsigned change, j, i;
	  ttl_il_node_list l, k;

	  change = 1;
	  while (change)
	    {
	      change = 0;

	      for (j = 0; j < len; j++)
		{
		  for (l = elem_lists[j]; l; l = l->next)
		    {
		      for (i = j + 1; i < len; i++)
			{
			  for (k = elem_lists[i]; k; k = k->next)
			    {
			    }
			}
		    }
		}
	    }
	}
      }

    case ast_function:
      {
/*             declare_fundef (state, expr); */
	ttl_il_node fun = define_fundef (state, expr);
	return ttl_il_cons (state, fun, NULL);
      }
      break;

    case ast_ann_expr:
      {
	ttl_il_node_list res = translate_expr (state, expr->d.ann_expr.expr);
	ttl_il_node_list result = NULL;
	while (res)
	  {
	    result = ttl_il_cons
	      (state,
	       ttl_make_il_ann_expr
	       (state, res->elem,
		expr->d.ann_expr.strength->d.integer.value,
		expr->filename, expr->start_line, expr->start_column,
		expr->end_line, expr->end_column),
	       result);
	    res = res->next;
	  }
	return result;
	break;
      }

    case ast_foreign_expr:
      {
	if (!state->compile_options->pragma_foreign)
	  {
	    state->errors++;
	    ttl_error_print_location (stderr, expr);
	    ttl_error_print_string
	      (stderr,
	       "foreign expressions only allowed with --pragma=foreign");
	    ttl_error_print_nl (stderr);
	  }
	return ttl_il_cons
	  (state,
	   ttl_make_il_foreign_expr
	   (state, expr->d.foreign_expr.expr,
	    expr->filename, expr->start_line, expr->start_column,
	    expr->end_line, expr->end_column),
	   NULL);
	break;
      }

    case ast_var_expr:
      {
	ttl_il_node_list res = translate_expr (state, expr->d.var_expr.expr);
	ttl_il_node_list result = NULL;
	while (res)
	  {
	    if (constrainable_type_p (res->elem->type))
	      result = ttl_il_cons
		(state,
		 ttl_make_il_var_expr
		 (state, res->elem,
		  ttl_make_constrained_type (state->pool, res->elem->type),
		  expr->filename, expr->start_line, expr->start_column,
		  expr->end_line, expr->end_column),
		 result);
	    res = res->next;
	  }
	return result;
	break;
      }

    case ast_deref_expr:
      {
	ttl_il_node_list res = translate_expr (state, expr->d.var_expr.expr);
	ttl_il_node_list result = NULL;
	while (res)
	  {
	    if (res->elem->type->kind == type_constrained)
	      result = ttl_il_cons
		(state,
		 ttl_make_il_deref_expr
		 (state, res->elem,
		  res->elem->type->d.constrained.base,
		  expr->filename, expr->start_line, expr->start_column,
		  expr->end_line, expr->end_column),
		 result);
	    res = res->next;
	  }
	return result;
	break;
      }

    default:
      ttl_error_print_location (stderr, expr);
      ttl_error_print_string (stderr, "NYI: compile_expr: ");
      ttl_error_print_node (stderr, expr);
      fprintf (stderr, " %d", expr->kind);
      ttl_error_print_nl (stderr);
      abort ();
      return NULL;
    }
}


/* Translate an AST if statement to its intermediate language
   counterpart.  */
static ttl_il_node
translate_if (ttl_compile_state state, ttl_ast_node stmt)
{
  ttl_il_node cond, thenstmt, elsestmt;

  cond = translate_singleton_expr (state, stmt->d.ifstmt.cond,
				       is_boolean_type);
  thenstmt = translate_stmt_list (state, stmt->d.ifstmt.thenstmt);
  elsestmt = translate_stmt_list (state, stmt->d.ifstmt.elsestmt);
  return ttl_make_il_if (state, cond, thenstmt, elsestmt,
			 stmt->filename,
			 stmt->start_line,
			 stmt->start_column,
			 stmt->end_line,
			 stmt->end_column);
}


/* Translate an AST `in' statement to its intermediate language
   counterpart.  */
static ttl_il_node
translate_in (ttl_compile_state state, ttl_ast_node stmt)
{
  ttl_il_node instmt;

  instmt = translate_stmt_list (state, stmt->d.instmt.instmt);
  return ttl_make_il_in (state, instmt,
			 stmt->filename,
			 stmt->start_line,
			 stmt->start_column,
			 stmt->end_line,
			 stmt->end_column);
}


/* Translate an AST while loop to its intermediate language
   counterpart.  */
static ttl_il_node
translate_while (ttl_compile_state state, ttl_ast_node stmt)
{
  ttl_il_node cond, dostmt;

  cond = translate_singleton_expr (state, stmt->d.whilestmt.cond,
				   is_boolean_type);
  dostmt = translate_stmt_list (state, stmt->d.whilestmt.dostmt);
  return ttl_make_il_while (state, cond, dostmt, 
			    stmt->filename,
			    stmt->start_line,
			    stmt->start_column,
			    stmt->end_line,
			    stmt->end_column);
}


/* Translate an AST return statement to its intermediate language
   counterpart.  */
static ttl_il_node
translate_return (ttl_compile_state state, ttl_ast_node stmt)
{
  if (stmt->d.returnstmt.expr)
    {
      ttl_il_node expr;
      if (state->current_return_type == state->void_type)
	{
	  state->errors++;
	  ttl_error_print_location (stderr, stmt->d.returnstmt.expr);
	  ttl_error_print_string (stderr,
				  "return with value in void function");
	  ttl_error_print_nl (stderr);
	}
      expr = translate_singleton_expr (state, stmt->d.returnstmt.expr,
				       is_current_return_type);
      return ttl_make_il_return (state, expr,
				 stmt->filename,
				 stmt->start_line,
				 stmt->start_column,
				 stmt->end_line,
				 stmt->end_column);
    }
  else
    {
      return ttl_make_il_return (state, NULL,
				 stmt->filename,
				 stmt->start_line,
				 stmt->start_column,
				 stmt->end_line,
				 stmt->end_column);
    }
}


/* Translate an AST require statement to its intermediate language
   counterpart.  */
static ttl_il_node
translate_require (ttl_compile_state state, ttl_ast_node stmt)
{
  ttl_il_node expr, req_stmt;
  ttl_il_node list = NULL, * listp = &list;
  ttl_ast_node l;

  ttl_permissive_checking++;
  l = stmt->d.require.expr;
  while (l)
    {
      expr = translate_singleton_expr (state, l->d.pair.car,
				       is_boolean_type);
      l = l->d.pair.cdr;
      if (expr->kind != il_error)
	{
	  *listp = ttl_make_il_pair (state, expr, NULL);
	  listp = &((*listp)->d.pair.cdr);
	}
    }
  ttl_permissive_checking--;
  if (stmt->d.require.stmt)
    req_stmt = translate_stmt (state, stmt->d.require.stmt);
  else
    req_stmt = NULL;

  return ttl_make_il_require (state, list, req_stmt,
			      stmt->filename,
			      stmt->start_line,
			      stmt->start_column,
			      stmt->end_line,
			      stmt->end_column);
}


/* Complain if this statement is in tail position and the return type
   of the function is not void, so that missing return statements are
   detected.  */
static void
check_missing_return (ttl_compile_state state, ttl_ast_node stmt)
{
  if (state->tail_position &&
      state->current_return_type != state->void_type)
    {
      state->errors++;
      ttl_error_print_location (stderr, stmt);
      ttl_error_print_string (stderr, "missing return statement");
      ttl_error_print_nl (stderr);
    }
}


/* Translate an AST statement to an intermediate language
   statement.  */
static ttl_il_node
translate_stmt (ttl_compile_state state, ttl_ast_node stmt)
{
  switch (stmt->kind)
    {
    case ast_vardef:
      {
	ttl_il_node stmts = NULL, * sp = &stmts;
	ttl_ast_node ls = stmt->d.vardef.list;
	check_missing_return (state, stmt);
	declare_vardef (state, stmt, 0);
	while (ls)
	  {
	    ttl_ast_node var = ls->d.pair.car;
	    if (var->d.variable.init)
	      {
		ttl_il_node op;
		op = translate_singleton_expr
		  (state,
		   ttl_make_ast_binop
		   (state->pool,
		    binop_assign,
		    var->d.variable.name,
		    var->d.variable.init,
		    var->filename,
		    var->start_line, var->start_column,
		    var->end_line, var->end_column),
		   is_void_type);
		if (op->kind == il_error)
		  {
#if 0
		    state->errors++;
		    ttl_error_print_location (stderr, var);
		    ttl_error_print_string
		      (stderr, "cannot type initialization expression: ");
		    ttl_error_print_node (stderr, var);
		    ttl_error_print_nl (stderr);
#endif
		  }
		else
		  {
		    *sp = ttl_make_il_pair (state, op, NULL);
		    sp = &((*sp)->d.pair.cdr);
		  }
	      }
	    ls = ls->d.pair.cdr;
	  }
	if (stmts)
	  return ttl_make_il_seq (state, stmts);
	else
	  return NULL;
      }

    case ast_constdef:
      {
	ttl_il_node stmts = NULL, * sp = &stmts;
	ttl_ast_node ls = stmt->d.constdef.list;
	check_missing_return (state, stmt);
	declare_vardef (state, stmt, 1);
	while (ls)
	  {
	    ttl_ast_node var = ls->d.pair.car;
	    if (var->d.variable.init)
	      {
		ttl_il_node op;
		op = translate_singleton_expr
		  (state,
		   ttl_make_ast_binop
		   (state->pool,
		    binop_assign_force,
		    var->d.variable.name,
		    var->d.variable.init,
		    var->filename,
		    var->start_line, var->start_column,
		    var->end_line, var->end_column),
		   is_void_type);
		if (op->kind == il_error)
		  {
#if 0
		    state->errors++;
		    ttl_error_print_location (stderr, var);
		    ttl_error_print_string
		      (stderr, "cannot type initialization expression: ");
		    ttl_error_print_node (stderr, var);
		    ttl_error_print_nl (stderr);
#endif
		  }
		else
		  {
		    *sp = ttl_make_il_pair (state, op, NULL);
		    sp = &((*sp)->d.pair.cdr);
		  }
	      }
	    ls = ls->d.pair.cdr;
	  }
	if (stmts)
	  return ttl_make_il_seq (state, stmts);
	else
	  return NULL;
      }

    case ast_typedef:
      check_missing_return (state, stmt);
      declare_typedef0 (state, stmt);
      declare_typedef1 (state, stmt);
      return NULL;

    case ast_function:
      {
	ttl_il_node fun;
	ttl_variable var;
	ttl_il_node il_var;

	check_missing_return (state, stmt);
	declare_fundef (state, stmt);
	fun = define_fundef (state, stmt);
	var = fun->d.function.function->variable;
	il_var = ttl_make_il_variable
				  (state,
				   stmt->d.function.name,
				   fun->d.function.function->unique_name,
				   fun->d.function.function->type, 1,
				   NULL, -1, -1, -1, -1);
	if (var)
	  il_var->d.variable.variable = var;
	return ttl_make_il_binop (state, il_binop_assign,
				  il_var,
				  fun,
				  state->void_type,
				  stmt->filename,
				  stmt->start_line,
				  stmt->start_column,
				  stmt->end_line,
				  stmt->end_column);
      }

/*     case ast_constraint: */
/*       declare_constraintdef (state, stmt); */
/*       return define_constraintdef (state, stmt); */

    case ast_if:
      return translate_if (state, stmt);

    case ast_while:
      return translate_while (state, stmt);

    case ast_in:
      return translate_in (state, stmt);

    case ast_return:
      return translate_return (state, stmt);

    case ast_require:
      check_missing_return (state, stmt);
      return translate_require (state, stmt);

    default:
      {
	check_missing_return (state, stmt);
	return translate_singleton_expr (state, stmt, is_void_type);
      }
    }
}

static void
pass (ttl_compile_state state, ttl_ast_node module, int pass)
{
  ttl_ast_node l;

  /* Go through the module and collect all top-level definitions.  */
  l = module->d.module.definitions;
  while (l)
    {
      switch (l->d.pair.car->kind)
	{
	case ast_vardef:
	  if (pass == 2)
	    {
	      declare_vardef (state, l->d.pair.car, 0);
	    }
	  else if (pass == 3)
	    {
	      ttl_ast_node stmt = l->d.pair.car;
	      ttl_il_node stmts = NULL, * sp = &stmts;
	      ttl_ast_node ls = stmt->d.vardef.list;

	      while (ls)
		{
		  ttl_ast_node var = ls->d.pair.car;
		  if (var->d.variable.init)
		    {
		      ttl_il_node op;
		      op = translate_singleton_expr
			(state,
			 ttl_make_ast_binop
			 (state->pool,
			  binop_assign,
			  var->d.variable.name,
			  var->d.variable.init,
			  var->filename,
			  var->start_line, var->start_column,
			  var->end_line, var->end_column),
			 is_void_type);
		      if (op->kind == il_error)
			{
			  state->errors++;
			  ttl_error_print_location (stderr, var);
			  ttl_error_print_string
			    (stderr, "cannot type initialization expression: ");
			  ttl_error_print_node (stderr, var);
			  ttl_error_print_nl (stderr);
			}
		      else
			{
			  *sp = ttl_make_il_pair (state, op, NULL);
			  sp = &((*sp)->d.pair.cdr);
			}
		    }
		  ls = ls->d.pair.cdr;
		}
	      if (stmts)
		{
		  ttl_il_node * sp = (ttl_il_node *) &state->init_stmts;
		  while (*sp)
		    sp = &((*sp)->d.pair.cdr);
		  *sp = stmts;
		}
	    }
	  break;
	case ast_constdef:
	  if (pass == 2)
	    {
	      declare_vardef (state, l->d.pair.car, 1);
	    }
	  else if (pass == 3)
	    {
	      ttl_ast_node stmt = l->d.pair.car;
	      ttl_il_node stmts = NULL, * sp = &stmts;
	      ttl_ast_node ls = stmt->d.constdef.list;

	      while (ls)
		{
		  ttl_ast_node var = ls->d.pair.car;
		  if (var->d.variable.init)
		    {
		      ttl_il_node op;
		      op = translate_singleton_expr
			(state,
			 ttl_make_ast_binop
			 (state->pool,
			  binop_assign_force,
			  var->d.variable.name,
			  var->d.variable.init,
			  var->filename,
			  var->start_line, var->start_column,
			  var->end_line, var->end_column),
			 is_void_type);
		      if (op->kind == il_error)
			{
			  state->errors++;
			  ttl_error_print_location (stderr, var);
			  ttl_error_print_string
			    (stderr, "cannot type initialization expression: ");
			  ttl_error_print_node (stderr, var);
			  ttl_error_print_nl (stderr);
			}
		      else
			{
			  *sp = ttl_make_il_pair (state, op, NULL);
			  sp = &((*sp)->d.pair.cdr);
			}
		    }
		  ls = ls->d.pair.cdr;
		}
	      if (stmts)
		{
		  ttl_il_node * sp = (ttl_il_node *) &state->init_stmts;
		  while (*sp)
		    sp = &((*sp)->d.pair.cdr);
		  *sp = stmts;
		}
	    }
	  break;
	case ast_typedef:
	  if (pass == 0)
	    declare_typedef0 (state, l->d.pair.car);
	  else if (pass == 1)
	    declare_typedef1 (state, l->d.pair.car);
	  break;
	case ast_function:
	  if (pass == 2)
	    declare_fundef (state, l->d.pair.car);
	  else if (pass == 3)
	    define_fundef (state, l->d.pair.car);
	  break;
	case ast_constraint:
	  if (pass == 2)
	    declare_constraintdef (state, l->d.pair.car);
	  else if (pass == 3)
	    define_constraintdef (state, l->d.pair.car);
	  break;
	case ast_datatype:
	  if (pass == 0)
	    declare_datatype (state, l->d.pair.car);
	  else if (pass == 1)
	    define_datatype (state, l->d.pair.car);
	  break;
	default:
	  fprintf (stderr, "invalid AST node in pass (%d)\n",
		   pass);
	  abort ();
	  break;
	}
      l = l->d.pair.cdr;
    }
}


/* Create a function containing all initializations of global
   variables and constants.  This function will be automatically
   called when the program starts up.  (Via the module's init
   function.)  */
static void
create_init_function (ttl_compile_state state)
{
  ttl_il_node init_stmts = (ttl_il_node) (state->init_stmts);
  ttl_function function, * f;
  ttl_type type;

  if (init_stmts)
    {
      type = ttl_make_function_type (state->pool, 0, NULL, state->void_type);
      function = ttl_make_function (state->pool, function_function, type);

      function->params = NULL;

      function->name = ttl_uniquify_name (state, state->module_name);
      function->unique_name = function->name;

      function->d.function.il_code = state->init_stmts;

      f = &(state->current_module->functions);
      while (*f)
	f = &((*f)->total_next);
      *f = function;
      f = &(state->current_module->toplevel_functions);
      while (*f)
	f = &((*f)->next);
      *f = function;
      
      state->has_init_stmts = 1;
    }
}

static void
translate_module (ttl_compile_state state, ttl_ast_node module,
		  struct ttl_compile_options * options)
{
  ttl_ast_node l;

  state->module_name = module->d.module.name;
#if 0
  state->module_exports = module->d.module.exports;
#endif
  state->env = ttl_environment_make (state->pool, state->env);
  state->current_module->env = state->env;
  state->current_module->documentation = module->d.module.documentation;
  {
    ttl_ast_node l = module->d.module.params;
    ttl_ast_node nl = NULL, * nlp = &nl;
    while (l)
      {
	*nlp = ttl_make_ast_pair (state->pool, l->d.pair.car->d.variable.name,
				 NULL);
	nlp = &((*nlp)->d.pair.cdr);
	l = l->d.pair.cdr;
      }
    state->current_module->params = nl;
  }

  l = module->d.module.params;
  while (l)
    {
      if (!l->d.pair.car->d.variable.type)
	{
	  ttl_binding bind;
	  ttl_type type = ttl_make_sum_type
	    (state->pool,
	     l->d.pair.car->d.variable.name->d.identifier.symbol,
	     l->d.pair.car->d.variable.name,
	     0, NULL);
	  bind = ttl_make_type_binding
	    (state->pool,
	     l->d.pair.car->d.variable.name->d.identifier.symbol,
	     l->d.pair.car->d.variable.name->d.identifier.symbol,
	     type);
	  ttl_environment_add (state->env, bind);
	  state->type_bindings = ttl_binding_cons (state->pool, bind,
						   state->type_bindings);
	}
      l = l->d.pair.cdr;
    }

  /* Collect type declarations.  */
  pass (state, module, 0);

  l = module->d.module.imports;
  while (l)
    {
      if (ttl_compare_identifiers (ttl_strip_annotation (l->d.pair.car),
				   state->current_module->module_ast_name))
	{
	  /* 	  fprintf (stderr, "self-importing forbidden.\n"); */
	}
      else
	{
	  ttl_module mod = ttl_module_find (l->d.pair.car,
					    state->current_module->imported);
	  if (!mod)
	    {
	      mod = load_module (state, l->d.pair.car);
	      if (mod)
		{
		  state->current_module->imported =
		    ttl_module_cons (state->pool, mod,
				     state->current_module->imported);
		}
	    }
	  if (mod)
	    {
	      import_module (state, mod, state->current_module,
			     l->d.pair.car->d.annotated_identifier.annotation,
			     l->d.pair.car->d.annotated_identifier.open_list,
			     1, state->type_bindings);
	    }
	}
      l = l->d.pair.cdr;
    }

  /* Now add a binding for the currently compiled module to its
     environment, so that local definitions can be accessed as
     qualified identifiers, too.  */
  ttl_environment_add (state->env,
		       ttl_make_module_binding
		       (state->pool,
			module->d.module.name->d.identifier.symbol,
			state->current_module));

#if 0
  /* Create the environment for the module.  */
  state->env = ttl_environment_make (state->pool, state->env);

  state->current_module->env = state->env;
#endif

  /* Compile type declarations.  */
  pass (state, module, 1);

  /* Now install the parameter variables and functions.  They are
     allowed to reference types defined in this module, so we do it
     later than the type parameters.  */
  l = module->d.module.params;
  while (l)
    {
      if (l->d.pair.car->d.variable.type)
	{
	  ttl_binding bind;
	  ttl_type type = translate_type (state,
					  l->d.pair.car->d.variable.type);
	  bind = ttl_make_variable_binding
	    (state->pool,
	     l->d.pair.car->d.variable.name->d.identifier.symbol,
	     l->d.pair.car->d.variable.name->d.identifier.symbol,
	     ttl_make_variable (state->pool, variable_global, type,
				0 /* FIXME: Not constant.  */));
	  ttl_environment_add (state->env, bind);
	}
      l = l->d.pair.cdr;
    }


  /* Collect subroutine declarations and variables.  */
  pass (state, module, 2);

  /* Import the module into itself, possibly instantiated.  */
  {
    ttl_ast_node old_exports = state->current_module->exports;
    ttl_ast_node l = module->d.module.definitions;

    state->current_module->exports = NULL;
    while (l)
      {
	ttl_ast_node def = l->d.pair.car;
	ttl_ast_node exp = NULL;

	switch (def->kind)
	  {
	  case ast_function:
	    {
	      ttl_ast_node param_types = NULL, * pt = &param_types;
	      ttl_ast_node params = def->d.function.params;
	      while (params)
		{
		  *pt = ttl_make_ast_pair (state->pool,
					   params->d.pair.car->d.variable.type,
					   NULL);
		  pt = &((*pt)->d.pair.cdr);
		  params = params->d.pair.cdr;
		}
	      exp = ttl_make_ast_function (state->pool,
					   def->d.function.name, 
					   ttl_make_ast_function_type
					   (state->pool,
					    param_types,
					    def->d.function.type,
					    NULL, -1, -1, -1, -1),
					   NULL, NULL,
					   1, /* Public.  */
					   0, /* Not handcoded.  */
					   NULL, /* No docs. */
					   NULL, -1, -1, -1, -1);
	    }
	    break;
	  default:
	    break;
	  }
	if (exp)
	  {
	    state->current_module->exports = ttl_make_ast_pair
	      (state->pool, exp, state->current_module->exports);
	  }
	l = l->d.pair.cdr;
      }
    l = module->d.module.imports;
    while (l)
      {
	if (ttl_compare_identifiers (ttl_strip_annotation (l->d.pair.car),
				     state->current_module->module_ast_name))
	  {
	    ttl_module mod = state->current_module;
	    import_module (state, mod, state->current_module,
			   l->d.pair.car->d.annotated_identifier.annotation,
			   l->d.pair.car->d.annotated_identifier.open_list,
			   1, state->type_bindings);
	  }
	l = l->d.pair.cdr;
      }
    state->current_module->exports = old_exports;
  }					   

  /* Compile subroutines.  */
  pass (state, module, 3);
  
  if (options->debug_dump_env)
    {
      state->current_module->recursing = 1;
      ttl_dump_env (stdout, state->current_module->env, 0);
      state->current_module->recursing = 0;
    }

  if (options->main)
    {
      ttl_function function = state->current_module->toplevel_functions;
      char * module_name = ttl_qualident_to_c_ident (state->pool,
						     state->module_name);
      char * main_func = ttl_string_append (state->pool, module_name,
					    "_main_pF1pLpS_pI");
      ttl_symbol main_sym = ttl_symbol_enter (state->symbol_table,
					      main_func,
					      strlen (main_func));
      while (function)
	{
	  if (function->unique_name == main_sym)
	    break;
	  function = function->next;
	}
      if (!function)
	{
	  state->errors++;
	  ttl_error_print_location (stderr, module);
	  ttl_error_print_string
	    (stderr,
	     "main module requires function `fun main(list of string): int'");
	  ttl_error_print_nl (stderr);
	}
    }

  /* Now go through export list and check whether all exported
     identifiers are defined.  If not issue error messages for the
     missing definitions.  */
  l = module->d.module.exports;
  while (l)
    {
      ttl_ast_node name;
      ttl_binding_list bind_list;

      name = l->d.pair.car;

      if (name->kind == ast_identifier)
	{
	  bind_list = ttl_environment_search (state->pool, state->env,
					      name->d.identifier.symbol);
	  if (!bind_list)
	    {
	      /* FIXME: Check that there is at least one non-module
		 identifier on the binding list.  */
	      state->errors++;
	      ttl_error_print_location (stderr, name);
	      ttl_error_print_string (stderr,
				      "exported identifier not defined: ");
	      ttl_error_print_node (stderr, name);
	      ttl_error_print_nl (stderr);
	    }
	}
      else
	{
	  state->errors++;
	  fprintf (stderr, "** Re-exporting not yet implemented.\n");
	}
      l = l->d.pair.cdr;
    }

  /* Check whether any errors occured during compiling, and if not,
     write the module's exported definitons to the interface file.  */
  if (state->errors == 0)
    write_interface (state, module);
}

enum doc_state {doc_closed, doc_deffn, doc_defvar, doc_defconst, doc_deftp};

static void
finish_doc (enum doc_state dstate, char * lastdoc)
{
  switch (dstate)
    {
    case doc_deffn:
      if (lastdoc)
	printf ("%s", lastdoc);
      else
	printf ("\n");
      lastdoc = NULL;
      printf ("@end deftypefn\n\n");
      break;
    case doc_defvar:
      if (lastdoc)
	printf ("%s", lastdoc);
      else
	printf ("\n");
      lastdoc = NULL;
      printf ("@end deftypevar\n\n");
      break;
    case doc_defconst:
      if (lastdoc)
	printf ("%s", lastdoc);
      else
	printf ("\n");
      lastdoc = NULL;
      printf ("@end deftypevr\n\n");
      break;
    default:
      break;
    }
}

static void
emit_turtledoc (ttl_compile_state state,
		ttl_ast_node module,
		struct ttl_compile_options * options)
{
  enum doc_state dstate = doc_closed;
  char * lastdoc = NULL;
  ttl_ast_node l = module->d.module.definitions;

  printf ("@node ");
  ttl_ast_print (stdout, ttl_strip_annotation (module->d.module.name), 0);
  printf (" module\n@subsection ");
  ttl_ast_print (stdout, ttl_strip_annotation (module->d.module.name), 0);
  printf (" module\n@cpindex @code{");
  ttl_ast_print (stdout, ttl_strip_annotation (module->d.module.name), 0);
  printf ("} (Module)\n\n");

  if (module->d.module.documentation)
    {
      printf ("%s\n", module->d.module.documentation);
    }
  while (l)
    {
      ttl_ast_node def = l->d.pair.car;
      switch (def->kind)
	{
	case ast_vardef:
	  {
	    ttl_ast_node vars = def->d.vardef.list;

	    if (!def->d.vardef.documentation ||
		def->d.vardef.documentation[0] != '-')
	      {
		if (dstate == doc_defvar && def->d.vardef.documentation &&
		    def->d.vardef.documentation[0] == '"')
		  {
		    printf ("@deftypevarx {} ");
		  }
		else
		  {
		    finish_doc (dstate, lastdoc);
		    lastdoc = NULL;
		    dstate = doc_defvar;
		    printf ("@deftypevar {} ");
		  }
		while (vars)
		  {
		    ttl_ast_node var = vars->d.pair.car;
			
		    ttl_ast_print (stdout, var->d.variable.name, 0);
		    printf (" : ");
		    ttl_ast_print (stdout, var->d.variable.type, 0);
		    printf ("\n");
		    if (def->d.vardef.documentation &&
			def->d.vardef.documentation[0] != '"')
		      lastdoc = def->d.vardef.documentation;
		    vars = vars->d.pair.cdr;
		    if (vars)
		      printf ("@deftypevarx {} ");
		  }
	      }
	  }
	  break;
	case ast_constdef:
	  {
	    ttl_ast_node vars = def->d.constdef.list;

	    if (!def->d.constdef.documentation ||
		def->d.constdef.documentation[0] != '-')
	      {
		if (dstate == doc_defconst && def->d.constdef.documentation &&
		    def->d.constdef.documentation[0] == '"')
		  {
		    printf ("@deftypevrx Constant {} ");
		  }
		else
		  {
		    finish_doc (dstate, lastdoc);
		    lastdoc = NULL;
		    dstate = doc_defconst;
		    printf ("@deftypevr Constant {} ");
		  }

		while (vars)
		  {
		    ttl_ast_node var = vars->d.pair.car;

		    ttl_ast_print (stdout, var->d.variable.name, 0);
		    printf (" : ");
		    ttl_ast_print (stdout, var->d.variable.type, 0);
		    printf ("\n");
		    if (def->d.constdef.documentation &&
			def->d.constdef.documentation[0] != '"')
		      lastdoc = def->d.constdef.documentation;
		    vars = vars->d.pair.cdr;
		    if (vars)
		      printf ("@deftypevrx Constant {} ");
		  }
	      }
	  }
	  break;
	case ast_typedef:
	  {
	    if (!def->d.typedf.documentation ||
		def->d.typedf.documentation[0] != '-')
	      {
		finish_doc (dstate, lastdoc);
		lastdoc = NULL;
		dstate = doc_closed;
		printf ("@deftp {Data type} ");
		ttl_ast_print (stdout, def->d.typedf.name, 0);
		printf ("\nDefined as:\n\n@example\n");
		ttl_ast_print (stdout, def, 0);
		printf ("\n@end example\n");
		if (def->d.typedf.documentation)
		  printf ("\n%s", def->d.typedf.documentation);
		printf ("@end deftp\n\n");
	      }
	  }
	  break;
	case ast_function:
	  {
	    ttl_ast_node v;

	    if (!def->d.function.documentation ||
		def->d.function.documentation[0] != '-')
	      {
		if (dstate == doc_deffn && def->d.function.documentation &&
		    def->d.function.documentation[0] == '"')
		  {
		    printf ("@deftypefnx {Function} {} ");
		  }
		else
		  {
		    finish_doc (dstate, lastdoc);
		    lastdoc = NULL;
		    dstate = doc_deffn;
		    printf ("@deftypefn {Function} {} ");
		  }
		ttl_ast_print (stdout, def->d.function.name, 0);
		printf (" (");
		v = def->d.function.params;
		while (v)
		  {
		    printf ("@var{");
		    ttl_ast_print (stdout, v->d.pair.car->d.variable.name, 0);
		    printf ("}: ");
		    ttl_ast_print (stdout, v->d.pair.car->d.variable.type, 0);
		    v = v->d.pair.cdr;;
		    if (v)
		      printf (", ");
		  }
		printf (")");
		if (def->d.function.type)
		  {
		    printf (": ");
		    ttl_ast_print (stdout, def->d.function.type, 0);
		  }
		printf ("\n");
		if (def->d.function.documentation &&
		    def->d.function.documentation[0] != '"')
		  lastdoc = def->d.function.documentation;
	      }
	  }
	  break;
	  /* 	case ast_constraint: */
	  /* 	  if (pass == 2) */
	  /* 	    declare_constraintdef (state, l->d.pair.car); */
	  /* 	  else if (pass == 3) */
	  /* 	    define_constraintdef (state, l->d.pair.car); */
	  /* 	  break; */
	case ast_datatype:
	  {
	    if (!def->d.datatype.documentation ||
		def->d.datatype.documentation[0] != '-')
	      {
		finish_doc (dstate, lastdoc);
		lastdoc = NULL;
		dstate = doc_closed;
		printf ("@deftp {Data type} ");
		ttl_ast_print (stdout, def->d.datatype.name, 0);
		printf ("\nDefined as:\n\n@example\n");
		ttl_ast_print (stdout, def, 0);
		printf ("\n@end example\n");
		if (def->d.datatype.documentation)
		  printf ("\n%s", def->d.datatype.documentation);
		printf ("@end deftp\n\n");
	      }
	  }
	  break;
	default:
	  fprintf (stderr, "invalid AST node in emit_turtledoc\n");
	  abort ();
	  break;
	}
      l = l->d.pair.cdr;
    }
  finish_doc (dstate, lastdoc);
}

static ttl_module_list
print_dependent_modules (FILE * f, ttl_compile_state state, ttl_module module,
			 ttl_module_list done, char * ext)
{
  ttl_module_list list = module->imported;

  while (list)
    {
      if (!list->module->recursing &&
	  !ttl_module_find (list->module->module_ast_name, done))
	{
	  ttl_module mod = list->module;
	  mod->recursing = 1;
	  done = print_dependent_modules (f, state, mod,
					  ttl_module_cons (state->pool, mod,
							   done), ext);
	  {
	    char * s;

	    s = ttl_qualident_to_filename
	      (state->pool, ttl_strip_annotation (mod->module_ast_name), ext);
	    fprintf (f, "\\\n %s", s);
	  }
	  list->module->recursing = 0;
	}
      list = list->next;
    }
  return done;
}

static void
print_deps (FILE * f, ttl_compile_state state, ttl_module module)
{
  char * s, * p;

  s = ttl_qualident_to_filename
    (state->pool, ttl_strip_annotation (module->module_ast_name), ".o");
  p = ttl_qualident_to_filename
    (state->pool, ttl_strip_annotation (module->module_ast_name), ".ifc");
  fprintf (f, "%s: %s", s, p);
  print_dependent_modules (f, state, module, NULL, ".ifc");
  fprintf (f, "\n");
}

static void
print_dependencies (ttl_compile_state state, ttl_module module,
		    int to_stdout)
{
  char * s;
  FILE * f;

  if (!to_stdout)
    {
      s = ttl_qualident_to_filename
	(state->pool,
	 ttl_strip_annotation (module->module_ast_name), ".P");
      f = fopen (s, "w");
      if (!f)
	{
	  fprintf (stderr, "%s: cannot open dependency file\n", s);
	  return;
	}
    }
  else
    f = stdout;
  print_deps (f, state, module);
  if (!to_stdout)
    fclose (f);
}


/* Allocate a new compilation state object, and initialize it with the
   data supplied by the parameters.  */
static ttl_compile_state
make_compile_state (ttl_pool pool, char * filename,
		    struct ttl_compile_options * options)
{
  ttl_compile_state state;

  state = ttl_malloc (pool, sizeof (struct ttl_compile_state));
  state->errors = 0;
  state->filename = ttl_malloc (pool, strlen (filename) + 1);
  strcpy (state->filename, filename);
  state->pool = pool;
  state->symbol_table  = ttl_make_symbol_table (state->pool);
  state->builtin_env = make_default_environment (state, state->symbol_table);
  state->env = state->builtin_env;
  state->module_name = NULL;
  state->module_exports = NULL;

  state->current_module = ttl_make_module (state->pool, state->env);
  state->current_function = NULL;
  state->current_function_ast = NULL;
  state->tail_position = 0;

  state->all_modules = ttl_module_cons (state->pool,
					state->current_module, NULL);

  state->type_bindings = NULL;

  /* Here we append the standard search directory for turtle library
     modules, so that this directory must not be given when importing
     standard modules.  */
  state->module_path = ttl_string_append (state->pool,
					  options->module_path,
					  ":" MODULE_DIR);
  state->include_path = ttl_string_append (state->pool,
					   options->include_path,
					   ":" INCLUDE_DIR);

  state->pragma_handcoded = options->pragma_handcoded;

  state->nesting_level = 0;
  state->has_init_stmts = 0;
  state->init_stmts = NULL;

  state->mapping = NULL;
  state->label_count = 0;
  state->next_label = 0;

  state->complain_unbound_types = 1;
 
  state->compile_options = options;
  return state;
}


static int
prepare_dependencies (ttl_compile_state state, ttl_ast_node module,
		      struct ttl_compile_options * options)
{
  ttl_ast_node module_name = ttl_strip_annotation (module->d.module.name);
  ttl_ast_node imports = module->d.module.imports;

  while (imports)
    {
      int recompile = 0;
      char * src_name;
      char * ifc_name;
      ttl_ast_node imp_mod = 
	imports->d.pair.car->d.annotated_identifier.identifier;

      if (!ttl_compare_identifiers (module_name, imp_mod))
	{
	  src_name = ttl_qualident_to_filename (state->pool, imp_mod, ".t");
	  ifc_name = ttl_qualident_to_filename (state->pool, imp_mod, ".ifc");
      
	  src_name = ttl_find_file (state->pool, src_name, state->module_path);
	  ifc_name = ttl_find_file (state->pool, ifc_name, state->module_path);

	  if (ifc_name && src_name)
	    {
	      struct stat stat1;
	      struct stat stat2;

	      stat (src_name, &stat1);
	      stat (ifc_name, &stat2);
	      if (stat1.st_mtime > stat2.st_mtime)
		{
		  fprintf (stderr,
			   "%s: warning: source file needs re-compilation\n",
			   src_name);
		  recompile = 1;
		}
	    }
	  else if (src_name)
	    {
	      fprintf
		(stderr,
		 "%s: warning: source file needs compilation\n", src_name);
	      recompile = 1;
	    }
	  /* else... if there is an interface file, the compiler will later
	     find any errors, and if not, the same.  That's why we don't do
	     anything about these cases here.  */
	  if (recompile)
	    {
	      struct ttl_compile_options opts;

	      opts = *options;
	      opts.main = 0;

	      if (ttl_compile (src_name, &opts))
		return 1;
	    }
	}
      imports = imports->d.pair.cdr;
    }
  return 0;
}

/* Parse the Turtle module from the file `filename', and return the
   abstract syntax tree for it.  If the file cannot be opened or a
   severe parsing error occurs, NULL is returned instead.  */
static ttl_ast_node
read_module (ttl_compile_state state, char * filename)
{
  FILE * f;
  ttl_scanner scanner;
  ttl_ast_node module;

  f = fopen (filename, "r");
  if (!f)
    {
      fprintf (stderr, "turtle: cannot open input file: %s\n", filename);
      return NULL;
    }
  if (state->compile_options->verbose > 0)
    printf ("parsing...\n");
  scanner = ttl_make_scanner (state->pool, state->symbol_table, f, filename);
  module = ttl_parse_module (state->pool, scanner);
  fclose (f);
  state->errors += scanner->scan_errors + scanner->parse_errors;
  return module;
}


/* Compile the Turtle file called `filename', reporting any errors to
   stderr.  The result is an interface file with the same basename as
   `filename', but the extension `.ifc', a C header file containing
   exported declaration with the extension `.h', a C source file
   containing the code with extension `.c', the result of the C
   compiler in a file with extension `.o', and, if a main module was
   compiled, the executable with the name given with the command line
   option `--main=NAME'.  

   Return value: 0 on success, > 0 if any compilation errors
   occured. */
int
ttl_compile (char * filename, struct ttl_compile_options * options)
{
  ttl_pool pool;
  ttl_compile_state state;
  ttl_ast_node module;
  int exit_code = 0;

  /* Set up global data structures.  */
  pool = ttl_create_pool ();
  state = make_compile_state (pool, filename, options);

  /* Now read in the program text.  If the input file cannot be
     opened, or not parsed at all, we do not invoke further
     passes.  */
  module = read_module (state, filename);
  if (module)
    {
      /* Dump AST module representation, if requested.  */
      if (options->debug_dump_ast)
	ttl_ast_print (stdout, module, 0);

      state->current_module->module_ast_name = module->d.module.name;
      state->current_module->params = module->d.module.params;
      state->current_module->imports = module->d.module.imports;
      state->current_module->exports = module->d.module.exports;

      if (options->pragma_turtledoc)
	{
	  if (state->errors == 0)
	    {
	      if (options->verbose > 0)
		printf ("generating Texinfo documentation...\n");
	      emit_turtledoc (state, module, options);
	    }
	  else
	    exit_code = 1;
	}
      else
	{
	  if (options->verbose > 0)
	    printf ("determining dependencies...\n");

	  if (prepare_dependencies (state, module, options))
	    {
	      exit_code = 1;
	      goto free_and_exit;
	    }

	  if (options->verbose > 0)
	    printf ("translating to high-level intermediate code...\n");
	  translate_module (state, module, options);

	  create_init_function (state);

#if 0
	  dump_il_module (stderr, state->current_module);
#endif

	  /* Dump HIL module representation, if requested.  */
	  if (options->debug_dump_il_file)
	    ttl_dump_il_module (stdout, state, state->current_module);

	  if (options->pragma_printdeps || options->pragma_printdepsstdout)
	    {
	      print_dependencies (state, state->current_module,
				  options->pragma_printdepsstdout);
	    }

	  /* Do not generate C code or try to compile it to object code if
	     any errors have been detected.  */
	  if (state->errors == 0)
	    {
	      if (options->verbose > 0)
		printf ("translating to low-level intermediate code...\n");
	      ttl_generate_code (state, state->current_module);
	      if (options->verbose > 0)
		printf ("generating C code...\n");
	      exit_code = ttl_emit_c (state, state->current_module, options);
	      if (options->verbose > 0)
		printf ("done.\n");
	    }
	  else
	    exit_code = 1;
	}
    }
  else
    exit_code = 1;
 free_and_exit:
  ttl_destroy_pool (state->pool);
  /* NOTE: Do not use STATE after here, since its pool has just been
     deallocated.  */
  return exit_code;
}


void
ttl_init_compile_options (struct ttl_compile_options * options)
{
  options->debug_dump_env = 0;
  options->debug_dump_ast = 0;
  options->debug_dump_il_file = 0;
  options->debug_dump_bytecode = 0;
  options->pragma_handcoded = 0;
  options->pragma_foreign = 0;
  options->pragma_compile_only = 0;
  options->pragma_turtledoc = 0;
  options->pragma_printdeps = 0;
  options->pragma_printdepsstdout = 0;
  options->main = 0;
  options->verbose = 0;
  options->opt_local_calls = 1;
  options->opt_local_jumps = 1;
  options->opt_merge_gc_checks = 1;
  options->opt_inline_constructors = 1;
  options->opt_gcc_level = 0;
  options->link_static = 0;
  options->program_name = "a.out";
  options->module_path = "";
  options->include_path = "";
}

/* End of compiler.c.  */
