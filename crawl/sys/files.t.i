/* sys/files.t.i -- C implementation for files.t.           -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `sys.files' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>


/* Function open: fun(string, int): ().  */
#define sys_files_open_pF1pS_pI_implementation \
{							\
  acc = env->locals[0];					\
  TTL_NULL_CHECK;					\
  TTL_SAVE_REGISTERS;					\
  {							\
    char * fn = ttl_malloc_c_string (ttl_global_acc);	\
    int fd;						\
    fd = open (fn, O_RDONLY);				\
    sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);	\
    free (fn);						\
    ttl_global_acc = TTL_INT_TO_VALUE (fd);		\
  }							\
  TTL_RESTORE_REGISTERS;				\
}


/* Function create: fun(string): int.  */
#define sys_files_create_pF1pS_pI_implementation \
{							\
  acc = env->locals[0];					\
  TTL_NULL_CHECK;					\
  TTL_SAVE_REGISTERS;					\
  {							\
    char * fn = ttl_malloc_c_string (ttl_global_acc);	\
    int fd;						\
    fd = creat (fn, 0600);				\
    sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);	\
    free (fn);						\
    ttl_global_acc = TTL_INT_TO_VALUE (fd);		\
  }							\
  TTL_RESTORE_REGISTERS;				\
}


/* Function close: fun(int): int.  */
#define sys_files_close_pF1pI_pI_implementation \
{							\
  TTL_SAVE_REGISTERS;					\
  {							\
    int fd = TTL_VALUE_TO_INT (env->locals[0]);		\
    int res;						\
    res = close (fd);					\
    sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);	\
    ttl_global_acc = TTL_INT_TO_VALUE (res);		\
  }							\
  TTL_RESTORE_REGISTERS;				\
}


/* Function write: fun(int, internal.binary.binary, int): int.  */
#define sys_files_write_pF3pIubinarypI_pI_implementation \
{									\
  acc = env->locals[1];							\
  TTL_NULL_CHECK;							\
  {									\
    int fd = TTL_VALUE_TO_INT (env->locals[0]);				\
    ttl_binary_array b = TTL_VALUE_TO_OBJ (ttl_binary_array, acc);	\
    int count = TTL_VALUE_TO_INT (env->locals[2]);			\
    ssize_t written;							\
									\
    if (count < 0 || count > TTL_SIZE (acc))				\
      TTL_RAISE (ttl_out_of_range_exception);				\
									\
    written = write (fd, b->data, count);				\
    sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);			\
    acc = TTL_INT_TO_VALUE (written);					\
  }									\
}

/* Function read: fun(int, internal.binary.binary, int): int.  */
#define sys_files_read_pF3pIubinarypI_pI_implementation \
{									\
  acc = env->locals[1];							\
  TTL_NULL_CHECK;							\
  {									\
    int fd = TTL_VALUE_TO_INT (env->locals[0]);				\
    ttl_binary_array b = TTL_VALUE_TO_OBJ (ttl_binary_array, acc);	\
    int count = TTL_VALUE_TO_INT (env->locals[2]);			\
    ssize_t rd;								\
									\
    if (count < 0 || count > TTL_SIZE (acc))				\
      TTL_RAISE (ttl_out_of_range_exception);				\
									\
    rd = read (fd, b->data, count);					\
    sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);			\
    acc = TTL_INT_TO_VALUE (rd);					\
  }									\
}


/* Function unlink: fun(string): int.  */
#define sys_files_unlink_pF1pS_pI_implementation \
{							\
  acc = env->locals[0];					\
  TTL_NULL_CHECK;					\
  {							\
    char * fn = ttl_malloc_c_string (acc);		\
    int ret;						\
							\
    ret = unlink (fn);					\
    sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);	\
    free (fn);						\
    acc = TTL_INT_TO_VALUE (ret);			\
  }							\
}

/* Function istat: fun(string, sys.files.stat): sys.files.stat.  */
#define sys_files_istat_pF2pSustat_ustat_implementation		\
{								\
  acc = env->locals[0];						\
  TTL_NULL_CHECK;						\
  TTL_SAVE_REGISTERS;						\
  {								\
    char * fn = ttl_malloc_c_string (ttl_global_acc);		\
    ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, env->locals[1]);	\
    struct stat s;						\
    int ret;							\
								\
    ret = stat (fn, &s);					\
    if (ret >= 0)						\
      {								\
	a->data[1] = TTL_INT_TO_VALUE ((int)s.st_dev);		\
	a->data[2] = TTL_INT_TO_VALUE (s.st_ino);		\
	a->data[3] = TTL_INT_TO_VALUE (s.st_mode);		\
	a->data[4] = TTL_INT_TO_VALUE (s.st_nlink);		\
	a->data[5] = TTL_INT_TO_VALUE (s.st_uid);		\
	a->data[6] = TTL_INT_TO_VALUE (s.st_gid);		\
	a->data[7] = TTL_INT_TO_VALUE ((int)s.st_rdev);		\
	TTL_VALUE_TO_OBJ (ttl_long, a->data[8])->value =	\
	  s.st_size;				\
	TTL_VALUE_TO_OBJ (ttl_long, a->data[9])->value =	\
	  s.st_blksize;			\
	TTL_VALUE_TO_OBJ (ttl_long, a->data[10])->value =	\
	  s.st_blocks;			\
	TTL_VALUE_TO_OBJ (ttl_long, a->data[11])->value =	\
	  s.st_atime;			\
	TTL_VALUE_TO_OBJ (ttl_long, a->data[12])->value =	\
	  s.st_mtime;			\
	TTL_VALUE_TO_OBJ (ttl_long, a->data[13])->value =	\
	  s.st_ctime;			\
	ttl_global_acc = env->locals[1];			\
      }								\
    else							\
      {								\
	sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);		\
	ttl_global_acc = TTL_NULL;				\
      }								\
    free (fn);							\
  }								\
  TTL_RESTORE_REGISTERS;					\
}

/* End of sys/files.t.i.  */

