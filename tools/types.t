// types.t -- Compile-time type handling for Turtle.
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

module types;

import ast;

//* Return true if the two types @var{t0} and @var{t1} are
//* equivalent, false otherwise.
//
public fun eq_types (t0: ast.ast, t1: ast.ast): bool
  var l0: list of ast.ast;
  var l1: list of ast.ast;

  if ast.string_type? (t0) then
    return ast.string_type? (t1);
  elsif ast.void_type? (t0) then
    return ast.void_type? (t1);
  elsif ast.array_type? (t0) and ast.array_type? (t1) then
    return eq_types (ast.base (t0), ast.base (t1));
  elsif ast.list_type? (t0) and ast.list_type? (t1) then
    return eq_types (ast.base (t0), ast.base (t1));
  elsif ast.constraint_type? (t0) and ast.constraint_type? (t1) then
    return eq_types (ast.base (t0), ast.base (t1));
  elsif ast.variable_type? (t0) and ast.variable_type? (t1) then
    return ast.eq_identifiers (ast.base (t0), ast.base (t1));
  elsif ast.tuple_type? (t0) and ast.tuple_type? (t1) then
    l0 := ast.types (t0);    
    l1 := ast.types (t1);
    while l0 <> null and l1 <> null do
      if not eq_types (hd l0, hd l1) then
	return false;
      end;
      l0 := tl l0;
      l1 := tl l1;
    end;
    return l0 = null and l1 = null;
  elsif ast.function_type? (t0) and ast.function_type? (t1) then
    l0 := ast.params (t0);    
    l1 := ast.params (t1);
    while l0 <> null and l1 <> null do
      if not eq_types (hd l0, hd l1) then
	return false;
      end;
      l0 := tl l0;
      l1 := tl l1;
    end;
    return l0 = null and l1 = null and eq_types (ast.ret (t0), ast.ret (t1));
  elsif ast.named_type? (t0) and ast.named_type? (t1) then
    if not ast.eq_identifiers (ast.name (t0), ast.name (t1)) then
      return false;
    end;
    l0 := ast.params (t0);    
    l1 := ast.params (t1);
    while l0 <> null and l1 <> null do
      if not eq_types (hd l0, hd l1) then
	return false;
      end;
      l0 := tl l0;
      l1 := tl l1;
    end;
    return l0 = null and l1 = null;
  else
    return false;
  end;
end;

var bindings: list of (ast.ast, ast.ast) := null;

public fun unify (vars: list of ast.ast, t0: ast.ast, t1: ast.ast):
  (bool, list of (ast.ast, ast.ast))
  bindings := null;
  var res: bool := unify_types (vars, t0, t1);
  return (res, bindings);
end;

fun unify_types (vars: list of ast.ast, t0: ast.ast, t1: ast.ast): bool
  fun variable? (t: ast.ast): bool
    var vs: list of ast.ast := vars;
    if not ast.named_type? (t) then
      return false;
    end;
    while vs <> null do
      if ast.eq_identifiers (ast.name (t), hd vs) then
	return true;
      end;
      vs := tl vs;
    end;
    return false;
  end;

  fun follow (t: ast.ast): ast.ast
    while ast.variable_type? (t) and ast.binding (t) <> null do
      ast.binding! (t, ast.binding (t));
      t := ast.binding (t);
    end;
    return t;
  end;

  var l0: list of ast.ast;
  var l1: list of ast.ast;

  t0 := follow (t0);
  t1 := follow (t1);

  if ast.variable_type? (t0) then
    ast.binding! (t0, t1);
    bindings := (t0, t1) :: bindings;
    return true;
  elsif ast.variable_type? (t1) then
    ast.binding! (t1, t0);
    bindings := (t1, t0) :: bindings;
    return true;
  elsif ast.string_type? (t0) then
    return ast.string_type? (t1);
  elsif ast.void_type? (t0) then
    return ast.void_type? (t1);
  elsif ast.array_type? (t0) and ast.array_type? (t1) then
    return unify_types (vars, ast.base (t0), ast.base (t1));
  elsif ast.list_type? (t0) and ast.list_type? (t1) then
    return unify_types (vars, ast.base (t0), ast.base (t1));
  elsif ast.constraint_type? (t0) and ast.constraint_type? (t1) then
    return unify_types (vars, ast.base (t0), ast.base (t1));
  elsif ast.variable_type? (t0) and ast.variable_type? (t1) then
    return ast.eq_identifiers (ast.base (t0), ast.base (t1));
  elsif ast.tuple_type? (t0) and ast.tuple_type? (t1) then
    l0 := ast.types (t0);    
    l1 := ast.types (t1);
    while l0 <> null and l1 <> null do
      if not unify_types (vars, hd l0, hd l1) then
	return false;
      end;
      l0 := tl l0;
      l1 := tl l1;
    end;
    return l0 = null and l1 = null;
  elsif ast.function_type? (t0) and ast.function_type? (t1) then
    l0 := ast.params (t0);    
    l1 := ast.params (t1);
    while l0 <> null and l1 <> null do
      if not unify_types (vars, hd l0, hd l1) then
	return false;
      end;
      l0 := tl l0;
      l1 := tl l1;
    end;
    return l0 = null and l1 = null and 
      unify_types (vars, ast.ret (t0), ast.ret (t1));
  elsif ast.named_type? (t0) and ast.named_type? (t1) then
    if not ast.eq_identifiers (ast.name (t0), ast.name (t1)) then
      return false;
    end;
    l0 := ast.params (t0);    
    l1 := ast.params (t1);
    while l0 <> null and l1 <> null do
      if not unify_types (vars, hd l0, hd l1) then
	return false;
      end;
      l0 := tl l0;
      l1 := tl l1;
    end;
    return l0 = null and l1 = null;
  else
    return false;
  end;
  
end;

// End of types.t.
