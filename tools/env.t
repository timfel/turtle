// env.t -- Compile-time environments for Turtle.
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

module env;

import io, ast, types;

public datatype definition = vardef (typ: ast.ast) or
                             constdef (typ: ast.ast) or
			     fundef (typ: ast.ast) or
			     constraintdef (typ: ast.ast) or
			     typedef (typ: ast.ast) or
			     datatypedef (name: ast.ast);

public datatype binding = binding (name: ast.ast, l: list of definition);

public datatype env = env (l: list of binding, next: env);

public var bool_type: ast.ast := 
	ast.named_type
 	 (ast.pos ("", 0, 0), 
	  ast.identifier (ast.pos ("", 0, 0), "bool"), null);

public fun contains (def: definition, defs: list of definition): bool
  var t0: ast.ast, t1: ast.ast;
  if vardef? (def) or constdef? (def) or fundef? (def) or
    constraintdef? (def) then
    t0 := typ (def);
    while defs <> null do
      if vardef? (hd defs) or constdef? (hd defs) or fundef? (hd defs) or
	constraintdef? (hd defs) then
	t1 := typ (hd defs);
	if types.eq_types (t0, t1) then
	  return true;
	end;
      end;
      defs := tl defs;
    end;
  else
    while defs <> null do
      if typedef? (hd defs) or datatypedef? (hd defs) then
	return true;
      end;
      defs := tl defs;
    end;
  end;
  return false;
end;

public fun lookup (name: ast.ast, env: env, deep: bool): list of definition
  var l: list of binding;
  var result: list of definition := null;
  var defs: list of definition;

  while env <> null do
    l := l (env);
    while l <> null do
      if ast.eq_identifiers (name, name (hd l)) then
	defs := l (hd l);
	while defs <> null do
	  if not contains (hd defs, result) then
	    result := hd defs :: result;
	  end;
	  defs := tl defs;
	end;
      end;
      l := tl l;
    end;
    if not deep then
      return result;
    end;
    env := next (env);
  end;
  return result;
end;

public fun bind (name: ast.ast, def: definition, env: env)
  var l: list of binding := l (env);
  while l <> null do
    if ast.eq_identifiers (name (hd l), name) then
      l! (hd l, def :: l (hd l));
      return;
    end;
    l := tl l;
  end;
  l! (env, binding (name, [def]) :: l (env));
end;
  

public fun builtin_env (): env
  var env: env := env ([], null);
  var sys: ast.ast := ast.identifier (ast.pos ("", 0, 0), "_system");
  bind (ast.identifier (ast.pos ("", 0, 0), "int"),
        datatypedef 
	(ast.named_type
 	 (ast.pos ("", 0, 0), 
	  ast.identifier (ast.pos ("", 0, 0), "int"), null)),
	env);
  bind (ast.identifier (ast.pos ("", 0, 0), "long"),
        datatypedef 
	(ast.named_type
 	 (ast.pos ("", 0, 0), 
	  ast.identifier (ast.pos ("", 0, 0), "long"), null)),
	env);
  bind (ast.identifier (ast.pos ("", 0, 0), "real"),
        datatypedef 
	(ast.named_type
 	 (ast.pos ("", 0, 0), 
	  ast.identifier (ast.pos ("", 0, 0), "real"), null)),
	env);
  bind (ast.identifier (ast.pos ("", 0, 0), "bool"), datatypedef (bool_type),
	env);
  bind (ast.identifier (ast.pos ("", 0, 0), "char"),
        datatypedef 
	(ast.named_type
 	 (ast.pos ("", 0, 0), 
	  ast.identifier (ast.pos ("", 0, 0), "char"), null)),
	env);
/*   l! (env, binding (ast.identifier (ast.pos ("", 0, 0), "int"),  */
/*                     [datatypedef  */
/*                      (ast.named_type */
/* 		      (ast.pos ("", 0, 0),  */
/* 		       ast.qualident (ast.pos ("", 0, 0), sys, ast.identifier (ast.pos ("", 0, 0), "int")), null))]) */
/* 		       :: l (env)); */
/*   l! (env, binding (ast.identifier (ast.pos ("", 0, 0), "long"),  */
/*                     [datatypedef  */
/* 		     (ast.named_type */
/* 		      (ast.pos ("", 0, 0),  */
/* 		       ast.qualident (ast.pos ("", 0, 0), sys, ast.identifier (ast.pos ("", 0, 0),  */
/*                                        "long")), null))]) :: l (env)); */
/*   l! (env, binding (ast.identifier (ast.pos ("", 0, 0), "real"),  */
/*                     [datatypedef  */
/* 		     (ast.named_type  */
/* 		      (ast.pos ("", 0, 0), */
/* 		       ast.qualident (ast.pos ("", 0, 0), sys, ast.identifier (ast.pos ("", 0, 0),  */
/*                                        "real")), null))]) :: l (env)); */
/*   l! (env, binding (ast.identifier (ast.pos ("", 0, 0), "char"),  */
/*                     [datatypedef  */
/* 		     (ast.named_type */
/* 		      (ast.pos ("", 0, 0), */
/* 		       ast.qualident (ast.pos ("", 0, 0), sys, ast.identifier (ast.pos ("", 0, 0),  */
/*                                        "char")), null))]) :: l (env)); */
/*   l! (env, binding (ast.identifier (ast.pos ("", 0, 0), "bool"),  */
/*                     [datatypedef  */
/* 		     (ast.named_type */
/* 		      (ast.pos ("", 0, 0), */
/* 		       ast.qualident (ast.pos ("", 0, 0), sys, ast.identifier (ast.pos ("", 0, 0),  */
/*                                        "bool")), null))]) :: l (env)); */
/* 				       dump_env (env); */
  return env;
end;

public fun dump_def (def: definition)
  if vardef? (def) then
    io.put ("var: ");
    ast.put (io.output, typ (def), 0);
  elsif constdef? (def) then
    io.put ("const: ");
    ast.put (io.output, typ (def), 0);
  elsif fundef? (def) then
    io.put ("function: ");
    ast.put (io.output, typ (def), 0);
  elsif constraintdef? (def) then
    io.put ("constraint: ");
    ast.put (io.output, typ (def), 0);
  elsif typedef? (def) then
    io.put ("type: ");
    ast.put (io.output, typ (def), 0);
  elsif datatypedef? (def) then
    io.put ("datatype: ");
    ast.put (io.output, name (def), 0);
  end;
end;

public fun dump_def_list (defs: list of definition)
  while defs <> null do
    dump_def (hd defs);
    defs := tl defs;
    if defs <> null then
      io.put (", ");
    end;
  end;
end;

public fun dump_env (env: env)
  io.put ("Environment:\n");
  var l: list of binding := l (env);
  while l <> null do
    io.put ("Binding: ");
    ast.put (io.output, name (hd l), 0);
    io.put (" [");
    dump_def_list (l (hd l));
    io.put ("]");
    io.nl ();
    l := tl l;
  end;
end;

// End of env.t.
