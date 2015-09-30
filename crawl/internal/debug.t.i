/* internal/debug.t.i -- C implementation for debug.t.       -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `internal.debug' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


/* Function abort: fun(): ().  */
#define internal_debug_abort_pF0_pV_implementation \
  abort ();

/* Function examine: fun(any): ().  */
#define internal_debug_examine_pF1pY_pV_implementation \
{								\
  acc = env->locals[0];						\
  TTL_SAVE_REGISTERS;						\
  ttl_examine ();						\
  TTL_RESTORE_REGISTERS;					\
}

/* End of internal/debug.t.i.  */
