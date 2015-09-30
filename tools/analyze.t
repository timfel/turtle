// analyze.t -- Turtle source code analyzer.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//  
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this package; see the file COPYING.  If not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.

// Commentary:
//
//* This is an analyzer for Turtle modules.

module analyze;

import io, lists<string>, scanner, parser, ast, env,
  lists<ast.ast>, listmap<ast.ast, list of ast.ast>, strings,
  listmap<ast.ast, ast.ast>, types;


//* Internal representation of a Turtle module.
//* 
//* @table @code
//* @item name
//* This is the module's name, which is always the modules name
//* without any annotations.
//* 
//* @item filename
//* This is the name of the file from which the modules definition
//* was read.
//* 
//* @item dependencies
//* List of modules on which this module directly depends.
//* 
//* @item instantiations
//* Imported modules, including the module parameter given in the
//* @code{import} statement.
//* 
//* @item body
//* The abstract syntax of the module.
//* @end table
//
datatype mod = mod (name: ast.ast,
                    filename: string,
                    dependencies: list of mod,
		    instantiations: list of ast.ast,
		    body: ast.ast,
		    env: env.env);

//* List of all modules appearing (transitively) in the module to
//* be analyzed.
//
var modules: list of mod := [];

//* List of all directories in which to look for module source
//* files.
//
var module_path: list of string := [".", "../crawl"];

//* Find the source file for the module named @var{mod_name} and
//* return a pair consisting of the abstract syntax of the module
//* and the file name from which the abstract syntax was read.  The
//* pair @code{(null, null)} is returned when the module source
//* file could not be found.
//
fun find_module (mod_name: ast.ast): (ast.ast, string)
  var filename: string;
  var l: list of string := module_path;

/*   io.put ("Searching for module "); */
/*   ast.put (io.output, mod_name, 0); */

  while l <> null do
    filename := hd l + "/" + ast.qualident_to_string (mod_name, "/") + ".t";
    var f: io.file := io.open (filename);
    if f <> null then
/*       io.put (" -> "); */
/*       io.put (filename); */
/*       io.nl (); */
      var s: scanner.state := scanner.state (filename, f);
      var mod: ast.ast := parser.parse (s);
      io.close (f);
      return mod, filename;
    end;
    l := tl l;
  end;
/*   io.nl (); */
  io.put (io.error, "analyze: cannot find source code for module: ");
  ast.put (io.error, mod_name, 0);
  io.nl (io.error);
  return null, null;
end;

public fun main (args: list of string): int
  var f: io.file;
  if lists.length (args) >= 2 then
    args := tl args;
    while args <> null do
      f := io.open (hd args);
      if f = null then
	io.put (io.error, "analyze: cannot open input file: ");
	io.put (io.error, hd args);
	io.nl (io.error);
      else
	var s: scanner.state;
	s := scanner.state (hd args, f);
	var mod: ast.ast := parser.parse (s);
	io.close (f);
	analyze (mod, hd args);
      end;
      args := tl args;
    end;
  else
    io.put (io.error, "analyze: No input files\n");
  end;
  return 0;
end;

fun analyze (m: ast.ast, filename: string)
  var the_mod: mod;
  io.put ("determining dependencies\n");
  the_mod :=  mod (ast.strip_annotation (ast.name (m)), filename, [], [], m,
                   null);
  modules := the_mod :: [];
  load_dependent_modules (the_mod);
  dump_dependencies (the_mod, 0);
  io.nl ();
  dump_instantiations (the_mod);
  io.nl ();

  var l: list of mod := modules;
  io.put ("Linking in: ");
  while l <> null do
    ast.put (io.output, name (hd l), 0);
    io.put (' ');
    l := tl l;
  end;
  io.nl ();

  var e: env.env;
  l := modules;
  while l <> null do
    if not ast.eq_identifiers (name (hd l), name (the_mod)) then
      e := env.env ([], env.builtin_env ());
      define_toplevel_bindings (hd l, e, false);
      env! (hd l, e);
      ast.put (io.output, name (hd l), 0);
      io.put ("=>\n");
//      env.dump_env (e);
    end;
    l := tl l;
  end;
  
  var env: env.env := env.env ([], env.builtin_env ());
  define_toplevel_bindings (the_mod, env, true);
  env! (the_mod, env);
  env.dump_env (env);
  check_toplevel_bindings (the_mod, env);

  test_unify ();
end;

//* Go through the import list of module @var{mod} and read in the
//* definitions for all mentioned modules.  All transitively
//* reachable modules are read and stored in the global variable
//* @var{modules}.  The instantiations (module names with module
//* parameters) are stored in the module @var{mod} as well as the
//* list of directly imported modules.
//
fun load_dependent_modules (mod: mod)
  var imp: list of ast.ast := ast.imp (body (mod));
  var la: list of ast.ast;
  var lm: list of mod;
  var found: bool;

  while imp <> null do
    lm := dependencies (mod);
    found := false;
    while lm <> null and not found do
      if ast.eq_identifiers (name (hd lm), hd imp) then
	found := true;
      end;
      lm := tl lm;
    end;
    if not found then
      lm := modules;
      found := false;
      while lm <> null and not found do
	if ast.eq_identifiers (name (hd lm), hd imp) then
	  found := true;
	end;
	if not found then
	  lm := tl lm;
	end;
      end;
      if found then
	dependencies! (mod, hd lm :: dependencies (mod));
      else
	var the_module: mod;
	var mimp: ast.ast, fname: string;
	mimp , fname := find_module (hd imp);
	if mimp <> null then
	  the_module := mod (ast.strip_annotation (hd imp), 
                             fname, [], [], mimp, null);
	  modules := the_module :: modules;
	  dependencies! (mod, the_module :: dependencies (mod));
	  load_dependent_modules (the_module);
	end;
      end;
    end;
    instantiations! (mod, hd imp :: instantiations (mod));
    imp := tl imp;
  end;
end;

fun dump_dependencies (m: mod, indent: int)
  var l: list of mod := dependencies (m);
  io.put (strings.replicate (' ', indent));
  ast.put (io.output, name (m), 0);
  io.put (" (");
  io.put (filename (m));
  io.put (")");
  io.nl ();
  while l <> null do
    io.put (strings.replicate (' ', indent + 2));
    ast.put (io.output, name (hd l), 0);
    io.put (" (");
    io.put (filename (hd l));
    io.put (")");
    io.nl ();
    l := tl l;
  end;    
end;

fun dump_instantiations (m: mod)
  var l: list of ast.ast := instantiations (m);
  io.put ("Instantiations:");
  while l <> null do
    io.put (' ');
    ast.put (io.output, hd l, 0);
    l := tl l;
  end;
  io.nl ();
end;


//* Take the definition @var{def} and enter it into the environment
//* @var{env}.  Emit an error message if a definition of the same
//* name and a conflicting kind (variable, constant, function,
//* constraint) is already defined in @var{env}.
//
fun define_binding (mod: mod, env: env.env, def: ast.ast, private?: bool)
  var l: list of ast.ast;
  var definitions: list of env.definition;
  var def: env.definition;

  if ast.vardef? (def) then
    if private? or ast.pub (def) then
    l := ast.vars (def);
    while l <> null do
      def := env.vardef (ast.typ (hd l));
      definitions := env.lookup (ast.name (hd l), env, false);
      if env.contains (def, definitions) then
	ast.put (io.error, ast.p (hd l));
	io.put (io.error, "duplicate identifier: ");
	ast.put (io.error, ast.name (hd l), 0);
	io.nl (io.error);
      else
	env.bind (ast.name (hd l), def, env);
      end;
      l := tl l;
    end;
    end;
  elsif ast.constdef? (def) then
    if private? or ast.pub (def) then
    l := ast.vars (def);
    while l <> null do
      def := env.constdef (ast.typ (hd l));
      definitions := env.lookup (ast.name (hd l), env, false);
      if env.contains (def, definitions) then
	ast.put (io.error, ast.p (hd l));
	io.put (io.error, "duplicate identifier: ");
	ast.put (io.error, ast.name (hd l), 0);
	io.nl (io.error);
      else
	env.bind (ast.name (hd l), def, env);
      end;
      l := tl l;
    end;
    end;
  elsif ast.fundef? (def) then
    if private? or ast.pub (def) then
    def := env.fundef (ast.function_type (ast.p (def), 
	                              listmap.map (fun (v: ast.ast): ast.ast
                                                     return ast.typ (v);
					           end,
						   ast.params (def)),
				      ast.ret_type (def)));
    definitions := env.lookup (ast.name (def), env, false);
    if env.contains (def, definitions) then
      ast.put (io.error, ast.p (def));
      io.put (io.error, "duplicate identifier: ");
      ast.put (io.error, ast.name (def), 0);
      io.nl (io.error);
    else
      env.bind (ast.name (def), def, env);
    end;
    end;
  elsif ast.typedef? (def) then
    if private? or ast.pub (def) then
    def := env.typedef (ast.typ (def));
    definitions := env.lookup (ast.name (def), env, false);
    if env.contains (def, definitions) then
      ast.put (io.error, ast.p (def));
      io.put (io.error, "duplicate identifier: ");
      ast.put (io.error, ast.name (def), 0);
      io.nl (io.error);
    else
      env.bind (ast.name (def), def, env);
    end;
    end;
  elsif ast.datatypedef? (def) then
    if private? or ast.pub (def) then
      def := env.datatypedef (ast.qualident (ast.p (def), name (mod), 
                              ast.strip_annotation (ast.name (def))));
      definitions := env.lookup (ast.strip_annotation (ast.name (def)), env,
                                 false);
      if env.contains (def, definitions) then
	ast.put (io.error, ast.p (def));
	io.put (io.error, "duplicate identifier: ");
	ast.put (io.error, ast.strip_annotation (ast.name (def)), 0);
	io.nl (io.error);
      else
	env.bind (ast.strip_annotation (ast.name (def)), def, env);
	var datype: ast.ast := ast.named_type (ast.p (def),
	                     ast.qualident (ast.p (def), name (mod), 
	                       ast.strip_annotation (ast.name (def))),
                             null);

	var variants: list of ast.ast := ast.variants (def);
	while variants <> null do
	  var variant: ast.ast := hd variants;
	  variants := tl variants;

	  var d: env.definition := env.fundef 
	    (ast.function_type (ast.p (def), 
	     listmap.map (fun (a: ast.ast): ast.ast
	                  return qualify (mod, env, ast.typ (a));
                        end, ast.fields (variant)), datype));
	  env.bind (ast.name (variant), d, env);

	  d := env.fundef 
	   (ast.function_type (ast.p (def), [datype], env.bool_type));

	  env.bind (ast.identifier (ast.p (def),
	            ast.value (ast.name (variant)) + "?"), d, env);

	  var fields: list of ast.ast := ast.fields (variant);
	  while fields <> null do
	    var field: ast.ast := hd fields;
	    fields := tl fields;

	    d := env.fundef 
	    (ast.function_type (ast.p (def), [datype], qualify (mod, env, ast.typ (field))));
	    env.bind (ast.name (field), d, env);
	    d := env.fundef 
	    (ast.function_type (ast.p (def), [datype, qualify (mod, env, ast.typ (field))], ast.void_type (ast.p (def))));
	    env.bind (ast.identifier (ast.p (def),
	              ast.value (ast.name (field)) + "!"), d, env);

	  end;
	end;
      end;
    end;
  end;
end;


fun qualify (mod: mod, env: env.env, typ: ast.ast): ast.ast
  if ast.named_type? (typ) and ast.identifier? (ast.name (typ)) then
    var defs: list of env.definition := env.lookup (ast.name (typ), env, true);
//    ast.put (io.output, ast.name (typ), 0);
//    io.put (" ********************\n");
    while defs <> null do
      if env.datatypedef? (hd defs) then
//	ast.put (io.output, env.name (hd defs), 0);
//    io.put (" ====================\n");
	return ast.named_type (ast.p (typ), env.name (hd defs), 
	                       ast.params (typ));
      elsif env.typedef? (hd defs) then
        return ast.named_type (ast.p (typ),
                       ast.qualident (ast.p (typ), name (mod), ast.name (typ)),
		       ast.params (typ));
//	return env.typ (hd defs);
      end;
      defs := tl defs;
    end;
    io.put ("undefined type: ");
    ast.put (io.output, typ, 0);
    io.nl ();
    return ast.named_type (ast.p (typ),
                       ast.qualident (ast.p (typ), name (mod), ast.name (typ)),
		       ast.params (typ));
  elsif ast.list_type? (typ) then
    return ast.list_type (ast.p (typ), qualify (mod, env, ast.base (typ)));
  elsif ast.array_type? (typ) then
    return ast.array_type (ast.p (typ), qualify (mod, env, ast.base (typ)));
  elsif ast.constraint_type? (typ) then
    return ast.constraint_type (ast.p (typ), 
      qualify (mod, env, ast.base (typ)));
  elsif ast.variable_type? (typ) then
    return ast.variable_type (ast.p (typ), 
      qualify (mod, env, ast.base (typ)), null);
  elsif ast.function_type? (typ) then
    return ast.function_type (ast.p (typ),
      listmap.map (fun (t: ast.ast): ast.ast
                     return qualify (mod, env, t);
		   end, ast.params (typ)),
      qualify (mod, env, ast.ret (typ)));
  elsif ast.tuple_type? (typ) then
    return ast.tuple_type (ast.p (typ),
      listmap.map (fun (t: ast.ast): ast.ast
                     return qualify (mod, env, t);
		   end, ast.types (typ)));
  else
    return typ;
  end;
end;

fun qualify_types (mod: mod, env: env.env, def: ast.ast, private?: bool)
  var l: list of ast.ast;
  var definitions: list of env.definition;
  if ast.vardef? (def) then
    if private? or ast.pub (def) then
    l := ast.vars (def);
    while l <> null do
      definitions := env.lookup (ast.name (hd l), env, false);
      while definitions <> null do
	if env.vardef? (hd definitions) then
	  env.typ! (hd definitions, qualify (mod, env, env.typ (hd definitions)));
	end;
	definitions := tl definitions;
      end;
      l := tl l;
    end;
    end;
  elsif ast.constdef? (def) then
    if private? or ast.pub (def) then
    l := ast.vars (def);
    while l <> null do
      definitions := env.lookup (ast.name (hd l), env, false);
      while definitions <> null do
	if env.constdef? (hd definitions) then
	  env.typ! (hd definitions, qualify (mod, env, env.typ (hd definitions)));
	end;
	definitions := tl definitions;
      end;
      l := tl l;
    end;
    end;
  elsif ast.fundef? (def) then
    if private? or ast.pub (def) then
    definitions := env.lookup (ast.name (def), env, false);
    while definitions <> null do
      if env.fundef? (hd definitions) then
	env.typ! (hd definitions, qualify (mod, env, env.typ (hd definitions)));
      end;
      definitions := tl definitions;
    end;
  end;
  elsif ast.typedef? (def) then
    if private? or ast.pub (def) then
    definitions := env.lookup (ast.name (def), env, false);
    while definitions <> null do
      if env.typedef? (hd definitions) then
	env.typ! (hd definitions, qualify (mod, env, env.typ (hd definitions)));
      end;
      definitions := tl definitions;
    end;
    end;
  elsif ast.datatypedef? (def) then
    if private? or ast.pub (def) then
    end;
  end;
end;

//* Go through the top-level definitions of module @var{mod} and
//* enter them into the environment @var{env}.
//
fun define_toplevel_bindings (mod: mod, env: env.env, private?: bool)
  var defs: list of ast.ast := ast.defs (body (mod));
  while defs <> null do
    define_binding (mod, env, hd defs, private?);
    defs := tl defs;
  end;
  if ast.annident? (ast.name (body (mod))) then
    defs := ast.ann (ast.name (body (mod)));
    while defs <> null do
      define_binding (mod, env, ast.datatypedef (ast.pos ("", 0, 0),
        false, hd defs, null, null, ""), true);
      defs := tl defs;
    end;
  end;
  defs := ast.defs (body (mod));
  while defs <> null do
    qualify_types (mod, env, hd defs, private?);
    defs := tl defs;
  end;
end;

fun lookup_qualident (mod: mod, name: ast.ast, env: env.env): 
  list of env.definition
  var mod_name: ast.ast := ast.mod (name);
  var ident: ast.ast := ast.ident (name);
  var mods: list of mod := dependencies (mod);
  while mods <> null do
//    ast.put (io.output, name (hd mods), 0); io.nl ();
//    ast.put (io.output, mod_name, 0); io.nl ();
    if ast.eq_identifiers (name (hd mods), mod_name) then
      return env.lookup (ident, env (hd mods), true);
    end;
    mods := tl mods;
  end;
  return null;
end;

fun check_expression (mod: mod, env: env.env, expr: ast.ast)
  var defs: list of env.definition;
  if ast.binary? (expr) then
    check_expression (mod, env, ast.op0 (expr));
    check_expression (mod, env, ast.op1 (expr));
  elsif ast.unary? (expr) then
    check_expression (mod, env, ast.op0 (expr));
  elsif ast.qualident? (expr) then
    defs := lookup_qualident (mod, expr, env);
    if defs = null then
      ast.put (io.error, ast.p (expr));
      io.put (io.error, "undefined identifier: ");
      ast.put (io.error, expr, 0);
      io.nl (io.error);
    end;
  elsif ast.identifier? (expr) then
    defs := env.lookup (expr, env, true);
    if defs = null then
      ast.put (io.error, ast.p (expr));
      io.put (io.error, "undefined identifier: ");
      ast.put (io.error, expr, 0);
      io.nl (io.error);
    end;
  elsif ast.array_constructor? (expr) then
    check_expression (mod, env, ast.size (expr));
    check_expression (mod, env, ast.init (expr));
  elsif ast.list_constructor? (expr) then
    check_expression (mod, env, ast.size (expr));
    check_expression (mod, env, ast.init (expr));
  elsif ast.string_constructor? (expr) then
    check_expression (mod, env, ast.size (expr));
    check_expression (mod, env, ast.init (expr));
  elsif ast.list_expr? (expr) then
    lists.foreach (fun (a: ast.ast) check_expression (mod, env, a); end,
                   ast.elems (expr));
  elsif ast.array_expr? (expr) then
    lists.foreach (fun (a: ast.ast) check_expression (mod, env, a); end,
                   ast.elems (expr));
  elsif ast.tuple_expr? (expr) then
    lists.foreach (fun (a: ast.ast) check_expression (mod, env, a); end,
                   ast.elems (expr));
  elsif ast.fun_expr? (expr) then
    var fparam_env: env.env := env.env ([], env);
    lists.foreach (fun (p: ast.ast)
      env.bind (ast.name (p), env.vardef (ast.typ (p)), fparam_env);
    end, ast.param (expr));
    var fbody_env: env.env := env.env ([], fparam_env);

    lists.foreach (fun (a: ast.ast) check_stmt (mod, fbody_env, a); end,
                   ast.body (expr));
  elsif ast.constraint_expr? (expr) then
    var cparam_env: env.env := env.env ([], env);
    lists.foreach (fun (p: ast.ast)
      env.bind (ast.name (p), env.vardef (ast.typ (p)), cparam_env);
    end, ast.param (expr));
    var cbody_env: env.env := env.env ([], cparam_env);

    lists.foreach (fun (a: ast.ast) check_stmt (mod, cbody_env, a); end,
                   ast.body (expr));
  elsif ast.call? (expr) then
    check_expression (mod, env, ast.func (expr));
    lists.foreach (fun (a: ast.ast) check_expression (mod, env, a); end,
                   ast.params (expr));
  elsif ast.index? (expr) then
    check_expression (mod, env, ast.arr (expr));
    check_expression (mod, env, ast.index (expr));
  end;
end;

fun check_stmt (mod: mod, env: env.env, stmt: ast.ast)
  if ast.vardef? (stmt) or ast.constdef? (stmt) or
    ast.fundef? (stmt) or ast.constraintdef? (stmt) then
    define_binding (mod, env, stmt, true);
    if ast.fundef? (stmt) or ast.constraintdef? (stmt) then
      check_fundef (mod, env, stmt);
    end;
  elsif ast.binary? (stmt) then
    check_expression (mod, env, stmt);
  elsif ast.call? (stmt) then
    check_expression (mod, env, stmt);
  elsif ast.ifstmt? (stmt) then
    var g: list of ast.ast := ast.guards (stmt);
    while g <> null do
      check_expression (mod, env, ast.cond (hd g));
      lists.foreach (fun (a: ast.ast) check_stmt (mod, env, a); end,
                     ast.stmts (hd g));
      g := tl g;
    end;
    lists.foreach (fun (a: ast.ast) check_stmt (mod, env, a); end,
                   ast.els (stmt));
  elsif ast.whilestmt? (stmt) then
    check_expression (mod, env, ast.cond (stmt));
    lists.foreach (fun (a: ast.ast) check_stmt (mod, env, a); end,
                   ast.stmts (stmt));
  elsif ast.returnstmt? (stmt) then
    if ast.expr (stmt) <> null then
      check_expression (mod, env, ast.expr (stmt));
    end;
  elsif ast.requirestmt? (stmt) then
    var cl: list of ast.ast := ast.conj (stmt);
    var cs: list of ast.ast := ast.strengths (stmt);
    lists.foreach (fun (a: ast.ast) check_expression (mod, env, a); end, cl);
    lists.foreach (fun (a: ast.ast) check_expression (mod, env, a); end, cs);
    var a: ast.ast := ast.body (stmt);
    if a <> null then
      check_stmt (mod, env, a);
    end;
  elsif ast.instmt? (stmt) then
    lists.foreach (fun (a: ast.ast) check_stmt (mod, env, a); end,
                   ast.body (stmt));
  end;
end;

fun check_fundef (mod: mod, env: env.env, def: ast.ast)
  var param_env: env.env := env.env ([], env);

  lists.foreach (fun (p: ast.ast)
    env.bind (ast.name (p), env.vardef (ast.typ (p)), param_env);
  end, ast.params (def));

/*   env.dump_env (param_env); */

  var body_env: env.env := env.env ([], param_env);

  lists.foreach (fun (stmt: ast.ast)
    check_stmt (mod, body_env, stmt);
  end, ast.body (def));

/*   env.dump_env (body_env); */
end;


//* Go through the top-level definitions of module @var{env} and
//* type-check all functions.  @var{env} is the top-level
//* environment of the module which must have been populated by a
//* call to @code{define_toplevel_bindings()}.
//
fun check_toplevel_bindings (mod: mod, env: env.env)
  var defs: list of ast.ast := ast.defs (body (mod));
  while defs <> null do
    if ast.fundef? (hd defs) then
      check_fundef (mod, env, hd defs);
    end;
    defs := tl defs;
  end;
end;

fun test_unify ()
  var t0: ast.ast;
  var t1: ast.ast;
  var p: ast.pos := ast.pos ("", 0, 0);

  t0 := ast.function_type (p, [ast.function_type (p, [ast.variable_type (p, ast.identifier (p, "A"), null)], ast.variable_type (p, ast.identifier (p, "B"), null)),
  ast.list_type (p, ast.variable_type (p, ast.identifier (p, "A"), null))], ast.list_type (p, ast.variable_type (p, ast.identifier (p, "B"), null)));

  t1 := ast.function_type (p, [ast.function_type (p, [ast.named_type (p, ast.identifier (p, "int"), null)], ast.named_type (p, ast.identifier (p, "real"), null)),
  ast.list_type (p, ast.named_type (p, ast.identifier (p, "int"), null))], 
  ast.variable_type (p, ast.identifier (p, "C"), null));

  var bindings: list of (ast.ast, ast.ast);
  var match: bool;

  io.put ("Matching:\n");
  ast.put (io.output, t0, 0);
  io.put ("\n");
  io.put ("versus:\n");
  ast.put (io.output, t1, 0);
  io.put ("\n");
  match, bindings := types.unify ([ast.identifier (p, "A"), ast.identifier (p, "B"), ast.identifier (p, "C")], t0, t1);

  if match then
    io.put ("Matching!\n");
    while bindings <> null do
      var name: ast.ast, bind: ast.ast;
      name, bind := hd bindings;
      ast.put (io.output, name, 0);
      io.put (" => ");
      ast.put (io.output, bind, 0);
      io.put (" -> ");
      ast.put (io.output, ast.binding (name), 0);
      io.put ("\n");
      bindings := tl bindings;
    end;
  else
    io.put ("No match!\n");
  end;
end;

// End of analyze.t.
