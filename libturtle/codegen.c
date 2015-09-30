/* libturtle/codegen.c -- Code generator.
 
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


#include <stdio.h>

#include "codegen.h"
#include "il.h"
#include "util.h"
#include "error.h"
#include "version.h"

#define TICKS 1

static char * opcode_names[] =
  {
    "load",
    "store",
    "push",
    "pop",
    "add",
    "sub",
    "mul",
    "div",
    "mod",
    "fadd",
    "fsub",
    "fmul",
    "fdiv",
    "fmod",
    "ladd",
    "lsub",
    "lmul",
    "ldiv",
    "lmod",
    "j",
    "j-proc",
    "jf",
    "jt",
    "call",
    "label",
    "cont-label",
    "proc-label",
    "note-label",
    "save",
    "restore",
    "je",
    "jne",
    "jl",
    "jnl",
    "jg",
    "jng",
    "fje",
    "fjne",
    "fjl",
    "fjnl",
    "fjg",
    "fjng",
    "lje",
    "ljne",
    "ljl",
    "ljnl",
    "ljg",
    "ljng",
    "not",
    "neg",
    "fneg",
    "lneg",
    "hd",
    "tl",
    "sizeof",
    "cons",
    "aload",
    "astore",
    "sload",
    "sstore",
    "load-int",
    "load-long",
    "load-null",
    "load-false",
    "load-true",
    "load-real",
    "load-string",
    "load-char",
    "make-env",
    "gc-check",
    "make-closure",
    "macro-call",
    "null-env-reg",
    "make-array",
    "make-constrained-array",
    "make-string",
    "make-list",
    "make-tuple",
    "make-data",
    "tuple-ref",
    "tuple-set",
    "create-array",
    "array-pop",
    "pop-env",
    "mapped-call",
    "dup",
    "over",
    "drop",
    "raise",
    "concat",
    "null-check",
    "add-int-constraint",
    "add-real-constraint",
    "resolve-int-constraint",
    "resolve-real-constraint",
    "remove-constraint",
    "tick",
    "make-int-variable",
    "make-real-variable",
    "variable-ref",
    "variable-set",
    "coerce-to-constrained-array",
    "coerce-to-constrained-list",
    "load-foreign"
  };

static int load_constrainable_variables = 0;
static int compiling_constraint = 0;

/* static int static_sp_value = 0; */

ttl_operand
ttl_make_operand (ttl_pool pool, enum ttl_operand_kind op, void * data)
{
  ttl_operand operand = ttl_malloc (pool, sizeof (struct ttl_operand));

  operand->op = op;
  operand->data = data;
  operand->unsigned_data = 0;
  return operand;
}

void
ttl_disassemble_operand (FILE * f, ttl_operand operand)
{
  switch (operand->op)
    {
    case operand_label:
      fprintf (f, "L%d", (int) operand->data);
      break;
    case operand_constant:
      fprintf (f, "#%d", (int) operand->data);
      break;
    case operand_local:
      fprintf (f, "env(%d, %d)", operand->unsigned_data, (int) operand->data);
      break;
    case operand_mem:
      ttl_symbol_print (f, (ttl_symbol) operand->data);
      break;
    }
}

ttl_instruction
ttl_make_instruction (ttl_pool pool, enum ttl_op_kind op,
		      ttl_operand op0, ttl_operand op1,
		      char * filename, int line)
{
  ttl_instruction instr = ttl_malloc (pool, sizeof (struct ttl_instruction));

  instr->op = op;
  instr->op0 = op0;
  instr->op1 = op1;
  instr->prev = NULL;
  instr->next = NULL;
  instr->filename = filename;
  instr->line = line;
  return instr;
}

ttl_operand
ttl_make_new_label (ttl_compile_state state)
{
  return ttl_make_operand (state->pool,
			   operand_label, (void *) state->next_label++);
}

ttl_instruction
ttl_make_new_label_stmt (ttl_compile_state state)
{
  return ttl_make_instruction (state->pool, op_label,
			       ttl_make_new_label (state), NULL, NULL, -1);
}

ttl_instruction
ttl_make_new_cont_label_stmt (ttl_compile_state state)
{
  return ttl_make_instruction (state->pool, op_cont_label,
			       ttl_make_new_label (state), NULL, NULL, -1);
}

ttl_instruction
ttl_make_new_proc_label_stmt (ttl_compile_state state)
{
  return ttl_make_instruction (state->pool, op_proc_label,
			       ttl_make_new_label (state), NULL, NULL, -1);
}

ttl_instruction
ttl_make_new_note_label_stmt (ttl_compile_state state)
{
  return ttl_make_instruction (state->pool, op_note_label,
			       ttl_make_new_label (state), NULL, NULL, -1);
}

ttl_instruction
ttl_make_label_stmt (ttl_compile_state state, ttl_operand label)
{
  return ttl_make_instruction (state->pool, op_label, label, NULL, NULL, -1);
}

ttl_instruction
ttl_make_cont_label_stmt (ttl_compile_state state, ttl_operand label)
{
  return ttl_make_instruction (state->pool, op_cont_label, label,
			       NULL, NULL, -1);
}

ttl_instruction
ttl_make_proc_label_stmt (ttl_compile_state state, ttl_operand label)
{
  return ttl_make_instruction (state->pool, op_proc_label, label,
			       NULL, NULL, -1);
}

ttl_instruction
ttl_make_note_label_stmt (ttl_compile_state state, ttl_operand label)
{
  return ttl_make_instruction (state->pool, op_note_label, label,
			       NULL, NULL, -1);
}

void
ttl_disassemble_instruction (FILE * f, ttl_instruction instr)
{
  if (instr->op == op_label || instr->op == op_cont_label ||
      instr->op == op_proc_label || instr->op == op_note_label)
    {
      ttl_disassemble_operand (f, instr->op0);
      fprintf (f, ":");
    }
  else
    {
      fprintf (f, "\t%s", opcode_names[(int)instr->op]);
      if (instr->op0)
	{
	  fprintf (f, "\t");
	  ttl_disassemble_operand (f, instr->op0);
	}
      if (instr->op1)
	{
	  fprintf (f, ", ");
	  ttl_disassemble_operand (f, instr->op1);
	}
    }
  fprintf (f, "\n");
}

ttl_object
ttl_make_object (ttl_pool pool)
{
  ttl_object obj = ttl_malloc (pool, sizeof (struct ttl_object));
  obj->first = NULL;
  obj->last = NULL;
  return obj;
}

void
ttl_disassemble_object (FILE * f, ttl_object obj)
{
  ttl_instruction instr = obj->first;
  while (instr)
    {
      ttl_disassemble_instruction (f, instr);
      instr = instr->next;
    }
}

void
ttl_append_instruction (ttl_object obj, ttl_instruction instr)
{
  if (obj->first)
    {
      instr->prev = obj->last;
      obj->last->next = instr;
      obj->last = instr;
    }
  else
    {
      obj->first = instr;
      obj->last = instr;
    }
}

static void
compile_link (ttl_compile_state state, ttl_object obj, enum ttl_link link,
	      ttl_operand target)
{
  switch (link)
    {
    case link_next:
      break;
    case link_return:
      ttl_append_instruction (obj,
			      ttl_make_instruction (state->pool,
						    op_restore_cont,
						    NULL, NULL, NULL, -1));
      break;
    case link_label:
      ttl_append_instruction (obj,
			      ttl_make_instruction (state->pool,
						    op_jump,
						    target, NULL, NULL, -1));
      break;
    case link_proc:
      ttl_append_instruction (obj,
			      ttl_make_instruction (state->pool,
						    op_call,
						    NULL, NULL, NULL, -1));
      break;
    }
}

static void
compile_expr (ttl_compile_state state, ttl_object obj, ttl_il_node node,
	      enum ttl_link link, ttl_operand target,
	      int sp_value);

/* Allocate an array of instruction pointers which is large enough to
   hold `mapping_count' pointers.  Initialize with NULLs.  The mapping
   array is needed to map label numbers to instructions and then to
   source code locations for generating debug information.  */
static ttl_instruction *
make_label_mapping (ttl_compile_state state, int mapping_count)
{
  int i;
  ttl_instruction * mapping = ttl_malloc (state->pool, state->label_count *
					  sizeof (ttl_instruction));
  for (i = 0; i < state->label_count; i++)
    mapping[i] = NULL;
  return mapping;
}

/* Add all label instructions of the code for function `func' to the
   mapping array `mapping'.  */
static void
enter_mapping (ttl_compile_state state, ttl_function func,
	       ttl_instruction * mapping, int mapping_count)
{
  ttl_instruction instr = ((ttl_object) func->asm_code)->first;
  while (instr)
    {
      if (instr->op == op_label || instr->op == op_cont_label ||
	  instr->op == op_proc_label || instr->op == op_note_label)
	{
	  int index = (int) instr->op0->data;
	  if (index >= 0 && index < mapping_count)
	    mapping[index] = instr;
	  else 
	    abort ();
	}
      instr = instr->next;
    }
}


/* Go through the code for function `function' and replace all
   references to functions (which are pointers to their function
   structures on generation time) with their integer entry point label
   number.  This is needed because we can encounter forward references
   while compiling direct function calls or closure expressions.  */
static void
fixup_calls (ttl_compile_state state, ttl_function function)
{
  ttl_object obj = (ttl_object) function->asm_code;
  ttl_instruction instr = obj->first;

  while (instr)
    {
      if (instr->op == op_jump_proc)
	{
	  ttl_function f = (ttl_function) (instr->op0->data);
	  instr->op0 = ttl_make_operand (state->pool, operand_label,
					 (void *) f->index);
	}
      else if (instr->op == op_make_closure)
	{
	  ttl_function f = (ttl_function) (instr->op0->data);
	  instr->op0 = ttl_make_operand (state->pool, operand_label,
					 (void *) f->index);
	}
      instr = instr->next;
    }
}

/* Append a heap check instruction which requires `words' words of
   heap space to the code list in `obj'.  */
static void
append_gc_check (ttl_compile_state state, ttl_object obj, int words)
{
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool, op_gc_check,
      ttl_make_operand (state->pool, operand_constant,
			(void *) ((words + 1) & ~1)),
      NULL, NULL, -1));
}


static void compile_constructor_body (ttl_compile_state state,
				      ttl_function function, ttl_object obj,
				      enum ttl_link link,
				      ttl_operand target);
static void compile_discriminator_body (ttl_compile_state state,
					ttl_function function, ttl_object obj,
					enum ttl_link link,
					ttl_operand target);
static void compile_accessor_body (ttl_compile_state state,
				   ttl_function function, ttl_object obj,
				   enum ttl_link link,
				   ttl_operand target);
static void compile_setter_body (ttl_compile_state state,
				 ttl_function function, ttl_object obj,
				 enum ttl_link link,
				 ttl_operand target);

static void
compile_parameters (ttl_compile_state state, ttl_il_node param,
		    ttl_object obj, int sp_value)
{
  int param_count = 0;
  while (param)
    {
#if 0
      if (param->d.pair.car->filename)
	{
	  ttl_instruction instr;
	  instr = ttl_make_new_note_label_stmt (state);
	  instr->filename = param->d.pair.car->filename;
	  instr->line = param->d.pair.car->start_line;
	  ttl_append_instruction (obj, instr);
	}
#endif
      compile_expr (state, obj, param->d.pair.car, link_next, NULL, sp_value);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_push, NULL, NULL,
			       param->d.pair.car->filename,
			       param->d.pair.car->start_line));
      sp_value++;
      param_count++;
      param = param->d.pair.cdr;
    }
}

static void
compile_call (ttl_compile_state state, ttl_object obj, ttl_il_node node,
	      enum ttl_link link, ttl_operand target, int sp_value)
{
  ttl_il_node param = node->d.call.args;
  ttl_operand cont_label = ttl_make_new_label (state);

  if (state->compile_options->opt_local_jumps &&
      node->d.call.function->kind == il_function)
    {
      ttl_function f = state->current_module->functions;
      while (f &&
	     node->d.call.function->d.function.mangled_name != f->unique_name)
	{
	  f = f->total_next;
	}
      if (f)
	{
	  if (!state->compile_options->opt_inline_constructors)
	    goto compile_normal_call;
	  switch (f->kind)
	    {
	    case function_constructor:
	      compile_parameters (state, node->d.call.args, obj, sp_value);
	      compile_constructor_body (state, f, obj, link, target);
	      break;
	    case function_discriminator:
	      compile_parameters (state, node->d.call.args, obj, sp_value);
	      compile_discriminator_body (state, f, obj, link, target);
	      break;
	    case function_accessor:
	      compile_parameters (state, node->d.call.args, obj, sp_value);
	      compile_accessor_body (state, f, obj, link, target);
	      break;
	    case function_setter:
	      compile_parameters (state, node->d.call.args, obj, sp_value);
	      compile_setter_body (state, f, obj, link, target);
	      break;
	    default:
	    compile_normal_call:
	      if (link != link_return)
		{
		  append_gc_check (state, obj, 5 + sp_value);
		  ttl_append_instruction
		    (obj,
		     ttl_make_instruction (state->pool, op_save_cont,
					   cont_label, 
					   ttl_make_operand
					   (state->pool,
					    operand_constant,
					    (void *) sp_value),
					   node->filename, node->start_line));
		  sp_value = 0;
		}
	      compile_parameters (state, node->d.call.args, obj, sp_value);
	      if (state->current_function->d.function.nesting_level >=
		  f->d.function.nesting_level)
		{
		  ttl_append_instruction
		    (obj,
		     ttl_make_instruction
		     (state->pool,
		      op_pop_env,
		      ttl_make_operand
		      (state->pool, operand_constant,
		       (void *)
		       (state->current_function->d.function.nesting_level
			- f->d.function.nesting_level + 1)),
		      NULL, NULL, -1));
		}
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool,
		  op_jump_proc,
		  ttl_make_operand (state->pool, operand_label,
				    (void *) f),
		  NULL, node->d.call.function->filename, 
		  node->d.call.function->start_line));
	      if (link != link_return)
		{
		  ttl_append_instruction
		    (obj,
		     ttl_make_cont_label_stmt (state, cont_label));
		  compile_link (state, obj, link, target);
		}
	      break;
	    }
	  return;
	}
    }

  if (link != link_return)
    {
      append_gc_check (state, obj, 5 + sp_value);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_save_cont, cont_label, 
			       ttl_make_operand (state->pool,
						 operand_constant,
						 (void *) sp_value),
			       node->filename, node->start_line));
      sp_value = 0;
    }

  compile_parameters (state, node->d.call.args, obj, sp_value);

  compile_expr (state, obj, node->d.call.function, link_proc, NULL, sp_value);
  if (link != link_return)
    {
      ttl_append_instruction
	(obj,
	 ttl_make_cont_label_stmt (state, cont_label));
      compile_link (state, obj, link, target);
    }
}


static void
compile_bool_expr (ttl_compile_state state, ttl_object obj, ttl_il_node node,
		   ttl_operand true_target,
		   ttl_operand false_target,
		   int sp_value)
{
  if (!node)
    return ;

  switch (node->kind)
    {
    case il_error:
      fprintf (stderr, "compile_bool_expr: error node encountered\n");
      break;

    case il_variable:
      {
	compile_expr (state, obj, node, link_next, NULL, sp_value);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_jump_if_false, false_target, NULL, NULL, -1));
	compile_link (state, obj, link_label, true_target);
      }
      break;

    case il_bool_const:
      {
	if (node->d.bool.value)
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool, op_jump, true_target, NULL, NULL, -1));
	else
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool, op_jump, false_target, NULL, NULL, -1));
#if 0
	if (node->d.bool.value)
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool, op_load_true, NULL, NULL, NULL, -1));
	else
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool, op_load_false, NULL, NULL, NULL, -1));
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_jump_if_false, false_target, NULL, NULL, -1));
	compile_link (state, obj, link_label, true_target);
#endif
      }
      break;

    case il_foreign_expr:
      {
	compile_expr (state, obj, node, link_next, NULL, sp_value);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_jump_if_false, false_target, NULL, NULL, -1));
	compile_link (state, obj, link_label, true_target);
      }
      break;

    case il_binop:
      switch (node->d.binop.op)
	{
	case il_binop_and:
	  {
	    ttl_operand next = ttl_make_new_label (state);
	    compile_bool_expr (state, obj,
			       node->d.binop.op0,
			       next,
			       false_target, sp_value);
	    ttl_append_instruction
	      (obj,
	       ttl_make_label_stmt (state, next));
	    compile_bool_expr (state, obj,
			       node->d.binop.op1,
			       true_target,
			       false_target, sp_value);
	  }
	  break;
	case il_binop_or:
	  {
	    ttl_operand next = ttl_make_new_label (state);
	    compile_bool_expr (state, obj,
			       node->d.binop.op0,
			       true_target,
			       next, sp_value);
	    ttl_append_instruction
	      (obj,
	       ttl_make_label_stmt (state, next));
	    compile_bool_expr (state, obj,
			       node->d.binop.op1,
			       true_target,
			       false_target, sp_value);
	  }
	  break;
	case il_binop_eq:
	case il_binop_ne:
	case il_binop_lt:
	case il_binop_le:
	case il_binop_gt:
	case il_binop_ge:
	  {
	    compile_expr (state, obj, node->d.binop.op0, link_next, NULL,
			  sp_value);
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_push, NULL, NULL, NULL,
				     -1));
	    compile_expr (state, obj, node->d.binop.op1, link_next, NULL,
			  sp_value + 1);
	    
	    if (node->d.binop.op0->type->kind == type_real)
	      {
		switch (node->d.binop.op)
		  {
		  case il_binop_eq:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_fequal, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_ne:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_not_fequal, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_lt:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_fless, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_le:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_not_fgtr, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_gt:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_fgtr, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_ge:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_not_fless, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_add:
		  case il_binop_sub:
		  case il_binop_or:
		  case il_binop_and:
		  case il_binop_mul:
		  case il_binop_div:
		  case il_binop_mod:
		  case il_binop_assign:
		  case il_binop_cons:
		    fprintf
		      (stderr,
		       "compile_bool_expr: invalid binop encountered (2)\n");
		    abort ();
		    break;
		  }
	      }
	    else if (node->d.binop.op0->type->kind == type_long)
	      {
		switch (node->d.binop.op)
		  {
		  case il_binop_eq:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_lequal, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_ne:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_not_lequal, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_lt:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_lless, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_le:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_not_lgtr, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_gt:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_lgtr, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_ge:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_not_lless, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_add:
		  case il_binop_sub:
		  case il_binop_or:
		  case il_binop_and:
		  case il_binop_mul:
		  case il_binop_div:
		  case il_binop_mod:
		  case il_binop_assign:
		  case il_binop_cons:
		    fprintf
		      (stderr,
		       "compile_bool_expr: invalid binop encountered (2)\n");
		    abort ();
		    break;
		  }
	      }
	    else
	      {
		switch (node->d.binop.op)
		  {
		  case il_binop_eq:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_equal, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_ne:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_not_equal, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_lt:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_less, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_le:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_not_gtr, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_gt:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_gtr, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_ge:
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool,
			op_jump_if_not_less, true_target, NULL, NULL, -1));
		    break;
		  case il_binop_add:
		  case il_binop_sub:
		  case il_binop_or:
		  case il_binop_and:
		  case il_binop_mul:
		  case il_binop_div:
		  case il_binop_mod:
		  case il_binop_assign:
		  case il_binop_cons:
		    fprintf
		      (stderr,
		       "compile_bool_expr: invalid binop encountered (1)\n");
		    abort ();
		    break;
		  }
	      }
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction
	       (state->pool,
		op_jump, false_target, NULL, NULL, -1));
	    break;

	  case il_binop_add:
	  case il_binop_sub:
	  case il_binop_mul:
	  case il_binop_div:
	  case il_binop_mod:
	  case il_binop_assign:
	  case il_binop_cons:
	    fprintf (stderr,
		     "compile_bool_expr: invalid binop encountered (3)\n");
	    abort ();
	    break;
	  }
	}
      break;

    case il_unop:
      switch (node->d.unop.op)
	{
	case il_unop_not:
	  compile_bool_expr (state, obj, node->d.unop.op0,
			     false_target, true_target, sp_value);
	  break;
	case il_unop_neg:
	case il_unop_hd:
	case il_unop_tl:
	case il_unop_sizeof:
	  fprintf (stderr,
		   "compile_bool_expr: invalid unnop encountered (2)\n");
	  abort ();
	  break;
	}
      break;
  
    case il_call:
      {
	compile_call (state, obj, node, link_next, NULL, sp_value);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_jump_if_false, false_target, NULL, NULL, -1));
	compile_link (state, obj, link_label, true_target);
      }
      break;

    case il_index:
      {
	compile_expr (state, obj, node->d.index.array, link_next,
		      NULL, sp_value);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_push, NULL, NULL, NULL, -1));
	compile_expr (state, obj, node->d.index.index, link_next, NULL,
		      sp_value + 1);
#if AUTO_DEREF
	if (node->d.index.array->type->d.array.element->kind ==
	    type_constrained)
	  {
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_aload, NULL, NULL,
				     NULL, -1));
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_variable_ref, NULL, NULL,
				     NULL, -1));
	  }
	else
#endif
	  {
	    if (node->d.index.array->type == state->string_type)
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool, op_sload, NULL, NULL,
				       NULL, -1));
	    else
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool, op_aload, NULL, NULL,
				       NULL, -1));
	  }
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_jump_if_false, false_target, NULL, NULL, -1));
	compile_link (state, obj, link_label, true_target);
      }
      break;

    case il_module:
    case il_pair:
    case il_string_const:
    case il_function:
    case il_char_const:
    case il_int_const:
    case il_long_const:
    case il_real_const:
    case il_null_const:
    case il_if:
    case il_while:
    case il_in:
    case il_return:
    case il_require:
    case il_array_expr:
    case il_list_expr:
    case il_array_constructor:
    case il_list_constructor:
    case il_string_constructor:
    case il_tuple_expr:
    case il_seq:
    case il_ann_expr:
    case il_var_expr:
    case il_deref_expr:
      fprintf (stderr,
	       "compile_bool_expr: invalid HIL node encountered: %d\n",
	       node->kind);
      ttl_il_print (stderr, node, 0);
      fprintf (stderr, "\n");
      abort ();
    }
}

static ttl_type
unconstrain (ttl_type t)
{
  if (t->kind == type_constrained)
    return t->d.constrained.base;
  else
    return t;
}

static void
compile_expr (ttl_compile_state state, ttl_object obj, ttl_il_node node,
	      enum ttl_link link, ttl_operand target, int sp_value)
{
  if (!node)
    return ;

  switch (node->kind)
    {
    case il_error:
      fprintf (stderr, "compile_expr: error node encountered\n");
      abort ();
      break;

    case il_string_const:
      {
	append_gc_check (state, obj, (node->d.string.length + 1) / 2);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_load_string, 
				 ttl_make_operand
				 (state->pool,
				  operand_constant,
				  node->d.string.text),
				 ttl_make_operand
				 (state->pool,
				  operand_constant,
				  (void *) node->d.string.length), 
				 node->filename, node->start_line));
	compile_link (state, obj, link, target);
	break;
      }

    case il_variable:
      if (node->d.variable.variable)
	{
	  ttl_operand var;
	  switch (node->d.variable.variable->kind)
	    {
	    case variable_local:
	    case variable_param:
	      {
		unsigned over = 0;
		ttl_function defining = node->d.variable.variable->defining;
		ttl_function curr = state->current_function;
		var = ttl_make_operand
		  (state->pool, operand_local,
		   (void *) node->d.variable.variable->index);
		while (curr != defining)
		  {
		    curr = curr->enclosing;
		    over++;
		  }
		if (over > 0)
		  var->unsigned_data = over;
		break;
	      }

	    default:
	      var = ttl_make_operand
		(state->pool, operand_mem,
		 (void *) node->d.variable.mangled_name);
	      break;
	    }
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction (state->pool, op_load, var, NULL, 
				   node->filename, node->start_line));
	}
      else
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_load,
	    ttl_make_operand (state->pool,
			      operand_mem,
			      (void *) node->d.variable.mangled_name), NULL,
	    node->filename, node->start_line));
#if AUTO_DEREF
      if (node->type->kind == type_constrained &&
	  !load_constrainable_variables)
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_variable_ref, NULL, NULL, NULL, -1));
#endif
      compile_link (state, obj, link, target);
      break;

    case il_function:
      {
	ttl_function func = node->d.function.function;
	if (func->enclosing)
	  {
	    append_gc_check (state, obj, 4);
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction
	       (state->pool, op_make_closure,
		ttl_make_operand (state->pool, operand_label,
				  (void *) func), NULL, NULL, -1));
	  }
	else
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool, op_load,
	      ttl_make_operand (state->pool,
				operand_mem,
				(void *) node->d.function.mangled_name),
	      NULL, NULL, -1));
	compile_link (state, obj, link, target);
      }
      break;

    case il_char_const:
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool, op_load_char,
	  ttl_make_operand (state->pool,
			    operand_constant,
			    (void *) node->d.character.value), NULL,
	  node->filename, node->start_line));
      compile_link (state, obj, link, target);
      break;
      
    case il_int_const:
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool, op_load_int,
	  ttl_make_operand (state->pool,
			    operand_constant,
			    (void *) node->d.integer.value), NULL, 
	  node->filename, node->start_line));
      compile_link (state, obj, link, target);
      break;

    case il_long_const:
      append_gc_check (state, obj, 4);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool, op_load_long,
	  ttl_make_operand (state->pool,
			    operand_constant,
			    (void *) &(node->d.longint.value)),
	  NULL, NULL, -1));
      compile_link (state, obj, link, target);
      break;

    case il_real_const:
      append_gc_check (state, obj, 4);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool, op_load_real,
	  ttl_make_operand (state->pool,
			    operand_constant,
			    (void *) &(node->d.real.value)), NULL, NULL, -1));
      compile_link (state, obj, link, target);
      break;

    case il_bool_const:
      if (node->d.bool.value)
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_load_true, NULL, NULL, NULL, -1));
      else
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_load_false, NULL, NULL, NULL, -1));
      compile_link (state, obj, link, target);
      break;

    case il_null_const:
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool, op_load_null, NULL, NULL, NULL, -1));
      compile_link (state, obj, link, target);
      break;

    case il_binop:
      switch (node->d.binop.op)
	{
	case il_binop_add:
	case il_binop_sub:
	case il_binop_mul:
	case il_binop_div:
	case il_binop_mod:
	  {
	    compile_expr (state, obj, node->d.binop.op0, link_next, NULL,
			  sp_value);
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction
	       (state->pool, op_push, NULL, NULL, NULL, -1));
	    compile_expr (state, obj, node->d.binop.op1, link_next, NULL,
			  sp_value + 1);
	    if (unconstrain (node->type)->kind == type_real)
	      append_gc_check (state, obj, 4);
	    else if (unconstrain (node->type)->kind == type_long)
	      append_gc_check (state, obj, 4);
	    switch (node->d.binop.op)
	      {
	      case il_binop_add:
		ttl_append_instruction
		  (obj,
		   ttl_make_instruction
		   (state->pool, 
		    unconstrain(node->type)->kind == type_integer ? op_add : 
		    unconstrain(node->type)->kind == type_string ? op_concat : 
		    unconstrain(node->type)->kind == type_long ? op_ladd :
		    op_fadd,
		    NULL, NULL, NULL, -1));
		break;
	      case il_binop_sub:
		ttl_append_instruction
		  (obj,
		   ttl_make_instruction
		   (state->pool,
		    unconstrain(node->type)->kind == type_integer ? op_sub : 
		    unconstrain(node->type)->kind == type_long ? op_lsub :
		    op_fsub,
		    NULL, NULL, NULL, -1));
		break;
	      case il_binop_mul:
		ttl_append_instruction
		  (obj,
		   ttl_make_instruction
		   (state->pool,
		    unconstrain(node->type)->kind == type_integer ? op_mul : 
		    unconstrain(node->type)->kind == type_long ? op_lmul : 
		    op_fmul,
		    NULL, NULL, NULL, -1));
		break;
	      case il_binop_div:
		ttl_append_instruction
		  (obj,
		   ttl_make_instruction
		   (state->pool,
		    unconstrain(node->type)->kind == type_integer ? op_div :
		    unconstrain(node->type)->kind == type_long ? op_ldiv :
		    op_fdiv,
		    NULL, NULL, NULL, -1));
		break;
	      case il_binop_mod:
		ttl_append_instruction
		  (obj,
		   ttl_make_instruction
		   (state->pool,
		    unconstrain(node->type)->kind == type_integer ? op_mod :
		    unconstrain(node->type)->kind == type_long ? op_lmod :
		    op_fmod,
		    NULL, NULL, NULL, -1));
		break;

	      case il_binop_and:
	      case il_binop_or:
	      case il_binop_eq:
	      case il_binop_ne:
	      case il_binop_lt:
	      case il_binop_le:
	      case il_binop_gt:
	      case il_binop_ge:
	      case il_binop_assign:
	      case il_binop_cons:
		fprintf (stderr,
			 "compile_expr: invalie binop encountered (1)\n");
		abort ();
		break;
	      }
	    compile_link (state, obj, link, target);
	  }
	  break;

	case il_binop_and:
	case il_binop_or:
	case il_binop_eq:
	case il_binop_ne:
	case il_binop_lt:
	case il_binop_le:
	case il_binop_gt:
	case il_binop_ge:
	  {
	    ttl_operand t_lab = ttl_make_new_label (state);
	    ttl_operand f_lab = ttl_make_new_label (state);
	    ttl_operand e_lab = ttl_make_new_label (state);
	    compile_bool_expr (state, obj, node, t_lab, f_lab, sp_value);
	    ttl_append_instruction (obj,
				    ttl_make_label_stmt (state, t_lab));
	    ttl_append_instruction 
	      (obj,
	       ttl_make_instruction (state->pool, op_load_true, NULL, NULL,
				     NULL, -1));
	    ttl_append_instruction 
	      (obj,
	       ttl_make_instruction (state->pool, op_jump, e_lab, NULL,
				     NULL, -1));
	    ttl_append_instruction
	      (obj, ttl_make_label_stmt (state, f_lab));
	    ttl_append_instruction 
	      (obj,
	       ttl_make_instruction (state->pool, op_load_false, NULL, NULL,
				     NULL, -1));
	    ttl_append_instruction
	      (obj, ttl_make_label_stmt (state, e_lab));
	    compile_link (state, obj, link, target);
	  }
	  break;

	case il_binop_cons:
	  compile_expr (state, obj, node->d.binop.op0, link_next, NULL,
			sp_value);
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool, op_push, NULL, NULL, NULL, -1));
	  compile_expr (state, obj, node->d.binop.op1, link_next, NULL,
			sp_value + 1);
	  append_gc_check (state, obj, 2);
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction (state->pool, op_cons, NULL, NULL,
				   NULL, -1));
	  compile_link (state, obj, link, target);
	  break;

	case il_binop_assign:
	  fprintf (stderr, "compile_expr: invalid binop encountered (2)\n");
	  abort ();
	  break;
	}
      break;

    case il_unop:
      {
	compile_expr (state, obj, node->d.unop.op0, link_next, NULL, sp_value);
	switch (node->d.unop.op)
	  {
	  case il_unop_neg:
	    if (unconstrain(node->type)->kind == type_real)
	      append_gc_check (state, obj, 4);
	    else if (unconstrain(node->type)->kind == type_long)
	      append_gc_check (state, obj, 4);
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction
	       (state->pool,
		unconstrain(node->type)->kind == type_integer ? op_neg :
		unconstrain(node->type)->kind == type_long ? op_lneg :
		op_fneg,
		NULL, NULL, NULL, -1));
	    break;
	  case il_unop_not:
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_not, NULL, NULL,
				     NULL, -1));
	    break;
	  case il_unop_hd:
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_hd, NULL, NULL,
				     NULL, -1));
#if AUTO_DEREF
	    if (node->type->kind == type_constrained &&
		!load_constrainable_variables)
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool, op_variable_ref,
				       NULL, NULL, NULL, -1));
#endif
	    break;
	  case il_unop_tl:
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_tl, NULL, NULL,
				     NULL, -1));
	    break;
	  case il_unop_sizeof:
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_sizeof, NULL, NULL,
				     NULL, -1));
	    break;
	  }
	compile_link (state, obj, link, target);
      }
      break;
  
    case il_call:
      {
	compile_call (state, obj, node, link, target, sp_value);
      }
      break;

    case il_index:
      {
	compile_expr (state, obj, node->d.index.array, link_next,
		      NULL, sp_value);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_push, NULL, NULL, NULL, -1));
	compile_expr (state, obj, node->d.index.index, link_next, NULL,
		      sp_value + 1);
#if AUTO_DEREF
	if (node->d.index.array->type->kind == type_array &&
	    node->d.index.array->type->d.array.element->kind ==
	    type_constrained &&
	    !load_constrainable_variables)
	  {
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_aload, NULL, NULL,
				     NULL, -1));
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_variable_ref, NULL, NULL,
				     NULL, -1));
	  }
	else
#endif
	  {
	    if (node->d.index.array->type == state->string_type)
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool, op_sload, NULL, NULL,
				       NULL, -1));
	    else
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool, op_aload, NULL, NULL,
				       NULL, -1));
	  }
	compile_link (state, obj, link, target);
      }
      break;

    case il_array_expr:
      {
	ttl_il_node elems = node->d.array_expr.elements;
	unsigned count = 0;

	while (elems)
	  {
	    compile_expr (state, obj, elems->d.pair.car, link_next, NULL,
			  sp_value);
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_push, NULL, NULL,
				     NULL, -1));
	    sp_value++;
	    elems = elems->d.pair.cdr;
	    count++;
	  }
	append_gc_check (state, obj, count + 1);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_create_array,
	    ttl_make_operand (state->pool, operand_constant,
			      (void *) count), NULL,
	    NULL, -1));
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_array_pop,
	    ttl_make_operand (state->pool, operand_constant,
			      (void *) count), NULL, NULL, -1));
	compile_link (state, obj, link, target);
      }
      break;

    case il_list_expr:
      {
	ttl_il_node elems = node->d.list_expr.elements;
	while (elems)
	  {
	    compile_expr (state, obj, elems->d.pair.car, link_next, NULL,
			  sp_value);
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_push, NULL, NULL,
				     NULL, -1));
	    sp_value++;
	    elems = elems->d.pair.cdr;
	  }
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_load_null, NULL, NULL,
				 NULL, -1));

	elems = node->d.list_expr.elements;
	while (elems)
	  {
	    append_gc_check (state, obj, 2);
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_cons, NULL, NULL,
				     NULL, -1));
	    elems = elems->d.pair.cdr;
	  }
	compile_link (state, obj, link, target);
      }
      break;

    case il_tuple_expr:
      {
	ttl_il_node elems = node->d.tuple_expr.elements;
	int count = 0;
	while (elems)
	  {
	    compile_expr (state, obj, elems->d.pair.car,
			  link_next, NULL, sp_value);
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_push, NULL, NULL,
				     NULL, -1));
	    sp_value++;
	    elems = elems->d.pair.cdr;
	    count++;
	  }
	append_gc_check (state, obj, count + 1);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_make_tuple,
	    ttl_make_operand (state->pool, operand_constant,
			      (void *) count), NULL, NULL, -1));
	compile_link (state, obj, link, target);
      }
      break;

    case il_array_constructor:
      compile_expr (state, obj, node->d.array_constructor.initial, link_next,
		    NULL, sp_value);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_push, NULL, NULL, NULL, -1));
      compile_expr (state, obj, node->d.array_constructor.size, link_next,
		    NULL, sp_value + 1);
      if (node->type->d.array.element->kind == type_constrained)
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_make_constrained_array,
				 NULL, NULL,
				 NULL, -1));
      else
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_make_array, NULL, NULL,
				 NULL, -1));
      compile_link (state, obj, link, target);
      break;

    case il_list_constructor:
      compile_expr (state, obj, node->d.list_constructor.initial, link_next,
		    NULL, sp_value);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_push, NULL, NULL, NULL, -1));
      compile_expr (state, obj, node->d.list_constructor.size, link_next,
		    NULL, sp_value + 1);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_make_list, NULL, NULL,
			       NULL, -1));
      compile_link (state, obj, link, target);
      break;

    case il_string_constructor:
      compile_expr (state, obj, node->d.string_constructor.initial, link_next,
		    NULL, sp_value);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_push, NULL, NULL, NULL, -1));
      compile_expr (state, obj, node->d.string_constructor.size, link_next,
		    NULL, sp_value + 1);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_make_string, NULL, NULL,
			       NULL, -1));
      compile_link (state, obj, link, target);
      break;

    case il_foreign_expr:
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool, op_load_foreign,
	  ttl_make_operand
	  (state->pool,
	   operand_constant,
	   (void *) node->d.foreign_expr.expr), NULL, 
	  node->filename, node->start_line));
      compile_link (state, obj, link, target);
      break;

    case il_var_expr:
      compile_expr (state, obj, node->d.var_expr.expr, link_next, NULL,
		    sp_value);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_push, NULL, NULL, NULL, -1));
      if (node->d.var_expr.expr->type->kind == type_integer)
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_make_int_variable, NULL,
				 NULL, NULL, -1));
      else
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_make_real_variable, NULL,
				 NULL, NULL, -1));
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_variable_set, NULL, NULL, NULL,
			       -1));
      compile_link (state, obj, link, target);
      break;

    case il_deref_expr:
      compile_expr (state, obj, node->d.deref_expr.expr, link_next, NULL,
		    sp_value);
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_variable_ref, NULL, NULL, NULL,
			       -1));
      compile_link (state, obj, link, target);
      break;

    case il_require:
    case il_module:
    case il_pair:
    case il_if:
    case il_while:
    case il_in:
    case il_return:
    case il_seq:
    case il_ann_expr:
      fprintf (stderr, "compile_expr: invalid HIL node encountered\n");
      abort ();
    }
}

static void
compile_stmt_list (ttl_compile_state state, ttl_object obj, ttl_il_node code,
		   enum ttl_link link, ttl_operand target);

static void
compile_store (ttl_compile_state state, ttl_object obj,
	       ttl_il_node lvalue, enum ttl_link link,
	       ttl_operand target, int sp_value)
{
  switch (lvalue->kind)
    {
    case il_variable:
      {
	ttl_operand var;

	if (lvalue->d.variable.variable)
	  {
	    switch (lvalue->d.variable.variable->kind)
	      {
	      case variable_local:
	      case variable_param:
		{
		  unsigned over = 0;
		  ttl_function defining =
		    lvalue->d.variable.variable->defining;
		  ttl_function curr = state->current_function;
		  var = ttl_make_operand
		    (state->pool, operand_local,
		     (void *) lvalue->d.variable.variable->index);
		  while (curr != defining)
		    {
		      curr = curr->enclosing;
		      over++;
		    }
		  if (over > 0)
		    var->unsigned_data = over;
		  break;
		}

	      default:
		var = ttl_make_operand
		  (state->pool, operand_mem,
		   (void *) lvalue->d.variable.mangled_name);
		break;
	      }
	  }
	else
	  var = ttl_make_operand
	    (state->pool,
	     operand_mem,
	     (void *) lvalue->d.variable.mangled_name);

#if AUTO_DEREF
	if (lvalue->type->kind == type_constrained)
	  {
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_push, NULL, NULL,
				     NULL, -1));
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_load, var, NULL,
				     NULL, -1));
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_variable_set, NULL, NULL,
				     NULL, -1));
	  }
	else
#endif
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction (state->pool, op_store, var, NULL,
				   NULL, -1));
	compile_link (state, obj, link, target);
	break;
      }
    case il_index:
      {
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_push, NULL, NULL, NULL, -1));
	compile_expr (state, obj, lvalue->d.index.array,
		      link_next, NULL, sp_value + 1);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_push, NULL, NULL, NULL, -1));
	compile_expr (state, obj, lvalue->d.index.index,
		      link_next, NULL, sp_value + 2);
#if AUTO_DEREF
	if (lvalue->type->kind == type_constrained)
	  {
	    if (lvalue->d.index.array->type == state->string_type)
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool,
				       op_sload, NULL, NULL, NULL, -1));
	    else
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool,
				       op_aload, NULL, NULL, NULL, -1));
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_variable_set, NULL, NULL,
				     NULL, -1));
	  }
	else
#endif
	  {
	    if (lvalue->d.index.array->type == state->string_type)
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool,
				       op_sstore, NULL, NULL, NULL, -1));
	    else
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool,
				       op_astore, NULL, NULL, NULL, -1));
	  }
	compile_link (state, obj, link, target);
	break;
      }
    case il_tuple_expr:
      {
	unsigned index = 0;
	ttl_il_node elems =
	  lvalue->d.tuple_expr.elements;
	unsigned size =
	  lvalue->type->d.tuple.elem_type_count;

	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_null_check, NULL, NULL, NULL, -1));
	while (index < size)
	  {
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction
	       (state->pool, op_tuple_ref,
		ttl_make_operand (state->pool,
				  operand_constant,
				  (void *) (size - index - 1)),
		NULL, NULL, -1));
	    sp_value++;
	    index++;
	  }
	while (elems)
	  {
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction
	       (state->pool, op_pop, NULL, NULL, NULL, -1));
	    compile_store (state, obj, elems->d.pair.car,
			   link_next, NULL, sp_value);
	    sp_value--;
	    elems = elems->d.pair.cdr;
	  }
	compile_link (state, obj, link, target);
      }
      break;
      
    case il_error:
    case il_module:
    case il_pair:
    case il_string_const:
    case il_function:
    case il_char_const:
    case il_int_const:
    case il_long_const:
    case il_real_const:
    case il_bool_const:
    case il_null_const:
    case il_binop:
    case il_unop:
    case il_if:
    case il_while:
    case il_in:
    case il_call:
    case il_return:
    case il_require:
    case il_array_expr:
    case il_list_expr:
    case il_array_constructor:
    case il_list_constructor:
    case il_string_constructor:
    case il_seq:
    case il_ann_expr:
    case il_foreign_expr:
    case il_var_expr:
    case il_deref_expr:
      fprintf (stderr, "compile_store: invalid HIL node encountered\n");
      abort ();
    }
}

static int
contains_constrainable_variables (ttl_compile_state state,
				  ttl_il_node node)
{
  switch (node->kind)
    {
    case il_error:
      fprintf
	(stderr, "contains_constrainable_variables: error node encountered\n");
      abort ();
      break;

    case il_variable:
      if (node->type->kind == type_constrained)
	return 1;
      else if (node->type->kind == type_array &&
	       node->type->d.array.element->kind == type_constrained)
	return 1;
      else if (node->type->kind == type_list &&
	       node->type->d.list.element->kind == type_constrained)
	return 1;
      else
	return 0;

    case il_binop:
      return contains_constrainable_variables (state, node->d.binop.op0) ||
	contains_constrainable_variables (state, node->d.binop.op1);

    case il_unop:
      return contains_constrainable_variables (state, node->d.binop.op0);

    case il_ann_expr:
      return contains_constrainable_variables (state, node->d.ann_expr.expr);

    case il_index:
      if (node->d.index.array->type->d.array.element->kind == type_constrained)
	return 1;
      else
	return 0;

    default:
      return 0;
    }
  return 0;
}

#if OLD_COMPILE_CONSTRAINT
static ttl_il_node constant_term;
static ttl_il_node_list variables;
static ttl_il_node_list coefficients;
static ttl_il_node_list const_variables;
static ttl_il_node_list const_coefficients;

static void
compile_constraint (ttl_compile_state state, ttl_object obj, ttl_il_node node,
		    enum ttl_link link, ttl_operand target,
		    int sp_value, int lhs)
{
  switch (node->kind)
    {
    case il_error:
      fprintf
	(stderr, "compile_constraint: error node encountered\n");
      abort ();
      break;

    case il_real_const:
      constant_term = ttl_make_il_binop
	(state, lhs ? il_binop_sub : il_binop_add,
	 constant_term, node,
	 state->real_type,
	 NULL, -1, -1, -1, -1);
      break;

    case il_int_const:
      constant_term = ttl_make_il_binop
	(state, lhs ? il_binop_sub : il_binop_add,
	 constant_term, node,
	 state->int_type,
	 NULL, -1, -1, -1, -1);
      break;

    case il_variable:
      if (node->type->kind == type_constrained)
	{
	  if (node->type->d.constrained.base->kind == type_real)
	    {
	      variables = ttl_il_cons (state, node, variables);
	      coefficients = ttl_il_cons
		(state, ttl_make_il_real (state, lhs ? 1.0 : -1.0, 
					  NULL, -1, -1, -1, -1),
		 coefficients);
	    }
	  else
	    {
	      variables = ttl_il_cons (state, node, variables);
	      coefficients = ttl_il_cons
		(state, ttl_make_il_integer (state, lhs ? 1 : -1, 
					  NULL, -1, -1, -1, -1),
		 coefficients);
	    }
	}
      else
	{
	  if (ttl_constrained_type_base (node->type)->kind == type_real)
	    {
	      const_variables = ttl_il_cons (state, node, const_variables);
	      const_coefficients = ttl_il_cons
		(state, ttl_make_il_real (state, lhs ? -1.0 : 1.0, 
					  NULL, -1, -1, -1, -1),
		 const_coefficients);
	    }
	  else
	    {
	      const_variables = ttl_il_cons (state, node, const_variables);
	      const_coefficients = ttl_il_cons
		(state, ttl_make_il_integer (state, lhs ? -1 : 1, 
					     NULL, -1, -1, -1, -1),
		 const_coefficients);
	    }
	}
      break;

    case il_binop:
      {
	switch (node->d.binop.op)
	  {
	  case il_binop_add:
	    if (node->d.binop.op0->kind == il_variable)
	      {
		compile_constraint (state, obj, node->d.binop.op0,
				    link, target, sp_value, lhs);
	      }
	    else if (node->d.binop.op0->kind == il_real_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op0,
		   state->real_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op0->kind == il_int_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op0,
		   state->int_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op0->kind == il_binop &&
		     (node->d.binop.op0->d.binop.op == il_binop_add ||
		      node->d.binop.op0->d.binop.op == il_binop_mul))
	      {
		compile_constraint (state, obj, node->d.binop.op0,
				    link, target, sp_value, lhs);
	      }

	    if (node->d.binop.op1->kind == il_variable)
	      {
		compile_constraint (state, obj, node->d.binop.op1,
				    link, target, sp_value, lhs);
	      }
	    else if (node->d.binop.op1->kind == il_real_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op1,
		   state->real_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op1->kind == il_int_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op1,
		   state->int_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op1->kind == il_binop &&
		     (node->d.binop.op1->d.binop.op == il_binop_add ||
		      node->d.binop.op1->d.binop.op == il_binop_mul))
	      {
		compile_constraint (state, obj, node->d.binop.op1,
				    link, target, sp_value, lhs);
	      }
	    break;

	  case il_binop_sub:
	    if (node->d.binop.op0->kind == il_variable)
	      {
		compile_constraint (state, obj, node->d.binop.op0,
				    link, target, sp_value, lhs);
	      }
	    else if (node->d.binop.op0->kind == il_real_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op0,
		   state->real_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op0->kind == il_int_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op0,
		   state->int_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op0->kind == il_binop &&
		     (node->d.binop.op0->d.binop.op == il_binop_add ||
		      node->d.binop.op0->d.binop.op == il_binop_mul))
	      {
		compile_constraint (state, obj, node->d.binop.op0,
				    link, target, sp_value, lhs);
	      }

	    if (node->d.binop.op1->kind == il_variable)
	      {
		compile_constraint (state, obj, node->d.binop.op1,
				    link, target, sp_value, !lhs);
	      }
	    else if (node->d.binop.op1->kind == il_real_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_add : il_binop_sub,
		   constant_term, node->d.binop.op1,
		   state->real_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op1->kind == il_int_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_add : il_binop_sub,
		   constant_term, node->d.binop.op1,
		   state->int_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op1->kind == il_binop &&
		     (node->d.binop.op1->d.binop.op == il_binop_add ||
		      node->d.binop.op1->d.binop.op == il_binop_mul))
	      {
		compile_constraint (state, obj, node->d.binop.op1,
				    link, target, sp_value, !lhs);
	      }
	    break;
	  case il_binop_mul:
	    {
	      ttl_il_node o0 = node->d.binop.op0;
	      ttl_il_node o1 = node->d.binop.op1;
	      if (o0->kind == il_real_const && o1->kind == il_variable)
		{
		  if (o1->type->kind == type_constrained)
		    {
		      variables = ttl_il_cons (state, o1, variables);
		      coefficients = ttl_il_cons
			(state, ttl_make_il_real (state,
						  (lhs ? 1.0 : -1.0) *
						  o0->d.real.value, 
						  NULL, -1, -1, -1, -1),
			 coefficients);
		    }
		  else
		    {
		      const_variables = ttl_il_cons (state, o1,
						     const_variables);
		      const_coefficients = ttl_il_cons
			(state, ttl_make_il_real (state, 
						  (lhs ? -1.0 : 1.0) *
						  o0->d.real.value,
						  NULL, -1, -1, -1, -1),
			 const_coefficients);
		    }
		}
	      else if (o1->kind == il_real_const && o0->kind == il_variable)
		{
		  if (o0->type->kind == type_constrained)
		    {
		      variables = ttl_il_cons (state, o0, variables);
		      coefficients = ttl_il_cons
			(state, ttl_make_il_real (state,
						  (lhs ? 1.0 : -1.0) *
						  o1->d.real.value, 
						  NULL, -1, -1, -1, -1),
			 coefficients);
		    }
		  else
		    {
		      const_variables = ttl_il_cons (state, o0,
						     const_variables);
		      const_coefficients = ttl_il_cons
			(state, ttl_make_il_real (state, 
						  (lhs ? -1.0 : 1.0) *
						  o1->d.real.value,
						  NULL, -1, -1, -1, -1),
			 const_coefficients);
		    }
		}
	      else if (o0->kind == il_int_const && o1->kind == il_variable)
		{
		  if (o1->type->kind == type_constrained)
		    {
		      variables = ttl_il_cons (state, o1, variables);
		      coefficients = ttl_il_cons
			(state, ttl_make_il_integer (state,
						  (lhs ? 1 : -1) *
						  o0->d.integer.value, 
						  NULL, -1, -1, -1, -1),
			 coefficients);
		    }
		  else
		    {
		      const_variables = ttl_il_cons (state, o1,
						     const_variables);
		      const_coefficients = ttl_il_cons
			(state, ttl_make_il_integer (state, 
						  (lhs ? -1 : 1) *
						  o0->d.integer.value,
						  NULL, -1, -1, -1, -1),
			 const_coefficients);
		    }
		}
	      else if (o1->kind == il_int_const && o0->kind == il_variable)
		{
		  if (o0->type->kind == type_constrained)
		    {
		      variables = ttl_il_cons (state, o0, variables);
		      coefficients = ttl_il_cons
			(state, ttl_make_il_integer (state,
						  (lhs ? 1 : -1) *
						  o1->d.integer.value, 
						  NULL, -1, -1, -1, -1),
			 coefficients);
		    }
		  else
		    {
		      const_variables = ttl_il_cons (state, o0,
						     const_variables);
		      const_coefficients = ttl_il_cons
			(state, ttl_make_il_integer (state, 
						  (lhs ? -1 : 1) *
						  o1->d.integer.value,
						  NULL, -1, -1, -1, -1),
			 const_coefficients);
		    }
		}
	    }
	    break;
	  case il_binop_div:
	  case il_binop_mod:
	  case il_binop_and:
	  case il_binop_or:
	    break;
	  case il_binop_eq:
	  case il_binop_ne:
	  case il_binop_lt:
	  case il_binop_le:
	  case il_binop_gt:
	  case il_binop_ge:
	    compile_constraint (state, obj, node->d.binop.op0, link, target,
				sp_value, 1);
	    compile_constraint (state, obj, node->d.binop.op1, link, target,
				sp_value, 0);
	    break;
	  case il_binop_assign:
	  case il_binop_cons:
	    break;
	  }
	break;
      }

    case il_call:
      load_constrainable_variables++;
      compile_call (state, obj, node, link, target, sp_value);
      load_constrainable_variables--;
      break;

    case il_unop:
      if (contains_constrainable_variables (state, node))
	{
	  if (ttl_constrained_type_base (node->d.unop.op0->type)->kind ==
	      type_real)
	    {
	      variables = ttl_il_cons (state, node, variables);
	      coefficients = ttl_il_cons
		(state, ttl_make_il_real (state, lhs ? 1.0 : -1.0, 
					  NULL, -1, -1, -1, -1),
		 coefficients);
	    }
	  else
	    {
	      variables = ttl_il_cons (state, node, variables);
	      coefficients = ttl_il_cons
		(state, ttl_make_il_integer (state, lhs ? 1 : -1, 
					  NULL, -1, -1, -1, -1),
		 coefficients);
	    }
	}
      else
	{
	  if (ttl_constrained_type_base (node->d.unop.op0->type)->kind ==
	      type_real)
	    {
	      const_variables = ttl_il_cons (state, node, const_variables);
	      const_coefficients = ttl_il_cons
		(state, ttl_make_il_real (state, lhs ? -1.0 : 1.0, 
					  NULL, -1, -1, -1, -1),
		 const_coefficients);
	    }
	  else
	    {
	      const_variables = ttl_il_cons (state, node, const_variables);
	      const_coefficients = ttl_il_cons
		(state, ttl_make_il_integer (state, lhs ? -1 : 1, 
					     NULL, -1, -1, -1, -1),
		 const_coefficients);
	    }
	}
      break;

    case il_module:
    case il_pair:
    case il_string_const:
    case il_function:
    case il_char_const:
    case il_long_const:
    case il_bool_const:
    case il_null_const:
    case il_index:
    case il_array_expr:
    case il_list_expr:
    case il_array_constructor:
    case il_list_constructor:
    case il_string_constructor:
    case il_tuple_expr:
    case il_seq:
    case il_return:
    case il_if:
    case il_while:
    case il_in:
    case il_require:
    case il_ann_expr:
    case il_foreign_expr:
    case il_var_expr:
    case il_deref_expr:
      fprintf
	(stderr,
	 "compile_constraint: invalid node encountered: ");
      ttl_il_print (stderr, node, 0);
      fprintf (stderr, "\n");
      abort ();
      break;
    }
}

static void
maybe_coerce_to_constrained (ttl_compile_state state,
			     ttl_object obj,
			     ttl_il_node lvalue, ttl_il_node rvalue,
			     ttl_il_node node)
{
  if (lvalue->type->kind == type_array &&
      lvalue->type->d.array.element->kind == type_constrained &&
      rvalue->type->d.array.element->kind != type_constrained)
    {
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_coerce_to_constrained_array, NULL, NULL, NULL, -1));
    }
  else if (lvalue->type->kind == type_list &&
	   lvalue->type->d.list.element->kind == type_constrained &&
	   rvalue->type->d.list.element->kind != type_constrained)
    {
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_coerce_to_constrained_list, NULL, NULL, NULL, -1));
    }
}

static int
real_constraint_p (ttl_il_node expr)
{
  switch (expr->kind)
    {
    case il_binop:
      if (expr->d.binop.op0->type->kind == type_real ||
	  (expr->d.binop.op0->type->kind == type_constrained &&
	   expr->d.binop.op0->type->d.constrained.base->kind == type_real))
	return 1;
      break;
    case il_unop:
      if (expr->d.unop.op0->type->kind == type_real ||
	  (expr->d.unop.op0->type->kind == type_constrained &&
	   expr->d.unop.op0->type->d.constrained.base->kind == type_real))
	return 1;
      break;
    case il_ann_expr:
      return real_constraint_p (expr->d.ann_expr.expr);
    default:
      break;
    }
  return 0;
}

static void compile_stmt (ttl_compile_state state, ttl_object obj,
			  ttl_il_node node, enum ttl_link link,
			  ttl_operand target);

static void
compile_require_stmt (ttl_compile_state state, ttl_object obj,
		      ttl_il_node node,
		      enum ttl_link link, ttl_operand target)
{
  ttl_il_node conjunction = node->d.require.expr;
  ttl_il_node constraint;
  int constrainable = 0;
  int real_constraints = 0;
  int int_constraints = 0;

  while (conjunction)
    {
      constraint = conjunction->d.pair.car;
      if (contains_constrainable_variables (state, constraint))
	constrainable = 1;
      conjunction = conjunction->d.pair.cdr;

      if (constraint->kind == il_call ||
	  (constraint->kind == il_ann_expr &&
	   constraint->d.ann_expr.expr->kind == il_call))
	{
	  compile_constraint (state, obj, constraint, link_next, NULL, 0, 0);
	  continue;
	}

      if (constrainable)
	{
	  int variable_count = 0;
	  int strength = 0;
	  ttl_il_node n = constraint;
	    
	  if (real_constraint_p (n))
	    {
	      constant_term = ttl_make_il_real (state, 0.0, NULL, -1, -1,
						-1, -1);
	      variables = NULL;
	      coefficients = NULL;
	      const_variables = NULL;
	      const_coefficients = NULL;

	      if (n->kind == il_ann_expr)
		{
		  strength = n->d.ann_expr.strength;
		  n = n->d.ann_expr.expr;
		}
	      compile_constraint (state, obj, n, link_next, NULL, 0, 0);
	      {
		ttl_il_node_list l0 = const_variables;
		ttl_il_node_list l1 = const_coefficients;
		while (l0)
		  {
		    constant_term = ttl_make_il_binop
		      (state, il_binop_add, constant_term, 
		       ttl_make_il_binop (state, il_binop_mul,
					  l1->elem, l0->elem, state->real_type,
					  NULL, -1, -1, -1, -1),
		       state->real_type, NULL, -1, -1, -1, -1);
		    l0 = l0->next;
		    l1 = l1->next;
		  }
	      }
#if 0
	      fprintf (stderr, "constant term: ");
	      ttl_il_print (stderr, constant_term, 0);
	      fprintf (stderr, "\n");
	    
	      fprintf (stderr, "variables: ");
	      ttl_il_print_list (stderr, variables);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "coefficients: ");
	      ttl_il_print_list (stderr, coefficients);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "constant variables: ");
	      ttl_il_print_list (stderr, const_variables);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "constant coefficients: ");
	      ttl_il_print_list (stderr, const_coefficients);
	      fprintf (stderr, "\n");
#endif

	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_load_int,
		  ttl_make_operand (state->pool, operand_constant,
				    (void *) strength),
		  NULL, NULL, -1));
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_push, NULL, NULL, NULL, -1));
	      compile_expr (state, obj, constant_term, link_next, NULL, 0);
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_push, NULL, NULL, NULL, -1));
	      {
		ttl_il_node_list l0 = variables;
		ttl_il_node_list l1 = coefficients;
		while (l0)
		  {
		    ttl_il_node node = l0->elem;

		    load_constrainable_variables++;
		    compile_expr (state, obj, node, link_next, NULL, 0);
		    load_constrainable_variables--;

#if 0
		    if (node->d.variable.variable)
		      {
			ttl_operand var;
			switch (node->d.variable.variable->kind)
			  {
			  case variable_local:
			  case variable_param:
			    {
			      unsigned over = 0;
			      ttl_function defining =
				node->d.variable.variable->defining;
			      ttl_function curr = state->current_function;
			      var = ttl_make_operand
				(state->pool, operand_local,
				 (void *) node->d.variable.variable->index);
			      while (curr != defining)
				{
				  curr = curr->enclosing;
				  over++;
				}
			      if (over > 0)
				var->unsigned_data = over;
			      break;
			    }
			
			  default:
			    var = ttl_make_operand
			      (state->pool, operand_mem,
			       (void *) node->d.variable.mangled_name);
			    break;
			  }
			ttl_append_instruction
			  (obj,
			   ttl_make_instruction (state->pool, op_load, var, NULL, 
						 node->filename,
						 node->start_line));
		      }
		    else
		      ttl_append_instruction
			(obj,
			 ttl_make_instruction
			 (state->pool, op_load,
			  ttl_make_operand (state->pool,
					    operand_mem,
					    (void *)
					    node->d.variable.mangled_name), NULL,
			  node->filename, node->start_line));
#endif
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool, op_push, NULL, NULL, NULL, -1));
		    compile_expr (state, obj, l1->elem, link_next, NULL, 0);
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool, op_push, NULL, NULL, NULL, -1));
		    l0 = l0->next;
		    l1 = l1->next;
		    variable_count++;
		  }
	      }
	      {
		ttl_operand var_count = ttl_make_operand
		  (state->pool, operand_constant, (void *) variable_count);
		ttl_operand eq_type = NULL;
		switch (n->d.binop.op)
		  {
		  case il_binop_eq:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 1);
		    break;
		  case il_binop_le:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 2);
		    break;
		  case il_binop_ge:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 3);
		    break;
		  default:
		    fprintf (stderr, "compile_constraint: invalid relation\n");
		    abort ();
		  }
		real_constraints++;
		ttl_append_instruction
		  (obj,
		   ttl_make_instruction
		   (state->pool, op_add_real_constraint, eq_type, var_count,
		    NULL, -1));
	      }
	    }
	  else
	    {
	      constant_term = ttl_make_il_integer (state, 0, NULL, -1, -1,
						   -1, -1);
	      variables = NULL;
	      coefficients = NULL;
	      const_variables = NULL;
	      const_coefficients = NULL;

	      if (n->kind == il_ann_expr)
		{
		  strength = n->d.ann_expr.strength;
		  n = n->d.ann_expr.expr;
		}
	      compile_constraint (state, obj, n, link_next, NULL, 0, 0);
	      {
		ttl_il_node_list l0 = const_variables;
		ttl_il_node_list l1 = const_coefficients;
		while (l0)
		  {
		    constant_term = ttl_make_il_binop
		      (state, il_binop_add, constant_term, 
		       ttl_make_il_binop (state, il_binop_mul,
					  l1->elem, l0->elem, state->int_type,
					  NULL, -1, -1, -1, -1),
		       state->int_type, NULL, -1, -1, -1, -1);
		    l0 = l0->next;
		    l1 = l1->next;
		  }
	      }
#if 0
	      fprintf (stderr, "constant term: ");
	      ttl_il_print (stderr, constant_term, 0);
	      fprintf (stderr, "\n");
	    
	      fprintf (stderr, "variables: ");
	      ttl_il_print_list (stderr, variables);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "coefficients: ");
	      ttl_il_print_list (stderr, coefficients);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "constant variables: ");
	      ttl_il_print_list (stderr, const_variables);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "constant coefficients: ");
	      ttl_il_print_list (stderr, const_coefficients);
	      fprintf (stderr, "\n");
#endif

	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_load_int,
		  ttl_make_operand (state->pool, operand_constant,
				    (void *) strength),
		  NULL, NULL, -1));
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_push, NULL, NULL, NULL, -1));
	      compile_expr (state, obj, constant_term, link_next, NULL, 0);
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_push, NULL, NULL, NULL, -1));
	      {
		ttl_il_node_list l0 = variables;
		ttl_il_node_list l1 = coefficients;
		while (l0)
		  {
		    ttl_il_node node = l0->elem;

		    load_constrainable_variables++;
		    compile_expr (state, obj, node, link_next, NULL, 0);
		    load_constrainable_variables--;

#if 0
		    if (node->d.variable.variable)
		      {
			ttl_operand var;
			switch (node->d.variable.variable->kind)
			  {
			  case variable_local:
			  case variable_param:
			    {
			      unsigned over = 0;
			      ttl_function defining =
				node->d.variable.variable->defining;
			      ttl_function curr = state->current_function;
			      var = ttl_make_operand
				(state->pool, operand_local,
				 (void *) node->d.variable.variable->index);
			      while (curr != defining)
				{
				  curr = curr->enclosing;
				  over++;
				}
			      if (over > 0)
				var->unsigned_data = over;
			      break;
			    }
			
			  default:
			    var = ttl_make_operand
			      (state->pool, operand_mem,
			       (void *) node->d.variable.mangled_name);
			    break;
			  }
			ttl_append_instruction
			  (obj,
			   ttl_make_instruction (state->pool, op_load, var, NULL, 
						 node->filename,
						 node->start_line));
		      }
		    else
		      ttl_append_instruction
			(obj,
			 ttl_make_instruction
			 (state->pool, op_load,
			  ttl_make_operand (state->pool,
					    operand_mem,
					    (void *)
					    node->d.variable.mangled_name), NULL,
			  node->filename, node->start_line));
#endif
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool, op_push, NULL, NULL, NULL, -1));
		    compile_expr (state, obj, l1->elem, link_next, NULL, 0);
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool, op_push, NULL, NULL, NULL, -1));
		    l0 = l0->next;
		    l1 = l1->next;
		    variable_count++;
		  }
	      }
	      {
		ttl_operand var_count = ttl_make_operand
		  (state->pool, operand_constant, (void *) variable_count);
		ttl_operand eq_type = NULL;
		switch (n->d.binop.op)
		  {
		  case il_binop_eq:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 1);
		    break;
		  case il_binop_le:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 2);
		    break;
		  case il_binop_ge:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 3);
		    break;
		  case il_binop_lt:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 4);
		    break;
		  case il_binop_gt:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 5);
		    break;
		  case il_binop_ne:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 6);
		    break;
		  default:
		    fprintf (stderr, "compile_constraint: invalid relation\n");
		    abort ();
		  }
		int_constraints++;
		ttl_append_instruction
		  (obj,
		   ttl_make_instruction
		   (state->pool, op_add_int_constraint, eq_type, var_count,
		    NULL, -1));
	      }
	    }
	}
      else
	{
	  ttl_operand f_lab = ttl_make_new_label (state);
	  ttl_operand e_lab = ttl_make_new_label (state);
	
	  compile_bool_expr (state, obj, constraint, e_lab, f_lab,
			     0);

	  ttl_append_instruction (obj, ttl_make_label_stmt (state, f_lab));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool, op_load,
	      ttl_make_operand (state->pool,
				operand_mem,
				(void *) ttl_symbol_enter
				(state->symbol_table,
				 "ttl_require_exception",
				 sizeof ("ttl_require_exception") -1)),
	      NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_raise,
	      NULL, NULL, NULL, -1));
	  ttl_append_instruction
	    (obj, ttl_make_label_stmt (state, e_lab));
	  /* 	  compile_link (state, obj, link, target); */
	}
    }
  if (int_constraints && !compiling_constraint)
    ttl_append_instruction
      (obj,
       ttl_make_instruction
       (state->pool, op_resolve_int_constraint, NULL, NULL, NULL, -1));
  if (real_constraints && !compiling_constraint)
    ttl_append_instruction
      (obj,
       ttl_make_instruction
       (state->pool, op_resolve_real_constraint, NULL, NULL, NULL, -1));
  if (node->d.require.stmt)
    compile_stmt (state, obj, node->d.require.stmt, link, target);
  else
    compile_link (state, obj, link, target);
}

#else /* OLD_COMPILE_CONSTRAINT */

static ttl_il_node constant_term;
static ttl_il_node_list variables;
static ttl_il_node_list coefficients;
static ttl_il_node_list const_variables;
static ttl_il_node_list const_coefficients;

static void
compile_constraint (ttl_compile_state state, ttl_object obj, ttl_il_node node,
		    enum ttl_link link, ttl_operand target,
		    int sp_value, int lhs)
{
  switch (node->kind)
    {
    case il_error:
      fprintf
	(stderr, "compile_constraint: error node encountered\n");
      abort ();
      break;

    case il_real_const:
      constant_term = ttl_make_il_binop
	(state, lhs ? il_binop_sub : il_binop_add,
	 constant_term, node,
	 state->real_type,
	 NULL, -1, -1, -1, -1);
      break;

    case il_int_const:
      constant_term = ttl_make_il_binop
	(state, lhs ? il_binop_sub : il_binop_add,
	 constant_term, node,
	 state->int_type,
	 NULL, -1, -1, -1, -1);
      break;

    case il_variable:
    case il_var_expr:
      if (node->type->kind == type_constrained)
	{
	  if (node->type->d.constrained.base->kind == type_real)
	    {
	      variables = ttl_il_cons (state, node, variables);
	      coefficients = ttl_il_cons
		(state, ttl_make_il_real (state, lhs ? 1.0 : -1.0, 
					  NULL, -1, -1, -1, -1),
		 coefficients);
	    }
	  else
	    {
	      variables = ttl_il_cons (state, node, variables);
	      coefficients = ttl_il_cons
		(state, ttl_make_il_integer (state, lhs ? 1 : -1, 
					  NULL, -1, -1, -1, -1),
		 coefficients);
	    }
	}
      else
	{
	  if (ttl_constrained_type_base (node->type)->kind == type_real)
	    {
	      const_variables = ttl_il_cons (state, node, const_variables);
	      const_coefficients = ttl_il_cons
		(state, ttl_make_il_real (state, lhs ? -1.0 : 1.0, 
					  NULL, -1, -1, -1, -1),
		 const_coefficients);
	    }
	  else
	    {
	      const_variables = ttl_il_cons (state, node, const_variables);
	      const_coefficients = ttl_il_cons
		(state, ttl_make_il_integer (state, lhs ? -1 : 1, 
					     NULL, -1, -1, -1, -1),
		 const_coefficients);
	    }
	}
      break;

    case il_binop:
      {
	switch (node->d.binop.op)
	  {
	  case il_binop_add:
	    if (node->d.binop.op0->kind == il_variable)
	      {
		compile_constraint (state, obj, node->d.binop.op0,
				    link, target, sp_value, lhs);
	      }
	    else if (node->d.binop.op0->kind == il_real_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op0,
		   state->real_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op0->kind == il_int_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op0,
		   state->int_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op0->kind == il_binop &&
		     (node->d.binop.op0->d.binop.op == il_binop_add ||
		      node->d.binop.op0->d.binop.op == il_binop_mul))
	      {
		compile_constraint (state, obj, node->d.binop.op0,
				    link, target, sp_value, lhs);
	      }

	    if (node->d.binop.op1->kind == il_variable)
	      {
		compile_constraint (state, obj, node->d.binop.op1,
				    link, target, sp_value, lhs);
	      }
	    else if (node->d.binop.op1->kind == il_real_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op1,
		   state->real_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op1->kind == il_int_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op1,
		   state->int_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op1->kind == il_binop &&
		     (node->d.binop.op1->d.binop.op == il_binop_add ||
		      node->d.binop.op1->d.binop.op == il_binop_mul))
	      {
		compile_constraint (state, obj, node->d.binop.op1,
				    link, target, sp_value, lhs);
	      }
	    break;

	  case il_binop_sub:
	    if (node->d.binop.op0->kind == il_variable)
	      {
		compile_constraint (state, obj, node->d.binop.op0,
				    link, target, sp_value, lhs);
	      }
	    else if (node->d.binop.op0->kind == il_real_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op0,
		   state->real_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op0->kind == il_int_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_sub : il_binop_add,
		   constant_term, node->d.binop.op0,
		   state->int_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op0->kind == il_binop &&
		     (node->d.binop.op0->d.binop.op == il_binop_add ||
		      node->d.binop.op0->d.binop.op == il_binop_mul))
	      {
		compile_constraint (state, obj, node->d.binop.op0,
				    link, target, sp_value, lhs);
	      }

	    if (node->d.binop.op1->kind == il_variable)
	      {
		compile_constraint (state, obj, node->d.binop.op1,
				    link, target, sp_value, !lhs);
	      }
	    else if (node->d.binop.op1->kind == il_real_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_add : il_binop_sub,
		   constant_term, node->d.binop.op1,
		   state->real_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op1->kind == il_int_const)
	      {
		constant_term = ttl_make_il_binop
		  (state, lhs ? il_binop_add : il_binop_sub,
		   constant_term, node->d.binop.op1,
		   state->int_type,
		   NULL, -1, -1, -1, -1);
	      }
	    else if (node->d.binop.op1->kind == il_binop &&
		     (node->d.binop.op1->d.binop.op == il_binop_add ||
		      node->d.binop.op1->d.binop.op == il_binop_mul))
	      {
		compile_constraint (state, obj, node->d.binop.op1,
				    link, target, sp_value, !lhs);
	      }
	    break;
	  case il_binop_mul:
	    {
	      ttl_il_node o0 = node->d.binop.op0;
	      ttl_il_node o1 = node->d.binop.op1;
	      if (o0->kind == il_real_const && o1->kind == il_variable)
		{
		  if (o1->type->kind == type_constrained)
		    {
		      variables = ttl_il_cons (state, o1, variables);
		      coefficients = ttl_il_cons
			(state, ttl_make_il_real (state,
						  (lhs ? 1.0 : -1.0) *
						  o0->d.real.value, 
						  NULL, -1, -1, -1, -1),
			 coefficients);
		    }
		  else
		    {
		      const_variables = ttl_il_cons (state, o1,
						     const_variables);
		      const_coefficients = ttl_il_cons
			(state, ttl_make_il_real (state, 
						  (lhs ? -1.0 : 1.0) *
						  o0->d.real.value,
						  NULL, -1, -1, -1, -1),
			 const_coefficients);
		    }
		}
	      else if (o1->kind == il_real_const && o0->kind == il_variable)
		{
		  if (o0->type->kind == type_constrained)
		    {
		      variables = ttl_il_cons (state, o0, variables);
		      coefficients = ttl_il_cons
			(state, ttl_make_il_real (state,
						  (lhs ? 1.0 : -1.0) *
						  o1->d.real.value, 
						  NULL, -1, -1, -1, -1),
			 coefficients);
		    }
		  else
		    {
		      const_variables = ttl_il_cons (state, o0,
						     const_variables);
		      const_coefficients = ttl_il_cons
			(state, ttl_make_il_real (state, 
						  (lhs ? -1.0 : 1.0) *
						  o1->d.real.value,
						  NULL, -1, -1, -1, -1),
			 const_coefficients);
		    }
		}
	      else if (o0->kind == il_int_const && o1->kind == il_variable)
		{
		  if (o1->type->kind == type_constrained)
		    {
		      variables = ttl_il_cons (state, o1, variables);
		      coefficients = ttl_il_cons
			(state, ttl_make_il_integer (state,
						  (lhs ? 1 : -1) *
						  o0->d.integer.value, 
						  NULL, -1, -1, -1, -1),
			 coefficients);
		    }
		  else
		    {
		      const_variables = ttl_il_cons (state, o1,
						     const_variables);
		      const_coefficients = ttl_il_cons
			(state, ttl_make_il_integer (state, 
						  (lhs ? -1 : 1) *
						  o0->d.integer.value,
						  NULL, -1, -1, -1, -1),
			 const_coefficients);
		    }
		}
	      else if (o1->kind == il_int_const && o0->kind == il_variable)
		{
		  if (o0->type->kind == type_constrained)
		    {
		      variables = ttl_il_cons (state, o0, variables);
		      coefficients = ttl_il_cons
			(state, ttl_make_il_integer (state,
						  (lhs ? 1 : -1) *
						  o1->d.integer.value, 
						  NULL, -1, -1, -1, -1),
			 coefficients);
		    }
		  else
		    {
		      const_variables = ttl_il_cons (state, o0,
						     const_variables);
		      const_coefficients = ttl_il_cons
			(state, ttl_make_il_integer (state, 
						  (lhs ? -1 : 1) *
						  o1->d.integer.value,
						  NULL, -1, -1, -1, -1),
			 const_coefficients);
		    }
		}
	    }
	    break;
	  case il_binop_div:
	  case il_binop_mod:
	  case il_binop_and:
	  case il_binop_or:
	    break;
	  case il_binop_eq:
	  case il_binop_ne:
	  case il_binop_lt:
	  case il_binop_le:
	  case il_binop_gt:
	  case il_binop_ge:
	    compile_constraint (state, obj, node->d.binop.op0, link, target,
				sp_value, 1);
	    compile_constraint (state, obj, node->d.binop.op1, link, target,
				sp_value, 0);
	    break;
	  case il_binop_assign:
	  case il_binop_cons:
	    break;
	  }
	break;
      }

    case il_call:
      load_constrainable_variables++;
      compile_call (state, obj, node, link, target, sp_value);
      load_constrainable_variables--;
      break;

    case il_unop:
      if (contains_constrainable_variables (state, node))
	{
	  if (ttl_constrained_type_base (node->d.unop.op0->type)->kind ==
	      type_real)
	    {
	      variables = ttl_il_cons (state, node, variables);
	      coefficients = ttl_il_cons
		(state, ttl_make_il_real (state, lhs ? 1.0 : -1.0, 
					  NULL, -1, -1, -1, -1),
		 coefficients);
	    }
	  else
	    {
	      variables = ttl_il_cons (state, node, variables);
	      coefficients = ttl_il_cons
		(state, ttl_make_il_integer (state, lhs ? 1 : -1, 
					  NULL, -1, -1, -1, -1),
		 coefficients);
	    }
	}
      else
	{
	  if (ttl_constrained_type_base (node->d.unop.op0->type)->kind ==
	      type_real)
	    {
	      const_variables = ttl_il_cons (state, node, const_variables);
	      const_coefficients = ttl_il_cons
		(state, ttl_make_il_real (state, lhs ? -1.0 : 1.0, 
					  NULL, -1, -1, -1, -1),
		 const_coefficients);
	    }
	  else
	    {
	      const_variables = ttl_il_cons (state, node, const_variables);
	      const_coefficients = ttl_il_cons
		(state, ttl_make_il_integer (state, lhs ? -1 : 1, 
					     NULL, -1, -1, -1, -1),
		 const_coefficients);
	    }
	}
      break;

    case il_module:
    case il_pair:
    case il_string_const:
    case il_function:
    case il_char_const:
    case il_long_const:
    case il_bool_const:
    case il_null_const:
    case il_index:
    case il_array_expr:
    case il_list_expr:
    case il_array_constructor:
    case il_list_constructor:
    case il_string_constructor:
    case il_tuple_expr:
    case il_seq:
    case il_return:
    case il_if:
    case il_while:
    case il_in:
    case il_require:
    case il_ann_expr:
    case il_foreign_expr:
    case il_deref_expr:
      fprintf
	(stderr,
	 "compile_constraint: invalid node encountered: ");
      ttl_il_print (stderr, node, 0);
      fprintf (stderr, "\n");
      abort ();
      break;
    }
}

static void
maybe_coerce_to_constrained (ttl_compile_state state,
			     ttl_object obj,
			     ttl_il_node lvalue, ttl_il_node rvalue,
			     ttl_il_node node)
{
  if (lvalue->type->kind == type_array &&
      lvalue->type->d.array.element->kind == type_constrained &&
      rvalue->type->d.array.element->kind != type_constrained)
    {
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_coerce_to_constrained_array, NULL, NULL, NULL, -1));
    }
  else if (lvalue->type->kind == type_list &&
	   lvalue->type->d.list.element->kind == type_constrained &&
	   rvalue->type->d.list.element->kind != type_constrained)
    {
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_coerce_to_constrained_list, NULL, NULL, NULL, -1));
    }
}

static int
real_constraint_p (ttl_il_node expr)
{
  switch (expr->kind)
    {
    case il_binop:
      if (expr->d.binop.op0->type->kind == type_real ||
	  (expr->d.binop.op0->type->kind == type_constrained &&
	   expr->d.binop.op0->type->d.constrained.base->kind == type_real))
	return 1;
      break;
    case il_unop:
      if (expr->d.unop.op0->type->kind == type_real ||
	  (expr->d.unop.op0->type->kind == type_constrained &&
	   expr->d.unop.op0->type->d.constrained.base->kind == type_real))
	return 1;
      break;
    case il_ann_expr:
      return real_constraint_p (expr->d.ann_expr.expr);
    default:
      break;
    }
  return 0;
}

static void compile_stmt (ttl_compile_state state, ttl_object obj,
			  ttl_il_node node, enum ttl_link link,
			  ttl_operand target);

static void
compile_require_stmt (ttl_compile_state state, ttl_object obj,
		      ttl_il_node node,
		      enum ttl_link link, ttl_operand target)
{
  ttl_il_node conjunction = node->d.require.expr;
  ttl_il_node constraint;
  int constrainable = 0;
  int real_constraints = 0;
  int int_constraints = 0;
  int calls = 0;

  while (conjunction)
    {
      constraint = conjunction->d.pair.car;
      if (contains_constrainable_variables (state, constraint))
	constrainable = 1;
      conjunction = conjunction->d.pair.cdr;

      if (constraint->kind == il_call ||
	  (constraint->kind == il_ann_expr &&
	   constraint->d.ann_expr.expr->kind == il_call))
	{
	  compile_constraint (state, obj, constraint, link_next, NULL, 0, 0);
	  calls++;
	  continue;
	}

      if (constrainable)
	{
	  int variable_count = 0;
	  int strength = 0;
	  ttl_il_node n = constraint;
	    
	  if (real_constraint_p (n))
	    {
	      constant_term = ttl_make_il_real (state, 0.0, NULL, -1, -1,
						-1, -1);
	      variables = NULL;
	      coefficients = NULL;
	      const_variables = NULL;
	      const_coefficients = NULL;

	      if (n->kind == il_ann_expr)
		{
		  strength = n->d.ann_expr.strength;
		  n = n->d.ann_expr.expr;
		}
	      compile_constraint (state, obj, n, link_next, NULL, 0, 0);
	      {
		ttl_il_node_list l0 = const_variables;
		ttl_il_node_list l1 = const_coefficients;
		while (l0)
		  {
		    constant_term = ttl_make_il_binop
		      (state, il_binop_add, constant_term, 
		       ttl_make_il_binop (state, il_binop_mul,
					  l1->elem, l0->elem, state->real_type,
					  NULL, -1, -1, -1, -1),
		       state->real_type, NULL, -1, -1, -1, -1);
		    l0 = l0->next;
		    l1 = l1->next;
		  }
	      }
#if 0
	      fprintf (stderr, "constant term: ");
	      ttl_il_print (stderr, constant_term, 0);
	      fprintf (stderr, "\n");
	    
	      fprintf (stderr, "variables: ");
	      ttl_il_print_list (stderr, variables);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "coefficients: ");
	      ttl_il_print_list (stderr, coefficients);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "constant variables: ");
	      ttl_il_print_list (stderr, const_variables);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "constant coefficients: ");
	      ttl_il_print_list (stderr, const_coefficients);
	      fprintf (stderr, "\n");
#endif

	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_load_int,
		  ttl_make_operand (state->pool, operand_constant,
				    (void *) strength),
		  NULL, NULL, -1));
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_push, NULL, NULL, NULL, -1));
	      compile_expr (state, obj, constant_term, link_next, NULL, 0);
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_push, NULL, NULL, NULL, -1));
	      {
		ttl_il_node_list l0 = variables;
		ttl_il_node_list l1 = coefficients;
		while (l0)
		  {
		    ttl_il_node node = l0->elem;

		    load_constrainable_variables++;
		    compile_expr (state, obj, node, link_next, NULL, 0);
		    load_constrainable_variables--;

#if 0
		    if (node->d.variable.variable)
		      {
			ttl_operand var;
			switch (node->d.variable.variable->kind)
			  {
			  case variable_local:
			  case variable_param:
			    {
			      unsigned over = 0;
			      ttl_function defining =
				node->d.variable.variable->defining;
			      ttl_function curr = state->current_function;
			      var = ttl_make_operand
				(state->pool, operand_local,
				 (void *) node->d.variable.variable->index);
			      while (curr != defining)
				{
				  curr = curr->enclosing;
				  over++;
				}
			      if (over > 0)
				var->unsigned_data = over;
			      break;
			    }
			
			  default:
			    var = ttl_make_operand
			      (state->pool, operand_mem,
			       (void *) node->d.variable.mangled_name);
			    break;
			  }
			ttl_append_instruction
			  (obj,
			   ttl_make_instruction (state->pool, op_load, var, NULL, 
						 node->filename,
						 node->start_line));
		      }
		    else
		      ttl_append_instruction
			(obj,
			 ttl_make_instruction
			 (state->pool, op_load,
			  ttl_make_operand (state->pool,
					    operand_mem,
					    (void *)
					    node->d.variable.mangled_name), NULL,
			  node->filename, node->start_line));
#endif
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool, op_push, NULL, NULL, NULL, -1));
		    compile_expr (state, obj, l1->elem, link_next, NULL, 0);
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool, op_push, NULL, NULL, NULL, -1));
		    l0 = l0->next;
		    l1 = l1->next;
		    variable_count++;
		  }
	      }
	      {
		ttl_operand var_count = ttl_make_operand
		  (state->pool, operand_constant, (void *) variable_count);
		ttl_operand eq_type = NULL;
		switch (n->d.binop.op)
		  {
		  case il_binop_eq:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 1);
		    break;
		  case il_binop_le:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 2);
		    break;
		  case il_binop_ge:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 3);
		    break;
		  default:
		    fprintf (stderr, "compile_constraint: invalid relation\n");
		    abort ();
		  }
		real_constraints++;
		ttl_append_instruction
		  (obj,
		   ttl_make_instruction
		   (state->pool, op_add_real_constraint, eq_type, var_count,
		    NULL, -1));
	      }
	    }
	  else
	    {
	      constant_term = ttl_make_il_integer (state, 0, NULL, -1, -1,
						   -1, -1);
	      variables = NULL;
	      coefficients = NULL;
	      const_variables = NULL;
	      const_coefficients = NULL;

	      if (n->kind == il_ann_expr)
		{
		  strength = n->d.ann_expr.strength;
		  n = n->d.ann_expr.expr;
		}
	      compile_constraint (state, obj, n, link_next, NULL, 0, 0);
	      {
		ttl_il_node_list l0 = const_variables;
		ttl_il_node_list l1 = const_coefficients;
		while (l0)
		  {
		    constant_term = ttl_make_il_binop
		      (state, il_binop_add, constant_term, 
		       ttl_make_il_binop (state, il_binop_mul,
					  l1->elem, l0->elem, state->int_type,
					  NULL, -1, -1, -1, -1),
		       state->int_type, NULL, -1, -1, -1, -1);
		    l0 = l0->next;
		    l1 = l1->next;
		  }
	      }
#if 0
	      fprintf (stderr, "constant term: ");
	      ttl_il_print (stderr, constant_term, 0);
	      fprintf (stderr, "\n");
	    
	      fprintf (stderr, "variables: ");
	      ttl_il_print_list (stderr, variables);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "coefficients: ");
	      ttl_il_print_list (stderr, coefficients);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "constant variables: ");
	      ttl_il_print_list (stderr, const_variables);
	      fprintf (stderr, "\n");
	      fprintf (stderr, "constant coefficients: ");
	      ttl_il_print_list (stderr, const_coefficients);
	      fprintf (stderr, "\n");
#endif

	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_load_int,
		  ttl_make_operand (state->pool, operand_constant,
				    (void *) strength),
		  NULL, NULL, -1));
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_push, NULL, NULL, NULL, -1));
	      compile_expr (state, obj, constant_term, link_next, NULL, 0);
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool, op_push, NULL, NULL, NULL, -1));
	      {
		ttl_il_node_list l0 = variables;
		ttl_il_node_list l1 = coefficients;
		while (l0)
		  {
		    ttl_il_node node = l0->elem;

		    load_constrainable_variables++;
		    compile_expr (state, obj, node, link_next, NULL, 0);
		    load_constrainable_variables--;

#if 0
		    if (node->d.variable.variable)
		      {
			ttl_operand var;
			switch (node->d.variable.variable->kind)
			  {
			  case variable_local:
			  case variable_param:
			    {
			      unsigned over = 0;
			      ttl_function defining =
				node->d.variable.variable->defining;
			      ttl_function curr = state->current_function;
			      var = ttl_make_operand
				(state->pool, operand_local,
				 (void *) node->d.variable.variable->index);
			      while (curr != defining)
				{
				  curr = curr->enclosing;
				  over++;
				}
			      if (over > 0)
				var->unsigned_data = over;
			      break;
			    }
			
			  default:
			    var = ttl_make_operand
			      (state->pool, operand_mem,
			       (void *) node->d.variable.mangled_name);
			    break;
			  }
			ttl_append_instruction
			  (obj,
			   ttl_make_instruction (state->pool, op_load, var, NULL, 
						 node->filename,
						 node->start_line));
		      }
		    else
		      ttl_append_instruction
			(obj,
			 ttl_make_instruction
			 (state->pool, op_load,
			  ttl_make_operand (state->pool,
					    operand_mem,
					    (void *)
					    node->d.variable.mangled_name), NULL,
			  node->filename, node->start_line));
#endif
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool, op_push, NULL, NULL, NULL, -1));
		    compile_expr (state, obj, l1->elem, link_next, NULL, 0);
		    ttl_append_instruction
		      (obj,
		       ttl_make_instruction
		       (state->pool, op_push, NULL, NULL, NULL, -1));
		    l0 = l0->next;
		    l1 = l1->next;
		    variable_count++;
		  }
	      }
	      {
		ttl_operand var_count = ttl_make_operand
		  (state->pool, operand_constant, (void *) variable_count);
		ttl_operand eq_type = NULL;
		switch (n->d.binop.op)
		  {
		  case il_binop_eq:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 1);
		    break;
		  case il_binop_le:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 2);
		    break;
		  case il_binop_ge:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 3);
		    break;
		  case il_binop_lt:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 4);
		    break;
		  case il_binop_gt:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 5);
		    break;
		  case il_binop_ne:
		    eq_type = ttl_make_operand
		      (state->pool, operand_constant, (void *) 6);
		    break;
		  default:
		    fprintf (stderr, "compile_constraint: invalid relation\n");
		    abort ();
		  }
		int_constraints++;
		ttl_append_instruction
		  (obj,
		   ttl_make_instruction
		   (state->pool, op_add_int_constraint, eq_type, var_count,
		    NULL, -1));
	      }
	    }
	}
      else
	{
	  ttl_operand f_lab = ttl_make_new_label (state);
	  ttl_operand e_lab = ttl_make_new_label (state);
	
	  compile_bool_expr (state, obj, constraint, e_lab, f_lab,
			     0);

	  ttl_append_instruction (obj, ttl_make_label_stmt (state, f_lab));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool, op_load,
	      ttl_make_operand (state->pool,
				operand_mem,
				(void *) ttl_symbol_enter
				(state->symbol_table,
				 "ttl_require_exception",
				 sizeof ("ttl_require_exception") -1)),
	      NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_raise,
	      NULL, NULL, NULL, -1));
	  ttl_append_instruction
	    (obj, ttl_make_label_stmt (state, e_lab));
	  /* 	  compile_link (state, obj, link, target); */
	}
    }
  if (calls || (int_constraints && !compiling_constraint))
    ttl_append_instruction
      (obj,
       ttl_make_instruction
       (state->pool, op_resolve_int_constraint, NULL, NULL, NULL, -1));
  if (calls || (real_constraints && !compiling_constraint))
    ttl_append_instruction
      (obj,
       ttl_make_instruction
       (state->pool, op_resolve_real_constraint, NULL, NULL, NULL, -1));
  if (node->d.require.stmt)
    compile_stmt (state, obj, node->d.require.stmt, link, target);
  else
    compile_link (state, obj, link, target);
}

#endif

static void
compile_stmt (ttl_compile_state state, ttl_object obj, ttl_il_node node,
	      enum ttl_link link, ttl_operand target)
{
  if (!node)
    return;

  switch (node->kind)
    {
    case il_error:
      fprintf (stderr, "compile_expr: error node encountered\n");
      abort ();
      break;

    case il_binop:
      switch (node->d.binop.op)
	{
	case il_binop_assign:
	  {
	    switch (node->d.binop.op0->kind)
	      {
	      case il_variable:
		{
		  compile_expr (state, obj, node->d.binop.op1, link_next,
				NULL, 0);
		  maybe_coerce_to_constrained (state, obj,
					       node->d.binop.op0,
					       node->d.binop.op1,
					       node);
		  compile_store (state, obj, node->d.binop.op0, link, target,
				 0);
		  break;
		}
	      case il_index:
		{
		  compile_expr (state, obj, node->d.binop.op1, link_next,
				NULL, 0);
		  maybe_coerce_to_constrained (state, obj,
					       node->d.binop.op0,
					       node->d.binop.op1,
					       node);
		  compile_store (state, obj, node->d.binop.op0, link, target,
				 0);
		  break;
		}
	      case il_tuple_expr:
		{
		  ttl_il_node lvalue = node->d.binop.op0;
		  ttl_il_node rvalue = node->d.binop.op1;
		  int sp_value = 0;
		  if (rvalue->kind == il_tuple_expr)
		    {
		      ttl_il_node elems = rvalue->d.tuple_expr.elements;
		      int count = 0;
		      while (elems)
			{
			  compile_expr (state, obj, elems->d.pair.car,
					link_next, NULL, sp_value);
			  ttl_append_instruction
			    (obj,
			     ttl_make_instruction (state->pool, op_push,
						   NULL, NULL, NULL, -1));
			  sp_value++;
			  elems = elems->d.pair.cdr;
			}
		      elems = ttl_il_reverse (state,
					      lvalue->d.tuple_expr.elements);
		      while (elems)
			{
			  ttl_append_instruction
			    (obj,
			     ttl_make_instruction
			     (state->pool, op_pop, NULL, NULL, NULL, -1));
			  compile_store (state, obj, elems->d.pair.car,
					 link_next, NULL, sp_value);
			  sp_value--;
			  elems = elems->d.pair.cdr;
			}
		      compile_link (state, obj, link, target);
		    }
		  else
		    {
		      compile_expr (state, obj, node->d.binop.op1, link_next,
				    NULL, 0);
		      compile_store (state, obj, node->d.binop.op0, link,
				     target, 0);
		    }
		  break;
		}
	      case il_error:
	      case il_module:
	      case il_pair:
	      case il_string_const:
	      case il_function:
	      case il_char_const:
	      case il_int_const:
	      case il_long_const:
	      case il_real_const:
	      case il_bool_const:
	      case il_null_const:
	      case il_binop:
	      case il_unop:
	      case il_if:
	      case il_while:
	      case il_in:
	      case il_call:
	      case il_return:
	      case il_require:
	      case il_array_expr:
	      case il_list_expr:
	      case il_array_constructor:
	      case il_list_constructor:
	      case il_string_constructor:
	      case il_seq:
	      case il_ann_expr:
	      case il_foreign_expr:
	      case il_var_expr:
	      case il_deref_expr:
		fprintf (stderr, "compile_stmt: invalid lvalue encountered\n");
		abort ();
		break;
	      }
	    break;
	  }
	case il_binop_add:
	case il_binop_sub:
	case il_binop_or:
	case il_binop_and:
	case il_binop_mul:
	case il_binop_div:
	case il_binop_mod:
	case il_binop_eq:
	case il_binop_ne:
	case il_binop_lt:
	case il_binop_le:
	case il_binop_gt:
	case il_binop_ge:
	case il_binop_cons:
	  fprintf (stderr, "compile_stmt: invalid binop encountered\n");
	  abort ();
	  break;
	}
      break;

    case il_if:
      {
	ttl_operand t_lab = ttl_make_new_label (state);
	ttl_operand f_lab = ttl_make_new_label (state);
	ttl_operand e_lab = NULL;

	if (link == link_next)
	  e_lab = ttl_make_new_label (state);

	compile_bool_expr (state, obj, node->d.ifstmt.cond, t_lab, f_lab, 0);
	ttl_append_instruction (obj, ttl_make_label_stmt (state, t_lab));
	if (node->d.ifstmt.elsestmt)
	  {
	    if (link == link_next)
	      compile_stmt_list (state, obj, node->d.ifstmt.thenstmt,
				 link_label, e_lab);
	    else
	      compile_stmt_list (state, obj, node->d.ifstmt.thenstmt,
				 link, target);

	    ttl_append_instruction (obj,
				    ttl_make_label_stmt (state, f_lab));
	    compile_stmt_list (state, obj, node->d.ifstmt.elsestmt,
			       link, target);

	    if (link == link_next)
	      ttl_append_instruction (obj, ttl_make_label_stmt (state, e_lab));
	  }
	else
	  {
	    compile_stmt_list (state, obj, node->d.ifstmt.thenstmt,
			       link, target);
	    ttl_append_instruction (obj, ttl_make_label_stmt (state, f_lab));
	    compile_link (state, obj, link, target);
	  }
      }		      
      break;

    case il_while:
      {
	ttl_operand top_lab = ttl_make_new_label (state);
#if TICKS
	ttl_operand tick_lab = ttl_make_new_label (state);
#endif
	ttl_operand start_lab = ttl_make_new_label (state);
	ttl_operand exit_lab = ttl_make_new_label (state);

	ttl_append_instruction (obj,
				ttl_make_instruction
				(state->pool,
				 op_jump,
				 start_lab,
				 NULL, NULL, -1));
	ttl_append_instruction (obj, ttl_make_label_stmt (state, top_lab));
#if TICKS
	append_gc_check (state, obj, 5);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction (state->pool, op_tick, tick_lab, NULL,
				 NULL, -1));
	ttl_append_instruction (obj, ttl_make_cont_label_stmt (state,
							       tick_lab));
#endif
	compile_stmt_list (state, obj, node->d.whilestmt.dostmt,
			   link_next, NULL);
	ttl_append_instruction (obj, ttl_make_label_stmt (state, start_lab));
	compile_bool_expr (state, obj, node->d.whilestmt.cond,
			   top_lab, exit_lab, 0);
	ttl_append_instruction (obj, ttl_make_label_stmt (state, exit_lab));
	compile_link (state, obj, link, target);
	
      }
      break;

    case il_in:
      compile_stmt_list (state, obj, node->d.instmt.instmt, link, target);
      break;

    case il_call:
      compile_call (state, obj, node, link, target, 0);
      break;

    case il_return:
      if (node->d.returnstmt.expr)
	{
	  compile_expr (state, obj, node->d.returnstmt.expr,
			link_return, NULL, 0);
	}
      else
	compile_link (state, obj, link_return, NULL);
      break;

    case il_require:
      compile_require_stmt (state, obj, node, link, target);
      break;

    case il_function:
      break;

    case il_seq:
      compile_stmt_list (state, obj, node->d.seq.stmts, link, target);
      break;

    case il_module:
    case il_pair:
    case il_string_const:
    case il_variable:
    case il_char_const:
    case il_int_const:
    case il_long_const:
    case il_real_const:
    case il_bool_const:
    case il_null_const:
    case il_unop:
    case il_index:
    case il_array_expr:
    case il_list_expr:
    case il_array_constructor:
    case il_list_constructor:
    case il_string_constructor:
    case il_tuple_expr:
    case il_ann_expr:
    case il_foreign_expr:
    case il_var_expr:
    case il_deref_expr:
      fprintf (stderr, "compile_stmt: invalid HIL node encountered\n");
      abort ();
      break;
    }
}

static void
compile_stmt_list (ttl_compile_state state, ttl_object obj, ttl_il_node code,
		   enum ttl_link link, ttl_operand target)
{
  if (code)
    {
      while (code->d.pair.cdr)
	{
	  ttl_instruction instr;

	  compile_stmt (state, obj, code->d.pair.car, link_next, NULL);
	  if (code->d.pair.car->filename)
	    {
	      obj->last->filename = code->d.pair.car->filename;
	      obj->last->line = code->d.pair.car->start_line;
	    }
	  code = code->d.pair.cdr;
#if 0
	  if (code->d.pair.car->filename)
	    {
	      instr = ttl_make_new_note_label_stmt (state);
	      instr->filename = code->d.pair.car->filename;
	      instr->line = code->d.pair.car->start_line;
	      ttl_append_instruction (obj, instr);
	    }
#endif
	}
      compile_stmt (state, obj, code->d.pair.car, link, target);
    }
  else
    compile_link (state, obj, link, target);
}

static int
jump_instr_p (ttl_instruction instr)
{
  switch (instr->op)
    {
    case op_jump:
    case op_jump_proc:
    case op_jump_if_false:
    case op_jump_if_true:
    case op_jump_if_equal:
    case op_jump_if_not_equal:
    case op_jump_if_less:
    case op_jump_if_not_less:
    case op_jump_if_gtr:
    case op_jump_if_not_gtr:
    case op_jump_if_fequal:
    case op_jump_if_not_fequal:
    case op_jump_if_fless:
    case op_jump_if_not_fless:
    case op_jump_if_fgtr:
    case op_jump_if_not_fgtr:
    case op_jump_if_lequal:
    case op_jump_if_not_lequal:
    case op_jump_if_lless:
    case op_jump_if_not_lless:
    case op_jump_if_lgtr:
    case op_jump_if_not_lgtr:
      return 1;
    default:
      return 0;
    }
}

static int
end_of_basic_block_p (ttl_instruction instr)
{
  switch (instr->op)
    {
    case op_label:
    case op_cont_label:
    case op_proc_label:
/*     case op_note_label: */
/*     case op_save_cont: */
    case op_restore_cont:
    case op_jump:
    case op_jump_proc:
    case op_jump_if_false:
    case op_jump_if_true:
    case op_jump_if_equal:
    case op_jump_if_not_equal:
    case op_jump_if_less:
    case op_jump_if_not_less:
    case op_jump_if_gtr:
    case op_jump_if_not_gtr:
    case op_jump_if_fequal:
    case op_jump_if_not_fequal:
    case op_jump_if_fless:
    case op_jump_if_not_fless:
    case op_jump_if_fgtr:
    case op_jump_if_not_fgtr:
    case op_jump_if_lequal:
    case op_jump_if_not_lequal:
    case op_jump_if_lless:
    case op_jump_if_not_lless:
    case op_jump_if_lgtr:
    case op_jump_if_not_lgtr:
    case op_call:
/*     case op_load_real: */
/*     case op_load_string: */
    case op_macro_call:
    case op_make_array:
    case op_make_string:
    case op_make_list:
      return 1;
    default:
      return 0;
    }
}

static int
conditional_jump_instr_p (ttl_instruction instr)
{
  switch (instr->op)
    {
    case op_jump_if_false:
    case op_jump_if_true:
    case op_jump_if_equal:
    case op_jump_if_not_equal:
    case op_jump_if_less:
    case op_jump_if_not_less:
    case op_jump_if_gtr:
    case op_jump_if_not_gtr:
    case op_jump_if_fequal:
    case op_jump_if_not_fequal:
    case op_jump_if_fless:
    case op_jump_if_not_fless:
    case op_jump_if_fgtr:
    case op_jump_if_not_fgtr:
    case op_jump_if_lequal:
    case op_jump_if_not_lequal:
    case op_jump_if_lless:
    case op_jump_if_not_lless:
    case op_jump_if_lgtr:
    case op_jump_if_not_lgtr:
      return 1;
    default:
      return 0;
    }
}

static enum ttl_op_kind
complement_conditional_jump (enum ttl_op_kind kind)
{
  switch (kind)
    {
    case op_jump_if_false:
      return op_jump_if_true;
    case op_jump_if_true:
      return op_jump_if_false;
    case op_jump_if_equal:
      return op_jump_if_not_equal;
    case op_jump_if_not_equal:
      return op_jump_if_equal;
    case op_jump_if_less:
      return op_jump_if_not_less;
    case op_jump_if_not_less:
      return op_jump_if_less;
    case op_jump_if_gtr:
      return op_jump_if_not_gtr;
    case op_jump_if_not_gtr:
      return op_jump_if_gtr;
    case op_jump_if_fequal:
      return op_jump_if_not_fequal;
    case op_jump_if_not_fequal:
      return op_jump_if_fequal;
    case op_jump_if_fless:
      return op_jump_if_not_fless;
    case op_jump_if_not_fless:
      return op_jump_if_fless;
    case op_jump_if_fgtr:
      return op_jump_if_not_fgtr;
    case op_jump_if_not_fgtr:
      return op_jump_if_fgtr;
    case op_jump_if_lequal:
      return op_jump_if_not_lequal;
    case op_jump_if_not_lequal:
      return op_jump_if_lequal;
    case op_jump_if_lless:
      return op_jump_if_not_lless;
    case op_jump_if_not_lless:
      return op_jump_if_lless;
    case op_jump_if_lgtr:
      return op_jump_if_not_lgtr;
    case op_jump_if_not_lgtr:
      return op_jump_if_lgtr;
    default:
      fprintf (stderr, "Invalid Opcode in complement_conditional_jump()\n");
      abort ();
    }
}

static ttl_operand
jump_instruction_label (ttl_instruction instr)
{
  switch (instr->op)
    {
    case op_jump:
    case op_jump_if_false:
    case op_jump_if_true:
    case op_jump_if_equal:
    case op_jump_if_not_equal:
    case op_jump_if_less:
    case op_jump_if_not_less:
    case op_jump_if_gtr:
    case op_jump_if_not_gtr:
    case op_jump_if_fequal:
    case op_jump_if_not_fequal:
    case op_jump_if_fless:
    case op_jump_if_not_fless:
    case op_jump_if_fgtr:
    case op_jump_if_not_fgtr:
    case op_jump_if_lequal:
    case op_jump_if_not_lequal:
    case op_jump_if_lless:
    case op_jump_if_not_lless:
    case op_jump_if_lgtr:
    case op_jump_if_not_lgtr:
      return instr->op0;
    default:
      fprintf (stderr, "Invalid Opcode in jump_instruction_label()\n");
      abort ();
    }
}

static void
set_jump_instruction_label (ttl_instruction instr, ttl_operand lab)
{
  switch (instr->op)
    {
    case op_jump:
    case op_jump_if_false:
    case op_jump_if_true:
    case op_jump_if_equal:
    case op_jump_if_not_equal:
    case op_jump_if_less:
    case op_jump_if_not_less:
    case op_jump_if_gtr:
    case op_jump_if_not_gtr:
    case op_jump_if_fequal:
    case op_jump_if_not_fequal:
    case op_jump_if_fless:
    case op_jump_if_not_fless:
    case op_jump_if_fgtr:
    case op_jump_if_not_fgtr:
    case op_jump_if_lequal:
    case op_jump_if_not_lequal:
    case op_jump_if_lless:
    case op_jump_if_not_lless:
    case op_jump_if_lgtr:
    case op_jump_if_not_lgtr:
      instr->op0 = lab;
      break;
    default:
      fprintf (stderr, "Invalid Opcode in set_jump_instruction_label()\n");
      abort ();
    }
}

static int
same_operands_p (ttl_operand op0, ttl_operand op1)
{
  switch (op0->op)
    {
    case operand_label:
    case operand_constant:
    case operand_local:
    case operand_mem:
      return op0->data == op1->data;
    default:
      return 0;
    }
}

static void
peephole_opt (ttl_compile_state state, ttl_object obj)
{
  ttl_instruction instr = obj->first;

  while (instr)
    {
      if (instr->next)
	{
	  if (instr->op == op_gc_check &&
	      state->compile_options->opt_merge_gc_checks)
	    {
	      ttl_instruction i = instr->next;
	      int found = 0;
	      int amount = (int) (instr->op0->data);
	      while (i && !end_of_basic_block_p (i))
		{
		  if (i->op == op_gc_check)
		    {
		      i->prev->next = i->next;
		      if (i->next)
			i->next->prev = i->prev;
		      else
			obj->last = i->prev;
		      found = 1;
		      amount += (int) (i->op0->data);
		    }
		  i = i->next;
		}
	      if (found)
		{
		  instr->op0->data = (void *) amount;
		}
	      instr = instr->next;
	      continue;
	    }
	  /* At least two instructions remaining... */
	  if (instr->next->next)
	    {
	      /* At least three instructions remaining... */
	      ttl_instruction jif = instr,
		j = instr->next,
		l = instr->next->next;

	      if (conditional_jump_instr_p (jif) &&
		  j->op == op_jump &&
		  l->op == op_label &&
		  same_operands_p (jump_instruction_label (jif), l->op0))
		{
		  jif->op = complement_conditional_jump (jif->op);
		  set_jump_instruction_label (jif,
					      jump_instruction_label (j));
		  set_jump_instruction_label (j, l->op0);
		  continue;
		}
	    }
	  if (instr->op == op_jump &&
	      instr->next->op == op_label &&
	      same_operands_p (jump_instruction_label (instr),
			       instr->next->op0))
	    {
	      ttl_instruction prev = instr->prev;
	      ttl_instruction next = instr->next;
	      if (prev)
		{
		  prev->next = next;
		  instr = next;
		}
	      else
		{
		  obj->first = next;
		  instr = next;
		}
	      if (next)
		next->prev = prev;
	      else
		obj->last = prev;
	      continue;
	    }
	}
      instr = instr->next;
    }
}

static void
compile_constructor_body (ttl_compile_state state, ttl_function function,
			  ttl_object obj, enum ttl_link link,
			  ttl_operand target)
{
  unsigned mask = function->d.constr_discrim.constraint_mask;
  int variables = 0;
  while (mask != 0) 
    {
      if ((mask & 1) != 0)
	variables++;
      mask >>= 1;
    }

  append_gc_check (state, obj, function->d.constr_discrim.field_count + 2 +
		   (variables * 2)); /* XXX: Must be size of variable obj.  */

  {
    ttl_operand oper = ttl_make_operand
      (state->pool, operand_constant,
       (void *) (function->d.constr_discrim.field_count));
    ttl_operand mask = ttl_make_operand
      (state->pool, operand_constant,
       (void *) (function->d.constr_discrim.constraint_mask));
    oper->unsigned_data = function->d.constr_discrim.variant;
    ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_make_data,
      oper,
      mask, NULL, -1));
  }
  compile_link (state, obj, link, target);
}

static void
compile_constructor (ttl_compile_state state, ttl_function function)
{
  ttl_object obj = ttl_make_object (state->pool);

  function->index = state->next_label;
  ttl_append_instruction (obj, ttl_make_new_proc_label_stmt (state));
  compile_constructor_body (state, function, obj, link_return, NULL);
  function->asm_code = obj;
}

static void
compile_discriminator_body (ttl_compile_state state, ttl_function function,
			    ttl_object obj, enum ttl_link link,
			    ttl_operand target)
{
  ttl_operand falsel;
  ttl_operand end_label = NULL;

  if (function->variant_count == 1)
    {
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_pop, NULL, NULL, NULL, -1));
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_null_check, NULL, NULL,
			       NULL, -1));
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_load_true, NULL, NULL,
			       NULL, -1));

      compile_link (state, obj, link, target);
      return;
    }

  ttl_append_instruction
    (obj,
     ttl_make_instruction (state->pool, op_dup, NULL, NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction (state->pool, op_pop, NULL, NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction (state->pool, op_null_check, NULL, NULL, NULL, -1));

  falsel = ttl_make_new_label (state);

  if (link == link_next)
    end_label = ttl_make_new_label (state);

  ttl_append_instruction
    (obj,
     ttl_make_instruction (state->pool, op_pop, NULL, NULL, NULL, -1));

  {
    ttl_operand oper = ttl_make_operand (state->pool, operand_constant,
					 (void *) 0);
    ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool, op_tuple_ref, oper, NULL, NULL, -1));
  }

  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_load_int,
      ttl_make_operand (state->pool, operand_constant,
			(void *) (function->d.constr_discrim.variant)),
      NULL, NULL, -1));

  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_jump_if_not_equal,
      falsel,
      NULL, NULL, -1));

  ttl_append_instruction
    (obj,
     ttl_make_instruction (state->pool, op_load_true, NULL, NULL, NULL, -1));

  if (link == link_next)
    compile_link (state, obj, link_label, end_label);
  else
    compile_link (state, obj, link, target);

  ttl_append_instruction (obj, ttl_make_label_stmt (state, falsel));

  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool, op_load_false, NULL, NULL, NULL, -1));

  if (link == link_next)
    ttl_append_instruction (obj, ttl_make_label_stmt (state, end_label));
  else
    compile_link (state, obj, link, target);
}

static void
compile_discriminator (ttl_compile_state state, ttl_function function)
{
  ttl_object obj = ttl_make_object (state->pool);

  function->index = state->next_label;
  ttl_append_instruction (obj, ttl_make_new_proc_label_stmt (state));
  compile_discriminator_body (state, function, obj, link_return, NULL);
  function->asm_code = obj;
}


static void
compile_accessor_body (ttl_compile_state state, ttl_function function,
		       ttl_object obj, enum ttl_link link, ttl_operand target)
{
  ttl_field_list field_list = function->d.accessor.field_list;
  ttl_operand next_label;
  ttl_operand end_label = NULL;

  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_dup, NULL, NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_pop, NULL, NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_null_check, NULL, NULL, NULL, -1));

  if (function->variant_count == 1)
    {
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_pop,
	  NULL,
	  NULL, NULL, -1));
      {
	ttl_operand oper = ttl_make_operand
	  (state->pool, operand_constant,
	   (void *) (field_list->offset + 1));
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool,
	    op_tuple_ref,
	    oper,
	    NULL, NULL, -1));
      }
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_pop,
	  NULL,
	  NULL, NULL, -1));

#if AUTO_DEREF
      if (field_list->constrainable)
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_variable_ref, NULL, NULL, NULL, -1));
#endif
      compile_link (state, obj, link, target);
      return;
    }

  if (link == link_next)
    end_label = ttl_make_new_label (state);
  next_label = ttl_make_new_label (state);
  while (field_list)
    {
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_pop,
	  NULL,
	  NULL, NULL, -1));

      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_push,
	  NULL,
	  NULL, NULL, -1));

      {
	ttl_operand oper = ttl_make_operand (state->pool, operand_constant,
					     (void *) 0);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool,
	    op_tuple_ref,
	    oper,
	    NULL, NULL, -1));
      }

      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_load_int,
	  ttl_make_operand (state->pool, operand_constant,
			    (void *) (field_list->variant)),
	  NULL, NULL, -1));

      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_jump_if_not_equal,
	  next_label,
	  NULL, NULL, -1));

      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_pop,
	  NULL,
	  NULL, NULL, -1));
      {
	ttl_operand oper = ttl_make_operand
	  (state->pool, operand_constant,
	   (void *) (field_list->offset + 1));
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool,
	    op_tuple_ref,
	    oper,
	    NULL, NULL, -1));
      }
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_pop,
	  NULL,
	  NULL, NULL, -1));

#if AUTO_DEREF
      if (field_list->constrainable)
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool, op_variable_ref, NULL, NULL, NULL, -1));
#endif

      if (link == link_next)
	compile_link (state, obj, link_label, end_label);
      else
	compile_link (state, obj, link, target);

      field_list = field_list->next;
      if (field_list)
	{
	  ttl_append_instruction (obj,
				  ttl_make_label_stmt (state, next_label));
	  next_label = ttl_make_new_label (state);
	}
    }
  ttl_append_instruction (obj, ttl_make_label_stmt (state, next_label));

  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_drop,
      NULL,
      NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool, op_load,
      ttl_make_operand (state->pool,
			operand_mem,
			(void *) ttl_symbol_enter
			(state->symbol_table,
			 "ttl_wrong_variant_exception",
			 sizeof ("ttl_wrong_variant_exception") -1)),
      NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_raise,
      NULL,
      NULL, NULL, -1));

  if (link == link_next)
    ttl_append_instruction (obj, ttl_make_label_stmt (state, end_label));
}

static void
compile_accessor (ttl_compile_state state, ttl_function function)
{
  ttl_object obj = ttl_make_object (state->pool);

  function->index = state->next_label;
  ttl_append_instruction (obj, ttl_make_new_proc_label_stmt (state));
  compile_accessor_body (state, function, obj, link_return, NULL);
  function->asm_code = obj;
}

static void
compile_setter_body (ttl_compile_state state, ttl_function function,
		       ttl_object obj, enum ttl_link link, ttl_operand target)
{
  ttl_field_list field_list = function->d.accessor.field_list;
  ttl_operand next_label;
  ttl_operand end_label = NULL;

  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_over, NULL, NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_pop, NULL, NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_null_check, NULL, NULL, NULL, -1));

  if (function->variant_count == 1)
    {
#if AUTO_DEREF
      if (field_list->constrainable)
	{
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_tuple_ref,
	      ttl_make_operand (state->pool, operand_constant,
				(void *) (field_list->offset + 1)),
	      NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_pop, NULL, NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_variable_set, NULL, NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_drop, NULL, NULL, NULL, -1));
	}
      else
#endif
	{
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_pop, NULL, NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_tuple_set,
	      ttl_make_operand (state->pool, operand_constant,
				(void *) (field_list->offset + 1)),
	      NULL, NULL, -1));
	}
      compile_link (state, obj, link, target);
      return;
    }
  next_label = ttl_make_new_label (state);
  if (link == link_next)
    end_label = ttl_make_new_label (state);
  while (field_list)
    {
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_over,
	  NULL,
	  NULL, NULL, -1));
      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_pop,
	  NULL,
	  NULL, NULL, -1));

      {
	ttl_operand oper = ttl_make_operand (state->pool, operand_constant,
					     (void *) 0);
	ttl_append_instruction
	  (obj,
	   ttl_make_instruction
	   (state->pool,
	    op_tuple_ref,
	    oper,
	    NULL, NULL, -1));
      }

      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_load_int,
	  ttl_make_operand (state->pool, operand_constant,
			    (void *) (field_list->variant)),
	  NULL, NULL, -1));

      ttl_append_instruction
	(obj,
	 ttl_make_instruction
	 (state->pool,
	  op_jump_if_not_equal,
	  next_label,
	  NULL, NULL, -1));

#if AUTO_DEREF
      if (field_list->constrainable)
	{
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_over, NULL, NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_pop, NULL, NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_tuple_ref,
	      ttl_make_operand (state->pool, operand_constant,
				(void *) (field_list->offset + 1)),
	      NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_pop, NULL, NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_variable_set, NULL, NULL, NULL, -1));
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_drop, NULL, NULL, NULL, -1));
	}
      else
#endif
	{
	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_pop,
	      NULL,
	      NULL, NULL, -1));

	  ttl_append_instruction
	    (obj,
	     ttl_make_instruction
	     (state->pool,
	      op_tuple_set,
	      ttl_make_operand (state->pool, operand_constant,
				(void *) (field_list->offset + 1)),
	      NULL, NULL, -1));
	}
      if (link == link_next)
	compile_link (state, obj, link_label, end_label);
      else
	compile_link (state, obj, link, target);

      field_list = field_list->next;
      if (field_list)
	{
	  ttl_append_instruction (obj,
				  ttl_make_label_stmt (state, next_label));
	  next_label = ttl_make_new_label (state);
	}
    }
  ttl_append_instruction (obj, ttl_make_label_stmt (state, next_label));

  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_drop,
      NULL,
      NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_drop,
      NULL,
      NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool, op_load,
      ttl_make_operand (state->pool,
			operand_mem,
			(void *) ttl_symbol_enter
			(state->symbol_table,
			 "ttl_wrong_variant_exception",
			 sizeof ("ttl_wrong_variant_exception") -1)),
      NULL, NULL, -1));
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_raise,
      NULL,
      NULL, NULL, -1));

  if (link == link_next)
    ttl_append_instruction (obj, ttl_make_label_stmt (state, end_label));
}

static void
compile_setter (ttl_compile_state state, ttl_function function)
{
  ttl_object obj = ttl_make_object (state->pool);

  function->index = state->next_label;
  ttl_append_instruction (obj, ttl_make_new_proc_label_stmt (state));
  compile_setter_body (state, function, obj, link_return, NULL);
  function->asm_code = obj;
}

static void
compile_function (ttl_compile_state state, ttl_function function)
{
  ttl_object obj = ttl_make_object (state->pool);
  ttl_operand entry_label;
  ttl_instruction entry_instr;
  ttl_operand tick_lab;

  if (function->kind == function_constraint)
    compiling_constraint++;

  state->current_function = function;

  function->index = state->next_label;

  entry_label = ttl_make_new_label (state);
  entry_instr = ttl_make_proc_label_stmt (state, entry_label);
  if (function->d.function.il_code)
    {
      ttl_il_node il = (ttl_il_node) (function->d.function.il_code);
/*       ttl_il_print (stderr, il->d.pair.car, 0); */
      entry_instr->filename = il->d.pair.car->filename;
      entry_instr->line = il->d.pair.car->start_line;
/*       fprintf (stderr, "%s:%d\n", entry_instr->filename, entry_instr->line); */
    }

  ttl_append_instruction (obj, entry_instr);
#if 1
  if (!function->enclosing)
    ttl_append_instruction (obj,
			    ttl_make_instruction (state->pool,
						  op_null_env_reg,
						  NULL, NULL, NULL, -1));
#endif
  append_gc_check (state, obj,
		   function->param_count + function->local_count + 2);
  ttl_append_instruction
    (obj,
     ttl_make_instruction
     (state->pool,
      op_make_env,
      ttl_make_operand (state->pool, operand_constant,
			(void *) function->param_count),
      ttl_make_operand (state->pool, operand_constant,
			(void *) function->local_count),
      NULL, -1));

#if AUTO_DEREF
  {
    int i = function->param_count;
    ttl_variable var = function->locals;
    while (var)
      {
	if (var->type->kind == type_constrained)
	  {
	    ttl_operand v = ttl_make_operand (state->pool, operand_local,
					      (void *) i);
	    if (var->type->d.constrained.base->kind == type_integer ||
		var->type->d.constrained.base->kind == type_long ||
		var->type->d.constrained.base->kind == type_bool ||
		var->type->d.constrained.base->kind == type_char)
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool, op_make_int_variable, NULL,
				       NULL, NULL, -1));
	    else if (var->type->d.constrained.base->kind == type_real)
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction (state->pool, op_make_real_variable,
				       NULL, NULL, NULL, -1));
	    else
	      {
		fprintf (stderr, "Invalid type for constrained variable\n");
		abort ();
	      }
	    ttl_append_instruction
	      (obj,
	       ttl_make_instruction (state->pool, op_store, v, NULL, 
				     NULL, -1));
	  }
	i++;
	var = var->next;
      }
  }
#endif
  if (function->d.function.handcoded)
    {
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool,
			       op_macro_call,
			       ttl_make_operand
			       (state->pool,
				operand_mem,
				(void *) function->unique_name),
			       NULL, NULL, -1));
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_restore_cont, NULL, NULL,
			       NULL, -1));
    }
  else if (function->d.function.mapped)
    {
      ttl_type t = function->type;
      ttl_type * pt = t->d.function.param_types;
      unsigned i = 0;

      while (i < t->d.function.param_type_count)
	{
	  if (pt[i]->kind != type_integer &&
	      pt[i]->kind != type_char &&
	      pt[i]->kind != type_bool)
	    {
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool,
		  op_load,
		  ttl_make_operand (state->pool, operand_local,
				    (void *) i),
		  NULL, NULL, -1));
	      ttl_append_instruction
		(obj,
		 ttl_make_instruction
		 (state->pool,
		  op_null_check, NULL, NULL, NULL, -1));
	    }
	      
	  i++;
	}
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool,
			       op_mapped_call,
			       ttl_make_operand
			       (state->pool,
				operand_mem,
				(void *) function->d.function.alias),
			       ttl_make_operand
			       (state->pool,
				operand_constant,
				(void *) function->param_count),
			       NULL, -1));
      ttl_append_instruction
	(obj,
	 ttl_make_instruction (state->pool, op_restore_cont, NULL, NULL,
			       NULL, -1));
    }
  else
    {
#if TICKS
      tick_lab = ttl_make_new_label (state);
      append_gc_check (state, obj, 5);
      ttl_append_instruction (obj, ttl_make_instruction (state->pool,
							 op_tick, tick_lab,
							 NULL, NULL, -1));
      ttl_append_instruction
	(obj, ttl_make_cont_label_stmt (state, tick_lab));
#endif
      compile_stmt_list (state, obj,
			 (ttl_il_node) (function->d.function.il_code),
			 link_return, NULL);
      peephole_opt (state, obj);
    }
  function->asm_code = obj;

  if (function->kind == function_constraint)
    compiling_constraint--;
}

void
ttl_generate_code (ttl_compile_state state, ttl_module module)
{
  ttl_function function = module->functions;
  ttl_variable variable = module->globals;
  ttl_module_list imported = module->imported;

  function = module->toplevel_functions;
  while (function)
    {
      if (function->kind == function_function &&
	  function->d.function.handcoded)
	{
	  state->compile_options->pragma_handcoded = 1;
	  break;
	}
      function = function->next;
    }

  function = module->functions;
  while (function)
    {
      switch (function->kind)
	{
	case function_function:
	  compile_function (state, function);
	  break;
	case function_constructor:
	  compile_constructor (state, function);
	  break;
	case function_discriminator:
	  compile_discriminator (state, function);
	  break;
	case function_accessor:
	  compile_accessor (state, function);
	  break;
	case function_setter:
	  compile_setter (state, function);
	  break;
	case function_constraint:
#if 1
	  compile_function (state, function);
#else
	  fprintf (stderr, "Cannot translate constraints yet.\n");
	  abort ();
#endif
	  break;
	}
      function = function->total_next;
    }
  state->label_count = state->next_label;
  state->mapping = make_label_mapping (state, state->label_count);
  function = module->functions;
  while (function)
    {
      fixup_calls (state, function);
      enter_mapping (state, function, (ttl_instruction *) state->mapping,
			   state->label_count);
      function = function->total_next;
    }

  if (state->compile_options->debug_dump_bytecode)
    {
      function = module->functions;
      while (function)
	{
	  ttl_disassemble_object (stdout, function->asm_code);
	  function = function->total_next;
	}
    }
}

/* End of codegen.c.  */
