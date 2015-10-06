/* sys/times.t.i -- C implementation for times.t.           -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `sys.times' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


#include <time.h>

/* Function igmtime: fun(sys.times.tm, long): sys.times.tm.  */
#define	sys_times_igmtime_pF2utmpL_utm_implementation			\
{									\
  acc = env->locals[1];							\
  TTL_NULL_CHECK;							\
  {									\
    time_t t = TTL_VALUE_TO_OBJ (ttl_long, env->locals[1])->value;	\
    ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, env->locals[0]);		\
    struct tm * tm = gmtime (&t);					\
    a->data[1] = TTL_INT_TO_VALUE (tm->tm_sec);				\
    a->data[2] = TTL_INT_TO_VALUE (tm->tm_min);				\
    a->data[3] = TTL_INT_TO_VALUE (tm->tm_hour);			\
    a->data[4] = TTL_INT_TO_VALUE (tm->tm_mday);			\
    a->data[5] = TTL_INT_TO_VALUE (tm->tm_mon);				\
    a->data[6] = TTL_INT_TO_VALUE (tm->tm_year);			\
    a->data[7] = TTL_INT_TO_VALUE (tm->tm_wday);			\
    a->data[8] = TTL_INT_TO_VALUE (tm->tm_yday);			\
    a->data[9] = TTL_INT_TO_VALUE (tm->tm_isdst);			\
    acc = env->locals[0];						\
  }									\
}

/* Function ilocaltime: fun(sys.times.tm, long): sys.times.tm.  */
#define sys_times_ilocaltime_pF2utmpL_utm_implementation		\
{									\
  acc = env->locals[1];							\
  TTL_NULL_CHECK;							\
  {									\
    time_t t = TTL_VALUE_TO_OBJ (ttl_long, env->locals[1])->value;	\
    ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, env->locals[0]);		\
    struct tm * tm = localtime (&t);					\
    a->data[1] = TTL_INT_TO_VALUE (tm->tm_sec);				\
    a->data[2] = TTL_INT_TO_VALUE (tm->tm_min);				\
    a->data[3] = TTL_INT_TO_VALUE (tm->tm_hour);			\
    a->data[4] = TTL_INT_TO_VALUE (tm->tm_mday);			\
    a->data[5] = TTL_INT_TO_VALUE (tm->tm_mon);				\
    a->data[6] = TTL_INT_TO_VALUE (tm->tm_year);			\
    a->data[7] = TTL_INT_TO_VALUE (tm->tm_wday);			\
    a->data[8] = TTL_INT_TO_VALUE (tm->tm_yday);			\
    a->data[9] = TTL_INT_TO_VALUE (tm->tm_isdst);			\
    acc = env->locals[0];						\
  }									\
}

/* Function itime: fun(long): long.  */
#define	sys_times_itime_pF1pL_pL_implementation			\
{								\
  long t = time (NULL);						\
  TTL_VALUE_TO_OBJ (ttl_long, env->locals[0])->value = t;	\
  acc = env->locals[0];						\
}

/* Function iclock: fun(real): real.  */
#define	sys_times_iclock_pF1pR_pR_implementation		\
{								\
  long t = clock();						\
  TTL_VALUE_TO_OBJ (ttl_real, env->locals[0])->value = ((float)t / (float)CLOCKS_PER_SEC); \
  acc = env->locals[0];						\
}

/* Function asctime: fun(sys.times.tm): string.  */
#define sys_times_asctime_pF1utm_pS_implementation		\
{								\
  acc = env->locals[0];						\
  TTL_NULL_CHECK;						\
  TTL_SAVE_REGISTERS;						\
  {								\
    ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, env->locals[0]);	\
    struct tm tm;						\
    tm.tm_sec = TTL_VALUE_TO_INT (a->data[1]);			\
    tm.tm_min = TTL_VALUE_TO_INT (a->data[2]);			\
    tm.tm_hour = TTL_VALUE_TO_INT (a->data[3]);			\
    tm.tm_mday = TTL_VALUE_TO_INT (a->data[4]);			\
    tm.tm_mon = TTL_VALUE_TO_INT (a->data[5]);			\
    tm.tm_year = TTL_VALUE_TO_INT (a->data[6]);			\
    tm.tm_wday = TTL_VALUE_TO_INT (a->data[7]);			\
    tm.tm_yday = TTL_VALUE_TO_INT (a->data[8]);			\
    tm.tm_isdst = TTL_VALUE_TO_INT (a->data[9]);		\
    ttl_global_acc = ttl_string_to_value (asctime (&tm), -1);	\
  }								\
  TTL_RESTORE_REGISTERS;					\
}
 
/* Function ctime: fun(long): string.  */
#define sys_times_ctime_pF1pL_pS_implementation				\
{									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  TTL_SAVE_REGISTERS;							\
  {									\
    long t = TTL_VALUE_TO_OBJ (ttl_long, env->locals[0])->value;	\
    ttl_global_acc = ttl_string_to_value (ctime (&t), -1);		\
  }									\
  TTL_RESTORE_REGISTERS;						\
}

/* End of sys/times.t.i.  */

