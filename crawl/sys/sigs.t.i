/* sys/sigs.t.i -- C implementation for sigs.t.       -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `sys.sigs' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


#include <signal.h>


/* Function signal: fun(int, fun(int): ()): ().  */
#define sys_sigs_signal_pF2pIpF1pI_pV_pV_implementation \
  ttl_install_signal_handler (TTL_VALUE_TO_INT (env->locals[0]),	\
			      env->locals[1]);

/* End of sys/sigs.t.i.  */
