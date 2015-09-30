/* internal/random.t.i -- C implementation for random.t.       -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `internal.random' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */


ttl_value
ttl_srand (ttl_value seed)
{
  srand (TTL_VALUE_TO_INT (seed));
}


ttl_value
ttl_rand (void)
{
  /* To make it fit into 30 bits we strip off the two least
     significant bits.  */
  return TTL_INT_TO_VALUE (rand () >> 2);
}

/* End of internal/random.t.i.  */
