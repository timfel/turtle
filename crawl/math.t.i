/* math.t.i -- C implementation for math.t.              -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `math' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


/* Function sin: fun(real): real.  */
#define math_sin_pF1pR_pR_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  TTL_SAVE_REGISTERS;							\
  {									\
    double sin (double);						\
    double r = TTL_VALUE_TO_OBJ (ttl_real, ttl_global_acc)->value;	\
    ttl_global_acc = ttl_real_to_value (sin (r));			\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* Function asin: fun(real): real.  */
#define math_asin_pF1pR_pR_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  TTL_SAVE_REGISTERS;							\
  {									\
    double asin (double);						\
    double r = TTL_VALUE_TO_OBJ (ttl_real, ttl_global_acc)->value;	\
    ttl_global_acc = ttl_real_to_value (asin (r));			\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* Function cos: fun(real): real.  */
#define math_cos_pF1pR_pR_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  TTL_SAVE_REGISTERS;							\
  {									\
    double cos (double);						\
    double r = TTL_VALUE_TO_OBJ (ttl_real, ttl_global_acc)->value;	\
    ttl_global_acc = ttl_real_to_value (cos (r));			\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* Function acos: fun(real): real.  */
#define math_acos_pF1pR_pR_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  TTL_SAVE_REGISTERS;							\
  {									\
    double acos (double);						\
    double r = TTL_VALUE_TO_OBJ (ttl_real, ttl_global_acc)->value;	\
    ttl_global_acc = ttl_real_to_value (acos (r));			\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* Function tan: fun(real): real.  */
#define math_tan_pF1pR_pR_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  TTL_SAVE_REGISTERS;							\
  {									\
    double r = TTL_VALUE_TO_OBJ (ttl_real, ttl_global_acc)->value;	\
    ttl_global_acc = ttl_real_to_value (tan (r));			\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* Function atan: fun(real): real.  */
#define math_atan_pF1pR_pR_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  TTL_SAVE_REGISTERS;							\
  {									\
    double r = TTL_VALUE_TO_OBJ (ttl_real, ttl_global_acc)->value;	\
    ttl_global_acc = ttl_real_to_value (atan (r));			\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* Function atan: fun(real, real): real.  */
#define math_atan_pF2pRpR_pR_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  acc = env->locals[1];							\
  TTL_NULL_CHECK;							\
  TTL_SAVE_REGISTERS;							\
  {									\
    double r1 = TTL_VALUE_TO_OBJ (ttl_real, env->locals[0])->value;	\
    double r2 = TTL_VALUE_TO_OBJ (ttl_real, env->locals[1])->value;	\
    ttl_global_acc = ttl_real_to_value (atan2 (r1, r2));		\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* End of math.t.i.  */
