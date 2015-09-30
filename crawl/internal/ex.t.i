/* internal/ex.t.i -- C implementation for ex.t.            -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `internal.ex' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


/* Function raise: fun(string): ().  */
#define	internal_ex_raise_pF1pS_pV_implementation \
{							\
  TTL_RAISE (env->locals[0]);				\
}


/* Function handle: fun(fun(): (), fun(string): ()): ().  */
#define	internal_ex_handle_pF2pF0_pVpF1pS_pV_pV_implementation \
{									\
  *sp++ = ttl_global_cont;					\
  acc = env->locals[1];							\
  TTL_CONS;								\
  *sp++ = acc;						\
  acc = ttl_exception_handler;						\
  TTL_CONS;								\
  ttl_exception_handler = acc;						\
  pc = TTL_VALUE_TO_OBJ (ttl_descr, env->locals[0]);			\
  goto save_regs_and_return;						\
}


/* Function null_pointer_exception: fun(): string.  */
#define internal_ex_null_pointer_exception_pF0_pS_implementation \
{									\
  acc = ttl_null_pointer_exception;					\
}

/* Function out_of_range_exception: fun(): string.  */
#define internal_ex_out_of_range_exception_pF0_pS_implementation \
{									\
  acc = ttl_out_of_range_exception;					\
}

/* Function subscript_exception: fun(): string.  */
#define internal_ex_subscript_exception_pF0_pS_implementation \
{									\
  acc = ttl_subscript_exception;					\
}

/* Function wrong_variant_exception: fun(): string.  */
#define internal_ex_wrong_variant_exception_pF0_pS_implementation \
{									\
  acc = ttl_wrong_variant_exception;					\
}

/* Function require_exception: fun(): string.  */
#define internal_ex_require_exception_pF0_pS_implementation \
{								\
  acc = ttl_require_exception;					\
}

/* End of internal/ex.t.i.  */
