/* sys/dirs.t.i -- C implementation for dirs.t.           -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `sys.dirs' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


#include <dirent.h>
#include <errno.h>

#include <sys/types.h>


/* Function opendir: fun(string): sys.dirs.dir.  */
#define sys_dirs_opendir_pF1pS_udir_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  TTL_GC_CHECK (3);							\
  TTL_ALLOC (acc, 3);							\
  acc = TTL_OBJ_TO_VALUE (acc);						\
  {									\
    ttl_untraced_array b = TTL_VALUE_TO_OBJ (ttl_untraced_array, acc);	\
    b->data[0] = 0;					\
    b->header = TTL_MAKE_HEADER (TTL_TC_NONTRACED_ARRAY, 2);		\
    {									\
      DIR * dir;							\
      char * name = ttl_malloc_c_string (env->locals[0]);		\
      dir = opendir (name);						\
      free (name);							\
      sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);			\
      if (dir)								\
	{								\
	  b->data[1] = (ttl_word) dir;					\
	  /* acc already contains the result.  */			\
	}								\
      else								\
	acc = TTL_NULL;							\
    }									\
  }									\
}


/* Function readdir: fun(sys.dirs.dir): string.  */
#define sys_dirs_readdir_pF1udir_pS_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  {									\
    ttl_untraced_array b = TTL_VALUE_TO_OBJ (ttl_untraced_array,	\
					     env->locals[0]);		\
    DIR * dir = (DIR *) b->data[1];					\
    struct dirent * de = readdir (dir);					\
    sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);			\
    if (de)								\
      {									\
	TTL_SAVE_REGISTERS;						\
	ttl_global_acc = ttl_string_to_value (de->d_name, -1);		\
	TTL_RESTORE_REGISTERS;						\
      }									\
    else								\
      acc = TTL_NULL;							\
  }									\
}


/* Function closedir: fun(sys.dirs.dir): int.  */
#define sys_dirs_closedir_pF1udir_pI_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  {									\
    ttl_untraced_array b = TTL_VALUE_TO_OBJ (ttl_untraced_array,	\
					     env->locals[0]);		\
    DIR * dir = (DIR *) b->data[1];					\
    int res = closedir (dir);						\
    sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);			\
    b->data[1] = (ttl_word) TTL_NULL;					\
    acc = TTL_INT_TO_VALUE (res);					\
  }									\
}


/* Function rewinddir: fun(sys.dirs.dir): ().  */
#define sys_dirs_rewinddir_pF1udir_pV_implementation \
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  {									\
    ttl_untraced_array b = TTL_VALUE_TO_OBJ (ttl_untraced_array,	\
					     env->locals[0]);		\
    DIR * dir = (DIR *) b->data[1];					\
    rewinddir (dir);							\
  }									\
}

/* End of sys/dirs.t.i.  */
