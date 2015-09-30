/* internal/binary.t.i -- C implementation for binary.t.        -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `internal.binary' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


/* Function make: fun(int): internal.binary.binary.  */
#define internal_binary_make_pF1pI_ubinary_implementation \
{									\
  TTL_SAVE_REGISTERS;							\
  {									\
    int len = TTL_VALUE_TO_INT (env->locals[0]);			\
    ttl_global_acc = ttl_alloc_binary_array (len);			\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* Function get: fun(internal.binary.binary, int): int.  */
#define internal_binary_get_pF2ubinarypI_pI_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  TTL_SAVE_REGISTERS;							\
  {									\
    ttl_binary_array arr = TTL_VALUE_TO_OBJ (ttl_binary_array,		\
					     env->locals[0]);		\
    int index = TTL_VALUE_TO_INT (env->locals[1]);			\
									\
    if (index < 0 || index >= TTL_SIZE (env->locals[0]))		\
      TTL_RAISE (ttl_subscript_exception);				\
									\
    ttl_global_acc = TTL_INT_TO_VALUE (arr->data[index]);		\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* Function binary_set: fun(internal.binary.binary, int, int): ().  */
#define internal_binary_set_pF3ubinarypIpI_pV_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  TTL_SAVE_REGISTERS;							\
  {									\
    ttl_binary_array arr = TTL_VALUE_TO_OBJ (ttl_binary_array,		\
					     env->locals[0]);		\
    int index = TTL_VALUE_TO_INT (env->locals[1]);			\
    int value = TTL_VALUE_TO_INT (env->locals[2]);			\
									\
    if (index < 0 || index >= TTL_SIZE (env->locals[0]))		\
      TTL_RAISE (ttl_subscript_exception);				\
									\
    if (value < 0 || value >= 256)					\
      TTL_RAISE (ttl_out_of_range_exception);				\
									\
    arr->data[index] = value;						\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* Function binary_size: fun(internal.binary.binary): int.  */
#define internal_binary_size_pF1ubinary_pI_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  acc = TTL_INT_TO_VALUE (TTL_SIZE (env->locals[0]));			\
}

/* End of internal.binary.t.i.  */
