/* sys/users.t.i -- C implementation for users.t.              -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `sys.users' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */

#include <stdlib.h>

#include <pwd.h>
#include <grp.h>


/* Function getuid: fun(): int.  */
#define sys_users_getuid_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (getuid ());

/* Function geteuid: fun(): int.  */
#define sys_users_geteuid_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (geteuid ());

static void
copy_pw_to_pw_struct (ttl_value pw_struct, struct passwd * pw)
{
  ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, pw_struct);

  a->data[0] = TTL_INT_TO_VALUE (0);
  a->data[1] = ttl_string_to_value (pw->pw_name, -1);
  a->data[2] = ttl_string_to_value (pw->pw_passwd, -1);
  a->data[3] = TTL_INT_TO_VALUE (pw->pw_uid);
  a->data[4] = TTL_INT_TO_VALUE (pw->pw_gid);
  a->data[5] = ttl_string_to_value (pw->pw_gecos, -1);
  a->data[6] = ttl_string_to_value (pw->pw_dir, -1);
  a->data[7] = ttl_string_to_value (pw->pw_shell, -1);
}

/* Function getpwnam: fun(string): sys.users.passwd.  */
#define sys_users_getpwnam_pF1pS_upasswd_implementation \
{								\
  TTL_GC_CHECK (9);						\
  acc = env->locals[0];						\
  TTL_NULL_CHECK;						\
  TTL_SAVE_REGISTERS;						\
  {								\
    struct passwd * pw;						\
    char * user_name = ttl_malloc_c_string (env->locals[0]);	\
    ttl_value pw_struct = ttl_unsafe_alloc_array (8);		\
    ttl_global_acc = pw_struct;					\
    pw = getpwnam (user_name);					\
    if (pw)							\
      {								\
	copy_pw_to_pw_struct (pw_struct, pw);			\
	ttl_global_acc = pw_struct;				\
      }								\
    else							\
      ttl_global_acc = TTL_NULL;				\
    free (user_name);						\
  }								\
  TTL_RESTORE_REGISTERS;					\
}

/* Function getpwuid: fun(int): sys.users.passwd.  */
#define sys_users_getpwuid_pF1pI_upasswd_implementation \
{								\
  TTL_GC_CHECK (9);						\
  TTL_SAVE_REGISTERS;						\
  {								\
    struct passwd * pw;						\
    int uid = TTL_VALUE_TO_INT (env->locals[0]);		\
    ttl_value pw_struct = ttl_unsafe_alloc_array (8);		\
    ttl_global_acc = pw_struct;					\
    pw = getpwuid (uid);					\
    if (pw)							\
      {								\
	copy_pw_to_pw_struct (pw_struct, pw);			\
	ttl_global_acc = pw_struct;				\
      }								\
    else							\
      ttl_global_acc = TTL_NULL;				\
  }								\
  TTL_RESTORE_REGISTERS;					\
}

/* Function getpwent: fun(): sys.users.passwd.  */
#define sys_users_getpwent_pF0_upasswd_implementation \
{								\
  TTL_GC_CHECK (9);						\
  TTL_SAVE_REGISTERS;						\
  {								\
    struct passwd * pw;						\
    ttl_value pw_struct = ttl_unsafe_alloc_array (8);		\
    ttl_global_acc = pw_struct;					\
    pw = getpwent ();						\
    if (pw)							\
      {								\
	copy_pw_to_pw_struct (pw_struct, pw);			\
	ttl_global_acc = pw_struct;				\
      }								\
    else							\
      ttl_global_acc = TTL_NULL;				\
  }								\
  TTL_RESTORE_REGISTERS;					\
}


/* Function setpwent: fun(): ().  */
#define sys_users_setpwent_pF0_pV_implementation \
{							\
  setpwent ();						\
}


/* Function endpwent: fun(): ().  */
#define sys_users_endpwent_pF0_pV_implementation \
{							\
  endpwent ();						\
}


static void
copy_gr_to_gr_struct (ttl_value gr_struct, struct group * gr)
{
  ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, gr_struct);
  size_t len = 0, idx;
  char ** p = gr->gr_mem;

  while (*p)
    {
      p++;
      len++;
    }

  a->data[0] = TTL_INT_TO_VALUE (0);
  a->data[1] = ttl_string_to_value (gr->gr_name, -1);
  a->data[2] = ttl_string_to_value (gr->gr_passwd, -1);
  a->data[3] = TTL_INT_TO_VALUE (gr->gr_gid);
  a->data[4] = ttl_alloc_array (len);
  a = TTL_VALUE_TO_OBJ (ttl_array, a->data[4]);
  p = gr->gr_mem;
  for (idx = 0; idx < len; idx++)
    a->data[idx] = ttl_string_to_value (*p++, -1);
}


/* Function getgrnam: fun(string): sys.users.group.  */
#define sys_users_getgrnam_pF1pS_ugroup_implementation \
{								\
  TTL_GC_CHECK (6);						\
  acc = env->locals[0];						\
  TTL_NULL_CHECK;						\
  TTL_SAVE_REGISTERS;						\
  {								\
    struct group * gr;						\
    char * group_name = ttl_malloc_c_string (env->locals[0]);	\
    ttl_value gr_struct = ttl_unsafe_alloc_array (5);		\
    ttl_global_acc = gr_struct;					\
    gr = getgrnam (group_name);					\
    if (gr)							\
      {								\
	copy_gr_to_gr_struct (gr_struct, gr);			\
	ttl_global_acc = gr_struct;				\
      }								\
    else							\
      ttl_global_acc = TTL_NULL;				\
    free (group_name);						\
  }								\
  TTL_RESTORE_REGISTERS;					\
}


/* Function getgrgid: fun(int): sys.users.group.  */
#define sys_users_getgrgid_pF1pI_ugroup_implementation \
{								\
  TTL_GC_CHECK (6);						\
  TTL_SAVE_REGISTERS;						\
  {								\
    struct group * gr;						\
    int gid = TTL_VALUE_TO_INT (env->locals[0]);		\
    ttl_value gr_struct = ttl_unsafe_alloc_array (5);		\
    ttl_global_acc = gr_struct;					\
    gr = getgrgid (gid);					\
    if (gr)							\
      {								\
	copy_gr_to_gr_struct (gr_struct, gr);			\
	ttl_global_acc = gr_struct;				\
      }								\
    else							\
      ttl_global_acc = TTL_NULL;				\
  }								\
  TTL_RESTORE_REGISTERS;					\
}


/* Function getgrent: fun(): sys.users.group.  */
#define sys_users_getgrent_pF0_ugroup_implementation \
{								\
  TTL_GC_CHECK (6);						\
  TTL_SAVE_REGISTERS;						\
  {								\
    struct group * gr;						\
    ttl_value gr_struct = ttl_unsafe_alloc_array (5);		\
    ttl_global_acc = gr_struct;					\
    gr = getgrent ();						\
    if (gr)							\
      {								\
	copy_gr_to_gr_struct (gr_struct, gr);			\
	ttl_global_acc = gr_struct;				\
      }								\
    else							\
      ttl_global_acc = TTL_NULL;				\
  }								\
  TTL_RESTORE_REGISTERS;					\
}


/* Function setgrent: fun(): ().  */
#define sys_users_setgrent_pF0_pV_implementation \
{							\
  setgrent ();						\
}


/* Function endgrent: fun(): ().  */
#define sys_users_endgrent_pF0_pV_implementation \
{							\
  endgrent ();						\
}

static ttl_value
ttl_getlogin (void)
{
  char * val = getlogin ();
  
  if (val)
    return ttl_string_to_value (val, -1);
  else
    return ttl_string_to_value ("", 0);
}

/* End of sys/users.t.i.  */
