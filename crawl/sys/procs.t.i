/* sys/procs.t.i -- C implementation for procs.t.           -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `sys.procs' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */

#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>

/* Function getpid: fun(): int.  */
#define sys_procs_getpid_pF0_pI_implementation \
{							\
  acc = TTL_INT_TO_VALUE (getpid ());			\
}


/* Function getppid: fun(): int.  */
#define sys_procs_getppid_pF0_pI_implementation \
{							\
  acc = TTL_INT_TO_VALUE (getppid ());			\
}


/* Function exit: fun(int): ().  */
#define sys_procs_exit_pF1pI_pV_implementation \
  exit (TTL_VALUE_TO_INT (env->locals[0]));


/* Function sleep: fun(int): int.  */
#define sys_procs_sleep_pF1pI_pI_implementation \
  acc = TTL_INT_TO_VALUE (sleep (TTL_VALUE_TO_INT (env->locals[0])));


/* Function kill: fun(int, int): int.  */
#define sys_procs_kill_pF2pIpI_pI_implementation			\
{									\
  acc = TTL_INT_TO_VALUE (kill (TTL_VALUE_TO_INT (env->locals[0]),	\
                                TTL_VALUE_TO_INT (env->locals[1])));	\
  sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);			\
}


/* Function fork: fun(): int.  */
#define sys_procs_fork_pF0_pI_implementation \
{						\
  acc = TTL_INT_TO_VALUE (fork ());		\
  sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);	\
}


/* Function wait: fun(): (int, int).  */
#define sys_procs_wait_pF0_pT2pIpI_implementation \
{									      \
  TTL_GC_CHECK (3);							      \
  TTL_SAVE_REGISTERS;							      \
  {									      \
    int pid, status;							      \
    ttl_value tup;							      \
									      \
    pid = wait (&status);						      \
    tup = ttl_unsafe_alloc_array (2);					      \
    TTL_VALUE_TO_OBJ (ttl_array, tup)->data[0] = TTL_INT_TO_VALUE (pid);    \
    TTL_VALUE_TO_OBJ (ttl_array, tup)->data[1] = TTL_INT_TO_VALUE (status); \
    ttl_global_acc = tup;						      \
  }									      \
  TTL_RESTORE_REGISTERS;						      \
}


/* Function waitpid: fun(int, int): int.  */
#define sys_procs_waitpid_pF2pIpI_pT2pIpI_implementation \
{									    \
  TTL_GC_CHECK (3);							    \
  TTL_SAVE_REGISTERS;							    \
  {									    \
    int pid, status, options;						    \
    ttl_value tup;							    \
									    \
    pid = waitpid (TTL_VALUE_TO_INT (env->locals[0]), &status,		    \
		   TTL_VALUE_TO_INT (env->locals[1]));			    \
    tup = ttl_unsafe_alloc_array (2);					    \
    TTL_VALUE_TO_OBJ (ttl_array, tup)->data[0] = TTL_INT_TO_VALUE (pid);    \
    TTL_VALUE_TO_OBJ (ttl_array, tup)->data[1] = TTL_INT_TO_VALUE (status); \
    ttl_global_acc = tup;						    \
  }									    \
  TTL_RESTORE_REGISTERS;						    \
}


/* Function WIFEXITED: fun(int): bool.  */
#define sys_procs_WIFEXITED_pF1pI_pB_implementation \
  acc = TTL_BOOL_TO_VALUE (WIFEXITED (TTL_VALUE_TO_INT (env->locals[0])));


/* Function WEXITSTATUS: fun(int): int.  */
#define sys_procs_WEXITSTATUS_pF1pI_pI_implementation \
  acc = TTL_INT_TO_VALUE (WEXITSTATUS (TTL_VALUE_TO_INT (env->locals[0])));


/* Function WIFSIGNALED: fun(int): bool.  */
#define sys_procs_WIFSIGNALED_pF1pI_pB_implementation \
  acc = TTL_BOOL_TO_VALUE (WIFSIGNALED (TTL_VALUE_TO_INT (env->locals[0])));


/* Function WTERMSIG: fun(int): int.  */
#define sys_procs_WTERMSIG_pF1pI_pI_implementation \
  acc = TTL_INT_TO_VALUE (WTERMSIG (TTL_VALUE_TO_INT (env->locals[0])));


/* Function WIFSTOPPED: fun(int): bool.  */
#define sys_procs_WIFSTOPPED_pF1pI_pB_implementation \
  acc = TTL_BOOL_TO_VALUE (WIFSTOPPED (TTL_VALUE_TO_INT (env->locals[0])));


/* Function WSTOPSIG: fun(int): int.  */
#define sys_procs_WSTOPSIG_pF1pI_pI_implementation \
  acc = TTL_INT_TO_VALUE (WSTOPSIG (TTL_VALUE_TO_INT (env->locals[0])));


/* Function execve: fun(string, array of string, array of string): int.  */
#define sys_procs_execve_pF3pSpApSpApS_pI_implementation		\
{									\
  int i;								\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  acc = env->locals[1];							\
  TTL_NULL_CHECK;							\
  for (i = 0; i < TTL_SIZE (env->locals[1]); i++)			\
    {									\
      acc = TTL_VALUE_TO_OBJ (ttl_array, env->locals[1])->data[i];	\
      TTL_NULL_CHECK;							\
    }									\
  acc = env->locals[2];							\
  TTL_NULL_CHECK;							\
  for (i = 0; i < TTL_SIZE (env->locals[2]); i++)			\
    {									\
      acc = TTL_VALUE_TO_OBJ (ttl_array, env->locals[2])->data[i];	\
      TTL_NULL_CHECK;							\
    }									\
  TTL_SAVE_REGISTERS;							\
  {									\
    char * filename;							\
    char ** argv, ** envp;						\
    int res;								\
    filename = ttl_malloc_c_string (env->locals[0]);			\
    argv = ttl_malloc_c_string_array (env->locals[1]);			\
    envp = ttl_malloc_c_string_array (env->locals[2]);			\
									\
    res = execve (filename, argv, envp);				\
    ttl_global_acc = TTL_INT_TO_VALUE (res);				\
    sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);			\
    ttl_free_c_string_array (argv);					\
    ttl_free_c_string_array (envp);					\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

static ttl_value
ttl_getenv (ttl_value varname)
{
  char * vn = ttl_malloc_c_string (varname);
  char * val;

  val = getenv (vn);
  free (vn);

  if (val)
    return ttl_string_to_value (val, -1);
  else
    return ttl_string_to_value ("", 0);
}

/* End of sys/procs.t.i.  */
