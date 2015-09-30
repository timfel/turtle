/* core.t.i -- C implementation for core.t.              -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `core' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


#include <stdlib.h>


/* Function write_char: fun(int, char): ().  */
#define core_write_char_pF2pIpC_pV_implementation \
{							\
  int fd = TTL_VALUE_TO_INT (env->locals[0]);		\
  char c = TTL_VALUE_TO_CHAR (env->locals[1]);		\
  write (fd, &c, 1);					\
}

/* Function read_char: fun(int): char.  */
#define core_read_char_pF1pI_pC_implementation \
{							\
  int fd = TTL_VALUE_TO_INT (env->locals[0]);		\
  char c;						\
  int rd = read (fd, &c, 1);				\
  if (rd == 0)						\
    acc = TTL_CHAR_TO_VALUE (65535);			\
  else							\
    acc = TTL_CHAR_TO_VALUE (c);			\
}

/* Function real_to_string: fun(real): string.  */
#define core_real_to_string_pF1pR_pS_implementation \
{									     \
  acc = env->locals[0];							     \
  TTL_NULL_CHECK;							     \
  TTL_SAVE_REGISTERS;							     \
  {									     \
    char buf[32];							     \
    sprintf (buf, "%g", TTL_VALUE_TO_OBJ (ttl_real, ttl_global_acc)->value); \
    ttl_global_acc = ttl_string_to_value (buf, -1);			     \
  }									     \
  TTL_RESTORE_REGISTERS;						     \
}


ttl_value
ttl_string_to_real (ttl_value s)
{
  ttl_string tmp;
  char * p;
  int i;
  double d;

  if (!s)
    return ttl_real_to_value (0.0);
  tmp = TTL_VALUE_TO_OBJ (ttl_string, s);
  p = (char *) malloc (TTL_SIZE (s) + 1);
  for (i = 0; i < TTL_SIZE (s); i++)
    p[i] = (char) tmp->data[i];
  p[TTL_SIZE (s)] = '\0';
  d = strtod (p, NULL);
  free (p);
  return ttl_real_to_value (d);
}


/* Function long_to_string: fun(long): string.  */
#define core_long_to_string_pF1pL_pS_implementation \
{									     \
  acc = env->locals[0];							     \
  TTL_NULL_CHECK;							     \
  TTL_SAVE_REGISTERS;							     \
  {									     \
    char buf[32];							     \
    sprintf (buf, "%ld", TTL_VALUE_TO_OBJ (ttl_long, ttl_global_acc)->value); \
    ttl_global_acc = ttl_string_to_value (buf, -1);			     \
  }									     \
  TTL_RESTORE_REGISTERS;						     \
}


ttl_value
ttl_string_to_long (ttl_value s)
{
  ttl_string tmp;
  char * p;
  int i;
  long l;

  if (!s)
    return ttl_long_to_value (0);
  tmp = TTL_VALUE_TO_OBJ (ttl_string, s);
  p = (char *) malloc (TTL_SIZE (s) + 1);
  for (i = 0; i < TTL_SIZE (s); i++)
    p[i] = (char) tmp->data[i];
  p[TTL_SIZE (s)] = '\0';
  l = strtol (p, NULL, 10);
  free (p);
  return ttl_long_to_value (l);
}


/* Function chr: fun(int): char.  */
#define core_chr_pF1pI_pC_implementation \
{								\
  acc = TTL_CHAR_TO_VALUE (TTL_VALUE_TO_INT (env->locals[0]));	\
}

/* Function ord: fun(char): int.  */
#define core_ord_pF1pC_pI_implementation \
{								\
  acc = TTL_INT_TO_VALUE (TTL_VALUE_TO_CHAR (env->locals[0]));	\
}

/* ttl_value */
/* ttl_int_to_real (ttl_value i) */
/* { */
/*   int ii = TTL_VALUE_TO_INT (i); */
/*   double d = ii; */
/*   return ttl_real_to_value (d); */
/* } */

/* Function int_to_real: fun(int): real.  */
#define core_int_to_real_pF1pI_pR_implementation \
{							\
  TTL_GC_CHECK (4);					\
  TTL_MAKE_REAL (TTL_VALUE_TO_INT (env->locals[0]));	\
}


/* Function real_to_int: fun(real): int.  */
#define core_real_to_int_pF1pR_pI_implementation \
{									     \
  acc = env->locals[0];							     \
  TTL_NULL_CHECK;							     \
  {									     \
    int i = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;			     \
    acc = TTL_INT_TO_VALUE (i);						     \
  }									     \
}

/* Function long_to_int: fun(long): int.  */
#define core_long_to_int_pF1pL_pI_implementation \
{							\
  acc = env->locals[0];					\
  TTL_NULL_CHECK;					\
  {							\
    long l = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;	\
    int i = l;						\
    acc = TTL_INT_TO_VALUE (i);				\
  }							\
}

/* Function int_to_long: fun(int): long.  */
#define core_int_to_long_pF1pI_pL_implementation \
{							\
  TTL_GC_CHECK (4);					\
  TTL_MAKE_LONG (TTL_VALUE_TO_INT (env->locals[0]));	\
}

/* Function long_to_real: fun(long): real.  */
#define core_long_to_real_pF1pL_pR_implementation \
{							\
  TTL_GC_CHECK (4);					\
  acc = env->locals[0];					\
  TTL_NULL_CHECK;					\
  {							\
    long l = TTL_VALUE_TO_OBJ (ttl_long, acc)->value;	\
    double d = l;					\
    TTL_MAKE_REAL (d);					\
  }							\
}

/* Function real_to_long: fun(real): long.  */		\
#define core_real_to_long_pF1pR_pL_implementation \
{							\
  TTL_GC_CHECK (4);					\
  acc = env->locals[0];					\
  TTL_NULL_CHECK;					\
  {							\
    double d = TTL_VALUE_TO_OBJ (ttl_real, acc)->value;	\
    long l = d;						\
    TTL_MAKE_LONG (l);					\
  }							\
}

/* End of core.t.i.  */
