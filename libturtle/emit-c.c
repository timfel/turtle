/* libturtle/emit-c.c -- C code emitter.
 
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

#define OLD_SP 0

#if HAVE_CONFIG_H
# include <config.h>
#endif 

#include <stdio.h>
#include <string.h>

#include <version.h>

#include <libturtle/turtle-path.h>

#include "memory.h"
#include "error.h"
#include "util.h"
#include "env.h"
#include "il.h"
#include "compiler.h"
#include "codegen.h"


/* #define this to 1 to automatically link the module `turtle0' into
    every Turtle program.  */
#define LINK_TURTLE0 0

/* #define this to 1 to create shared object versions of compiled
    modules.  */
#define COMPILE_SHARED 0

/* Emit the C code for referencing a local variable of nesting depth
   `over', at environment slot `index'.  */
static void
emit_nested_ref (FILE * f, unsigned over, int index)
{
  if (over == 1)
    fprintf (f, "env->parent");
  else
    {
      fprintf (f, "(TTL_VALUE_TO_OBJ (ttl_environment, ");
      emit_nested_ref (f, over - 1, index);
      fprintf (f, ")->parent)");
    }
}


/* Emit the C code for referencing the given operand.  */
static void
emit_operand (FILE * f, ttl_operand operand)
{
  switch (operand->op)
    {
    case operand_label:
      fprintf (f, "L%d", (int) operand->data);
      break;
    case operand_constant:
      fprintf (f, "%d", (int) operand->data);
      break;
    case operand_local:
      if (operand->unsigned_data)
	{
	  unsigned over = operand->unsigned_data;
	  fprintf (f, "TTL_VALUE_TO_OBJ (ttl_environment, ");
	  emit_nested_ref (f, over, (int) operand->data);
	  fprintf (f, ")->locals[%d]", (int) operand->data);
	}
      else
	fprintf (f, "env->locals[%d]", (int) operand->data);
      break;
    case operand_mem:
      ttl_symbol_print (f, (ttl_symbol) operand->data);
      break;
    }
}


/* Emit the instruction `instr' to the C code file `f'.  */
static void
emit_instruction (FILE * f, ttl_instruction instr)
{
  if (instr->filename)
    {
      fprintf (f, "  /* %s:%d */\n", instr->filename, instr->line + 1);
    }
  switch (instr->op)
    {
    case op_make_closure:
      fprintf (f, "\tTTL_MAKE_CLOSURE (descriptors + %d);",
	       (int)(instr->op0->data));
      break;
      
    case op_macro_call:
      fprintf (f, "\t");
      emit_operand (f, instr->op0);
      fprintf (f, "_implementation");
      break;

    case op_sload:
#if OLD_SP
      fprintf (f, "\t{\n\t  TTL_RANGE_CHECK (TTL_VALUE_TO_INT (acc), ttl_stack[sp - 1]);\n");
      fprintf (f, "\t  acc = TTL_CHAR_TO_VALUE (TTL_VALUE_TO_OBJ (ttl_string, ttl_stack[--sp])->data[TTL_VALUE_TO_INT (acc)]);\n\t}");
#else
      fprintf (f, "\t{\n\t  TTL_RANGE_CHECK (TTL_VALUE_TO_INT (acc), *(sp - 1));\n");
      fprintf (f, "\t  acc = TTL_CHAR_TO_VALUE (TTL_VALUE_TO_OBJ (ttl_string, *(--sp))->data[TTL_VALUE_TO_INT (acc)]);\n\t}");
#endif
      break;

    case op_sstore:
#if OLD_SP
      fprintf (f, "\t{\n\t  int idx = TTL_VALUE_TO_INT (acc);\n");
      fprintf (f, "\t  ttl_value arr = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_RANGE_CHECK (idx, arr);\n");
      fprintf (f, "\t  TTL_VALUE_TO_OBJ (ttl_string, arr)->data[idx] = TTL_VALUE_TO_CHAR (ttl_stack[--sp]);\n\t}");
#else
      fprintf (f, "\t{\n\t  int idx = TTL_VALUE_TO_INT (acc);\n");
      fprintf (f, "\t  ttl_value arr = *(--sp);\n");
      fprintf (f, "\t  TTL_RANGE_CHECK (idx, arr);\n");
      fprintf (f, "\t  TTL_VALUE_TO_OBJ (ttl_string, arr)->data[idx] = TTL_VALUE_TO_CHAR (*--sp);\n\t}");
#endif
      break;

    case op_aload:
#if OLD_SP
      fprintf (f, "\t{\n\t  TTL_RANGE_CHECK (TTL_VALUE_TO_INT (acc), ttl_stack[sp - 1]);\n");
      fprintf (f, "\t  acc = TTL_VALUE_TO_OBJ (ttl_array, ttl_stack[--sp])->data[TTL_VALUE_TO_INT (acc)];\n\t}");
#else
      fprintf (f, "\t{\n\t  TTL_RANGE_CHECK (TTL_VALUE_TO_INT (acc), *(sp - 1));\n");
      fprintf (f, "\t  acc = TTL_VALUE_TO_OBJ (ttl_array, *(--sp))->data[TTL_VALUE_TO_INT (acc)];\n\t}");
#endif
      break;

    case op_astore:
#if OLD_SP
      fprintf (f, "\t{\n\t  int idx = TTL_VALUE_TO_INT (acc);\n");
      fprintf (f, "\t  ttl_value arr = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_RANGE_CHECK (idx, arr);\n");
      fprintf (f, "\t  TTL_VALUE_TO_OBJ (ttl_array, arr)->data[idx] = ttl_stack[--sp];\n\t}");
#else
      fprintf (f, "\t{\n\t  int idx = TTL_VALUE_TO_INT (acc);\n");
      fprintf (f, "\t  ttl_value arr = *(--sp);\n");
      fprintf (f, "\t  TTL_RANGE_CHECK (idx, arr);\n");
      fprintf (f, "\t  TTL_VALUE_TO_OBJ (ttl_array, arr)->data[idx] = *(--sp);\n\t}");
#endif
      break;

    case op_tuple_ref:
      fprintf (f, "\tTTL_TUPLE_REF (%d);", (int) instr->op0->data);
      break;

    case op_tuple_set:
      fprintf (f, "\tTTL_TUPLE_SET (%d);", (int) instr->op0->data);
      break;

    case op_array_pop:
      {
	unsigned count = (unsigned) instr->op0->data;
	while (count-- > 0)
	  {
#if OLD_SP
	    fprintf (f, "\tTTL_VALUE_TO_OBJ (ttl_array, acc)->data[%d] = ttl_stack[--sp];\n", count);
#else
	    fprintf (f, "\tTTL_VALUE_TO_OBJ (ttl_array, acc)->data[%d] = *(--sp);\n", count);
#endif
	  }
      }
      break;

    case op_create_array:
      {
	unsigned count = (unsigned) instr->op0->data;
	fprintf (f, "\tTTL_MAKE_UNINITIALIZED_ARRAY (%d);", count);
      }
#if 0
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf (f, "\tttl_global_acc = ttl_unsafe_alloc_array (TTL_VALUE_TO_INT (ttl_global_acc));\n");
      fprintf (f, "\tTTL_RESTORE_REGISTERS;");
#endif
      break;

    case op_make_constrained_array:
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf
	(f,
	 "\tttl_global_acc = ttl_alloc_constrained_array (TTL_VALUE_TO_INT (ttl_global_acc));\n");
      fprintf (f, "\tttl_fill_constrained_array (ttl_global_acc, ttl_stack[--ttl_global_sp]);\n");
      fprintf (f, "\tTTL_RESTORE_REGISTERS;");
      break;

    case op_make_array:
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf
	(f,
	 "\tttl_global_acc = ttl_alloc_array (TTL_VALUE_TO_INT (ttl_global_acc));\n");
      fprintf
	(f,
	 "\tttl_fill_array (ttl_global_acc, ttl_stack[--ttl_global_sp]);\n");
      fprintf (f, "\tTTL_RESTORE_REGISTERS;");
      break;

    case op_make_string:
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf (f, "\tttl_global_acc = ttl_alloc_string (TTL_VALUE_TO_INT (ttl_global_acc));\n");
      fprintf (f, "\tttl_fill_string (ttl_global_acc, TTL_VALUE_TO_CHAR (ttl_stack[--ttl_global_sp]));\n");
      fprintf (f, "\tTTL_RESTORE_REGISTERS;");
      break;

    case op_make_list:
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf (f, "\tttl_global_acc = ttl_make_list (TTL_VALUE_TO_INT (ttl_global_acc), ttl_stack[ttl_global_sp - 1]);\n");
      fprintf (f, "\tttl_global_sp--;\n");
      fprintf (f, "\tTTL_RESTORE_REGISTERS;");
      break;

    case op_make_tuple:
      {
	unsigned count = (unsigned) instr->op0->data;
#if OLD_SP
	fprintf (f, "\tTTL_MAKE_UNINITIALIZED_ARRAY (%d);\n", count);
#if 0
	fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
	fprintf (f, "\tttl_global_acc = ttl_unsafe_alloc_array (%d);\n",
		 count);
	fprintf (f, "\tTTL_RESTORE_REGISTERS;\n");
#endif
	while (count-- > 0)
	  {
	    fprintf (f,
		     "\tTTL_VALUE_TO_OBJ (ttl_array, acc)->data[%d] = ttl_stack[--sp];\n",
		     count);
	  }
#else
	fprintf (f, "\tTTL_MAKE_UNINITIALIZED_ARRAY (%d);\n", count);
	while (count-- > 0)
	  {
	    fprintf (f,
		     "\tTTL_VALUE_TO_OBJ (ttl_array, acc)->data[%d] = *(--sp);\n",
		     count);
	  }
#endif
      }
      break;

    case op_make_data:
      {
	unsigned count = (unsigned) instr->op0->data;
	unsigned mask = (unsigned) instr->op1->data;
	unsigned variant = instr->op0->unsigned_data;
#if OLD_SP
	fprintf (f, "\tTTL_MAKE_UNINITIALIZED_ARRAY (%d);\n", count + 1);
#if 0
	fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
	fprintf (f, "\tttl_global_acc = ttl_unsafe_alloc_array (%d);\n",
		 count + 1);
	fprintf (f, "\tTTL_RESTORE_REGISTERS;\n");
#endif
	while (count-- > 0)
	  {
	    fprintf (f,
		     "\tTTL_VALUE_TO_OBJ (ttl_array, acc)->data[%d] = ttl_stack[--sp];\n",
		     count + 1);
	  }
	count = 0;
	while (count < (unsigned) instr->op0->data)
	  {
	    if (mask & 1)
	      {
		fprintf (f, "\tTTL_PUSH();\n");
		fprintf (f, "\tacc = TTL_VALUE_TO_OBJ (ttl_array, ttl_stack[sp - 1])->data[%d];\n", count + 1);
		fprintf (f,
			 "\tTTL_VALUE_TO_OBJ (ttl_array, ttl_stack[sp - 1])->data[%d] = ttl_alloc_constrainable_variable ();\n",
			 count + 1);
		fprintf (f,
			 "\tTTL_VALUE_TO_OBJ (ttl_constrainable_variable, TTL_VALUE_TO_OBJ (ttl_array, ttl_stack[sp - 1])->data[%d])->value = acc;\n",
			 count + 1);
		fprintf (f, "\tTTL_POP();\n");
	      }
	    mask = mask >> 1;
	    count++;
	  }
	fprintf (f,
		 "\tTTL_VALUE_TO_OBJ (ttl_array, acc)->data[0] = TTL_INT_TO_VALUE (%d);\n",
		 variant);
#else
	fprintf (f, "\tTTL_MAKE_UNINITIALIZED_ARRAY (%d);\n", count + 1);
	while (count-- > 0)
	  {
	    fprintf (f,
		     "\tTTL_VALUE_TO_OBJ (ttl_array, acc)->data[%d] = *(--sp);\n",
		     count + 1);
	  }
	count = 0;
	while (count < (unsigned) instr->op0->data)
	  {
	    if (mask & 1)
	      {
		fprintf (f, "\tTTL_PUSH();\n");
		fprintf (f, "\tacc = TTL_VALUE_TO_OBJ (ttl_array, *(sp - 1))->data[%d];\n", count + 1);
		fprintf (f,
			 "\tTTL_VALUE_TO_OBJ (ttl_array, *(sp - 1))->data[%d] = ttl_alloc_constrainable_variable ();\n",
			 count + 1);
		fprintf (f,
			 "\tTTL_VALUE_TO_OBJ (ttl_constrainable_variable, TTL_VALUE_TO_OBJ (ttl_array, *(sp - 1))->data[%d])->value = acc;\n",
			 count + 1);
		fprintf (f, "\tTTL_POP();\n");
	      }
	    mask = mask >> 1;
	    count++;
	  }
	fprintf (f,
		 "\tTTL_VALUE_TO_OBJ (ttl_array, acc)->data[0] = TTL_INT_TO_VALUE (%d);\n",
		 variant);
#endif
      }
      break;

    case op_gc_check:
      fprintf (f, "\tTTL_GC_CHECK (%d);",
	       (int) (instr->op0->data));
      break;

    case op_null_env_reg:
      fprintf (f, "\tenv = NULL;");
      break;

    case op_label:
      emit_operand (f, instr->op0);
      fprintf (f, ":");
      if (instr->next == NULL || instr->next->op != op_label)
	fprintf (f, "\n\tpc = descriptors + %d;", (int) instr->op0->data);
      break;
    case op_cont_label:
      fprintf (f, "    case %d:\n", (int) instr->op0->data);
      emit_operand (f, instr->op0);
      fprintf (f, ":");
      break;
    case op_proc_label:
      fprintf (f, "    case %d:\n", (int) instr->op0->data);
      emit_operand (f, instr->op0);
      fprintf (f, ":");
      break;
    case op_note_label:
      fprintf (f, "\tpc = descriptors + %d;", (int) instr->op0->data);
      break;
    case op_load:
      fprintf (f, "\tacc = ");
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;
    case op_load_int:
      fprintf (f, "\tacc = TTL_INT_TO_VALUE (");
      emit_operand (f, instr->op0);
      fprintf (f, ");");
      break;
    case op_load_long:
      fprintf (f, "\tTTL_MAKE_LONG (%ld);", *((long *) (instr->op0->data)));
      break;
    case op_load_null:
      fprintf (f, "\tacc = TTL_NULL;");
      break;
    case op_load_false:
      fprintf (f, "\tacc = TTL_FALSE;");
      break;
    case op_load_true:
      fprintf (f, "\tacc = TTL_TRUE;");
      break;
    case op_load_real:
      fprintf (f, "\tTTL_MAKE_REAL (%0.20f);", *((double *) (instr->op0->data)));
      break;
    case op_load_char:
      fprintf (f, "\tacc = TTL_CHAR_TO_VALUE (");
      emit_operand (f, instr->op0);
      fprintf (f, ");");
      break;
    case op_load_string:
      {
	unsigned len = (unsigned) (instr->op1->data);
	char * s = (char *) (instr->op0->data);
	unsigned i;
	fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
	fprintf (f, "\tttl_global_acc = ttl_unsafe_string_to_value (\"");
	for (i = 0; i < len; i++)
	  ttl_print_escaped_char (f, *s++);
	fprintf (f, "\", %d);\n", len);
	fprintf (f, "\tTTL_RESTORE_REGISTERS;");
      }
      break;
    case op_store:
      fprintf (f, "\t");
      emit_operand (f, instr->op0);
      fprintf (f, " = acc;");
      break;
    case op_push:
      fprintf (f, "\tTTL_PUSH();");
      break;
    case op_pop:
      fprintf (f, "\tTTL_POP();");
      break;
    case op_dup:
      fprintf (f, "\tTTL_DUP();");
      break;
    case op_over:
      fprintf (f, "\tTTL_OVER();");
      break;
    case op_drop:
      fprintf (f, "\tTTL_DROP();");
      break;
    case op_mul:
#if OLD_SP
      fprintf (f, "\tacc = (ttl_value) ((((int)ttl_stack[--sp] >> 2) *  ((int)acc >> 2)) << 2);");
#else
      fprintf (f, "\tacc = (ttl_value) ((((int)*(--sp) >> 2) *  ((int)acc >> 2)) << 2);");
#endif
      break;
    case op_div:
#if OLD_SP
      fprintf (f, "\tacc = (ttl_value) ((((int)ttl_stack[--sp] >> 2) / ((int)acc >> 2)) << 2);");
#else
      fprintf (f, "\tacc = (ttl_value) ((((int)*(--sp) >> 2) / ((int)acc >> 2)) << 2);");
#endif
      break;
    case op_mod:
#if OLD_SP
      fprintf (f, "\tacc = (ttl_value) ((((int)ttl_stack[--sp] >> 2) %% ((int)acc >> 2)) << 2);");
#else
      fprintf (f, "\tacc = (ttl_value) ((((int)*(--sp) >> 2) %% ((int)acc >> 2)) << 2);");
#endif
      break;
    case op_add:
#if OLD_SP
      fprintf (f, "\tacc = (ttl_value) ((((int)ttl_stack[--sp]  >> 2) + ((int)acc >> 2)) << 2);");
#else
/*       fprintf (f, "\tacc = (ttl_value) ((((int)*(--sp)  >> 2) + ((int)acc >> 2)) << 2);"); */
      fprintf (f, "\tacc = (ttl_value) ((((int)*(--sp)) + ((int)acc)));");
#endif
      break;
    case op_sub:
#if OLD_SP
      fprintf (f, "\tacc = (ttl_value) ((((int)ttl_stack[--sp] >> 2) - ((int)acc >> 2)) << 2);");
#else
/*       fprintf (f, "\tacc = (ttl_value) ((((int)*(--sp) >> 2) - ((int)acc >> 2)) << 2);"); */
      fprintf (f, "\tacc = (ttl_value) ((((int)*(--sp)) - ((int)acc)));");
#endif
      break;

    case op_neg:
      fprintf (f, "\tacc = TTL_INT_TO_VALUE (-TTL_VALUE_TO_INT(acc));");
      break;

    case op_not:
      fprintf (f, "\tacc = TTL_BOOL_TO_VALUE (!TTL_VALUE_TO_BOOL(acc));");
      break;

    case op_fadd:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r0, r1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r1 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL (r0 + r1);\n\t}");
#else
      fprintf (f, "\t{\n\t  double r0, r1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r1 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL (r0 + r1);\n\t}");
#endif
      break;

    case op_fsub:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r0, r1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r1 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL (r0 - r1);\n\t}");
#else
      fprintf (f, "\t{\n\t  double r0, r1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r1 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL (r0 - r1);\n\t}");
#endif
      break;

    case op_fmul:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r0, r1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r1 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL (r0 * r1);\n\t}");
#else
      fprintf (f, "\t{\n\t  double r0, r1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r1 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL (r0 * r1);\n\t}");
#endif
      break;

    case op_fdiv:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r0, r1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r1 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL (r0 / r1);\n\t}");
#else
      fprintf (f, "\t{\n\t  double r0, r1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r1 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL (r0 / r1);\n\t}");
#endif
      break;

    case op_fmod:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r0, r1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r1 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL ((double) ((int) r0 %% (int) r1));\n\t}");
#else
      fprintf (f, "\t{\n\t  double r0, r1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r1 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL ((double) ((int) r0 %% (int) r1));\n\t}");
#endif
      break;

    case op_fneg:
      fprintf (f, "\t{\n\t  double r0;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r0 = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_REAL ((double) (-r0));\n\t}");
      break;

    case op_ladd:
#if OLD_SP
      fprintf (f, "\t{\n\t  long l0, l1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l1 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (l0 + l1);\n\t}");
#else
      fprintf (f, "\t{\n\t  long l0, l1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l1 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (l0 + l1);\n\t}");
#endif
      break;

    case op_lsub:
#if OLD_SP
      fprintf (f, "\t{\n\t  long l0, l1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l1 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (l0 - l1);\n\t}");
#else
      fprintf (f, "\t{\n\t  long l0, l1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l1 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (l0 - l1);\n\t}");
#endif
      break;

    case op_lmul:
#if OLD_SP
      fprintf (f, "\t{\n\t  long l0, l1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l1 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (l0 * l1);\n\t}");
#else
      fprintf (f, "\t{\n\t  long l0, l1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l1 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (l0 * l1);\n\t}");
#endif
      break;

    case op_ldiv:
#if OLD_SP
      fprintf (f, "\t{\n\t  long l0, l1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l1 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (l0 / l1);\n\t}");
#else
      fprintf (f, "\t{\n\t  long l0, l1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l1 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (l0 / l1);\n\t}");
#endif
      break;

    case op_lmod:
#if OLD_SP
      fprintf (f, "\t{\n\t  long l0, l1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l1 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (l0 %% l1);\n\t}");
#else
      fprintf (f, "\t{\n\t  long l0, l1;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l1 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (l0 %% l1);\n\t}");
#endif
      break;

    case op_lneg:
      fprintf (f, "\t{\n\t  long l0;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l0 = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
      fprintf (f, "\t  TTL_MAKE_LONG (-l0);\n\t}");
      break;

    case op_jump:
      fprintf (f, "\tgoto ");
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;

    case op_jump_proc:
      fprintf (f, "\tttl_stats.direct_call_count++;\n");
      fprintf (f, "\tgoto ");
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;

    case op_jump_if_false:
      fprintf (f, "\tif (acc == TTL_FALSE) goto ");
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;

    case op_jump_if_true:
      fprintf (f, "\tif (acc == TTL_TRUE) goto ");
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;

    case op_jump_if_equal:
#if OLD_SP
      fprintf (f, "\tif (ttl_stack[--sp] == acc) goto ");
#else
      fprintf (f, "\tif (*(--sp) == acc) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;

    case op_jump_if_not_equal:
#if OLD_SP
      fprintf (f, "\tif (ttl_stack[--sp] != acc) goto ");
#else
      fprintf (f, "\tif (*(--sp) != acc) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;

    case op_jump_if_less:
#if OLD_SP
      fprintf (f, "\tif ((int) ttl_stack[--sp] < (int) acc) goto ");
#else
      fprintf (f, "\tif ((int) *(--sp) < (int) acc) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;

    case op_jump_if_not_less:
#if OLD_SP
      fprintf (f, "\tif ((int) ttl_stack[--sp] >= (int) acc) goto ");
#else
      fprintf (f, "\tif ((int) *(--sp) >= (int) acc) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;

    case op_jump_if_gtr:
#if OLD_SP
      fprintf (f, "\tif ((int) ttl_stack[--sp] > (int) acc) goto ");
#else
      fprintf (f, "\tif ((int) *(--sp) > (int) acc) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;

    case op_jump_if_not_gtr:
#if OLD_SP
      fprintf (f, "\tif ((int) ttl_stack[--sp] <= (int) acc) goto ");
#else
      fprintf (f, "\tif ((int) *(--sp) <= (int) acc) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";");
      break;

    case op_jump_if_fequal:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value == r) goto ");
#else
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value == r) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_not_fequal:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value != r) goto ");
#else
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value != r) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_fless:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value < r) goto ");
#else
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value < r) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_not_fless:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value >= r) goto ");
#else
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value >= r) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_fgtr:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value > r) goto ");      
#else
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value > r) goto ");      
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_not_fgtr:
#if OLD_SP
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value <= r) goto ");
#else
      fprintf (f, "\t{\n\t  double r;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  r = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;\n");
      fprintf (f, "\t  acc = *(--sp);\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_real, acc)->value <= r) goto ");
#endif
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_lequal:
      fprintf (f, "\t{\n\t  long l;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
#if OLD_SP
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
#else
      fprintf (f, "\t  acc = *(--sp);\n");
#endif
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_long, acc)->value == l) goto ");
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_not_lequal:
      fprintf (f, "\t{\n\t  long l;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
#if OLD_SP
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
#else
      fprintf (f, "\t  acc = *(--sp);\n");
#endif
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_long, acc)->value != l) goto ");
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_lless:
      fprintf (f, "\t{\n\t  long l;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
#if OLD_SP
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
#else
      fprintf (f, "\t  acc = *(--sp);\n");
#endif
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_long, acc)->value < l) goto ");
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_not_lless:
      fprintf (f, "\t{\n\t  long l;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
#if OLD_SP
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
#else
      fprintf (f, "\t  acc = *(--sp);\n");
#endif
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_long, acc)->value >= l) goto ");
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_lgtr:
      fprintf (f, "\t{\n\t  long l;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
#if OLD_SP
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
#else
      fprintf (f, "\t  acc = *(--sp);\n");
#endif
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_long, acc)->value > l) goto ");      
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_jump_if_not_lgtr:
      fprintf (f, "\t{\n\t  long l;\n");
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  l = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;\n");
#if OLD_SP
      fprintf (f, "\t  acc = ttl_stack[--sp];\n");
#else
      fprintf (f, "\t  acc = *(--sp);\n");
#endif
      fprintf (f, "\t  TTL_NULL_CHECK;\n");
      fprintf (f, "\t  if (TTL_VALUE_TO_OBJ (ttl_long, acc)->value <= l) goto ");
      emit_operand (f, instr->op0);
      fprintf (f, ";\n\t}");
      break;

    case op_save_cont:
/*       fprintf (f, "\tTTL_GC_CHECK (4);\n"); */
      fprintf (f, "\tTTL_SAVE_CONT (descriptors + %d, %d);",
	       (int)instr->op0->data,
	       (int)instr->op1->data);
      break;
    case op_restore_cont:
      fprintf (f, "\tTTL_RESTORE_CONT;\n");
      fprintf (f, "\tbreak;");
      break;
    case op_call:
      fprintf (f, "\tpc = TTL_VALUE_TO_OBJ(ttl_descr, acc);\n");
      fprintf (f, "\tbreak;");
      break;
    case op_make_env:
      {
	int i;
	int params =(int) instr->op0->data;
	int locals =(int) instr->op1->data;
	fprintf (f, "\tTTL_MAKE_ENV (%d, %d);\n",
		 params, locals);
	for (i = params; i > 0; i--)
#if OLD_SP
	  fprintf (f, "\tenv->locals[%d] = ttl_stack[--sp];\n", i - 1);
#else
	  fprintf (f, "\tenv->locals[%d] = *(--sp);\n", i - 1);
#endif
      }
      break;
    case op_hd:
      fprintf (f, "\tTTL_NULL_CHECK;\n");
      fprintf (f, "\tacc = TTL_CAR (acc);");
      break;
    case op_tl:
      fprintf (f, "\tTTL_NULL_CHECK;\n");
      fprintf (f, "\tacc = TTL_CDR (acc);");
      break;
    case op_cons:
      fprintf (f, "\tTTL_CONS;");
      break;
    case op_sizeof:
      fprintf (f, "\tTTL_NULL_CHECK;\n");
      fprintf (f, "\tacc = TTL_INT_TO_VALUE (TTL_SIZE (acc));");
      break;

    case op_pop_env:
      {
	int i = (int) instr->op0->data;
	while (i > 0)
	  {
	    fprintf (f, "\tenv = TTL_VALUE_TO_OBJ (ttl_environment, env->parent);\n");
	    i--;
	  }
      }
      break;

    case op_mapped_call:
      {
	int count = (int) instr->op1->data;
	int i;
	fprintf (f, "\tTTL_SAVE_REGISTERS;\n\tttl_global_acc = ");
	emit_operand (f, instr->op0);
	fprintf (f, " (");
	for (i = 0; i < count; i++)
	  {
	    fprintf (f, "env->locals[%d]", i);
	    if (i < count - 1)
	      fprintf (f, ", ");
	  }
	fprintf (f, ");\n\tTTL_RESTORE_REGISTERS;");
      }
      break;

    case op_raise:
#if 0
      fprintf (f, "\tTTL_RAISE (acc);\n\tbreak;");
#endif
      fprintf (f, "\tgoto raise_exception;");
      break;

    case op_concat:
      fprintf (f, "\tif (!acc) goto raise_null_pointer_exception;\n");
#if OLD_SP
      fprintf
	(f,
	 "\tif (!ttl_stack[sp - 1]) goto raise_null_pointer_exception;\n");
#else
      fprintf
	(f,
	 "\tif (!*(sp - 1)) goto raise_null_pointer_exception;\n");
#endif
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf (f, "\tttl_global_acc = ttl_append_strings (ttl_stack[ttl_global_sp - 1], ttl_global_acc);\n");
      fprintf (f, "\tTTL_RESTORE_REGISTERS;\n");
      fprintf (f, "\tTTL_DROP();");
      break;

    case op_null_check:
      fprintf (f, "\tif (!acc) goto raise_null_pointer_exception;");
      break;
      
    case op_add_int_constraint:
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf (f, "\tttl_add_fd_constraint (%d, %d);\n",
	       (int) instr->op0->data,
	       (int) instr->op1->data);
      fprintf (f, "\tTTL_RESTORE_REGISTERS;");
      break;

    case op_add_real_constraint:
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf (f, "\tttl_add_real_constraint (%d, %d);\n",
	       (int) instr->op0->data,
	       (int) instr->op1->data);
      fprintf (f, "\tTTL_RESTORE_REGISTERS;");
      break;

    case op_resolve_int_constraint:
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf (f, "\tttl_fd_resolve ();\n");
      fprintf (f, "\tTTL_RESTORE_REGISTERS;");
      break;

    case op_resolve_real_constraint:
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf (f, "\tttl_real_resolve ();\n");
      fprintf (f, "\tTTL_RESTORE_REGISTERS;");
      break;

    case op_remove_constraint:
      fprintf (f, "\t/* Nothing to do yet.  */");
      break;

    case op_tick:
      fprintf (f, "\tif (--ttl_time_slice < 0)\n");
      fprintf (f, "\t  {\n\t    TTL_SAVE_CONT (descriptors + %d, 0);\n",
	       (int) instr->op0->data);
      fprintf (f, "\t    goto save_regs_and_return_tick;\n");
      fprintf (f, "\t  }");
      break;
      
    case op_make_int_variable:
      fprintf (f, "\tTTL_MAKE_FD_VARIABLE ();");
      break;

    case op_make_real_variable:
      fprintf (f, "\tTTL_MAKE_REAL_VARIABLE ();");
      break;

    case op_variable_ref:
      fprintf (f, "\tacc = TTL_VALUE_TO_OBJ (ttl_constrainable_variable, acc)->value;");
      break;

    case op_variable_set:
#if OLD_SP
      fprintf
	(f,
	 "\tTTL_VALUE_TO_OBJ (ttl_constrainable_variable, acc)->value = ttl_stack[--sp];");
#else
      fprintf
	(f,
	 "\tTTL_VALUE_TO_OBJ (ttl_constrainable_variable, acc)->value = *(--sp);");
#endif
      break;

    case op_coerce_to_constrained_array:
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf (f, "\tttl_coerce_to_constrained_array ();\n");
      fprintf (f, "\tTTL_RESTORE_REGISTERS;\n");
      break;

    case op_coerce_to_constrained_list:
      fprintf (f, "\tTTL_SAVE_REGISTERS;\n");
      fprintf (f, "\tttl_coerce_to_constrained_list ();\n");
      fprintf (f, "\tTTL_RESTORE_REGISTERS;\n");
      break;

    case op_load_foreign:
      fprintf (f, "\tacc = (ttl_value) ");
      ttl_symbol_print (f, (ttl_symbol) instr->op0->data);
      fprintf (f, ";");
      break;
    }
  fprintf (f, "\n");
}


/* Emit all instructions of the code object `obj' to the C source code
   file `f'.  */
static void
emit_object (FILE * f, ttl_object obj)
{
  ttl_instruction instr = obj->first;
  while (instr)
    {
      emit_instruction (f, instr);
      instr = instr->next;
    }
}

/* Emit the C code for the virtual instructions of function
   `function'.  This is expected to be called after the function
   prologue of the host procedure was emitted, and before the
   `default' case of the dispatch switch statement.  */
static void
emit_function (FILE * code_f, ttl_compile_state state, ttl_function function)
{
  fprintf (code_f, "/* Function ");
  ttl_symbol_print (code_f, function->name);
  fprintf (code_f, ": ");
  ttl_print_type (code_f, function->type);
  fprintf (code_f, ".  */\n");

  emit_object (code_f, (ttl_object) function->asm_code);
  fprintf (code_f, "\n");
}


/* Write all public declarations (variable and function locations) to
   the header file for module `module'.  */
static int
emit_header_file (ttl_compile_state state, ttl_module module,
		  struct ttl_compile_options * options)
{
  ttl_function function;
  ttl_variable variable;
  ttl_module_list module_list;
  FILE * header_f;
  char * header_name;

  header_name = ttl_basename (state->pool, state->filename);
  header_name = ttl_replace_file_ext (state->pool, header_name, ".h");
  header_f = fopen (header_name, "w");
  if (!header_f)
    {
      state->errors++;
      ttl_error_print_string (stderr, "turtle: cannot open header file: ");
      ttl_error_print_string (stderr, header_name);
      ttl_error_print_nl (stderr);
      return 1;
    }

  fprintf (header_f, "/* Created by Turtle %s -- DO NOT EDIT -- -*-c-*-  */\n\n",
	   __turtle_version);

  fprintf (header_f, "#include <libturtle/libturtlert.h>\n\n");

  variable = module->globals;
  while (variable)
    {
      if (variable->exported)
	{
	  fprintf (header_f, "/* Variable ");
	  ttl_symbol_print (header_f, variable->name);
	  fprintf (header_f, ": ");
	  ttl_print_type (header_f, variable->type);
	  fprintf (header_f, ".  */\n");

	  fprintf (header_f, "ttl_value ");
	  ttl_symbol_print (header_f, variable->unique_name);
	  fprintf (header_f, ";\n\n");
	}

      variable = variable->next;
    }

  function = module->functions;
  while (function)
    {
      if (function->exported)
	{
	  fprintf (header_f, "/* Function ");
	  ttl_symbol_print (header_f, function->name);
	  fprintf (header_f, ": ");
	  ttl_print_type (header_f, function->type);
	  fprintf (header_f, ".  */\n");

	  fprintf (header_f, "ttl_value ");
	  ttl_symbol_print (header_f, function->unique_name);
	  fprintf (header_f, ";\n\n");
	}
      function = function->total_next;
    }

  fprintf (header_f, "void _init_%s",
	   ttl_qualident_to_c_ident
	   (state->pool,
	    ttl_strip_annotation (module->module_ast_name)));
  fprintf (header_f, " (void);\n\n");

  fprintf (header_f, "/* End of file.  */\n");
  fclose (header_f);
  return 0;
}

static ttl_module_list
append_all_modules (char * buf, ttl_compile_state state, ttl_module module,
		    ttl_module_list done)
{
  ttl_module_list list = module->imported;

  while (list)
    {
      if (!list->module->recursing &&
	  !ttl_module_find (list->module->module_ast_name, done))
	{
	  list->module->recursing = 1;
	  done = append_all_modules (buf, state, list->module,
				     ttl_module_cons (state->pool,
						      list->module, done));
	  {
	    char * s, * p;
	    ttl_module mod = list->module;

	    s = ttl_qualident_to_filename
	      (state->pool, ttl_strip_annotation (mod->module_ast_name), ".o");

	    p = ttl_find_file (state->pool, s, state->module_path);
	    if (!p)
	      {
		state->errors++;
		fprintf (stderr, "turtle: cannot find file: %s\n", s);
	      }
	    else
	      {
		strcat (buf, " '");
		strcat (buf, p);
		strcat (buf, "'");
	      }
	  }
	  list->module->recursing = 0;
	}
      list = list->next;
    }
  return done;
}

static void
append_include_dirs (char * buf, char * module_path)
{
  unsigned len;
  char * sp;

  while (*module_path)
    {
      sp = module_path;
      while (*sp && *sp != ':')
	sp++;
      if (sp > module_path)
	{
	  strcat (buf, " -I");
	  len = strlen (buf);
	  memmove (buf + len, module_path, sp - module_path);
	  len += sp - module_path;
	  buf[len] = '\0';
	}
      if (*sp)
	module_path = sp + 1;
      else
	module_path = sp;
    }
}

static void
append_library_dirs (char * buf, char * module_path)
{
  unsigned len;
  char * sp;
  while (*module_path)
    {
      sp = module_path;
      while (*sp && *sp != ':')
	sp++;
      if (sp > module_path)
	{
	  strcat (buf, " -L");
	  len = strlen (buf);
	  memmove (buf + len, module_path, sp - module_path);
	  len += sp - module_path;
	  buf[len] = '\0';
	}
      if (*sp)
	module_path = sp + 1;
      else
	module_path = sp;
    }
}

/* Run the C compiler on the produced C source code.  Return 0 on
   success, != 0 if an error occurs.  */
static int
c_compile (ttl_compile_state state, ttl_module module, 
	 struct ttl_compile_options * options)
{
  char * c_name;
  char * o_name;
  char * so_name;
  char * base_name;
  static char buf[1024 * 4];
  int ret;

  base_name = ttl_basename (state->pool, state->filename);
  c_name = ttl_replace_file_ext (state->pool, base_name, ".c");
  o_name = ttl_replace_file_ext (state->pool, base_name, ".o");
  so_name = ttl_replace_file_ext (state->pool, base_name, ".so");

  sprintf (buf, "gcc -g -c -I" TOP_SRC_DIR " ");

  append_include_dirs (buf, state->module_path);
  append_include_dirs (buf, state->include_path);
  append_library_dirs (buf, state->library_path);

  if (options->opt_gcc_level > 2)
    sprintf (buf + strlen (buf), " -O%d -fomit-frame-pointer -funroll-loops ",
	     options->opt_gcc_level);
  else
    sprintf (buf + strlen (buf), " -O%d", options->opt_gcc_level);

  sprintf (buf + strlen (buf), " '%s' -o '%s'", c_name, o_name);

  if (options->verbose > 0)
    fprintf (stderr, "[%s]\n", buf);
  if (system (buf) != 0)
    {
      state->errors++;
      ttl_error_print_string (stderr, "turtle: cannot run the C compiler (");
      ttl_error_print_string (stderr, buf);
      ttl_error_print_string (stderr, ")");
      ttl_error_print_nl (stderr);
      return 1;
    }

#if COMPILE_SHARED
  {
    char * hackdir = getenv ("TURTLE_HACKING");
    sprintf (buf, "gcc -g -fpic -shared -lturtlert -L%s -I" TOP_SRC_DIR " ",
	     hackdir ? hackdir : LIBRARY_DIR);

    append_include_dirs (buf, state->module_path);
    append_include_dirs (buf, state->include_path);
    append_library_dirs (buf, state->library_path);

    if (options->opt_gcc_level > 2)
      sprintf (buf + strlen (buf), " -O%d -fomit-frame-pointer -funroll-loops ",
	       options->opt_gcc_level);
    else
      sprintf (buf + strlen (buf), " -O%d", options->opt_gcc_level);

    sprintf (buf + strlen (buf), " '%s' -o '%s'", c_name, so_name);

    {
      char * p = ttl_string_append (state->pool, buf, " -lm");
#if HAVE_LIBNSL
      p = ttl_string_append (state->pool, p, " -lnsl");
#endif
#if HAVE_LIBSOCKET
      p = ttl_string_append (state->pool, p, " -lsocket");
#endif
#if HAVE_LIBDL
      p = ttl_string_append (state->pool, p, " -ldl");
#endif
      if (options->verbose > 0)
	fprintf (stderr, "[%s]\n", p);
      if (system (p) != 0)
	{
	  state->errors++;
	  ttl_error_print_string (stderr, "turtle: cannot run the C compiler (");
	  ttl_error_print_string (stderr, p);
	  ttl_error_print_string (stderr, ")");
	  ttl_error_print_nl (stderr);
	  return 1;
	}
    }
  }
#endif

  if (options->main)
    {
      char * exe_name;
      char * hackdir;

      if (options->program_name && strlen (options->program_name) > 0)
	exe_name = options->program_name;
      else
	{
	  exe_name = ttl_basename (state->pool, state->filename);
	  exe_name = ttl_replace_file_ext (state->pool, exe_name, "");
	}

      /* The following is a hack to allow building turtle programs
	 with an uninstalled compiler.  Just set the environment
	 variable TURTLE_HACKING to the directory where your turtle
	 libraries are (the real ones, not the libtool libraries). */
      hackdir = getenv ("TURTLE_HACKING");
      if (!hackdir) {
	  hackdir = options->library_path;
      }

      sprintf (buf,
	       "gcc %s-g '%s' -lturtlert -L%s -o '%s' ",
	       options->link_static ? "-static " : "",
	       o_name, hackdir ? hackdir : LIBRARY_DIR, exe_name);
#if LINK_TURTLE0
      {
	char * p;
	p = ttl_find_file (state->pool, "turtle0.o", state->module_path);
	if (!p)
	  {
	    state->errors++;
	    fprintf (stderr, "turtle: cannot find file: turtle0.o\n");
	    return 1;
	  }
	else
	  {
	    strcat (buf, " '");
	    strcat (buf, p);
	    strcat (buf, "'");
	  }
      }
#endif
      append_all_modules (buf, state, module,
			  ttl_module_cons (state->pool, module, NULL));
      {
	char * p = ttl_string_append (state->pool, buf, " -lm");
#if HAVE_LIBNSL
	p = ttl_string_append (state->pool, p, " -lnsl");
#endif
#if HAVE_LIBSOCKET
	p = ttl_string_append (state->pool, p, " -lsocket");
#endif
#if HAVE_LIBDL
	p = ttl_string_append (state->pool, p, " -ldl");
#endif
	if (options->verbose > 0)
	  fprintf (stderr, "[%s]\n", p);
	if (system (p) != 0)
	  {
	    state->errors++;
	    ttl_error_print_string
	      (stderr, "turtle: cannot run the C compiler (");
	    ttl_error_print_string (stderr, p);
	    ttl_error_print_string (stderr, ")");
	    ttl_error_print_nl (stderr);
	    return 1;
	  }
      }
    }
  return 0;
}

static int
find_location (ttl_function functions, int label_no, int * line)
{
  ttl_object obj;
  ttl_instruction instr;
  int last_line = -1;

  while (functions)
    {
      obj = (ttl_object) functions->asm_code;
      functions = functions->total_next;

      instr = obj->first;
      while (instr)
	{
	  if (instr->filename)
	    {
	      last_line = instr->line;
	    }
	  if (instr->op == op_label || instr->op == op_cont_label ||
	      instr->op == op_proc_label)
	    {
	      if ((int) (instr->op0->data) == label_no)
		break;
	    }
	  instr = instr->next;
	}
      if (instr)
	{
	  while (instr)
	    {
	      if (instr->filename)
		break;
	      instr = instr->next;
	    }

	  if (instr)
	    {
	      *line = last_line;
	      return 1;
	    }
	}
    }
  return 0;
}


/* Find the handcoded implementation file of module `module' and copy
   it to the C source code file `code_f'.  Return 0 on success and !=
   0 if anything goes wrong.  */
static int
copy_handcoded_part (ttl_compile_state state, FILE * code_f, ttl_module module)
{
  char * include_name;
  FILE * include_f;

  include_name = ttl_replace_file_ext (state->pool, state->filename, ".t.i");
  include_f = fopen (include_name, "r");
  if (!include_f)
    {
      state->errors++;
      ttl_error_print_string (stderr, "turtle: cannot open handcoded file: ");
      ttl_error_print_string (stderr, include_name);
      ttl_error_print_nl (stderr);
      return 1;
    }

  {
    int c = fgetc (include_f);
    while (c != EOF)
      {
	fputc (c, code_f);
	c = fgetc (include_f);
      }
  }
  fclose (include_f);
  return 0;
}

/* Emit the compiled code of module `module' as C source code,
   producing the `.h' and `.c' files necessary for the module, then
   invoke the C compiler to produce the object and (if requested) the
   executable for the module.  Return 0 if all went well, != 0
   otherwise.  */
int
ttl_emit_c (ttl_compile_state state, ttl_module module,
	    struct ttl_compile_options * options)
{
  ttl_function function;
  ttl_variable variable;
  ttl_module_list module_list;
  FILE * code_f;
  char * code_name;
  int ret;

  code_name = ttl_basename (state->pool, state->filename);
  code_name = ttl_replace_file_ext (state->pool, code_name, ".c");
  code_f = fopen (code_name, "w");
  if (!code_f)
    {
      state->errors++;
      ttl_error_print_string (stderr, "turtle: cannot open code file: ");
      ttl_error_print_string (stderr, code_name);
      ttl_error_print_nl (stderr);
      return 1;
    }

  fprintf (code_f, "/* Created by Turtle %s -- DO NOT EDIT -- -*-c-*-  */\n\n",
	   __turtle_version);

  fprintf (code_f, "#include <stdio.h>\n\n");
  fprintf (code_f, "#include <libturtle/libturtlert.h>\n\n");

  module_list = module->imported;
  while (module_list)
    {
      ttl_module mod = module_list->module;
      fprintf (code_f, "#include \"%s\"\n",
	       ttl_qualident_to_filename
	       (state->pool,
		ttl_strip_annotation (mod->module_ast_name), ".h"));
      module_list = module_list->next;
    }
  fprintf (code_f, "\n");

  if (options->pragma_handcoded)
    {
      if ((ret = copy_handcoded_part (state, code_f, module)) != 0)
	return ret;
    }

  variable = module->globals;
  while (variable)
    {
      fprintf (code_f, "/* Variable ");
      ttl_symbol_print (code_f, variable->name);
      fprintf (code_f, ": ");
      ttl_print_type (code_f, variable->type);
      fprintf (code_f, ".  */\n");

      if (!variable->exported)
	fprintf (code_f, "static ");
      fprintf (code_f, "ttl_value ");
      ttl_symbol_print (code_f, variable->unique_name);
      fprintf (code_f, ";\n\n");

      variable = variable->next;
    }

  fprintf (code_f, "static struct ttl_descr descriptors[];\n\n");

  function = module->toplevel_functions;
  while (function)
    {
      fprintf (code_f, "/* Function ");
      ttl_symbol_print (code_f, function->name);
      fprintf (code_f, ": ");
      ttl_print_type (code_f, function->type);
      fprintf (code_f, ".  */\n");

      if (!function->exported)
	fprintf (code_f, "static ");
      fprintf (code_f, "ttl_value ");
      ttl_symbol_print (code_f, function->unique_name);
      fprintf (code_f, " = TTL_OBJ_TO_VALUE (descriptors + %d)",
	       function->index);
      fprintf (code_f, ";\n\n");
      function = function->next;
    }


  fprintf (code_f, "\n");

  function = module->functions;
  while (function)
    {
      fprintf (code_f, "static struct ttl_function_info func_info%d", function->index);
      fprintf (code_f, " = {\"");
      ttl_symbol_print (code_f, function->name);
/*       fprintf (code_f, ": "); */
/*       ttl_print_type (code_f, function->type); */
      fprintf (code_f, "\", \"");
      ttl_ast_print (code_f, module->module_ast_name, 0);
      fprintf (code_f, "\", \"%s\"};\n", state->filename);
      function = function->total_next;
    }

  fprintf (code_f, "\n");

  fprintf (code_f,
	   "static int host_procedure (void);\n\n");

  fprintf (code_f,
	   "static struct ttl_descr descriptors[] =\n  {\n");

  {
    int i = 0;
    int last_index = module->functions ? module->functions->index : -1;
    int line = -1;
    while (i < state->label_count)
      {
	line = -1;
	if (((ttl_instruction *)state->mapping)[i])
	  {
	    int j = i;
	    line = ((ttl_instruction *)state->mapping)[j]->line;
	    while (j > 0 && line < 0)
	      {
		j--;
		if (((ttl_instruction *)state->mapping)[j])
		  line = ((ttl_instruction *)state->mapping)[j]->line;
	      }
	  }

	function = module->functions;
	while (function && function->index != (unsigned) i)
	  function = function->total_next;
	if (function)
	  last_index = function->index;
	if (last_index >= 0)
	  {
	    fprintf
	      (code_f,
	       "    {TTL_DESCRIPTOR_HEADER, host_procedure, &func_info%d, %d} /* %d */",
	       last_index, line, i);
	  }
	else
	  {
	    fprintf
	      (code_f,
	       "    {TTL_DESCRIPTOR_HEADER, host_procedure, NULL, %d} /* %d */", 
	       line, i);
	  }
	i++;
	if (i < state->label_count)
	  fprintf (code_f, ",\n");
	else
	  fprintf (code_f, "\n");
      }

    fprintf (code_f, "  };\n\n");
  }


  fprintf (code_f, "\nstatic int\n"
	   "host_procedure (void)\n"
	   "{\n"
	   "  ttl_value acc;\n"
	   "  ttl_value * sp;\n"
	   "  ttl_value * alloc;\n"
	   "  ttl_environment env;\n"
	   "  ttl_descr pc;\n"
	   "  ttl_closure self = NULL;\n\n"
	   "  TTL_RESTORE_REGISTERS;\n"
	   " L_jump:\n"
	   "  switch (pc - descriptors)\n"
	   "    {\n");
  {
    function = module->functions;
    while (function)
      {
	emit_function (code_f, state, function);
	function = function->total_next;
      }
  }

  fprintf (code_f,
	   "    default:\n"
	   "      if (pc->host == host_procedure)\n"
	   "	{\n"
	   "	  self = (ttl_closure) pc;\n"
	   "	  pc = TTL_VALUE_TO_OBJ (ttl_descr, self->code);\n"
	   "	  env = TTL_VALUE_TO_OBJ (ttl_environment, self->env);\n"
	   "	  ttl_stats.closure_call_count++;\n"
	   "	  goto L_jump;\n"
	   "	}\n"
	   "      break;\n"
	   "    restore_cont:\n"
	   "      TTL_RESTORE_CONT_REALLY;\n"
	   "    }\n");
  if (options->opt_local_calls)
    fprintf (code_f,
	     "  if (pc->host == host_procedure)\n"
	     "    {\n"
	     "      ttl_stats.local_call_count++;\n"
	     "      goto L_jump;\n"
	     "    }\n");
  fprintf (code_f, 
	   "save_regs_and_return:\n"
	   "  TTL_SAVE_REGISTERS;\n"
	   "  return 0;\n"
	   "save_regs_and_return_tick:\n"
	   "  TTL_SAVE_REGISTERS;\n"
	   "  return 1;\n"
	   "raise_null_pointer_exception:\n"
	   "  acc = ttl_null_pointer_exception;\n"
	   "  goto raise_exception;\n"
	   "raise_subscript_exception:\n"
	   "  acc = ttl_subscript_exception;\n"
	   "  goto raise_exception;\n"
	   "raise_exception:\n"
	   "  TTL_RAISE (acc);\n"
	   "}\n\n");

  fprintf (code_f, "void\n_init_%s", 
	   ttl_qualident_to_c_ident
	   (state->pool,
	    ttl_strip_annotation (module->module_ast_name)));
  fprintf (code_f,
	   " (void)\n{\n  static int initialize = 1;\n\n"
	   "  if (initialize)\n    {\n      initialize = 0;\n");
#if LINK_TURTLE0
  if (options->main)
    {
      fprintf (code_f, "      _init_turtle0 ();\n");
    }
#endif
  module_list = module->imported;
  while (module_list)
    {
      ttl_module mod = module_list->module;
      fprintf (code_f, "      _init_%s ();\n",
	       ttl_qualident_to_c_ident
	       (state->pool,
		ttl_strip_annotation (mod->module_ast_name)));
      module_list = module_list->next;
    }
#if 0
  function = module->toplevel_functions;
  while (function)
    {
      fprintf (code_f, "      ");
      ttl_symbol_print (code_f, function->unique_name);
      fprintf (code_f, " = TTL_OBJ_TO_VALUE (descriptors + %d);\n",
	       function->index);
      function = function->next;
    }
#endif
  variable = module->globals;
  while (variable)
    {
      /* Do not register variables for primitive types as garbage
	 collection roots.  */
      ttl_type type = variable->type;
      if (type->kind != type_integer && type->kind != type_bool &&
	  type->kind != type_char)
	{
	  fprintf (code_f, "      ttl_register_root (&");
	  ttl_symbol_print (code_f, variable->unique_name);
	  fprintf (code_f, ");\n");
	}
      variable = variable->next;
    }
  variable = module->globals;
  while (variable)
    {
      /* Do not register variables for primitive types as garbage
	 collection roots.  */
      ttl_type type = variable->type;
      if (type->kind == type_constrained)
	{
	  fprintf (code_f, "      ");
	  ttl_symbol_print (code_f, variable->unique_name);
	  if (type->d.constrained.base->kind == type_integer ||
	      type->d.constrained.base->kind == type_long ||
	      type->d.constrained.base->kind == type_bool ||
	      type->d.constrained.base->kind == type_char)
	    fprintf (code_f, " = ttl_alloc_fd_variable ();\n");
	  else if (type->d.constrained.base->kind == type_real)
	    fprintf (code_f, " = ttl_alloc_real_variable ();\n");
	  else
	    abort ();
	}
      variable = variable->next;
    }
  if (state->has_init_stmts)
    {
      function = module->toplevel_functions;
      while (function->next)
	function = function->next;
      fprintf (code_f, "      ttl_init_dispatcher (");
      ttl_symbol_print (code_f, function->unique_name);
      fprintf (code_f, ");\n");
    }
  fprintf (code_f, "    }\n}\n\n");

  if (options->main)
    {
      fprintf (code_f,
	       "int\n"
	       "main (int argc, char * argv[])\n"
	       "{\n");
      fprintf (code_f,
	       "  ttl_initialize (argc, argv);\n");
      fprintf (code_f,
	       "  _init_%s ();\n",
	       ttl_qualident_to_c_ident
	       (state->pool,
		ttl_strip_annotation (module->module_ast_name)));

      fprintf (code_f,
	       "  ttl_dispatcher (%s_main_pF1pLpS_pI);\n"
	       "  return 0;\n"
	       "}\n\n",
	       ttl_qualident_to_c_ident
	       (state->pool,
		ttl_strip_annotation (module->module_ast_name)));
    }
  fprintf (code_f, "/* End of file.  */\n");
  fclose (code_f);

  if ((ret = emit_header_file (state, module, options)) != 0)
    return ret;
  if (!options->pragma_compile_only)
    return c_compile (state, module, options);
  return 0;
}

/* End of emit-c.c.  */
