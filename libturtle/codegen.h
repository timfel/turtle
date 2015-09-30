/* libturtle/codegen.h - Code generator.
 
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

#ifndef TTL_CODEGEN_H
#define TTL_CODEGEN_H

#include "memory.h"
#include "compiler.h"

enum ttl_link
  {
    link_next,
    link_return,
    link_label,
    link_proc
  };

enum ttl_operand_kind
  {
    operand_label,
    operand_constant,
    operand_local,
    operand_mem
  };

typedef struct ttl_operand * ttl_operand;
struct ttl_operand
{
  enum ttl_operand_kind op;
  void * data;
  unsigned unsigned_data;
};

ttl_operand ttl_make_operand (ttl_pool pool, enum ttl_operand_kind op,
			      void * data);

enum ttl_op_kind
  {op_load,
   op_store,
   op_push,
   op_pop,
   op_add,
   op_sub,
   op_mul,
   op_div,
   op_mod,
   op_fadd,
   op_fsub,
   op_fmul,
   op_fdiv,
   op_fmod,
   op_ladd,
   op_lsub,
   op_lmul,
   op_ldiv,
   op_lmod,
   op_jump,
   op_jump_proc,
   op_jump_if_false,
   op_jump_if_true,
   op_call,
   op_label,
   op_cont_label,
   op_proc_label,
   op_note_label,
   op_save_cont,
   op_restore_cont,
   op_jump_if_equal,
   op_jump_if_not_equal,
   op_jump_if_less,
   op_jump_if_not_less,
   op_jump_if_gtr,
   op_jump_if_not_gtr,
   op_jump_if_fequal,
   op_jump_if_not_fequal,
   op_jump_if_fless,
   op_jump_if_not_fless,
   op_jump_if_fgtr,
   op_jump_if_not_fgtr,
   op_jump_if_lequal,
   op_jump_if_not_lequal,
   op_jump_if_lless,
   op_jump_if_not_lless,
   op_jump_if_lgtr,
   op_jump_if_not_lgtr,
   op_not,
   op_neg,
   op_fneg,
   op_lneg,
   op_hd,
   op_tl,
   op_sizeof,
   op_cons,
   op_aload,
   op_astore,
   op_sload,
   op_sstore,
   op_load_int,
   op_load_long,
   op_load_null,
   op_load_false,
   op_load_true,
   op_load_real,
   op_load_string,
   op_load_char,
   op_make_env,
   op_gc_check,
   op_make_closure,
   op_macro_call,
   op_null_env_reg,
   op_make_array,
   op_make_constrained_array,
   op_make_string,
   op_make_list,
   op_make_tuple,
   op_make_data,
   op_tuple_ref,
   op_tuple_set,
   op_create_array,
   op_array_pop,
   op_pop_env,
   op_mapped_call,
   op_dup,
   op_over,
   op_drop,
   op_raise,
   op_concat,
   op_null_check,
   op_add_int_constraint,
   op_add_real_constraint,
   op_resolve_int_constraint,
   op_resolve_real_constraint,
   op_remove_constraint,
   op_tick,
   op_make_int_variable,
   op_make_real_variable,
   op_variable_ref,
   op_variable_set,
   op_coerce_to_constrained_array,
   op_coerce_to_constrained_list,
   op_load_foreign
  };

typedef struct ttl_instruction * ttl_instruction;
struct ttl_instruction
{
  ttl_instruction prev;
  ttl_instruction next;
  enum ttl_op_kind op;
  ttl_operand op0;
  ttl_operand op1;
  char * filename;
  int line;
};

ttl_instruction ttl_make_instruction (ttl_pool pool, enum ttl_op_kind op,
				      ttl_operand op0,
				      ttl_operand op1,
				      char * filename,
				      int line);

typedef struct ttl_object * ttl_object;
struct ttl_object
{
  ttl_instruction first;
  ttl_instruction last;
};

ttl_object ttl_make_object (ttl_pool pool);

void ttl_append_instruction (ttl_object obj, ttl_instruction instr);

void ttl_generate_code (ttl_compile_state state, ttl_module module);

#endif /* not TTL_CODEGEN_H */
