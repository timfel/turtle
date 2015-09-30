/* internal/version.t.i -- C implementation for version.t.      -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `internal.version' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


/* Function version: fun(): string.  */
#define internal_version_version_pF0_pS_implementation \
{									\
  TTL_SAVE_REGISTERS;							\
  {									\
    ttl_global_acc = ttl_string_to_value (ttl_version_string (), -1);	\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* End of internal/version.t.i.  */
