/* internal/stats.t.i -- C implementation for stats.t.           -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `internal.stats' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */

/* Function dispatch_call_count: fun(): int.  */
#define internal_stats_dispatch_call_count_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.dispatch_call_count);

/* Function direct_call_count: fun(): int.  */
#define internal_stats_direct_call_count_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.direct_call_count);

/* Function local_call_count: fun(): int.  */
#define internal_stats_local_call_count_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.local_call_count);

/* Function closure_call_count: fun(): int.  */
#define internal_stats_closure_call_count_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.closure_call_count);

/* Function gc_checks: fun(): int.  */
#define	internal_stats_gc_checks_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.gc_checks);

/* Function gc_calls: fun(): int.  */
#define	internal_stats_gc_calls_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.gc_calls);

/* Function allocations: fun(): int.  */
#define	internal_stats_allocations_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.allocations);

/* Function alloced_words: fun(): int.  */
#define	internal_stats_alloced_words_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.alloced_words);

/* Function forwarded_words: fun(): int.  */
#define internal_stats_forwarded_words_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.forwarded_words);

/* Function save_cont_count: fun(): int.  */
#define internal_stats_save_cont_count_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.save_cont_count);

/* Function restore_cont_count: fun(): int.  */
#define internal_stats_restore_cont_count_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.restore_cont_count);

/* Function total_run_time: fun(): int.  */
#define	internal_stats_total_run_time_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.total_run_time);

/* Function total_gc_time: fun(): int.  */
#define	internal_stats_total_gc_time_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.total_gc_time);

/* Function min_gc_time: fun(): int.  */
#define	internal_stats_min_gc_time_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.min_gc_time);

/* Function max_gc_time: fun(): int.  */
#define	internal_stats_max_gc_time_pF0_pI_implementation \
  acc = TTL_INT_TO_VALUE (ttl_stats.max_gc_time);


/* End of internal/stats.t.i.  */
