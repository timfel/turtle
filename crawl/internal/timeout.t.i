/* internal/timeout.t.i -- C implementation for timeout.t.       -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `internal.timeout' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


/* Function set: fun(fun(): ()): ().  */
#define internal_timeout_set_pF1pF0_pV_pV_implementation \
  ttl_install_timer_handler (env->locals[0]);

/* Function clear: fun(): ().  */
#define internal_timeout_clear_pF0_pV_implementation \
  ttl_install_timer_handler (NULL);

/* End of internal/timeout.t.i.  */
