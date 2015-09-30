/* sys/errno.t.i -- C implementation for errno.t.           -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `sys.errno' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */

#include <string.h>

/* Function strerror: fun(int): string.  */
#define sys_errno_strerror_pF1pI_pS_implementation \
{							\
  TTL_SAVE_REGISTERS;					\
  ttl_global_acc = ttl_string_to_value 			\
    (strerror (TTL_VALUE_TO_INT (env->locals[0])), -1);	\
  TTL_RESTORE_REGISTERS;				\
}

/* End of sys/errno.t.i.  */
