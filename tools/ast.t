// ast.t -- Turtle abstract syntax.
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
// This the abstract syntax of Turtle programs.

module ast;

import io, strings, chars;


public const NEG: int := 0;
public const NOT: int := 1;
public const HD: int := 2;
public const TL: int := 3;
public const SIZEOF: int := 4;

public const MUL: int := 10;
public const DIV: int := 11;
public const MOD: int := 12;
public const ADD: int := 13;
public const SUB: int := 14;
public const CONS: int := 15;
public const AND: int := 16;
public const OR: int := 17;
public const EQ: int := 18;
public const NE: int := 19;
public const LT: int := 20;
public const LE: int := 21;
public const GT: int := 22;
public const GE: int := 23;
public const ASSIGN: int := 24;

const opnames: array of string := 
{"-", "not", "hd", "tl", "sizeof", "", "", "", "", "",
 "*", "/", "%", "+", "-", "::", "and", "or", "=", "<>", "<", "<=",
 ">", ">=", ":="};

public datatype pos = pos (file: string, line: int, col: int);

public fun put (f: io.file, p: pos)
  io.put (f, file (p));
  io.put (f, ":");
  io.put (f, line (p) + 1);
  io.put (f, ":");
  io.put (f, col (p) + 1);
  io.put (f, ": ");
end;

public datatype ast = error (p: pos, reason: string) or
                      unary (p: pos, op: int, op0: ast) or
		      binary (p: pos, op: int, op0: ast, op1: ast) or
		      str_const (p: pos, value: string) or
		      int_const (p: pos, value: int) or
		      long_const (p: pos, value: long) or
		      real_const (p: pos, value: real) or
		      char_const (p: pos, value: char) or
		      bool_const (p: pos, value: bool) or
		      null_const (p: pos) or
		      identifier (p: pos, value: string) or
		      qualident (p: pos, mod: ast, ident: ast) or
		      annident (p: pos, name: ast, ann: list of ast) or
		      string_type (p: pos) or
		      array_type (p: pos, base: ast) or
		      list_type (p: pos, base: ast) or
		      named_type (p: pos, name: ast, params: list of ast) or
		      void_type (p: pos) or
		      tuple_type (p: pos, types: list of ast) or
		      function_type (p: pos, params: list of ast, ret: ast) or
		      constraint_type (p: pos, base: ast) or
		      variable_type (p: pos, base: ast, binding: ast) or
		      array_constructor (p: pos, size: ast, init: ast) or
		      list_constructor (p: pos, size: ast, init: ast) or
		      string_constructor (p: pos, size: ast, init: ast) or
		      list_expr (p: pos, elems: list of ast) or
		      array_expr (p: pos, elems: list of ast) or
		      fun_expr (p: pos, param: list of ast, ret: ast, 
			body: list of ast) or
		      constraint_expr (p: pos, param: list of ast, 
			body: list of ast) or
		      call (p: pos, func: ast, params: list of ast) or
		      index (p: pos, arr: ast, index: ast) or
		      tuple_expr (p: pos, elems: list of ast) or
		      variable (p: pos, name: ast, typ: ast, init: ast) or
		      vardef (p: pos, pub: bool, vars: list of ast,
		        comment: string) or
		      constdef (p: pos, pub: bool, vars: list of ast,
		        comment: string) or
		      ifstmt (p: pos, guards: list of ast, els: list of ast) or
		      guarded (p: pos, cond: ast, stmts: list of ast) or
		      whilestmt (p: pos, cond: ast, stmts: list of ast) or
		      returnstmt (p: pos, expr: ast) or
		      fundef (p: pos, pub: bool, name: ast, params: list of ast,
		        ret_type: ast, body: list of ast, handcoded: bool,
			mapped: bool, mapping: string, comment: string) or
		      constraintdef (p: pos, pub: bool, name: ast, params: list of ast,
		        body: list of ast, comment: string) or
		      typedef (p: pos, pub: bool, name: ast, typ: ast,
		        comment: string) or
		      datatypefield (p: pos, name: ast, typ: ast) or
		      datatypevariant (p: pos, name: ast, 
		        fields: list of ast) or
		      datatypedef (p: pos, pub: bool, name: ast, 
		        params: list of ast, variants: list of ast,
			comment: string) or
		      moduledef (p: pos, name: ast, imp: list of ast.ast,
			exp: list of ast.ast, defs: list of ast.ast,
			comment: string) or
		      foreign_expr (p: pos, expr: ast) or
		      instmt (p: pos, body: list of ast) or
		      requirestmt (p: pos, conj: list of ast,
		        strengths: list of ast, body: ast);

fun put (f: io.file, op: int)
  io.put (f, opnames[op]);
end;

fun put (f: io.file, al: list of ast, indent: int)
  if al <> null then
    put (f, hd al, indent);
    al := tl al;
    while al <> null do
      io.put (f, ", ");
      put (f, hd al, indent);
      al := tl al;
    end;
  end;
end;

fun indent (f: io.file, spaces: int)
  io.put (f, strings.replicate (' ', spaces));
end;

fun put_comment (f: io.file, s: string, indent: int)
  var lines: list of string := strings.split (s, '\n');
  while lines <> null do
    if sizeof hd lines > 0 or tl lines <> null then
      indent (f, indent);
      io.put (f, "//* ");
      io.put (f, hd lines);
      io.nl (f);
    end;
    lines := tl lines;
  end;
end;

fun put_escaped (f: io.file, c: char)
  if c = '\"' then
    io.put (f, "\\\"");
  elsif c = '\'' then
    io.put (f, "\\\'");
  elsif c = '\\' then
    io.put (f, "\\\\");
  elsif c >= ' ' and c < chars.chr (128) then
    io.put (f, c);
  elsif c = '\n' then
    io.put (f, "\\n");
  elsif c = '\t' then
    io.put (f, "\\t");
  elsif c = '\v' then
    io.put (f, "\\v");
  elsif c = '\b' then
    io.put (f, "\\b");
  elsif c = '\r' then
    io.put (f, "\\r");
  elsif c = '\a' then
    io.put (f, "\\a");
  elsif c = '\f' then
    io.put (f, "\\f");
  else
    io.put (f, "\\");
    io.put (f, chars.ord (c));
  end;
end;

fun put_escaped (f: io.file, s: string)
  var idx: int := 0;
  while idx < sizeof s do
    put_escaped (f, s[idx]);
    idx := idx + 1;
  end;
end;

fun put_stmts (f: io.file, al: list of ast, indent: int)
  while al <> null do
    indent (f, indent);
    put (f, hd al, indent);
    io.put (f, ";\n");
    al := tl al;
  end;
end;

public fun put (f: io.file, a: ast, indent: int)
  var s: string;
  if error? (a) then
    io.put (f, "<error>");
  elsif unary? (a) then
    put (f, op (a));
    io.put (f, " ");
    put (f, op0 (a), indent);
  elsif binary? (a) then
    if binary? (op0 (a)) then
      io.put (f, "(");
    end;
    put (f, op0 (a), indent);
    if binary? (op0 (a)) then
      io.put (f, ")");
    end;
    io.put (f, " ");
    put (f, op (a));
    io.put (f, " ");
    if binary? (op1 (a)) then
      io.put (f, "(");
    end;
    put (f, op1 (a), indent);
    if binary? (op1 (a)) then
      io.put (f, ")");
    end;
  elsif str_const? (a) then
    s := value (a);
    io.put (f, "\"");
    put_escaped (f, s);
    io.put (f, "\"");
  elsif int_const? (a) then
    var i: int := value (a);
    io.put (f, i);
  elsif long_const? (a) then
    var l: long := value (a);
    io.put (f, l);
    io.put (f, "L");
  elsif real_const? (a) then
    var r: real := value (a);
    io.put (f, r);
  elsif char_const? (a) then
    var c: char := value (a);
    io.put (f, "\'");
    put_escaped (f, c);
    io.put (f, "\'");
  elsif bool_const? (a) then
    var b: bool := value (a);
    io.put (f, b);
  elsif null_const? (a) then
    io.put (f, "null");
  elsif identifier? (a) then
    s := value (a);
    io.put (f, s);
  elsif qualident? (a) then
    put (f, mod (a), indent);
    io.put (f, ".");
    put (f, ident (a), indent);
  elsif annident? (a) then 
    put (f, name (a), indent);
    io.put (f, "<");
    put (f, ann (a), indent);
    io.put (f, ">");
  elsif string_type? (a) then
    io.put (f, "string");
  elsif array_type? (a) then
    io.put (f, "array of ");
    put (f, base (a), indent);
  elsif list_type? (a) then
    io.put (f, "list of ");
    put (f, base (a), indent);
  elsif named_type? (a) then
    put (f, name (a), indent);
    if params (a) <> null then
      io.put (f, "<");
      put (f, params (a), indent);
      io.put (f, ">");
    end;
  elsif void_type? (a) then
    io.put (f, "()");
  elsif tuple_type? (a) then
    io.put (f, "(");
    put (f, types (a), indent);
    io.put (f, ")");
  elsif function_type? (a) then
    io.put (f, "fun (");
    put (f, params (a), indent);
    io.put (f, ")");
    if ret (a) <> null then
      io.put (f, ": ");
      put (f, ret (a), indent);
    end;
  elsif constraint_type? (a) then
    io.put (f, "!");
    put (f, base (a), indent);
  elsif variable_type? (a) then
    io.put (f, "`");
    put (f, base (a), indent);
  elsif array_constructor? (a) then
    io.put (f, "array ");
    put (f, size (a), indent);
    io.put (f, " of ");
    put (f, init (a), indent);
  elsif list_constructor? (a) then
    io.put (f, "list ");
    put (f, size (a), indent);
    io.put (f, " of ");
    put (f, init (a), indent);
  elsif string_constructor? (a) then
    io.put (f, "string ");
    put (f, size (a), indent);
    io.put (f, " of ");
    put (f, init (a), indent);
  elsif list_expr? (a) then
    io.put (f, "[");
    put (f, elems (a), indent);
    io.put (f, "]");
  elsif array_expr? (a) then
    io.put (f, "{");
    put (f, elems (a), indent);
    io.put (f, "}");
  elsif fun_expr? (a) then
    io.put (f, "fun (");
    put (f, param (a), indent);
    io.put (f, ")");
    if ret (a) <> null then
      io.put (f, ": ");
      put (f, ret (a), indent);
    end;
    io.put (f, "\n");
    put_stmts (f, body (a), indent + 2);
    indent (f, indent);
    io.put (f, "end");
  elsif constraint_expr? (a) then
    io.put (f, "constraint (");
    put (f, param (a), indent);
    io.put (f, ")\n");
    put_stmts (f, body (a), indent + 2);
    indent (f, indent);
    io.put (f, "end");
  elsif call? (a) then
    put (f, func (a), indent);
    io.put (f, " (");
    put (f, params (a), indent);
    io.put (f, ")");
  elsif index? (a) then
    put (f, arr (a), indent);
    io.put (f, "[");
    put (f, index (a), indent);
    io.put (f, "]");
  elsif tuple_expr? (a) then
    io.put (f, "(");
    put (f, elems (a), indent);
    io.put (f, ")");
  elsif variable? (a) then 
    put (f, name (a), indent);
    io.put (f, ": ");
    put (f, typ (a), indent);
    if init (a) <> null then
      io.put (f, " := ");
      put (f, init (a), indent);
    end;
  elsif vardef? (a) then
    if sizeof comment (a) > 0 then
      put_comment (f, comment (a), indent);
    end;
    if pub (a) then
      io.put (f, "public ");
    end;
    io.put (f, "var ");
    put (f, vars (a), indent);
  elsif constdef? (a) then
    if sizeof comment (a) > 0 then
      put_comment (f, comment (a), indent);
    end;
    if pub (a) then
      io.put (f, "public ");
    end;
    io.put (f, "const ");
    put (f, vars (a), indent);
  elsif ifstmt? (a) then
    var g: list of ast := guards (a);
    io.put (f, "if ");
    put (f, cond (hd g), indent);
    io.put (f, " then\n");
    put_stmts (f, stmts (hd g), indent + 2);
    g := tl g;
    while g <> null do
      indent (f, indent);
      io.put (f, "elsif ");
      put (f, cond (hd g), indent);
      io.put (f, " then\n");
      put_stmts (f, stmts (hd g), indent + 2);
      g := tl g;
    end;
    if els (a) <> null then
      indent (f, indent);
      io.put (f, "else\n");
      put_stmts (f, els (a), indent + 2);
    end;
    indent (f, indent);
    io.put (f, "end");
  elsif whilestmt? (a) then
    io.put (f, "while ");
    put (f, cond (a), indent);
    io.put (f, " do\n");
    put_stmts (f, stmts (a), indent + 2);
    indent (f, indent);
    io.put (f, "end");
  elsif returnstmt? (a) then
    io.put (f, "return");
    if expr (a) <> null then
      io.put (f, " ");
      put (f, expr (a), indent);
    end;
  elsif fundef? (a) then
    if sizeof comment (a) > 0 then
      put_comment (f, comment (a), indent);
    end;
    if pub (a) then
      io.put (f, "public ");
    end;
    io.put (f, "fun ");
    put (f, name (a), indent);
    io.put (f, " (");
    put (f, params (a), indent);
    io.put (f, ")");
    if ret_type (a) <> null then
      io.put (": ");
      put (f, ret_type (a), indent);
    end;
    if handcoded (a) then
    elsif mapped (a) then
      io.put (f, " = ");
      io.put (f, mapping (a));
    else
      io.put (f, "\n");
      put_stmts (f, body (a), indent + 2);
      indent (f, indent);
      io.put (f, "end");
    end;
  elsif constraintdef? (a) then
    if sizeof comment (a) > 0 then
      put_comment (f, comment (a), indent);
    end;
    if pub (a) then
      io.put (f, "public ");
    end;
    io.put (f, "constraint ");
    put (f, name (a), indent);
    io.put (f, " (");
    put (f, params (a), indent);
    io.put (f, ")\n");
    put_stmts (f, body (a), indent + 2);
    indent (f, indent);
    io.put (f, "end");
  elsif typedef? (a) then
    if sizeof comment (a) > 0 then
      put_comment (f, comment (a), indent);
    end;
    if pub (a) then
      io.put (f, "public ");
    end;
    io.put (f, "type ");
    put (f, name (a), indent);
    io.put (f, " = ");
    put (f, typ (a), indent);
  elsif datatypefield? (a) then 
    put (f, name (a), indent);
    io.put (f, ": ");
    put (f, typ (a), indent);
  elsif datatypevariant? (a) then
    put (f, name (a), indent);
    if fields (a) <> null then
      io.put (f, " (");
      put (f, fields (a), indent);
      io.put (f, ")");
    end;
  elsif datatypedef? (a) then 
    if sizeof comment (a) > 0 then
      put_comment (f, comment (a), indent);
    end;
    if pub (a) then
      io.put (f, "public ");
    end;
    io.put (f, "datatype ");
    put (f, name (a), indent);
    if params (a) <> null then
      io.put (f, "<");
      put (f, params (a), indent);
      io.put (f, ">");
    end;
    io.put (f, " = ");
    var l: list of ast := variants (a);
    put (f, hd l, indent);
    l := tl l;
    while l <> null do
      if l <> null then
	io.put (f, " or\n");
      end;
      indent (f, indent + 2);
      put (f, hd l, indent);
      l := tl l;
    end;
  elsif moduledef? (a) then
    if sizeof comment (a) > 0 then
      put_comment (f, comment (a), indent);
    end;
    io.put (f, "module ");
    put (f, name (a), indent);
    io.put (f, ";\n\n");
    if imp (a) <> null then
      io.put (f, "import ");
      put (f, imp (a), indent);
      io.put (f, ";\n\n");
    end;
    if exp (a) <> null then
      io.put (f, "export ");
      put (f, exp (a), indent);
      io.put (f, ";\n\n");
    end;
    var d: list of ast := defs (a);
    while d <> null do
      put (f, hd d, indent);
      io.put (f, ";\n\n");
      d := tl d;
    end;
  elsif foreign_expr? (a) then
    io.put (f, "foreign ");
    put (f, expr (a), indent);
  elsif instmt? (a) then
    io.put (f, "in\n");
    put_stmts (f, body (a), indent + 2);
    indent (f, indent);
    io.put (f, "end");
  elsif requirestmt? (a) then
    io.put (f, "require ");
    var cl: list of ast := conj (a);
    var cs: list of ast := strengths (a);
    put (f, hd cl, indent);
    if hd cs <> null then
      io.put (f, " : ");
      put (f, hd cs, indent);
    end;
    cl := tl cl;
    cs := tl cs;
    while cl <> null do
      io.put (" and ");
      put (f, hd cl, indent);
      if hd cs <> null then
	io.put (f, " : ");
	put (f, hd cs, indent);
      end;
      cl := tl cl;
      cs := tl cs;
    end;
    var b: ast := body (a);
    if b <> null then
      io.put (f, " ");
      put (f, b, indent + 2);
    end;
  end;
end;

public fun rev (l: list of ast): list of ast
  fun rev_it (l: list of ast, r: list of ast): list of ast
    if l = null then
      return r;
    else
      return rev_it (tl l, hd l :: r);
    end;
  end;
  return rev_it (l, []);
end;

public fun strip_annotation (id: ast.ast): ast.ast
  while ast.annident? (id) do
    id := ast.name (id);
  end;
  return id;
end;

//* Return true if the identifiers @var{id0} and @var{id1} are
//* equal when all annotaions have been removed.
//
public fun eq_identifiers (id0: ast.ast, id1: ast.ast): bool
  id0 := strip_annotation (id0);
  id1 := strip_annotation (id1);
  if ast.identifier? (id0) and ast.identifier? (id1) then
    return strings.eq (ast.value (id0), ast.value (id1));
  elsif ast.qualident? (id0) and ast.qualident? (id1) then
    return eq_identifiers (ast.mod (id0), ast.mod (id1)) and
           eq_identifiers (ast.ident (id0), ast.ident (id1));
  else
    return false;
  end;
end;

//* Convert the (possibly) qualified and annotated identifier
//* @var{q} to a string, where @var{sep} is used to separate the
//* components of qualified names.  This is for example used to
//* construct file names from module names.
//
public fun qualident_to_string (q: ast.ast, sep: string): string
  if ast.identifier? (q) then
    return ast.value (q);
  elsif ast.annident? (q) then
    return qualident_to_string (ast.name (q), sep);
  else
    return qualident_to_string (ast.mod (q), sep) + sep + 
                                qualident_to_string (ast.ident (q), sep);
  end;
end;

// End of ast.t.
