// parser.t -- Turtle parser.
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
//* This is a parser for the Turtle language, written in Turtle.

module parser;

import io, scanner, ast;

// This is the current token at which the parser looks.
//
var token: scanner.token;

var error_count: int := 0;

//* Fetch the next token from the input and store it in the global
//* variable @var{token}.
//
fun get (s: scanner.state)
  token := scanner.scan (s);
end;

fun match (s: scanner.state, m: fun (scanner.token): bool)
  if not m (token) then
    scanner.error (s, "unexpected token: " + scanner.name (token));
    error_count := error_count + 1;
  end;
  token := scanner.scan (s);
end;

fun pos (s: scanner.state): ast.pos
  return ast.pos (scanner.name (s), scanner.start_line (s), 
                  scanner.start_col (s));
end;

fun parse_identifier (s: scanner.state): ast.ast
  var t: scanner.token := token;
  match (s, scanner.identifier?);
  return ast.identifier (pos (s), scanner.name (t));
end;

fun parse_qualident (s: scanner.state): ast.ast
  var ident: ast.ast := parse_identifier (s);
  while scanner.period? (token) do
    match (s, scanner.period?);
    ident := ast.qualident (pos (s), ident, parse_identifier (s));
  end;
  return ident;
end;

fun parse_type (s: scanner.state): ast.ast
  var p: ast.pos := pos (s);
  var typ: ast.ast;
  var types: list of ast.ast := [];

  if scanner.identifier? (token) then
    var name: ast.ast := parse_qualident (s);
    if scanner.lt? (token) then
      match (s, scanner.lt?);
      types := [parse_type (s)];
      while not scanner.eof? (token) and not scanner.gt? (token) do
	match (s, scanner.comma?);
	types := parse_type (s) :: types;
      end;
      match (s, scanner.gt?);
    end;
    return ast.named_type (p, name, ast.rev (types));
  elsif scanner.lparen? (token) then
    match (s, scanner.lparen?);
    if scanner.rparen? (token) then
      match (s, scanner.rparen?);
      return ast.void_type (p);
    else
      typ := parse_type (s);
      if scanner.comma? (token) then
	types := [typ];
	while scanner.comma? (token) do
	  match (s, scanner.comma?);
	  types := parse_type (s) :: types;
	end;
	match (s, scanner.rparen?);
	return ast.tuple_type (p, ast.rev (types));
      else
	match (s, scanner.rparen?);
	return typ;
      end;
    end;
  elsif scanner.tarray? (token) then
    match (s, scanner.tarray?);
    match (s, scanner.tof?);
    typ := parse_type (s);    
    return ast.array_type (p, typ);
  elsif scanner.tlist? (token) then
    match (s, scanner.tlist?);
    match (s, scanner.tof?);
    typ := parse_type (s);    
    return ast.list_type (p, typ);
  elsif scanner.tstring? (token) then
    match (s, scanner.tstring?);
    return ast.string_type (p);
  elsif scanner.exclamation? (token) then
    match (s, scanner.exclamation?);
    typ := parse_type (s);
    return ast.constraint_type (p, typ);
  elsif scanner.backquote? (token) then
    match (s, scanner.backquote?);
    typ := parse_identifier (s);
    return ast.variable_type (p, typ, null);
  elsif scanner.tfun? (token) then
    match (s, scanner.tfun?);
    match (s, scanner.lparen?);
    if not scanner.eof? (token) and not scanner.rparen? (token) then
      types := [parse_type (s)];
      while not scanner.eof? (token) and not scanner.rparen? (token) do
	match (s, scanner.comma?);
	types := parse_type (s) :: types;
      end;
    end;
    match (s, scanner.rparen?);
    typ := parse_optional_return_type (s);
    return ast.function_type (p, ast.rev (types), typ);
  end;
  return null;
end;

fun parse_actual_module_parameter (s: scanner.state): ast.ast
  return parse_type (s);
end;

fun parse_ann_identifier (s: scanner.state): ast.ast
  var name: ast.ast := parse_qualident (s);
  var params: list of ast.ast;
  var p: ast.pos := pos (s);

  if scanner.lt? (token) then
    match (s, scanner.lt?);
    params := [parse_actual_module_parameter (s)];
    while not scanner.gt? (token) and not scanner.eof? (token) do
      match (s, scanner.comma?);
      params := parse_actual_module_parameter (s) :: params;
    end;
    match (s, scanner.gt?);
    return ast.annident (p, name, ast.rev (params));
  else
    return name;
  end;
end;

fun parse_identifier_list (s: scanner.state): list of ast.ast
  var names: list of ast.ast := [parse_ann_identifier (s)];
  while scanner.comma? (token) do
    match (s, scanner.comma?);
    names := parse_ann_identifier (s) :: names;
  end;
  match (s, scanner.semicolon?);
  return ast.rev (names);
end;

fun parse_module_import (s: scanner.state): list of ast.ast
  match (s, scanner.timport?);
  return parse_identifier_list (s);
end;

fun parse_module_export (s: scanner.state): list of ast.ast
  match (s, scanner.texport?);
  return parse_identifier_list (s);
end;

fun parse_formal_module_parameter (s: scanner.state): ast.ast
  return parse_identifier (s);
end;

fun parse_module_name (s: scanner.state): ast.ast
  var name: ast.ast := parse_qualident (s);
  if scanner.lt? (token) then
    var params: list of ast.ast;
    match (s, scanner.lt?);
    params := [parse_formal_module_parameter (s)];
    while not scanner.gt? (token) and not scanner.eof? (token) do
      match (s, scanner.comma?);
      params := parse_formal_module_parameter (s) :: params;
    end;
    match (s, scanner.gt?);
    return ast.annident (pos (s), name, ast.rev (params));
  else
    return name;
  end;
end;

fun mul? (tok: scanner.token): bool
  return scanner.star? (tok) or scanner.slash? (tok) or
    scanner.percent? (tok);
end;

fun add? (tok: scanner.token): bool
  return scanner.plus? (tok) or scanner.minus? (tok);
end;

fun compare? (tok: scanner.token): bool
  return scanner.eq? (tok) or scanner.ne? (tok) or
    scanner.lt? (tok) or scanner.le? (tok) or
    scanner.gt? (tok) or scanner.ge? (tok);
end;

fun parse_fun_expression (s: scanner.state): ast.ast
  var p: ast.pos := pos (s);
  match (s, scanner.tfun?);
  var formals: list of ast.ast := parse_formal_parameters (s);
  var ret: ast.ast := parse_optional_return_type (s);
  var body: list of ast.ast := parse_function_body (s);
  return ast.fun_expr (p, formals, ret, body);
end;

fun parse_constraint_expression (s: scanner.state): ast.ast
  var p: ast.pos := pos (s);
  match (s, scanner.tconstraint?);
  var formals: list of ast.ast := parse_formal_parameters (s);
  var body: list of ast.ast := parse_function_body (s);
  return ast.constraint_expr (p, formals, body);
end;

fun parse_array_expression (s: scanner.state): ast.ast
  var elems: list of ast.ast := [];
  var p: ast.pos := pos (s);

  match (s, scanner.lbrace?);
  if not scanner.rbrace? (token) and not scanner.eof? (token) then
    elems := [parse_cons_expression (s)];
    while not scanner.rbrace? (token) and not scanner.eof? (token) do
      match (s, scanner.comma?);
      elems := parse_cons_expression (s) :: elems;
    end;
  end;
  match (s, scanner.rbrace?); 
  return ast.array_expr (p, ast.rev (elems));
end;

fun parse_list_expression (s: scanner.state): ast.ast
  var elems: list of ast.ast := [];
  var p: ast.pos := pos (s);

  match (s, scanner.lbracket?);
  if not scanner.rbracket? (token) and not scanner.eof? (token) then
    elems := [parse_cons_expression (s)];
    while not scanner.rbracket? (token) and not scanner.eof? (token) do
      match (s, scanner.comma?);
      elems := parse_cons_expression (s) :: elems;
    end;
  end;
  match (s, scanner.rbracket?); 
  return ast.list_expr (p, ast.rev (elems));
end;

fun parse_atomic_expression (s: scanner.state): ast.ast
  var ret: ast.ast, size: ast.ast, init: ast.ast;
  var p: ast.pos := pos (s);
  if scanner.identifier? (token) then
    return parse_qualident (s);
  elsif scanner.tforeign? (token) then
    match (s, scanner.tforeign?);
    if scanner.str_const? (token) then
      ret := ast.foreign_expr (pos (s), ast.str_const (pos (s), scanner.value (token)));
    else
      ret := ast.error (pos (s), "");
    end;
    match (s, scanner.str_const?);
    return ret;
  elsif scanner.int_const? (token) then
    ret := ast.int_const (pos (s), scanner.value (token));
    get (s);
    return ret;
  elsif scanner.long_const? (token) then
    ret := ast.long_const (pos (s), scanner.value (token));
    get (s);
    return ret;
  elsif scanner.real_const? (token) then
    ret := ast.real_const (pos (s), scanner.value (token));
    get (s);
    return ret;
  elsif scanner.char_const? (token) then
    ret := ast.char_const (pos (s), scanner.value (token));
    get (s);
    return ret;
  elsif scanner.str_const? (token) then
    ret := ast.str_const (pos (s), scanner.value (token));
    get (s);
    return ret;
  elsif scanner.tfalse? (token) or scanner.ttrue? (token) then
    ret := ast.bool_const (pos (s), false);
    get (s);
    return ret;
  elsif scanner.tnull? (token) then
    ret := ast.null_const (pos (s));
    get (s);
    return ret;
  elsif scanner.lparen? (token) then
    match (s, scanner.lparen?);
    ret := parse_tuple_expression (s);
    match (s, scanner.rparen?);
    return ret;
  elsif scanner.lbrace? (token) then
    return parse_array_expression (s);
  elsif scanner.lbracket? (token) then
    return parse_list_expression (s);
  elsif scanner.tfun? (token) then
    return parse_fun_expression (s);
  elsif scanner.tconstraint? (token) then
    return parse_constraint_expression (s);
  elsif scanner.tarray? (token) then
    match (s, scanner.tarray?);
    size := parse_add_expression (s);
    match (s, scanner.tof?);
    init := parse_tuple_expression (s);
    return ast.array_constructor (p, size, init);
  elsif scanner.tlist? (token) then
    match (s, scanner.tlist?);
    size := parse_add_expression (s);
    match (s, scanner.tof?);
    init := parse_tuple_expression (s);
    return ast.list_constructor (p, size, init);
  elsif scanner.tstring? (token) then
    match (s, scanner.tstring?);
    size := parse_add_expression (s);
    if scanner.tof? (token) then
      match (s, scanner.tof?);
      init := parse_simple_expression (s);
    else
      init := null;
    end;
    return ast.string_constructor (p, size, init);
  else
    scanner.error (s, "expression expected");
    return ast.error (pos (s), "");
  end;
end;

fun parse_index (s: scanner.state): ast.ast
  match (s, scanner.lbracket?);
  var idx: ast.ast := parse_add_expression (s);
  match (s, scanner.rbracket?);
  return idx; 
end;

fun parse_actual_params (s: scanner.state): list of ast.ast
  var parms: list of ast.ast := [];
  match (s, scanner.lparen?);
  if not scanner.rparen? (token) and not scanner.eof? (token) then
    parms := [parse_cons_expression (s)];
    while not scanner.rparen? (token) and not scanner.eof? (token) do
      match (s, scanner.comma?);
      parms := parse_cons_expression (s) :: parms;
    end;
  end;
  match (s, scanner.rparen?);
  return ast.rev (parms);
end;

fun parse_simple_expression (s: scanner.state): ast.ast
  var expr: ast.ast := parse_atomic_expression (s);
  while scanner.lparen? (token) or scanner.lbracket? (token) do
    if scanner.lparen? (token) then
      expr := ast.call (pos (s), expr, parse_actual_params (s));
    elsif scanner.lbracket? (token) then
      expr := ast.index (pos (s), expr, parse_index (s));
    end;
  end;
  return expr;
end;

fun parse_factor_expression (s: scanner.state): ast.ast
  if scanner.minus? (token) then
    match (s, scanner.minus?);
    return ast.unary (pos (s), ast.NEG, parse_factor_expression (s));
  elsif scanner.tnot? (token) then
    match (s, scanner.tnot?);
    return ast.unary (pos (s), ast.NOT, parse_factor_expression (s));
  elsif scanner.thd? (token) then
    match (s, scanner.thd?);
    return ast.unary (pos (s), ast.HD, parse_factor_expression (s));
  elsif scanner.ttl? (token) then
    match (s, scanner.ttl?);
    return ast.unary (pos (s), ast.TL, parse_factor_expression (s));
  elsif scanner.tsizeof? (token) then
    match (s, scanner.tsizeof?);
    return ast.unary (pos (s), ast.SIZEOF, parse_factor_expression (s));
  else
    return parse_simple_expression (s);
  end;
end;

fun parse_mul_expression (s: scanner.state): ast.ast
  var expr: ast.ast := parse_factor_expression (s);
  while mul? (token) do
    var op: int;
    if scanner.star? (token) then
      op := ast.MUL;
    elsif scanner.slash? (token) then
      op := ast.DIV;
    elsif scanner.percent? (token) then
      op := ast.MOD;
    end;
    match (s, mul?);
    expr := ast.binary (pos (s), op, expr, parse_factor_expression (s));
  end;
  return expr;
end;

fun parse_add_expression (s: scanner.state): ast.ast
  var expr: ast.ast := parse_mul_expression (s);
  while add? (token) do
    var op: int;
    if scanner.plus? (token) then
      op := ast.ADD;
    elsif scanner.minus? (token) then
      op := ast.SUB;
    end;
    match (s, add?);
    expr := ast.binary (pos (s), op, expr, parse_mul_expression (s));
  end;
  return expr;
end;

fun parse_compare_expression (s: scanner.state): ast.ast
  var expr: ast.ast := parse_add_expression (s);
  while compare? (token) do
    var op: int;
    if scanner.eq? (token) then
      op := ast.EQ;
    elsif scanner.ne? (token) then
      op := ast.NE;
    elsif scanner.lt? (token) then
      op := ast.LT;
    elsif scanner.le? (token) then
      op := ast.LE;
    elsif scanner.gt? (token) then
      op := ast.GT;
    elsif scanner.ge? (token) then
      op := ast.GE;
    end;
    match (s, compare?);
    expr := ast.binary (pos (s), op, expr, parse_add_expression (s));
  end;
  return expr;
end;

fun parse_and_expression (s: scanner.state): ast.ast
  var expr: ast.ast := parse_compare_expression (s);
  while scanner.tand? (token) do
    match (s, scanner.tand?);
    expr := ast.binary (pos (s), ast.AND, expr, parse_compare_expression (s));
  end;
  return expr;
end;

fun parse_or_expression (s: scanner.state): ast.ast
  var expr: ast.ast := parse_and_expression (s);
  while scanner.tor? (token) do
    match (s, scanner.tor?);
    expr := ast.binary (pos (s), ast.OR, expr, parse_and_expression (s));
  end;
  return expr;
end;

fun parse_cons_expression (s: scanner.state): ast.ast
  var expr: ast.ast := parse_or_expression (s);
  if scanner.cons? (token) then
    match (s, scanner.cons?);
    expr := ast.binary (pos (s), ast.CONS, expr, parse_cons_expression (s));
  end;
  return expr;
end;

fun parse_expression (s: scanner.state): ast.ast
  return parse_cons_expression (s);
end;

fun parse_tuple_expression (s: scanner.state): ast.ast
  var expr: ast.ast := parse_cons_expression (s);
  if scanner.comma? (token) then
    var exprs: list of ast.ast := [expr];
    while scanner.comma? (token) do
      match (s, scanner.comma?);
      exprs := parse_cons_expression (s) :: exprs;
    end;
    return ast.tuple_expr (pos (s), ast.rev (exprs));
  end;
  return expr;
end;

fun parse_assign_expression (s: scanner.state): ast.ast
  var lhs: ast.ast := parse_tuple_expression (s);
  if scanner.assign? (token) then
    match (s, scanner.assign?);
    return ast.binary (pos (s), ast.ASSIGN, lhs, parse_tuple_expression (s));
  end;
  return lhs;
end;

fun parse_variable (s: scanner.state): ast.ast
  var name: ast.ast := parse_identifier (s);
  match (s, scanner.colon?);
  var typ: ast.ast := parse_type (s);
  var init: ast.ast := null;
  if scanner.assign? (token) then
    match (s, scanner.assign?);
    init := parse_expression (s);
  end;
  return ast.variable (pos (s), name, typ, init);
end;

fun parse_variable_definition (s: scanner.state, pub: bool): ast.ast
  match (s, scanner.tvar?);
  var comment: string := scanner.get_comment (s);
  var vars: list of ast.ast := [parse_variable (s)];
  while scanner.comma? (token) do
    match (s, scanner.comma?);
    vars := parse_variable (s) :: vars;
  end;
  return ast.vardef (pos (s), pub, ast.rev (vars), comment);
end;

fun parse_constant_definition (s: scanner.state, pub: bool): ast.ast
  match (s, scanner.tconst?);
  var comment: string := scanner.get_comment (s);
  var vars: list of ast.ast := [parse_variable (s)];
  while scanner.comma? (token) do
    match (s, scanner.comma?);
    vars := parse_variable (s) :: vars;
  end;
  return ast.constdef (pos (s), pub, ast.rev (vars), comment);
end;

fun parse_formal_parameter (s: scanner.state): ast.ast
  var name: ast.ast := parse_identifier (s);
  match (s, scanner.colon?);
  var typ: ast.ast := parse_type (s);
  return ast.variable (pos (s), name, typ, null);
end;

fun parse_if_statement (s: scanner.state): ast.ast
  match (s, scanner.tif?);
  var cond: ast.ast := parse_expression (s);
  var stmts: list of ast.ast := [];
  match (s, scanner.tthen?);
  while not scanner.tend? (token) and 
        not scanner.telse? (token) and
	not scanner.telsif? (token) and
	not scanner.eof? (token) do
    stmts := parse_statement (s) :: stmts;
  end;
  var thens: list of ast.ast := [ast.guarded (pos (s), cond, ast.rev (stmts))];
  stmts := [];
  while scanner.telsif? (token) do
    match (s, scanner.telsif?);
    cond := parse_expression (s);
    match (s, scanner.tthen?);
    while not scanner.tend? (token) and 
          not scanner.telse? (token) and
	  not scanner.telsif? (token) and
	  not scanner.eof? (token) do
      stmts := parse_statement (s) :: stmts;
    end;
    thens := ast.guarded (pos (s), cond, ast.rev (stmts)) :: thens;
    stmts := [];
  end;
  if scanner.telse? (token) then
    match (s, scanner.telse?);
    while not scanner.tend? (token) and not scanner.eof? (token) do
      stmts := parse_statement (s) :: stmts;
    end;
  end;
  match (s, scanner.tend?);
  return ast.ifstmt (pos (s), ast.rev (thens), ast.rev (stmts));
end;

fun parse_while_statement (s: scanner.state): ast.ast
  match (s, scanner.twhile?);
  var cond: ast.ast := parse_expression (s);
  var stmts: list of ast.ast := [];
  match (s, scanner.tdo?);
  while not scanner.tend? (token) and not scanner.eof? (token) do
    stmts := parse_statement (s) :: stmts;
  end;
  match (s, scanner.tend?);
  return ast.whilestmt (pos (s), cond, ast.rev (stmts));
end;

fun parse_in_statement (s: scanner.state): ast.ast
  var stmts: list of ast.ast := [];
  match (s, scanner.tin?);
  while not scanner.tend? (token) and not scanner.eof? (token) do
    stmts := parse_statement (s) :: stmts;
  end;
  match (s, scanner.tend?);
  return ast.instmt (pos (s), ast.rev (stmts));
end;

fun parse_return_statement (s: scanner.state): ast.ast
  var expr: ast.ast := null;
  match (s, scanner.treturn?);
  if not scanner.semicolon? (token) and not scanner.eof? (token) then
    expr := parse_tuple_expression (s);
  end;
  return ast.returnstmt (pos (s), expr);
end;

fun parse_require_statement (s: scanner.state): ast.ast
  var conj: list of ast.ast;
  var strengths: list of ast.ast;
  var expr: ast.ast := null;
  var strength: ast.ast;
  var body: ast.ast;
  match (s, scanner.trequire?);

  expr := parse_compare_expression (s);
  if scanner.colon? (token) then
    match (s, scanner.colon?);
    strength := parse_atomic_expression (s);
  else
    strength := null;
  end;
  conj := [expr];
  strengths := [strength];
  while scanner.tand? (token) do
    match (s, scanner.tand?);
    expr := parse_compare_expression (s);
    if scanner.colon? (token) then
      match (s, scanner.colon?);
      strength := parse_atomic_expression (s);
    else
      strength := null;
    end;
    conj := expr :: conj;
    strengths := strength :: strengths;
  end;
  if scanner.semicolon? (token) then
    body := null;
  elsif scanner.twhile? (token) then
    body := parse_while_statement (s);
  elsif scanner.tin? (token) then
    body := parse_in_statement (s);
  else
    body := null;
  end;
  return ast.requirestmt (pos (s), ast.rev (conj), ast.rev (strengths), body);
end;

fun parse_statement (s: scanner.state): ast.ast
  var stmt: ast.ast;
  if scanner.twhile? (token) then
    stmt := parse_while_statement (s);
  elsif scanner.tif? (token) then
    stmt := parse_if_statement (s);
  elsif scanner.treturn? (token) then
    stmt := parse_return_statement (s);
  elsif scanner.tvar? (token) then
    stmt := parse_variable_definition (s, false);
  elsif scanner.tconst? (token) then
    stmt := parse_constant_definition (s, false);
  elsif scanner.tfun? (token) then
    stmt := parse_function_definition (s, false);
  elsif scanner.tconstraint? (token) then
    stmt := parse_constraint_definition (s, false);
  elsif scanner.trequire? (token) then
    stmt := parse_require_statement (s);
  elsif scanner.tin? (token) then
    stmt := parse_in_statement (s);
  else
    stmt := parse_assign_expression (s);
  end;
  match (s, scanner.semicolon?);
  return stmt;
end;

fun parse_function_body (s: scanner.state): list of ast.ast
  var b: list of ast.ast := [];
  while not scanner.tend? (token) and not scanner.eof? (token) do
    b := parse_statement (s) :: b;
  end;
  match (s, scanner.tend?);
  return ast.rev (b);
end;

fun parse_formal_parameters (s: scanner.state): list of ast.ast
  var params: list of ast.ast := [];
  match (s, scanner.lparen?);
  if not scanner.rparen? (token) and not scanner.eof? (token) then
    params := parse_formal_parameter (s) :: params;
    while not scanner.rparen? (token) and not scanner.eof? (token) do
      match (s, scanner.comma?);
      params := parse_formal_parameter (s) :: params;
    end;
  end;
  match (s, scanner.rparen?);
  return ast.rev (params);
end;

fun parse_optional_return_type (s: scanner.state): ast.ast
  if scanner.colon? (token) then
    match (s, scanner.colon?);
    return parse_type (s);
  else
    return ast.void_type (pos (s));
  end;
end;

fun parse_function_definition (s: scanner.state, pub: bool): ast.ast
  var handcoded: bool := false;
  var mapped: bool := false;
  match (s, scanner.tfun?);
  var comment: string := scanner.get_comment (s);
  var name: ast.ast := parse_identifier (s);
  var params: list of ast.ast := parse_formal_parameters (s);
  var ret_type: ast.ast := parse_optional_return_type (s);
  var body: list of ast.ast := null;
  var mapping: string := null;
  if scanner.semicolon? (token) then
    // Handcoded function with implementation macro.
    handcoded := true;
  elsif scanner.eq? (token) then
    // Handcoded function with mapping function.
    match (s, scanner.eq?);
    if scanner.str_const? (token) then
      mapping := scanner.value (token);
    end;
    match (s, scanner.str_const?);
    mapped := true;
  else
    body := parse_function_body (s);
  end;
  return ast.fundef (pos (s), pub, name, params, ret_type, body,
    handcoded, mapped, mapping, comment);
end;

fun parse_constraint_definition (s: scanner.state, pub: bool): ast.ast
  match (s, scanner.tconstraint?);
  var comment: string := scanner.get_comment (s);
  var name: ast.ast := parse_identifier (s);
  var params: list of ast.ast := parse_formal_parameters (s);
  var body: list of ast.ast := parse_function_body (s);
  return ast.constraintdef (pos (s), pub, name, params, body, comment);
end;

fun parse_type_definition (s: scanner.state, pub: bool): ast.ast
  match (s, scanner.ttype?);
  var comment: string := scanner.get_comment (s);
  var name: ast.ast := parse_identifier (s);
  match (s, scanner.eq?);
  var typ: ast.ast := parse_type (s);
  return ast.typedef (pos (s), pub, name, typ, comment);
end;

fun parse_datatype_field (s: scanner.state): ast.ast
  var name: ast.ast := parse_identifier (s);
  match (s, scanner.colon?);
  var typ: ast.ast := parse_type (s);
  return ast.datatypefield (pos (s), name, typ);
end;

fun parse_datatype_variant (s: scanner.state): ast.ast
  var fields: list of ast.ast := [];
  var name: ast.ast := parse_identifier (s);
  if scanner.lparen? (token) then
    match (s, scanner.lparen?);
    fields := [parse_datatype_field (s)];
    while not scanner.rparen? (token) and not scanner.eof? (token) do
      match (s, scanner.comma?);
      fields := parse_datatype_field (s) :: fields;
    end;
    match (s, scanner.rparen?);
  end;
  return ast.datatypevariant (pos (s), name, ast.rev (fields));
end;

fun parse_datatype_definition (s: scanner.state, pub: bool): ast.ast
  var variants: list of ast.ast := [];
  var params: list of ast.ast := [];
  match (s, scanner.tdatatype?);
  var comment: string := scanner.get_comment (s);
  var name: ast.ast := parse_identifier (s);
  if scanner.lt? (token) then
    match (s, scanner.lt?);
    params := [parse_identifier (s)];
    while not scanner.gt? (token) and not scanner.eof? (token) do
      match (s, scanner.comma?);
      params := parse_identifier (s) :: params;
    end;
    match (s, scanner.gt?);
  end;
  match (s, scanner.eq?);
  variants := [parse_datatype_variant (s)];
  while scanner.tor? (token) do
    match (s, scanner.tor?);
    variants := parse_datatype_variant (s) :: variants;
  end;
  return ast.datatypedef (pos (s), pub, name, ast.rev (params), 
                          ast.rev (variants), comment);
end;

fun parse_module_definitions (s: scanner.state): list of ast.ast
  var defs: list of ast.ast := [];
  while not scanner.eof? (token) do
    var def: ast.ast := null;
    var pub: bool := false;
    if scanner.tpublic? (token) then
      match (s, scanner.tpublic?);
      pub := true;
    end;
    if scanner.tvar? (token) then
      def := parse_variable_definition (s, pub);
    elsif scanner.tconst? (token) then
      def := parse_constant_definition (s, pub);
    elsif scanner.tfun? (token) then
      def := parse_function_definition (s, pub);
    elsif scanner.tconstraint? (token) then
      def := parse_constraint_definition (s, pub);
    elsif scanner.ttype? (token) then
      def := parse_type_definition (s, pub);
    elsif scanner.tdatatype? (token) then
      def := parse_datatype_definition (s, pub);
    end;
    match (s, scanner.semicolon?);
    if def <> null then
      defs := def :: defs;
    end;
  end;
  return ast.rev (defs);
end;

public fun parse (s: scanner.state): ast.ast
  var imp: list of ast.ast := null;
  var exp: list of ast.ast := null;
  get (s);
  var p: ast.pos := pos (s);
  match (s, scanner.tmodule?);
  var comment: string := scanner.get_comment (s);
  var name: ast.ast := parse_module_name (s);
  match (s, scanner.semicolon?);
  if scanner.timport? (token) then
    imp := parse_module_import (s);
  end;
  if scanner.texport? (token) then
    exp := parse_module_export (s);
  end;
  return ast.moduledef (p, name, imp, exp, parse_module_definitions (s),
                        comment);
end;

// End of parser.t.
