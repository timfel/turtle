// interpret.t -- Interpreter for mini language.
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

//* This is an interpreter for a minimal programming language,
//* basically resembling untyped lambda calculus with constants.
//*
//* Not very useful yet, because no parser is provided.

module interpret;

import io, strings;

datatype env = env (name: string, value: expr, parent: env);

datatype expr = variable (name: string) or
                intconst (value: int) or
                strconst (value: string) or
		call (function: expr, arg: expr) or
		closure (function: expr, env: env) or
		lambda (variable: string, body: expr) or
		put (arg: expr);

fun lookup (env: env, n: string): expr
  while env <> null do
    if strings.eq (name (env), n) then
      return value (env);
    end;
    env := parent (env);
  end;
  return null;
end;

fun extend (env: env, s: string, e: expr): env
  return env (s, e, env);
end;
  
fun interpret (env: env, expr: expr): expr
  if intconst? (expr) or strconst? (expr) then
    return expr;
  elsif variable? (expr) then
    return lookup (env, name (expr));
  elsif call? (expr) then
    var f: expr := interpret (env, function (expr));
    var a: expr := interpret (env, arg (expr));
    return interpret (extend (env (f), 
                              variable (function (f)), a), 
                      body (function (f)));
  elsif lambda? (expr) then
    return closure (expr, env);
  elsif put? (expr) then
    expr := interpret (env, arg (expr));
    if intconst? (expr) then
      var i: int := value (expr);
      io.put (i);
    elsif strconst? (expr) then
      var s: string := value (expr);
      io.put (s);
    elsif variable? (expr) then
      io.put ("<variable>");
    elsif call? (expr) then
      io.put ("<call>");
    elsif closure? (expr) then
      io.put ("<closure>");
    elsif lambda? (expr) then
      io.put ("<lambda>");
    end;
    io.nl ();
    return expr;
  end;
  return expr;
end;

fun main (args: list of string): int
  var inp: expr := put (call (lambda ("x", variable ("x")), variable ("y")));
  var res: expr;

  res := interpret (extend (null, "y", intconst (3)), inp);
  return 0;
end;

// End of interpret.t.
