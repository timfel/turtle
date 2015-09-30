/* internal/gc.t.i -- C implementation for gc.t.           -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `internal.gc' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


ttl_value
ttl_gc_checks (void)
{
  return TTL_INT_TO_VALUE (ttl_stats.gc_checks);
}


ttl_value
ttl_gc_calls (void)
{
  return TTL_INT_TO_VALUE (ttl_stats.gc_calls);
}


ttl_value
ttl_do_garbage_collect (void)
{
  ttl_garbage_collect (0);
  return TTL_NULL;
}

/* End of internal/gc.t.i.  */
