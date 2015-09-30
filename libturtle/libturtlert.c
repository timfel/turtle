/* libturtle/libturtlert.c -- The Turtle runtime.
 
  Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
 
  This is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.
  
  This software is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this package; see the file COPYING.  If not, write to the
  Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA 02111-1307, USA.  */

#include <stdlib.h>
#include <stdio.h>

#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#include "version.h"
#include "libturtlert.h"

#include "indigo.h"
#include "fd-solver.h"

/* Define this if you want a trace of the heap at garbage collection.  */
/* #define DRIBBLE 1 */
#if DRIBBLE
static FILE * dribble;
#endif

#define PRINT_DEBUG 1

/* Set this to 1 to enable memory usage statistics.  */
#define TTL_PROFILE_MEMORY 0

/* Convenience macro for the memory management code.  This is often
   needed because allocated objects on the heap must be allocated to
   even word-boundaries (that is 8-byte boundaries).  */
#define ROUND_TO_EVEN(c) (((c) + 1) & ~1)

/* This structure collects all statistics gathered during run
   time.  */
struct ttl_statistics ttl_stats;

/* These are set by the startup code when the user specifies the -:s
   or -:g options.  */
static int print_stats_on_exit = 0;
static int print_gc_messages = 0;


/* Memory-management related definitions and variables.  ========== */

/* How much to allocate for the heap on startup.  */
#define TTL_INITIAL_MEGABYTES 4

/* Don't let the user specify less than this.  */
#define TTL_MIN_MEGABYTES 1

/* Don't let the user specify more than this, and don't grow the heap
   larger than this.  */
#define TTL_MAX_MEGABYTES 128

/* When growing the heap, add these many megabytes each time.  */
#define TTL_INCR_MEGABYTES 2


/* This controls when to grow the heap.  If, for example, the heap
   should grow whenever it is 5/6 full after a garbage collection,
   specify 5 as the nominator and 6 as the denominator.  */
#define TTL_RATIO_NOMINATOR 3
#define TTL_RATIO_DENOMINATOR 4

/* Convenience definitions for use in the code.  */
#define TTL_INITIAL_IN_BYTES (TTL_INITIAL_MEGABYTES * 1024 * 1024)
#define TTL_MIN_IN_BYTES (TTL_MIN_MEGABYTES * 1024 * 1024)
#define TTL_MAX_IN_BYTES (TTL_MAX_MEGABYTES * 1024 * 1024)
#define TTL_MAX_IN_WORDS (TTL_MAX_IN_BYTES / sizeof (ttl_value))
#define TTL_INCR_IN_BYTES (TTL_INCR_MEGABYTES * 1024 * 1024)

/* This variable is used to set the actually allocated amount of
   memory at startup, it gets set by the startup code if the user
   specifies the -:hNUM option.  */
static unsigned heap_size_in_bytes = TTL_INITIAL_IN_BYTES;

/* The number of words allocated to each semi-space.  */
static unsigned semi_space_in_words;

/* Origin of memory area for space 0.  */
static ttl_value * space0orig;

/* The above, rounded up to 8-byte boundary.  */
static ttl_value * space0;

/* Pointer to first word after space 0.  */
static ttl_value * space0limit;

/* Origin of memory area for space 1.  */
static ttl_value * space1orig;

/* The above, rounded up to 8-byte boundary.  */
static ttl_value * space1;

/* Pointer to first word after space 1.  */
static ttl_value * space1limit;

/* The number of the current allocation space, must be 0 or 1.  */
static int current_space;

/* Base of current from-space, either space0 or space1.  */
static ttl_value * from_space;

/* Limit of current from-space, either space0limit or space1limit.  */
static ttl_value * from_space_limit;

/* Base of current to-space, either space0 or space1.  */
static ttl_value * to_space;

/* Limit of current to-space, either space0limit or space1limit.  */
static ttl_value * to_space_limit;

/* A garbage collection sets this to non-zero, if a collection did not
   yield enough free memory and thus the heap should be resized.
   Since we cannot resize the area we have just copied to, from-space
   is resized and the resize of to-space is deferred to the next
   collection, when it is empty.  */
static int heap_needs_resize = 0;

/* Limit of the global variables a program can have.  */
/* XXX: This should be a dynamic array, which can be resized.  Do it
   when we encounter a program with more than 1024 global
   pointer-typed variables.  */
#define MAX_GLOBAL_ROOTS 1024

/* This array holds the addresses of the global variables of a
   program, which should be considered roots for garbage
   collection.  */ 
static ttl_value * global_roots[MAX_GLOBAL_ROOTS];

/* Number of valid entries in the array above.  */
static int global_root_count;

/* This is the maximum size for the evaluation stack.  Since the stack
   is copied to a continuation on each nested function call, this
   should be enough for a while.  */
#define TTL_STACK_SIZE 1024

/* Registers of the Turtle machine.  */
ttl_value * ttl_alloc_ptr;	/* Points to the next free heap cell.  */
ttl_value * ttl_alloc_limit;	/* Points behind the last heap cell.  */
ttl_value ttl_global_pc;	/* Points to the code to be executed.  */
ttl_value ttl_global_acc;	/* Holds intermediate results.  */
ttl_value ttl_global_env;	/* Points to the current environment.  */
ttl_value ttl_global_cont;	/* Points to the current continuation.  */
ttl_value ttl_stack[TTL_STACK_SIZE]; /* Evaluation stack.  */
int ttl_global_sp = 0;		/* Top of the above stack.  */

/* The list of exception handlers.  It is currently maintained by
   library functions. */
ttl_value ttl_exception_handler;

/* When an exception is raised, the current chain of continuations is
   saved in this variable.  The standard exception handler uses it to
   display a backtrace.  */
ttl_value ttl_saved_continuations;

/* The following hold pre-allocated strings describing the exceptions
   which are raised by the runtime system or the virtual machine.  */
ttl_value ttl_null_pointer_exception;
ttl_value ttl_subscript_exception;
ttl_value ttl_out_of_range_exception;
ttl_value ttl_wrong_variant_exception;
ttl_value ttl_require_exception;


/* Timer and signal handling.  */

/* Default value of `ttl_time_quantum'.  */
#define TTL_DEFAULT_TIME_QUANTUM 1000000

/* This is the value which is used to initialize `ttl_time_slice'
   after each interrupt.  */
int ttl_time_quantum;

/* This variable holds the number of ticks remaining until the next
   interrupt.  Signal handlers set this to 0 so that an interrupt will
   occur on the next checkpoint.  */
int ttl_time_slice;

#define MAX_SIGNAL 64
static ttl_value timer_interrupt;
static ttl_value timer_handler;
static ttl_value signal_handler;
static int signals_pending = 0;
static int signal_mask[MAX_SIGNAL];
static ttl_value signal_handlers[MAX_SIGNAL];

static char * tc_names[] =
  {
    "broken heart",
    "continuation",
    "procedure",
    "closure",
    "string",
    "real",
    "array",
    "untraced array",
    "binary array",
    "environment",
    "variable",
    "constraint",
    "long"
  };

static void ttl_exit (int code);

#if TTL_PROFILE_MEMORY

static FILE * prof_file;
#define PROF_SAMPLE_LIMIT 10000
static unsigned prof_sample_count = 0;
static unsigned prof_time = 0;
static unsigned prof_samples[PROF_SAMPLE_LIMIT][2];

static void
signal_alarm (int no)
{
  if (prof_sample_count < PROF_SAMPLE_LIMIT)
    {
      unsigned amount;
      if (current_space == 0)
	amount = ttl_alloc_ptr - space0;
      else
	amount = ttl_alloc_ptr - space1;
      prof_samples[prof_sample_count][0] = amount;
      prof_samples[prof_sample_count][1] = prof_time;
      prof_sample_count++;
    }
  prof_time++;
}
#endif /* TTL_PROFILE_MEMORY */

static void
alloc_failure (int words)
{
  fprintf (stderr, "\nMemory exhausted while allocating %d words.\n",
	   words);
  abort ();
  ttl_exit (1);
}

/* Allocation routine for use in the garbage collector.  This does
   halt the program if not enough space for allocating `words' words
   is available.  */
static ttl_value
ttl_gc_alloc (int words)
{
  ttl_value v = (ttl_value) ttl_alloc_ptr;
  words = ROUND_TO_EVEN (words);
  ttl_alloc_ptr += words;
  if (ttl_alloc_ptr > ttl_alloc_limit)
    {
      ttl_alloc_ptr -= words;
      alloc_failure (words);
    }
  return v;
}


#if DRIBBLE
static void
walk (ttl_value v, int depth)
 {
  ttl_value * raw;
  int i;
#define SPACE_TO 1
#define SPACE_FROM 2
#define SPACE_OUTER 3
  int space = 0;
  static char * space_name[] = {"unknown", "to-space", "from-space", "outer space"};

 retry:
  for (i = 0; i < depth; i++)
    fprintf (dribble, "| ");
  if (TTL_IMMEDIATE_P (v))
    {
      fprintf (dribble, "Immediate %p (int: %d)\n", v,
	       TTL_VALUE_TO_INT (v));
      return;
    }

  raw = (ttl_value *) (((ttl_word) v) & ~3);
  if (raw < to_space || raw >= to_space_limit)
    {
      if (raw >= from_space && raw < from_space_limit)
	{
	  space = SPACE_FROM;
	}
      else
	space = SPACE_OUTER;
      if (raw == NULL)
	{
	  fprintf (dribble, "NULL\n");
	  return;
	}
    }
  else
    space = SPACE_TO;

  if (TTL_OBJECT_P (v))
    {
      unsigned size = TTL_SIZE (v);
      unsigned tc = TTL_TYPE_CODE (v);
      ttl_broken_heart heart = TTL_VALUE_TO_OBJ (ttl_broken_heart, v);

      fprintf (dribble, "Object (%d, %s, size = %u)", tc,
		 tc_names[tc], size);
      switch (tc)
	{
	case TTL_TC_BROKEN_HEART:
	  {
	    fprintf (dribble, " [ -> Broken heart ]");

	    if (TTL_OBJECT_P (heart->forward) &&
		TTL_TYPE_CODE (heart->forward) == TTL_TC_BROKEN_HEART)
	      {
		fprintf
		  (dribble,
		   " forwarding pointer refers to broken heart: bh: %p, forward: %p",
		   heart, heart->forward);
		abort ();
	      }
	    if (size != 1)
	      abort ();
	    fprintf (dribble, " (%s)\n", space_name[space]);
	    break;
	  }
	case TTL_TC_CONTINUATION:
	  {
	    int x;
	    ttl_continuation c = TTL_VALUE_TO_OBJ (ttl_continuation, v);

	    fprintf (dribble, " (%s)\n", space_name[space]);

	    walk (c->cont, depth + 1);
/* 	    walk (c->pc); */
	    walk (c->env, depth + 1);
	    for (x = 0; x < c->sp; x++)
	      walk (c->stack[x], depth + 1);
	    if (size != c->sp + TTL_SIZEOF_CONTINUATION)
	      abort ();
	    break;
	  }

	case TTL_TC_PROCEDURE:
	  {
	    if (size != 3)
	      abort ();
	    fprintf (dribble, " (%s)\n", space_name[space]);
	    break;		/* No need to copy, because procedure
				   descriptors are statically
				   allocated.  */
	  }

	case TTL_TC_CLOSURE:
	  {
	    ttl_closure c = TTL_VALUE_TO_OBJ (ttl_closure, v);

	    fprintf (dribble, " (%s)\n", space_name[space]);
	    walk (c->code, depth + 1);
	    walk (c->env, depth + 1);
	    if (size != TTL_SIZEOF_CLOSURE)
	      abort ();
	    break;
	  }

	case TTL_TC_STRING:
	  {
	    fprintf (dribble, " (%s)\n", space_name[space]);
	    break;
	  }

	case TTL_TC_REAL:
	  {
	    if (size != TTL_SIZEOF_REAL)
	      abort ();
	    fprintf (dribble, " (%s)\n", space_name[space]);
	    break;
	  }

	case TTL_TC_LONG:
	  {
	    if (size != TTL_SIZEOF_LONG)
	      abort ();
	    fprintf (dribble, " (%s)\n", space_name[space]);
	    break;
	  }

	case TTL_TC_ARRAY:
	  {
	    ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, v);
	    unsigned i;

	    fprintf (dribble, " (%s)\n", space_name[space]);
	    for (i = 0; i < size; i++)
	      walk (a->data[i], depth + 1);
	    break;
	  }

	case TTL_TC_NONTRACED_ARRAY:
	  {
	    fprintf (dribble, " (%s)\n", space_name[space]);
	    break;
	  }

	case TTL_TC_BINARY_ARRAY:
	  {
	    fprintf (dribble, " (%s)\n", space_name[space]);
	    break;
	  }

	case TTL_TC_ENVIRONMENT:
	  {
	    ttl_environment e = TTL_VALUE_TO_OBJ (ttl_environment, v);
	    unsigned i;

	    fprintf (dribble, " (%s)\n", space_name[space]);
	    walk (e->parent, depth + 1);
	    for (i = 0; i < size - 1; i++)
	      walk (e->locals[i], depth + 1);
	    if (size < TTL_SIZEOF_ENVIRONMENT)
	      abort ();
	    break;
	  }

	case TTL_TC_VARIABLE:
	  {
	    ttl_variable var = TTL_VALUE_TO_OBJ (ttl_variable, v);

	    fprintf (dribble, " (%s)\n", space_name[space]);
	    walk (var->value, depth + 1);

	    if (size != TTL_SIZEOF_VARIABLE)
	      abort ();
	    break;
	  }

	default:
	  fprintf (dribble, "gc (walk): invalid type code %d.\n", tc);
	  abort ();
	  break;
	}
      return;
    }

  if (TTL_PAIR_P (v))
    {
      ttl_value car;
      ttl_value cdr;

      fprintf (dribble, "Pair ");

      car = TTL_CAR (v);
      cdr = TTL_CDR (v);
      if (TTL_HEADER_P (car))
	{
	  fprintf (dribble, " [ -> Broken heart ]");
	  fprintf (dribble, " (%s)\n", space_name[space]);
	  return;
	}
      else
	{
	  fprintf (dribble, " (%s)\n", space_name[space]);
	  walk (car, depth + 1);
	  v = cdr;
	  goto retry;
	}
    }
  fprintf (dribble, "gc (walk): header tag found (v=%p).\n", v);
  abort ();
}
#endif


static ttl_value
copy (ttl_value v)
 {
  ttl_value * raw;

  if (TTL_IMMEDIATE_P (v))
    {
#if PRINT_DEBUG
      if (print_gc_messages)
	fprintf (stderr, "[ Copying immediate (%d)]\n", TTL_VALUE_TO_INT (v));
#endif
      return v;
    }

  raw = (ttl_value *) (((ttl_word) v) & ~3);
  if (raw < from_space || raw >= from_space_limit)
    {
      if (raw >= to_space && raw < to_space_limit)
	{
	  fprintf (stderr, "Pointer to copy points to to-space\n");
	  if (TTL_OBJECT_P (v) &&
	      TTL_VALUE_TO_OBJ (ttl_broken_heart, v) != NULL)
	    {
	      if (TTL_TYPE_CODE (v) == TTL_TC_BROKEN_HEART)
		fprintf (stderr, "  Points to broken heart, too.\n");
	      else if (TTL_TYPE_CODE (v) == TTL_TC_STRING)
		fprintf (stderr, "  Points to string, too.\n");
	      else if (TTL_TYPE_CODE (v) == 63)
		fprintf (stderr, "  Cleared out.\n");
	    }
/* 	  abort (); */
	}
      return v;
    }

  if (TTL_OBJECT_P (v))
    {
      unsigned size = TTL_SIZE (v);
      unsigned tc = TTL_TYPE_CODE (v);
      ttl_broken_heart heart = TTL_VALUE_TO_OBJ (ttl_broken_heart, v);

#if PRINT_DEBUG
      if (print_gc_messages)
	fprintf (stderr, "[ Copying object (%d, %s, size = %u)]\n", tc,
		 tc_names[tc], size);
#endif
      switch (tc)
	{
	case TTL_TC_BROKEN_HEART:
	  {
#if PRINT_DEBUG
	    if (print_gc_messages)
	      fprintf (stderr, "[ -> Broken heart ]\n");
#endif
	    if (TTL_OBJECT_P (heart->forward) &&
		TTL_TYPE_CODE (heart->forward) == TTL_TC_BROKEN_HEART)
	      {
		fprintf (stderr, "forwarding pointer refers to broken heart: bh: %p, forward: %p\n", heart, heart->forward);
		abort ();
	      }
	    if (size != 1)
	      abort ();
	    return heart->forward;
	  }
	case TTL_TC_CONTINUATION:
	  {
	    int x;
	    ttl_continuation c = TTL_VALUE_TO_OBJ (ttl_continuation, v);
	    ttl_continuation nc = (ttl_continuation) ttl_gc_alloc (1 + size);
	    ttl_value nv = TTL_OBJ_TO_VALUE (nc);
	    nc->header = c->header;
	    nc->cont = c->cont;
	    nc->pc = c->pc;
	    nc->env = c->env;
	    nc->sp = c->sp;
	    for (x = 0; x < c->sp; x++)
	      nc->stack[x] = c->stack[x];
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + size;
	    if (size != (unsigned) nc->sp + TTL_SIZEOF_CONTINUATION)
	      abort ();
	    return nv;
	  }

	case TTL_TC_PROCEDURE:
	  {
#if 0
	    fprintf (stderr, "*** forwarding procedure\n");
#endif
	    if (size != 3)
	      abort ();
	    return v;		/* No need to copy, because procedure
				   descriptors are statically
				   allocated.  */
	  }

	case TTL_TC_CLOSURE:
#if 0
	    fprintf (stderr, "*** forwarding closure\n");
#endif
	  {
	    ttl_closure c = TTL_VALUE_TO_OBJ (ttl_closure, v);
	    ttl_closure nc = (ttl_closure) ttl_gc_alloc (1 + size);
	    ttl_value nv = TTL_OBJ_TO_VALUE (nc);
	    nc->header = c->header;
	    nc->host = c->host;
	    nc->code = c->code;
	    nc->env = c->env;
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + size;
	    if (size != TTL_SIZEOF_CLOSURE)
	      abort ();
	    return nv;
	  }

	case TTL_TC_STRING:
	  {
	    ttl_string s = TTL_VALUE_TO_OBJ (ttl_string, v);
	    ttl_string ns = (ttl_string) ttl_gc_alloc (1 + (size + 1) / 2);
	    ttl_value nv = TTL_OBJ_TO_VALUE (ns);
	    unsigned i;
	    ns->header = s->header;
	    for (i = 0; i < size; i++)
	      ns->data[i] = s->data[i];
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + (size + 1) / 2;
	    return nv;
	  }

	case TTL_TC_REAL:
	  {
	    ttl_real r = TTL_VALUE_TO_OBJ (ttl_real, v);
	    ttl_real nr = (ttl_real) ttl_gc_alloc (size + 1);
	    ttl_value nv = TTL_OBJ_TO_VALUE (nr);
	    nr->header = r->header;
	    nr->value = r->value;
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 3;
	    if (size != TTL_SIZEOF_REAL)
	      abort ();
	    return nv;
	  }

	case TTL_TC_LONG:
	  {
	    ttl_long l = TTL_VALUE_TO_OBJ (ttl_long, v);
	    ttl_long nl = (ttl_long) ttl_gc_alloc (size + 1);
	    ttl_value nv = TTL_OBJ_TO_VALUE (nl);
	    nl->header = l->header;
	    nl->value = l->value;
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 3;
	    if (size != TTL_SIZEOF_LONG)
	      abort ();
	    return nv;
	  }

	case TTL_TC_ARRAY:
	  {
	    ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, v);
	    ttl_array na = (ttl_array) ttl_gc_alloc (1 + size);
	    ttl_value nv = TTL_OBJ_TO_VALUE (na);
	    unsigned i;
	    na->header = a->header;
	    for (i = 0; i < size; i++)
	      na->data[i] = a->data[i];
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + size;
	    return nv;
	  }

	case TTL_TC_NONTRACED_ARRAY:
	  {
	    ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, v);
	    ttl_array na = (ttl_array) ttl_gc_alloc (1 + size);
	    ttl_value nv = TTL_OBJ_TO_VALUE (na);
	    unsigned i;
	    na->header = a->header;
	    for (i = 0; i < size; i++)
	      na->data[i] = a->data[i];
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + size;
	    return nv;
	  }

	case TTL_TC_BINARY_ARRAY:
	  {
	    ttl_binary_array a = TTL_VALUE_TO_OBJ (ttl_binary_array, v);
	    ttl_binary_array na =
	      (ttl_binary_array) ttl_gc_alloc (1 + (size + 3) / 4);
	    ttl_value nv = TTL_OBJ_TO_VALUE (na);
	    unsigned i;
	    na->header = a->header;
	    for (i = 0; i < size; i++)
	      na->data[i] = a->data[i];
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + (size + 3) / 4;
	    return nv;
	  }

	case TTL_TC_ENVIRONMENT:
	  {
	    ttl_environment e = TTL_VALUE_TO_OBJ (ttl_environment, v);
	    ttl_environment ne = (ttl_environment) ttl_gc_alloc (1 + size);
	    ttl_value nv = TTL_OBJ_TO_VALUE (ne);
	    unsigned i;
	    ne->header = e->header;
	    ne->parent = e->parent;
	    for (i = 0; i < size - 1; i++)
	      ne->locals[i] = e->locals[i];
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + size;
	    if (size < TTL_SIZEOF_ENVIRONMENT)
	      abort ();
	    return nv;
	  }

#if 0
	case TTL_TC_IDG_VARIABLE:
	  {
	    ttl_idg_variable var = TTL_VALUE_TO_OBJ (ttl_idg_variable, v);
	    ttl_idg_variable nvar = (ttl_idg_variable) ttl_gc_alloc (1 + size);
	    ttl_value nv = TTL_OBJ_TO_VALUE (nvar);
	    nvar->header = var->header;
	    nvar->value = var->value;
	    nvar->val = var->val;
	    nvar->variable = var->variable;
	    nvar->variable->variable = nv;
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + size;
	    if (size != TTL_SIZEOF_IDG_VARIABLE)
	      abort ();
	    return nv;
	  }
#endif

	case TTL_TC_VARIABLE:
	  {
	    ttl_variable var = TTL_VALUE_TO_OBJ (ttl_variable, v);
	    ttl_variable nvar = (ttl_variable) ttl_gc_alloc (1 + size);
	    ttl_value nv = TTL_OBJ_TO_VALUE (nvar);
	    nvar->header = var->header;
	    nvar->value = var->value;
	    nvar->constraints = var->constraints;
	    nvar->determined_by = var->determined_by;
	    nvar->walk_strength = var->walk_strength;
	    nvar->mark = var->mark;
	    nvar->valid = var->valid;
	    
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + size;
	    if (size != TTL_SIZEOF_VARIABLE)
	      abort ();
	    return nv;
	  }

	case TTL_TC_CONSTRAINT:
	  {
	    ttl_constraint cnst = TTL_VALUE_TO_OBJ (ttl_constraint, v);
	    ttl_constraint ncnst = (ttl_constraint) ttl_gc_alloc (1 + size);
	    ttl_value nv = TTL_OBJ_TO_VALUE (ncnst);
	    ncnst->header = cnst->header;
	    ncnst->strength = cnst->strength;
	    ncnst->variables = cnst->variables;
	    ncnst->methods = cnst->methods;
	    ncnst->selected_method = cnst->selected_method;
	    ncnst->mark = cnst->mark;

	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + size;
	    if (size != TTL_SIZEOF_CONSTRAINT)
	      abort ();
	    return nv;
	  }

	case TTL_TC_METHOD:
	  {
	    ttl_method meth = TTL_VALUE_TO_OBJ (ttl_method, v);
	    ttl_method nmeth = (ttl_method) ttl_gc_alloc (1 + size);
	    ttl_value nv = TTL_OBJ_TO_VALUE (nmeth);
	    nmeth->header = meth->header;
	    nmeth->code = meth->code;
	    nmeth->inputs = meth->inputs;
	    nmeth->outputs = meth->outputs;
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + size;
	    if (size != TTL_SIZEOF_METHOD)
	      abort ();
	    return nv;
	  }

	case TTL_TC_CONSTRAINABLE_VARIABLE:
	  {
	    ttl_constrainable_variable var = TTL_VALUE_TO_OBJ
	      (ttl_constrainable_variable, v);
	    ttl_constrainable_variable nvar = (ttl_constrainable_variable)
	      ttl_gc_alloc (1 + size);
	    ttl_value nv = TTL_OBJ_TO_VALUE (nvar);
	    nvar->header = var->header;
	    nvar->value = var->value;
	    nvar->hook = var->hook;
	    nvar->hook->variable = nv;
	    heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	    heart->forward = nv;
	    ttl_stats.forwarded_words += 1 + size;
	    if (size != TTL_SIZEOF_CONSTRAINABLE_VARIABLE)
	      abort ();
	    return nv;
	  }

	default:
	  fprintf (stderr, "gc (copy): invalid type code %d.\n", tc);
	  abort ();
	  return NULL;
	}
    }

  if (TTL_PAIR_P (v))
    {
      ttl_value car;
      ttl_value cdr;

#if PRINT_DEBUG
      if (print_gc_messages)
	fprintf (stderr, "[ Copying pair ]\n");
#endif

      car = TTL_CAR (v);
      cdr = TTL_CDR (v);
      if (TTL_HEADER_P (car))
	{
#if PRINT_DEBUG
	  if (print_gc_messages)
	    fprintf (stderr, "[ -> Broken heart ]\n");
#endif
	  return cdr;
	}
      else
	{
	  ttl_broken_heart heart = (ttl_broken_heart) TTL_VALUE_TO_PAIR (v);
	  ttl_pair p = (ttl_pair) ttl_gc_alloc (2);
	  ttl_value nv = TTL_PAIR_TO_VALUE (p);
#if PRINT_DEBUG
	  if (print_gc_messages)
	    {
	      if (TTL_OBJECT_P (car) &&
		  TTL_TYPE_CODE (car) == TTL_TC_STRING)
		fprintf (stderr, "string length: %ld\n", TTL_SIZE (car));
	    }
#endif
	  p->car = car;
	  p->cdr = cdr;
	  heart->header = TTL_MAKE_HEADER (TTL_TC_BROKEN_HEART, 1);
	  heart->forward = nv;
	  ttl_stats.forwarded_words += 2;
	  return nv;
	}
    }

  fprintf (stderr, "gc (copy): header tag found (v=%p).\n", v);
  abort ();
  return NULL;
}

#if 1
ttl_value
check (ttl_value c)
{
  ttl_value * raw = (ttl_value *) (((ttl_word) c) & ~3);
  if (!TTL_IMMEDIATE_P(c) && raw != NULL &&
      (raw < to_space || raw >= to_space_limit) &&
      TTL_TYPE_CODE(c) != TTL_TC_PROCEDURE)
    {
      abort ();
      return c;
    }
  else
    return c;
}
#else
#define check(c) (c)
#endif

static void
trace (void)
{
  ttl_value * tracep = to_space;
#if PRINT_DEBUG
  ttl_value * oldtracep = tracep;
#endif

  while (tracep < ttl_alloc_ptr)
    {
#if PRINT_DEBUG
      if (print_gc_messages)
	{
	  fprintf (stderr, "tracep:  %p (%d)\n", tracep, tracep - oldtracep);
	  fprintf (stderr, "*tracep: %p\n", *tracep);
	  fprintf (stderr, "alloc:  %p\n", ttl_alloc_ptr);
	}
      oldtracep = tracep;
#endif
      if (TTL_HEADER_P (*tracep))
	{
	  ttl_value v = TTL_OBJ_TO_VALUE (tracep);
	  unsigned size = TTL_SIZE (v);
	  unsigned tc = TTL_TYPE_CODE (v);
	  
#if PRINT_DEBUG
	  if (print_gc_messages)
	    fprintf (stderr, "> Tracing object (%d, %s, size = %u)<\n", tc,
		     tc_names[tc], size);
#endif

	  switch (tc)
	    {
	    case TTL_TC_BROKEN_HEART:
	      {
		fprintf (stderr, "gc (trace): broken heart found.\n");
		abort ();
		break;
	      }

	    case TTL_TC_CONTINUATION:
	      {
		ttl_continuation c = TTL_VALUE_TO_OBJ (ttl_continuation, v);
		int sp;

		c->cont = check (copy (c->cont));
		c->pc = TTL_VALUE_TO_OBJ
		  (ttl_descr, check (copy (TTL_OBJ_TO_VALUE (c->pc))));
		c->env = check (copy (c->env));
		sp = c->sp;
		while (sp > 0)
		  {
		    c->stack[sp - 1] = check (copy (c->stack[sp - 1]));
		    sp--;
		  }
		tracep += ROUND_TO_EVEN (size + 1);
		break;
	      }

	    case TTL_TC_PROCEDURE:
	      {
		tracep += 2;
		break;
	      }

	    case TTL_TC_CLOSURE:
	      {
		ttl_closure c = TTL_VALUE_TO_OBJ (ttl_closure, v);

		/* c->host must not be forwarded.  */
		c->code = check(copy (c->code));
		c->env = check(copy (c->env));
		tracep += ROUND_TO_EVEN (size + 1);
		break;
	      }

	    case TTL_TC_STRING:
	      {
		ttl_string s = TTL_VALUE_TO_OBJ (ttl_string, v);
		unsigned i;

#if PRINT_DEBUG
		if (print_gc_messages)
		  fprintf (stderr, "-string length: %d\n", size);
#endif
		i = 0; 
		while (i < size)
		  {
		    if ((unsigned int) s->data[i] > 255)
		      abort ();
		    i++;
		  }
#if 1
		tracep += ROUND_TO_EVEN
		  (1 + (size + sizeof (unsigned short) - 1) /
		   sizeof (unsigned short));
#else
		tracep += ROUND_TO_EVEN
		  (1 + (size + ((sizeof (ttl_value) / 2) - 1)) /
		   (sizeof (ttl_value) / 2));
#endif
		break;
	      }

	    case TTL_TC_REAL:
	      {
		ttl_real r = TTL_VALUE_TO_OBJ (ttl_real, v);

		tracep += ROUND_TO_EVEN (size + 1);
		break;
	      }

	    case TTL_TC_LONG:
	      {
		ttl_long l = TTL_VALUE_TO_OBJ (ttl_long, v);

		tracep += ROUND_TO_EVEN (size + 1);
		break;
	      }

	    case TTL_TC_ARRAY:
	      {
		ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, v);
		unsigned i;

		for (i = 0; i < size; i++)
		  a->data[i] = check(copy (a->data[i]));
		tracep += ROUND_TO_EVEN (1 + size);
		break;
	      }

	    case TTL_TC_NONTRACED_ARRAY:
	      {
		ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, v);

		tracep += ROUND_TO_EVEN (1 + size);
		break;
	      }

	    case TTL_TC_BINARY_ARRAY:
	      {
		ttl_binary_array a = TTL_VALUE_TO_OBJ (ttl_binary_array, v);

		tracep += ROUND_TO_EVEN 
		  (1 + (size + (sizeof (ttl_value) - 1)) /
		   sizeof (ttl_value));
		break;
	      }

	    case TTL_TC_ENVIRONMENT:
	      {
		ttl_environment e = TTL_VALUE_TO_OBJ (ttl_environment, v);
		unsigned i;

		e->parent = check (copy (e->parent));
		for (i = 0; i < size - 1; i++)
		  e->locals[i] = check (copy (e->locals[i]));
		tracep += ROUND_TO_EVEN (1 + size);
		break;
	      }

#if 0
	    case TTL_TC_IDG_VARIABLE:
	      {
		ttl_idg_variable var = TTL_VALUE_TO_OBJ (ttl_idg_variable, v);

		var->value = check (copy (var->value));
		tracep += ROUND_TO_EVEN (size + 1);
		break;
	      }
#endif

	    case TTL_TC_VARIABLE:
	      {
		ttl_variable var = TTL_VALUE_TO_OBJ (ttl_variable, v);

		var->value = check (copy (var->value));
		var->constraints = check (copy (var->constraints));
		var->determined_by = check (copy (var->determined_by));

		tracep += ROUND_TO_EVEN (size + 1);
		break;
	      }

	    case TTL_TC_CONSTRAINT:
	      {
		ttl_constraint cnst = TTL_VALUE_TO_OBJ (ttl_constraint, v);

		cnst->variables = check (copy (cnst->variables));
		cnst->methods = check (copy (cnst->methods));
		cnst->selected_method = check (copy (cnst->selected_method));

		tracep += ROUND_TO_EVEN (size + 1);
		break;
	      }

	    case TTL_TC_METHOD:
	      {
		ttl_method meth = TTL_VALUE_TO_OBJ (ttl_method, v);

		meth->code = check (copy (meth->code));
		meth->inputs = check (copy (meth->inputs));
		meth->outputs = check (copy (meth->outputs));

		tracep += ROUND_TO_EVEN (size + 1);
		break;
	      }

	    case TTL_TC_CONSTRAINABLE_VARIABLE:
	      {
		ttl_constrainable_variable var =
		  TTL_VALUE_TO_OBJ (ttl_constrainable_variable, v);

		var->value = check (copy (var->value));
		tracep += ROUND_TO_EVEN (size + 1);
		break;
	      }

	    default:
	      fprintf (stderr, "gc (trace): invalid type code %d.\n", tc);
	      abort ();
	    }
	}
      else			/* Found a pair.  */
	{
	  ttl_pair p = (ttl_pair) tracep;

#if PRINT_DEBUG
	  if (print_gc_messages)
	    fprintf (stderr, "> Tracing pair <\n");
#endif
	  p->car = check (copy (p->car));
	  p->cdr = check (copy (p->cdr));

	  tracep += 2;
	}
    }
}

static void
flip (void)
{
  if (current_space == 0)
    {
      current_space = 1;
      ttl_alloc_ptr = space1;
      ttl_alloc_limit = space1limit;
      to_space = space1;
      to_space_limit = space1limit;
      from_space = space0;
      from_space_limit = space0limit;
    }
  else
    {
      current_space = 0;
      ttl_alloc_ptr = space0;
      ttl_alloc_limit = space0limit;
      to_space = space0;
      to_space_limit = space0limit;
      from_space = space1;
      from_space_limit = space1limit;
    }
}

static void
garbage_collect (int required)
{
  static struct tms begin_tms, end_tms;
  unsigned gc_time;
  int i;
  times (&begin_tms);

/*   fprintf (stderr, "\n**GC***\n"); */
  /* Do some statistics.  */
  ttl_stats.gc_calls++;

  /* Jump here to do another garbage collection immediately, that is
     if the last one did not free enough memory to satisfy the
     allocation request which triggered it.  */
 restart:

#if DRIBBLE
  fprintf (dribble, "*** About to walk the roots ***\n");
  walk (ttl_global_acc, 0);
  fprintf (dribble, "\n");
  walk (ttl_global_cont, 0);
  fprintf (dribble, "\n");
  walk (ttl_global_env, 0);
  fprintf (dribble, "\n");
  walk (ttl_global_pc, 0);
  fprintf (dribble, "\n");
  for (i = 0; i < ttl_global_sp; i++)
    walk (ttl_stack[i], 0);
  fprintf (dribble, "\n");

  fprintf (dribble, "*** Done ***\n\n");
  fflush (dribble);
#endif

  /* Flip the semi-spaces.  */
  flip ();

  /* Now copy the values held in machine registers.  */
#if PRINT_DEBUG
  if (print_gc_messages)
    fprintf (stderr, "{ acc:\n");
#endif
  ttl_global_acc = check (copy (ttl_global_acc));
#if PRINT_DEBUG
  if (print_gc_messages)
    {
      fprintf (stderr, "}\n");
      fprintf (stderr, "{ pc:\n");
    }
#endif
  ttl_global_pc = check (copy (ttl_global_pc));
#if PRINT_DEBUG
  if (print_gc_messages)
    {
      fprintf (stderr, "}\n");
      fprintf (stderr, "{ env:\n");
    }
#endif
  ttl_global_env = check (copy (ttl_global_env));
#if PRINT_DEBUG
  if (print_gc_messages)
    {
      fprintf (stderr, "}\n");
      fprintf (stderr, "{ cont:\n");
    }
#endif
  ttl_global_cont = check (copy (ttl_global_cont));
#if PRINT_DEBUG
  if (print_gc_messages)
    {
      fprintf (stderr, "}\n");

      fprintf (stderr, "{ stack:\n");
    }
#endif

  /* Copy the virtual machine's stack contents.  */
  for (i = 0; i < ttl_global_sp; i++)
    {
      ttl_stack[i] = check (copy (ttl_stack[i]));
    }
#if PRINT_DEBUG
  if (print_gc_messages)
    fprintf (stderr, "}\n");
#endif

  /* Copy the exception indicator strings.  */
  ttl_exception_handler = check (copy (ttl_exception_handler));
  ttl_saved_continuations = check (copy (ttl_saved_continuations));
  ttl_null_pointer_exception = check (copy (ttl_null_pointer_exception));
  ttl_subscript_exception = check (copy (ttl_subscript_exception));
  ttl_out_of_range_exception = check (copy (ttl_out_of_range_exception));
  ttl_wrong_variant_exception = check (copy (ttl_wrong_variant_exception));
  ttl_require_exception = check (copy (ttl_require_exception));
  
  /* Copy all externally registered roots.  */
  for (i = 0; i < global_root_count; i++)
    {
      ttl_value * rootp = global_roots[i];
      *rootp = check (copy (*rootp));
    }

  for (i = 0; i < MAX_SIGNAL; i++)
    {
      if (signal_handlers[i])
	signal_handlers[i] = check (copy (signal_handlers[i]));
    }
  if (timer_handler)
    timer_handler = check (copy (timer_handler));

  /* Trace phase, walk through to-space and copy all values reachable
     from to-space objects.  */
  trace ();

/*   memset (from_space, 0xff, */
/* 	  (from_space_limit - from_space) * sizeof (ttl_value)); */

  /* `heap_needs_resize' indicates that the last garbage collection
     could not free enough memory to keep 25% of the heap free.  So
     the old from-space was already resized and we now need to resize
     the old to-space (which is now from-space, how confusing).  */
  if (heap_needs_resize)
    {
      heap_needs_resize = 0;
#if 0
      fprintf (stderr, "heap needs resize\n");
#endif
      if (current_space == 1)
	{
	  ttl_value * heap;
	  
	  heap = malloc (sizeof (ttl_value) * (semi_space_in_words+3));
	  if (!heap)
	    {
	      fprintf (stderr, "turtle rt: out of virtual memory\n");
	      abort ();
	    }
	  free (space0orig);
	  space0orig = heap;
	  while (((unsigned) heap) & 0x3)
	    heap = (ttl_value *) (((unsigned) heap) + 1);
	  space0 = heap;
	  space0limit = space0 + semi_space_in_words;
	}
      else
	{
	  ttl_value * heap;

	  heap = malloc (sizeof (ttl_value) * (semi_space_in_words+3));
	  if (!heap)
	    {
	      fprintf (stderr, "turtle rt: out of virtual memory\n");
	      abort ();
	    }
	  free (space1orig);
	  space1orig = heap;
	  while (((unsigned) heap) & 0x3)
	    heap = (ttl_value *) (((unsigned) heap) + 1);
	  space1 = heap;
	  space1limit = space1 + semi_space_in_words;
	}
    }
  {
    size_t words_in_use = ttl_alloc_ptr - (current_space == 0 ? space0 : space1);
    size_t words_avail = semi_space_in_words;

    if (TTL_RATIO_DENOMINATOR * words_in_use >=
	TTL_RATIO_NOMINATOR * words_avail &&
	semi_space_in_words < (TTL_MAX_IN_WORDS / 2))
      {
	semi_space_in_words += ((TTL_INCR_IN_BYTES / 2) / sizeof (ttl_value));
	if (semi_space_in_words > TTL_MAX_IN_WORDS / 2)
	  semi_space_in_words = TTL_MAX_IN_WORDS;
#if 0
	fprintf (stderr, "resizing the heap to %d MB\n",
		 (semi_space_in_words * 2 * sizeof (ttl_value)) /
		 (1024 * 1024));
#endif
	heap_needs_resize = 1;
	if (current_space == 1)
	  {
	    ttl_value * heap;

	    heap = malloc (sizeof (ttl_value) * (semi_space_in_words+3));
	    if (!heap)
	      {
		fprintf (stderr, "turtle rt: out of virtual memory\n");
		abort ();
	      }
	    free (space0orig);
	    space0orig = heap;
	    while (((unsigned) heap) & 0x3)
	      heap = (ttl_value *) (((unsigned) heap) + 1);
	    space0 = heap;
	    space0limit = space0 + semi_space_in_words;
	  }
	else
	  {
	    ttl_value * heap;

	    heap = malloc (sizeof (ttl_value) * (semi_space_in_words+3));
	    if (!heap)
	      {
		fprintf (stderr, "turtle rt: out of virtual memory\n");
		abort ();
	      }
	    free (space1orig);
	    space1orig = heap;
	    while (((unsigned) heap) & 0x3)
	      heap = (ttl_value *) (((unsigned) heap) + 1);
	    space1 = heap;
	    space1limit = space1 + semi_space_in_words;
	  }
	ttl_stats.gc_grows++;
      }
  }

  /* Now see if we freed enough memory so that we can return to the
     caller.  If not, try it once more, hoping that the resized heap
     is large enough.  */

  if (ttl_alloc_ptr + ROUND_TO_EVEN (required) > ttl_alloc_limit)
    {
#if 0
      fprintf (stderr, "GC: not enough memory freed, should try again.\n");
#endif
      ttl_stats.gc_retries++;
      goto restart;
    }

  /* Finish statistics.  */
  times (&end_tms);

  gc_time = (end_tms.tms_utime + end_tms.tms_stime)
    - (begin_tms.tms_utime + begin_tms.tms_stime);
  if ((gc_time && !ttl_stats.min_gc_time) || (gc_time < ttl_stats.min_gc_time))
    ttl_stats.min_gc_time = gc_time;
  if (gc_time > ttl_stats.max_gc_time)
    ttl_stats.max_gc_time = gc_time;
  ttl_stats.total_gc_time += gc_time;
}

/* Register the location pointed to by `root' as a root for garbage
   collection.  */
void
ttl_register_root (ttl_value * root)
{
  if (global_root_count >= MAX_GLOBAL_ROOTS)
    {
      fprintf (stderr, "turtle rt: too many global roots\n");
      abort ();
    }
  global_roots[global_root_count++] = root;
}

void
ttl_garbage_collect (int required)
{
  garbage_collect (required);
}


/* Normal memory allocation routine.  This tries to allocate `words'
   words of storage and returns a pointer to the first word.  If not
   enough space is available, the garbage collector is invoked, and if
   the collection does not yield enough space, the process is
   halted.  */
/* MAY GC.  */
static ttl_value
ttl_alloc (int words)
{
  ttl_value v = (ttl_value) ttl_alloc_ptr;
  words = ROUND_TO_EVEN (words);
  ttl_alloc_ptr += words;
  if (ttl_alloc_ptr > ttl_alloc_limit)
    {
      ttl_alloc_ptr -= words;
      garbage_collect (words);
      v = (ttl_value) ttl_alloc_ptr;
      ttl_alloc_ptr += words;
      if (ttl_alloc_ptr > ttl_alloc_limit)
	{
	  ttl_alloc_ptr -= words;
	  alloc_failure (words);
	}
    }
  return v;
}

/* Unchecked heap space allocation.  Make sure that the heap has
   enough room to satisfy the request _before_ calling this
   function.  */
/* WILL NOT GC.  */
static ttl_value
ttl_unsafe_alloc (int words)
{
  ttl_value v = (ttl_value) ttl_alloc_ptr;
  words = ROUND_TO_EVEN (words);
  ttl_alloc_ptr += words;
  return v;
}

/* MAY GC.  */
static ttl_value
alloc_array (unsigned size, unsigned tc)
{
  ttl_array a = (ttl_array) ttl_alloc (size + 1);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += ROUND_TO_EVEN (size + 1);

  a->header = TTL_MAKE_HEADER (tc, size);
  return TTL_OBJ_TO_VALUE (a);
}

/* WILL NOT GC.  */
static ttl_value
unsafe_alloc_array (unsigned size, unsigned tc)
{
  ttl_array a = (ttl_array) ttl_unsafe_alloc (size + 1);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += ROUND_TO_EVEN (size + 1);

  a->header = TTL_MAKE_HEADER (tc, size);
  return TTL_OBJ_TO_VALUE (a);
}

/* MAY GC.  */
ttl_value
ttl_alloc_array (unsigned size)
{
  ttl_value v = alloc_array (size, TTL_TC_ARRAY);
  ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, v);
  unsigned i;

  for (i = 0; i < size; i++)
    a->data[i] = TTL_FALSE;

  return v;
}

#if 0
/* MAY GC.  */
static ttl_value
alloc_idg_variable (void)
{
  ttl_idg_variable v = (ttl_idg_variable) ttl_alloc (TTL_SIZEOF_IDG_VARIABLE +
						     1);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += ROUND_TO_EVEN (TTL_SIZEOF_IDG_VARIABLE + 1);

  v->header = TTL_MAKE_HEADER (TTL_TC_IDG_VARIABLE, TTL_SIZEOF_IDG_VARIABLE);
  v->value = TTL_NULL;
  return TTL_OBJ_TO_VALUE (v);
}

/* MAY GC.  */
ttl_value
ttl_alloc_idg_variable (void)
{
  return alloc_idg_variable ();
}
#endif

/* MAY GC.  */
static ttl_value
alloc_constrainable_variable (void)
{
  ttl_constrainable_variable v = (ttl_constrainable_variable)
    ttl_alloc (TTL_SIZEOF_CONSTRAINABLE_VARIABLE + 1);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += ROUND_TO_EVEN
    (TTL_SIZEOF_CONSTRAINABLE_VARIABLE + 1);

  v->header = TTL_MAKE_HEADER (TTL_TC_CONSTRAINABLE_VARIABLE,
			       TTL_SIZEOF_CONSTRAINABLE_VARIABLE);
  v->value = TTL_NULL;
  v->hook = NULL;
  return TTL_OBJ_TO_VALUE (v);
}

/* MAY GC.  */
ttl_value
ttl_alloc_constrainable_variable (void)
{
  return alloc_constrainable_variable ();
}

ttl_value
ttl_alloc_real_variable (void)
{
  ttl_value v = alloc_constrainable_variable ();
  TTL_VALUE_TO_OBJ (ttl_constrainable_variable, v)->hook =
    (ttl_solver_variable) idg_new_variable (ttl_genname (), v);
  return v;
}

ttl_value
ttl_alloc_fd_variable (void)
{
  ttl_value v = alloc_constrainable_variable ();
  TTL_VALUE_TO_OBJ (ttl_constrainable_variable, v)->hook = NULL;
  return v;
}

/* MAY GC.  */
ttl_value
ttl_alloc_constrained_array (unsigned size)
{
  ttl_value v = alloc_array (size, TTL_TC_ARRAY);
  ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, v);
  unsigned i;

  /* This double loop and the complicated reloading of `a' is necessary
     because `alloc_idg_variable()' might trigger a garbage
     collection.  */
  for (i = 0; i < size; i++)
    a->data[i] = TTL_FALSE;

  ttl_global_acc = v;
  for (i = 0; i < size; i++)
    {
      a = TTL_VALUE_TO_OBJ (ttl_array, ttl_global_acc);
      a->data[i] = alloc_constrainable_variable ();
    }
  return v;
}

/* MAY GC.  */
void
ttl_coerce_to_constrained_array (void)
{
  int size = TTL_SIZE (ttl_global_acc);
  ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, ttl_global_acc);

  while (size > 0)
    {
      ttl_value val = a->data[size - 1];
      a->data[size - 1] = alloc_constrainable_variable ();
      TTL_VALUE_TO_OBJ (ttl_constrainable_variable,
			a->data[size - 1])->value = val;
      size--;
    }
}

/* MAY GC.  */
void
ttl_coerce_to_constrained_list (void)
{
  ttl_value l = ttl_global_acc;
  while (l != TTL_NULL)
    {
      ttl_value val = TTL_CAR (l);
      TTL_CAR (l) = alloc_constrainable_variable ();
      TTL_VALUE_TO_OBJ (ttl_constrainable_variable, TTL_CAR (l))->value = val;
      l = TTL_CDR (l);
    }
}

/* WILL NOT GC.  */
ttl_value
ttl_unsafe_alloc_array (unsigned size)
{
  ttl_value v = unsafe_alloc_array (size, TTL_TC_ARRAY);
  ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, v);
  unsigned i;

  for (i = 0; i < size; i++)
    a->data[i] = TTL_FALSE;

  return v;
}

/* MAY GC.  */
ttl_value
ttl_alloc_nontraced_array (unsigned size)
{
  return alloc_array (size, TTL_TC_NONTRACED_ARRAY);
}

/* MAY GC.  */
ttl_value
ttl_alloc_binary_array (unsigned bytes)
{
  unsigned size = (bytes + (sizeof (ttl_value) - 1)) / sizeof (ttl_value);
  ttl_binary_array a = (ttl_binary_array) ttl_alloc (size + 1);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += ROUND_TO_EVEN (size + 1);

  a->header = TTL_MAKE_HEADER (TTL_TC_BINARY_ARRAY, bytes);
  return TTL_OBJ_TO_VALUE (a);
}

/* MAY GC.  */
ttl_value
ttl_alloc_string (unsigned chars)
{
  unsigned size = (chars + (sizeof (short) - 1)) / sizeof (short);
  ttl_string s = (ttl_string) ttl_alloc (size + 1);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += ROUND_TO_EVEN (size + 1);

  s->header = TTL_MAKE_HEADER (TTL_TC_STRING, chars);
  return TTL_OBJ_TO_VALUE (s);
}

/* WILL NOT GC.  */
ttl_value
ttl_unsafe_alloc_string (unsigned chars)
{
  unsigned size = (chars + (sizeof (short) - 1)) / sizeof (short);
  ttl_string s = (ttl_string) ttl_unsafe_alloc (size + 1);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += ROUND_TO_EVEN (size + 1);

  s->header = TTL_MAKE_HEADER (TTL_TC_STRING, chars);
  return TTL_OBJ_TO_VALUE (s);
}

/* MAY GC.  */
ttl_value
ttl_string_to_value (char * str, int len)
{
  unsigned chars = len < 0 ? strlen (str) : (unsigned) len;
  ttl_value v = ttl_alloc_string (chars);
  ttl_string s = TTL_VALUE_TO_OBJ (ttl_string, v);
  unsigned i;

  for (i = 0; i < chars; i++)
    s->data[i] = str[i];

  return v;
}

/* WILL NOT GC.  */
ttl_value
ttl_unsafe_string_to_value (char * str, int len)
{
  ttl_value v = ttl_unsafe_alloc_string (len);
  ttl_string s = TTL_VALUE_TO_OBJ (ttl_string, v);
  int i;

  for (i = 0; i < len; i++)
    s->data[i] = str[i];

  return v;
}

/* WILL NOT GC.  */
void
ttl_fill_string (ttl_value string, unsigned short fill)
{
  unsigned size = TTL_SIZE (string);
  ttl_string str = TTL_VALUE_TO_OBJ (ttl_string, string);
  unsigned i;

  for (i = 0; i < size; i++)
    str->data[i] = fill;
}

/* MAY GC.  */
ttl_value
ttl_append_strings (ttl_value s1, ttl_value s2)
{
  unsigned chars1 = TTL_SIZE (s1);
  unsigned chars2 = TTL_SIZE (s2);
  unsigned chars = chars1 + chars2;
  ttl_value v;
  ttl_string t1;
  ttl_string t2;
  ttl_string sv;
  unsigned i, j;

  ttl_stack[ttl_global_sp++] = s1;
  ttl_stack[ttl_global_sp++] = s2;
  v = ttl_alloc_string (chars);
  s2 = ttl_stack[--ttl_global_sp];
  s1 = ttl_stack[--ttl_global_sp];
  
  t1 = TTL_VALUE_TO_OBJ (ttl_string, s1);
  t2 = TTL_VALUE_TO_OBJ (ttl_string, s2);
  sv = TTL_VALUE_TO_OBJ (ttl_string, v);

  for (i = 0; i < chars1; i++)
    {
      sv->data[i] = t1->data[i];
    }
  for (i = chars1, j = 0; i < chars; i++, j++)
    {
      sv->data[i] = t2->data[j];
    }

  return v;
}

/* WILL NOT GC.  */
void
ttl_fill_array (ttl_value array, ttl_value fill)
{
  unsigned size = TTL_SIZE (array);
  ttl_array arr = TTL_VALUE_TO_OBJ (ttl_array, array);
  unsigned i;

  for (i = 0; i < size; i++)
    arr->data[i] = fill;
}

/* WILL NOT GC.  */
void
ttl_fill_constrained_array (ttl_value array, ttl_value fill)
{
  unsigned size = TTL_SIZE (array);
  ttl_array arr = TTL_VALUE_TO_OBJ (ttl_array, array);
  unsigned i;

  for (i = 0; i < size; i++)
    TTL_VALUE_TO_OBJ (ttl_constrainable_variable, arr->data[i])->value = fill;
}

/* MAY GC.  */
ttl_value
ttl_cons (ttl_value car, ttl_value cdr)
{
  ttl_pair p;

  ttl_stack[ttl_global_sp++] = car;
  ttl_stack[ttl_global_sp++] = cdr;
  p = (ttl_pair) ttl_alloc (2);
  cdr = ttl_stack[--ttl_global_sp];
  car = ttl_stack[--ttl_global_sp];

  ttl_stats.allocations++;
  ttl_stats.alloced_words += 2;

  p->car = car;
  p->cdr = cdr;
  return TTL_PAIR_TO_VALUE (p);
}

/* MAY GC.  */
ttl_value
ttl_make_list (unsigned elems, ttl_value fill)
{
  /* We use the accumulator for building the list because everything
     allocated so far must be safely stored in a register the garbage
     collector knows about.  */
  ttl_global_acc = TTL_NULL;
  while (elems-- > 0)
    ttl_global_acc = ttl_cons (fill, ttl_global_acc);
  return ttl_global_acc;
}

/* MAY GC.  */
ttl_value
ttl_real_to_value (double d)
{
  ttl_real r = (ttl_real) ttl_alloc (4);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += 4;

  r->value = d;
  r->header = TTL_MAKE_HEADER (TTL_TC_REAL, 3);
  return TTL_OBJ_TO_VALUE (r);
}

/* WILL NOT GC.  */
ttl_value
ttl_unsafe_real_to_value (double d)
{
  ttl_real r = (ttl_real) ttl_unsafe_alloc (4);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += 4;

  r->value = d;
  r->header = TTL_MAKE_HEADER (TTL_TC_REAL, 3);
  return TTL_OBJ_TO_VALUE (r);
}

/* MAY GC.  */
ttl_value
ttl_long_to_value (long i)
{
  ttl_long l = (ttl_long) ttl_alloc (4);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += 4;

  l->value = i;
  l->header = TTL_MAKE_HEADER (TTL_TC_LONG, 3);
  return TTL_OBJ_TO_VALUE (l);
}

/* WILL NOT GC.  */
ttl_value
ttl_unsafe_long_to_value (long i)
{
  ttl_long l = (ttl_long) ttl_unsafe_alloc (4);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += 4;

  l->value = i;
  l->header = TTL_MAKE_HEADER (TTL_TC_LONG, 3);
  return TTL_OBJ_TO_VALUE (l);
}

/* MAY GC.  */
ttl_value
ttl_alloc_variable (ttl_value initial_value)
{
  ttl_variable v = (ttl_variable) ttl_alloc (TTL_SIZEOF_VARIABLE + 1);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += ROUND_TO_EVEN (TTL_SIZEOF_VARIABLE + 1);

  v->header = TTL_MAKE_HEADER (TTL_TC_VARIABLE, TTL_SIZEOF_VARIABLE);
  v->value = initial_value;
  v->constraints = TTL_NULL;
  v->determined_by = TTL_NULL;
  v->walk_strength = TTL_WEAKEST_STRENGTH;
  v->mark = 0;
  v->valid = 1;
  return TTL_OBJ_TO_VALUE (v);
}

/* MAY GC.  */
ttl_value
ttl_alloc_constraint (int strength, ttl_value vars, ttl_value meths)
{
  ttl_constraint c = (ttl_constraint) ttl_alloc (TTL_SIZEOF_CONSTRAINT + 1);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += ROUND_TO_EVEN (TTL_SIZEOF_CONSTRAINT + 1);

  c->header = TTL_MAKE_HEADER (TTL_TC_CONSTRAINT, TTL_SIZEOF_CONSTRAINT);
  c->strength = strength;
  c->variables = vars;
  c->methods = meths;
  c->selected_method = TTL_NULL;
  c->mark = 0;
  return TTL_OBJ_TO_VALUE (c);
}

/* MAY GC.  */
ttl_value
ttl_alloc_method (ttl_value code, ttl_value inputs, ttl_value outputs)
{
  ttl_method m = (ttl_method) ttl_alloc (TTL_SIZEOF_METHOD + 1);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += ROUND_TO_EVEN (TTL_SIZEOF_METHOD + 1);

  m->header = TTL_MAKE_HEADER (TTL_TC_METHOD, TTL_SIZEOF_METHOD);
  m->code = code;
  m->inputs = inputs;
  m->outputs = outputs;
  return TTL_OBJ_TO_VALUE (m);
}

static int
weaker (int strength1, int strength2)
{
  return strength1 > strength2;
}

static int
enforced (ttl_value cnst)
{
  ttl_constraint cn = TTL_VALUE_TO_OBJ (ttl_constraint, cnst);
  return cn->selected_method != TTL_NULL;
}

static void
remove_constraint_from_list (ttl_value * cnst_list, ttl_value cnst)
{
  while (*cnst_list != TTL_NULL)
    {
      if (TTL_CAR (*cnst_list) == cnst)
	{
	  *cnst_list = TTL_CDR (*cnst_list);
	}
      else
	cnst_list = &(TTL_CDR (*cnst_list));
    }
}

static void
propagate_walk_strength (ttl_value cn_old_outputs)
{
}

static void
collect_unenforced (ttl_value * unenforced_cns, ttl_value cn_old_outputs,
		    int strength, int whatever)
{
}

static int
mvine_revoke_cn (ttl_value cn, int root_strength, int done_mark,
		 ttl_value * mvine_stack, ttl_value * redetermined_vars)
{
  return 0;
}

static int
mvine_enforce_cn (ttl_value cn, int root_strength, int done_mark,
		  ttl_value * mvine_stack, ttl_value * redetermined_vars)
{
  return 0;
}

static int
mvine_grow (int root_strength, int done_mark, ttl_value * mvine_stack,
	    ttl_value * redetermined_vars)
{
  if (*mvine_stack == TTL_NULL)
    return 1;
  else
    {
      int ok;
      ttl_value cnst = TTL_CAR (*mvine_stack);
      ttl_constraint cn = TTL_VALUE_TO_OBJ (ttl_constraint, cnst);

      *mvine_stack = TTL_CDR (*mvine_stack);
      if (cn->mark == done_mark)
	ok = mvine_grow (root_strength, done_mark, mvine_stack, 
			 redetermined_vars);
      else if (weaker (cn->strength, root_strength))
	ok = mvine_revoke_cn (cnst, root_strength, done_mark,
			      mvine_stack, redetermined_vars);
      else
	ok = mvine_enforce_cn (cnst, root_strength, done_mark,
			       mvine_stack, redetermined_vars);
      if (!ok)
	*mvine_stack = ttl_cons (cnst, *mvine_stack);
      return ok;
    }
}

static int
build_mvine (ttl_value cnst, ttl_value * redetermined_vars)
{
  return 0;
}

static void
update_method_graph (ttl_value * unenforced_cns, ttl_value * exec_roots)
{
  while (*unenforced_cns != TTL_NULL)
    {
      ttl_value redetermined_vars = TTL_NULL;
      int ok;
      int strength = TTL_WEAKEST_STRENGTH;
      ttl_value cn = TTL_NULL;
      ttl_value l = *unenforced_cns;
      while (l != TTL_NULL)
	{
	  if (weaker (strength,
		      TTL_VALUE_TO_OBJ (ttl_constraint,
					TTL_CAR (l))->strength))
	    {
	      cn = TTL_CAR (l);
	      strength = TTL_VALUE_TO_OBJ (ttl_constraint, cn)->strength;
	    }
	  l = TTL_CDR (l);
	}
      if (cn == TTL_NULL)
	abort ();
      remove_constraint_from_list (unenforced_cns, cn);
      ok = build_mvine (cn, &redetermined_vars);
      if (ok)
	{
	  ttl_value vars;

	  propagate_walk_strength (ttl_cons (cn, redetermined_vars));
	  collect_unenforced (unenforced_cns, redetermined_vars,
			      TTL_VALUE_TO_OBJ (ttl_constraint, cn)->strength,
			      0);
	  *exec_roots = ttl_cons (cn, *exec_roots);
	  for (vars = redetermined_vars; vars != TTL_NULL; 
	       vars = TTL_CDR (vars))
	    {
	      ttl_variable v = TTL_VALUE_TO_OBJ (ttl_variable,
						 TTL_CAR (vars));
	      if (v->determined_by == TTL_NULL)
		*exec_roots = ttl_cons (TTL_CAR (vars), *exec_roots);
	    }
	}
    }
}

static void
exec_from_roots (ttl_value exec_roots)
{
}

void
ttl_add_constraint (ttl_value cnst)
{
  ttl_value unenforced_cns = TTL_NULL;
  ttl_value exec_roots = TTL_NULL;
  ttl_constraint cn = TTL_VALUE_TO_OBJ (ttl_constraint, cnst);
  ttl_value vars;

  cn->selected_method = TTL_NULL;
  cn->mark = 0;
  for (vars = cn->variables; vars != TTL_NULL; vars = TTL_CDR (vars))
    {
      ttl_variable v = TTL_VALUE_TO_OBJ (ttl_variable, TTL_CAR (vars));
      v->constraints = ttl_cons (cnst, v->constraints);
    }
  unenforced_cns = ttl_cons (cnst, unenforced_cns);
  update_method_graph (&unenforced_cns, &exec_roots);
  exec_from_roots (exec_roots);
}

void
ttl_remove_constraint (ttl_value cnst)
{
  ttl_constraint cn = TTL_VALUE_TO_OBJ (ttl_constraint, cnst);
  ttl_value vars;
  
  for (vars = cn->variables; vars != TTL_NULL; vars = TTL_CDR (vars))
    {
      ttl_variable v = TTL_VALUE_TO_OBJ (ttl_variable, TTL_CAR (vars));
      remove_constraint_from_list (&(v->constraints), cnst);
    }
  if (enforced (cnst))
    {
      ttl_value unenforced_cns = TTL_NULL;
      ttl_value exec_roots = TTL_NULL;
      ttl_value cn_old_outputs =
	TTL_VALUE_TO_OBJ (ttl_method, cn->selected_method)->outputs;

      cn->selected_method = TTL_NULL;
      for (vars = cn_old_outputs; vars != TTL_NULL; vars = TTL_CDR (vars))
	{
	  ttl_variable v = TTL_VALUE_TO_OBJ (ttl_variable, TTL_CAR (vars));
	  v->determined_by = TTL_NULL;
	  v->walk_strength = TTL_WEAKEST_STRENGTH;
	}

      exec_roots = cn_old_outputs; /* Add to exec_roots, but
				      exec_roots is always empty
				      here. */

      propagate_walk_strength (cn_old_outputs);
      collect_unenforced (&unenforced_cns, cn_old_outputs, cn->strength, 1);
      update_method_graph (&unenforced_cns, &exec_roots);
      exec_from_roots (exec_roots);
    }
}

/* MAY GC.  */
static void
save_cont (ttl_descr next_pc, int sp_value)
{
  ttl_continuation c;

  if (ttl_global_sp != sp_value)
    {
      fprintf (stderr, "Aia! sp != sp_value (%d != %d)\n",
	       ttl_global_sp, sp_value);
      abort ();
    }
  c = (ttl_continuation) ttl_alloc (5 + sp_value);
  c->cont = ttl_global_cont;
  c->pc = next_pc;
  c->env = ttl_global_env;
  c->sp = ttl_global_sp;
  while (ttl_global_sp > 0)
    {
      ttl_global_sp--;
      c->stack[ttl_global_sp] = ttl_stack[ttl_global_sp];
    }
  c->header = TTL_MAKE_HEADER (TTL_TC_CONTINUATION, 4 + sp_value);
  ttl_global_cont = TTL_OBJ_TO_VALUE (c);

  ttl_stats.allocations++;
  ttl_stats.alloced_words += 5 + sp_value;

  ttl_stats.save_cont_count++;
}

/* WILL NOT GC.  */
static void
restore_cont (void)
{
  int x;
  ttl_continuation c = TTL_VALUE_TO_OBJ (ttl_continuation,
					 ttl_global_cont);
  for (x = 0; x < c->sp; x++)
    ttl_stack[x] = c->stack[x];
  ttl_global_sp = c->sp;
  ttl_global_pc = TTL_OBJ_TO_VALUE (c->pc);
  ttl_global_env = c->env;
  ttl_global_cont = c->cont;

  ttl_stats.restore_cont_count++;
}

/* The following is a hand-crafted version of the host procedures the
   compiler generates.  */

static int host_procedure (void);

static struct ttl_function_info func_info =
  {".start", "<runtime>", "libturtlert.c"};

static struct ttl_descr descriptors[] =
  {
    /* Exit continuation.  + 0 */
    {TTL_DESCRIPTOR_HEADER, host_procedure, &func_info, -1},
    /* Exception handler entry point.  + 1 */
    {TTL_DESCRIPTOR_HEADER, host_procedure, &func_info, -1},
    /* Timer interrupt entry point.  + 2 */
    {TTL_DESCRIPTOR_HEADER, host_procedure, &func_info, -1},
    /* Signal entry point.  + 3 */
    {TTL_DESCRIPTOR_HEADER, host_procedure, &func_info, -1},
    {TTL_DESCRIPTOR_HEADER, host_procedure, &func_info, -1},
    {TTL_DESCRIPTOR_HEADER, host_procedure, &func_info, -1},
    {TTL_DESCRIPTOR_HEADER, host_procedure, &func_info, -1}
  };

static void print_string (FILE * f, ttl_value v);

static void
c_signal_handler (int no)
{
  if (no >= 0 && no < MAX_SIGNAL)
    {
      signals_pending++;
      signal_mask[no]++;
      ttl_time_slice = 0;	/* Force immediate handling. */
      signal (no, c_signal_handler);
    }
  else
    abort ();
}


void
ttl_install_signal_handler (int no, ttl_value handler)
{
  if (no >= 0 && no < MAX_SIGNAL)
    {
      signal_handlers[no] = handler;
      if (handler)
	signal (no, c_signal_handler);
      else
	signal (no, SIG_DFL);
    }
}


void
ttl_install_timer_handler (ttl_value handler)
{
  timer_handler = handler;
}


static int
host_procedure (void)
{
  ttl_value acc;
  ttl_value * sp;
  ttl_value cont;
  ttl_value * alloc;
  ttl_value * alloc_limit;

  ttl_environment env;
  ttl_descr pc;
  ttl_closure self = NULL;

  TTL_RESTORE_REGISTERS;
 L_jump:
  switch (pc - descriptors)
    {
    case 0:
      TTL_SAVE_REGISTERS;
      ttl_exit (TTL_VALUE_TO_INT (ttl_global_acc));
      break;

    case 1:
      {
	ttl_value exc = *(--sp);
	if (exc == ttl_null_pointer_exception)
	  {
	    fprintf (stderr,
		     "\n\nturtle exception: null pointer\n");
	  }
	else if (exc == ttl_subscript_exception)
	  {
	    fprintf (stderr, "\n\nturtle exception: index out of bounds\n");
	  }
	else if (exc == ttl_out_of_range_exception)
	  {
	    fprintf (stderr, "\n\nturtle exception: value out of range\n");
	  }
	else if (exc == ttl_wrong_variant_exception)
	  {
	    fprintf (stderr, "\n\nturtle exception: wrong variant\n");
	  }
	else if (exc == ttl_require_exception)
	  {
	    fprintf (stderr,
		     "\n\nturtle exception: required expression failed\n");
	  }
	else
	  {
	    fprintf (stderr, "\n\nturtle exception: unknown exception: ");
	    print_string (stderr, exc);
	    fprintf (stderr, "\n");
	  }
	{
	  ttl_continuation c = TTL_VALUE_TO_OBJ (ttl_continuation,
						 ttl_saved_continuations);
	  struct ttl_function_info * last_function_info = NULL;
	  int last_line = -1;
	  int first = 1;

	  while (c)
	    {
	      if (c->pc && c->pc->line >= 0)
		{
		  if (c->pc->function_info != last_function_info ||
		      c->pc->line != last_line)
		    {
#if 0
		      if (first)
			fprintf (stderr, "         In line: ");
		      else
			fprintf (stderr, "     called from: ");
		      fprintf (stderr, "%s:%d",
			       c->pc->function_info->filename, 
			       c->pc->line + 1);
		      if (c->pc && c->pc->function_info->function)
			{
			  if (c->pc->function_info != last_function_info)
			    fprintf (stderr, " (function %s)",
				     c->pc->function_info->function);
			}
#else
		      if (first)
			fprintf (stderr, "     In function: ");
		      else
			fprintf (stderr, "     called from: ");
		      fprintf (stderr, "%s", c->pc->function_info->module);
		      fprintf (stderr, ".");
		      fprintf (stderr, "%s", c->pc->function_info->function);
		      {
			int i = strlen (c->pc->function_info->module)
			  + strlen (c->pc->function_info->function) + 18;
			while (i++ < 36)
			  fprintf (stderr, " ");
		      }
		      fprintf (stderr, " (%s:%d)",
			       c->pc->function_info->filename, 
			       c->pc->line + 1);
#endif
		      fprintf (stderr, "\n");
		      last_function_info = c->pc->function_info;
		      last_line = c->pc->line;
		      first = 0;
		    }
		}
	      c = TTL_VALUE_TO_OBJ (ttl_continuation, c->cont);
	    }
	}
	fprintf (stderr, "Program halted.\n");
	TTL_SAVE_REGISTERS;
	ttl_exit (1);
	break;
      }

    case 2:
      /* Timer interrupt.  */
      if (timer_handler)
	{
	  /* If a timer handler is installed, tail-call it.  */
	  pc = TTL_VALUE_TO_OBJ (ttl_descr, timer_handler);
	  goto save_regs_and_return;
	}
      else
	TTL_RESTORE_CONT;
      break;

    case 3:
      /* Signal handler.  */
      
      {
	int x = 0;
	/* Search for the first signal we caught.  */
	while (x < MAX_SIGNAL && !signal_mask[x])
	  x++;
	if (x < MAX_SIGNAL)
	  {
	    /* Found it, so delete it from list of pending signals.  */
	    signals_pending--;
	    signal_mask[x] = 0;
	    /* If there is a handler for this signal, tail-call it.  */
	    if (signal_handlers[x])
	      {
		*sp++ = TTL_INT_TO_VALUE (x);
		pc = TTL_VALUE_TO_OBJ(ttl_descr, signal_handlers[x]);
		goto save_regs_and_return;
	      }
	    /* If there is none, there must be an error somewhere.  */
	    abort ();
	  }
	/* If there is no signal, there must have been an error.  */
	abort ();
      }

      /* Never reached currently.  */
      fprintf (stderr, "turtle rt: received unhandled signal\n");
      TTL_SAVE_REGISTERS;
      ttl_exit (2);
      break;

    default:
      if (pc->host == host_procedure)
	{
	  self = (ttl_closure) pc;
	  pc = TTL_VALUE_TO_OBJ (ttl_descr, self->code);
	  env = TTL_VALUE_TO_OBJ (ttl_environment, self->env);
	  ttl_stats.closure_call_count++;
	  goto L_jump;
	}
      break;
    restore_cont:
      TTL_RESTORE_CONT_REALLY;
    }
  if (pc->host == host_procedure)
    {
      ttl_stats.local_call_count++;
      goto L_jump;
    }
 save_regs_and_return:
  TTL_SAVE_REGISTERS;
  return 0;
}


#if TTL_PROFILE_MEMORY
void
write_samples (void)
{
  if (prof_sample_count > 0)
    {
      unsigned c = prof_sample_count, i;

      for (i = 0; i < c; i++)
	fprintf (prof_file, "%u %u\n", prof_samples[i][1],
		 prof_samples[i][0]);
      prof_sample_count = 0;
    }
}
#endif /* TTL_PROFILE_MEMORY */


static struct tms last_tick_tms;

/* This function gets called by the dispatch loops whenever a host
   procedure ran out of its timeslice.  It determines whether this was
   due to a normal timeout or a signal, and sets the program counter
   accordingly.  */
static void
tick_function (void)
{
  struct tms now_tms;
  int elapsed;

  times (&now_tms);

  elapsed = (now_tms.tms_utime + now_tms.tms_stime)
    - (last_tick_tms.tms_utime + last_tick_tms.tms_stime);

  last_tick_tms = now_tms;

  ttl_time_slice = ttl_time_quantum;

  if (signals_pending > 0)
    {
      ttl_stats.signal_count++;
      ttl_global_pc = signal_handler;
    }
  else
    {
#if TTL_PROFILE_MEMORY
      write_samples ();
#endif /* TTL_PROFILE_MEMORY */
      ttl_stats.tick_count++;
      ttl_global_pc = timer_interrupt;
    }
}


void
ttl_dispatcher (ttl_value main)
{
  ttl_global_pc = main;
  while (1)
    {
      ttl_descr pc = TTL_VALUE_TO_OBJ (ttl_descr, ttl_global_pc);
      ttl_stats.dispatch_call_count++;
      if (pc->host ())
	tick_function ();
    }
}

void
ttl_init_dispatcher (ttl_value init)
{
  ttl_value start_cont = ttl_global_cont;
  ttl_global_pc = init;
  /* Push a dummy continuation, so that we can detect when it was
     popped.  */
  save_cont (descriptors + 0, ttl_global_sp);
  while (1)
    {
      ttl_descr pc = TTL_VALUE_TO_OBJ (ttl_descr, ttl_global_pc);
      ttl_stats.dispatch_call_count++;
      if (pc->host ())
	tick_function ();
      if (ttl_global_cont == start_cont)
	break;
    }
}

static void
print_string (FILE * f, ttl_value v)
{
  ttl_string s = TTL_VALUE_TO_OBJ (ttl_string, v);
  unsigned size = TTL_SIZE (v);
  unsigned i;

  for (i = 0; i < size; i++)
    if (s->data[i] >= 32 && s->data[i] <= 127)
      fprintf (f, "%c", (unsigned char) s->data[i]);
    else
      fprintf (f, "\\%d", s->data[i]);
}

static void
print_stats (void)
{
  double secs;

  fprintf (stderr, "dispatch calls:  %10u  direct calls:       %10u\n",
	   ttl_stats.dispatch_call_count, ttl_stats.direct_call_count);
  fprintf (stderr, "local calls:     %10u  closure calls:      %10u\n",
	   ttl_stats.local_call_count, ttl_stats.closure_call_count);
  fprintf (stderr, "GC checks:       %10u  GC calls:           %10u\n",
	   ttl_stats.gc_checks, ttl_stats.gc_calls);
  fprintf (stderr, "GC grows:        %10u  GC retries:         %10u\n",
	   ttl_stats.gc_grows, ttl_stats.gc_retries);
  fprintf (stderr, "allocations:     %10u\n", ttl_stats.allocations);
  fprintf (stderr, "allocated words: %10u (%u MB)\n",
	   ttl_stats.alloced_words, (ttl_stats.alloced_words * 4) /
	   (1024*1024));
  fprintf (stderr, "forwarded words: %10u  forwarded/GC:       %10u\n",
	   ttl_stats.forwarded_words,
	   ttl_stats.gc_calls > 0 ?
	   ttl_stats.forwarded_words / ttl_stats.gc_calls : 0);
  fprintf (stderr, "tick count:      %10u  signal count:       %10u\n",
	   ttl_stats.tick_count, ttl_stats.signal_count);
  fprintf (stderr, "save cont count: %10u  restore cont count: %10u\n\n",
	   ttl_stats.save_cont_count, ttl_stats.restore_cont_count);
  fprintf (stderr, "time:  total: %u  gc: %u (%u min/%u max)\n",
	   ttl_stats.total_run_time, ttl_stats.total_gc_time,
	   ttl_stats.min_gc_time, ttl_stats.max_gc_time);
  if (ttl_stats.total_run_time)
    {
      secs = ((double) ttl_stats.total_run_time) / CLOCKS_PER_SEC;
      fprintf (stderr, "allocation rate: %gMB/sec\n",
	       ((ttl_stats.alloced_words * 4) / (double) (1024 * 1024)) /
	       secs);
    }
  else
    fprintf (stderr, "allocation rate: N/A\n");
}

/* Reset the statistic counters.  */
static void
reset_stats (void)
{
  ttl_stats.dispatch_call_count = 0;
  ttl_stats.direct_call_count = 0;
  ttl_stats.local_call_count = 0;
  ttl_stats.closure_call_count = 0;
  ttl_stats.gc_checks = 0;
  ttl_stats.gc_calls = 0;
  ttl_stats.gc_grows = 0;
  ttl_stats.gc_retries = 0;
  ttl_stats.allocations = 0;
  ttl_stats.alloced_words = 0;
  ttl_stats.forwarded_words = 0;
  ttl_stats.save_cont_count = 0;
  ttl_stats.restore_cont_count = 0;
  ttl_stats.total_run_time = 0;
  ttl_stats.total_gc_time = 0;
  ttl_stats.min_gc_time = 0;
  ttl_stats.max_gc_time = 0;
  ttl_stats.tick_count = 0;
  ttl_stats.signal_count = 0;
}

static void
setup_command_line (char * argv0, int argc, char * argv[])
{
  ttl_value acc = TTL_NULL;
  unsigned i;

  for (i = argc; i > 0; i--)
    acc = ttl_cons (ttl_string_to_value (argv[i - 1], -1), acc);
  acc = ttl_cons (ttl_string_to_value (argv0, -1), acc);

  ttl_stack[ttl_global_sp++] = acc;
}

/* Initialize the registers of the virtual machine.  */
static void
setup_registers (void)
{
  ttl_global_pc = TTL_INT_TO_VALUE (0);
  ttl_global_acc = TTL_INT_TO_VALUE (0);
  ttl_global_env = TTL_OBJ_TO_VALUE (NULL);
  ttl_global_sp = 0;
  ttl_global_cont = TTL_OBJ_TO_VALUE (NULL);
}

/* Allocate the memory for the heap and set up the two semi-spaces.
   Initialize the allocation and allocation limit pointers.  */
static void
setup_heap (void)
{
  ttl_value * heap;

  semi_space_in_words = (heap_size_in_bytes / sizeof (ttl_word)) / 2;

  heap = malloc (sizeof (ttl_value) * (semi_space_in_words + 3));
  if (!heap)
    {
      fprintf (stderr, "turtle rt: out of virtual memory\n");
      abort ();
    }
  space0orig = heap;
  while (((unsigned) heap) & 0x3)
    heap = (ttl_value *) (((unsigned) heap) + 1);
  space0 = heap;
  space0limit = space0 + semi_space_in_words;


  heap = malloc (sizeof (ttl_value) * (semi_space_in_words + 3));
  if (!heap)
    {
      fprintf (stderr, "turtle rt: out of virtual memory\n");
      abort ();
    }
  space1orig = heap;
  while (((unsigned) heap) & 0x3)
    heap = (ttl_value *) (((unsigned) heap) + 1);
  space1 = heap;
  space1limit = space1 + semi_space_in_words;

  current_space = 0;
  ttl_alloc_ptr = space0;
  ttl_alloc_limit = space0limit;
  to_space = space0;
  to_space_limit = space0limit;
  from_space = space1;
  from_space_limit = space1limit;

#if 0
  fprintf (stderr, "heap_size: %d\n", heap_size_in_bytes);
  fprintf (stderr, "space0: %p\n", space0);
  fprintf (stderr, "space0limit: %p\n", space0limit);
  fprintf (stderr, "space0limit-space0: %d\n", (space0limit-space0)*4);
  fprintf (stderr, "space1: %p\n", space1);
  fprintf (stderr, "space1limit: %p\n", space1limit);
  fprintf (stderr, "space1limit-space1: %d\n", (space1limit-space1)*4);
  fprintf (stderr, "wasted: %d\n",
	   heap_size_in_bytes-(((space1limit-space1)+(space0limit-space0))*4));
  fprintf (stderr, "alloc: %p\n", ttl_alloc_ptr);
  fprintf (stderr, "alloclimit: %p\n", ttl_alloc_limit);
#endif
}

static struct tms begin_tms, end_tms;

/* This function gets called by the main modules right at the
   beginning of `main ()', before doing anything else.  */
void
ttl_initialize (int argc, char * argv[])
{
  char * argv0 = argv[0];

#if DRIBBLE
  dribble = fopen ("dribble", "w");
#endif

  /* Remember the start time, for timing the program run.  */  
  times (&begin_tms);

#if TTL_PROFILE_MEMORY
  {
    struct itimerval value;
    struct itimerval ovalue;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 1000;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 1000;
    if (setitimer (ITIMER_REAL, &value, &ovalue))
      {
	perror ("setitimer");
	abort ();
      }
    signal (SIGALRM, signal_alarm);
    prof_file = fopen ("turtle.prof", "w");
  }
#endif /* TTL_PROFILE_MEMORY */
  /* Check the command line and interpret Turtle special options.  */
  argv++;
  argc--;
  while (argc > 0)
    {
      if (argv[0][0] == '-' && argv[0][1] == ':')
	{
	  switch (argv[0][2])
	    {
	    case '?':
	      fprintf (stderr, "Options common to all Turtle programs:\n\n");
	      fprintf (stderr, "  -:hNUM   set heap size to NUM megabytes\n");
	      fprintf (stderr, "  -:s      print statistics on exit\n");
	      fprintf (stderr, "  -:g      switch on GC messages\n");
	      exit (0);
	      break;

	    case 'h':
	      {
		unsigned heap_bytes = atoi (argv[0] + 3);
		heap_size_in_bytes = heap_bytes * 1024 * 1024;
		if (heap_size_in_bytes < TTL_MIN_IN_BYTES)
		  heap_size_in_bytes = TTL_MIN_IN_BYTES;
		else if (heap_size_in_bytes > TTL_MAX_IN_BYTES)
		  heap_size_in_bytes = TTL_MAX_IN_BYTES;
		fprintf (stderr, "turtle rt: setting heap size to %uMB\n",
			 heap_size_in_bytes / (1024 * 1024));
	      }
	      break;

	    case 's':
	      print_stats_on_exit = 1;
	      fprintf (stderr, "turtle rt: switching on statistics\n");
	      break;

	    case 'g':
	      print_gc_messages = 1;
	      fprintf (stderr, "turtle rt: switching on GC messages\n");
	      break;
	    }
	  argc--;
	  argv++;
	}
      else
	break;
    }

  setup_registers ();
  setup_heap ();
  reset_stats ();
  /* Install root continuation, which exits the program.  */
  save_cont (descriptors + 0, 0);
  setup_command_line (argv0, argc, argv);

  /* Set up exception handling.  */
  ttl_null_pointer_exception = ttl_string_to_value ("null-pointer-exception",
						    -1);
  ttl_subscript_exception = ttl_string_to_value ("subscript-exception", -1);
  ttl_out_of_range_exception = ttl_string_to_value ("out-of-range-exception",
						    -1);
  ttl_wrong_variant_exception = ttl_string_to_value ("wrong-variant", -1);
  ttl_exception_handler = ttl_cons
    (ttl_cons (TTL_NULL, TTL_OBJ_TO_VALUE (descriptors + 1)),
     TTL_NULL);

  ttl_time_quantum = TTL_DEFAULT_TIME_QUANTUM;
  ttl_time_slice = ttl_time_quantum;
  timer_interrupt = TTL_OBJ_TO_VALUE (descriptors + 2);
  signal_handler = TTL_OBJ_TO_VALUE (descriptors + 3);
  times (&last_tick_tms);
}

/* Terminate the process with exit code `code', printing statistics if
   that was requested on startup.  */
static void
ttl_exit (int code)
{
#if DRIBBLE
  fclose (dribble)
#endif

#if TTL_PROFILE_MEMORY
  write_samples ();
  fclose (prof_file);
#endif /* TTL_PROFILE_MEMORY */
  times (&end_tms);
  ttl_stats.total_run_time = (end_tms.tms_utime + end_tms.tms_stime)
    - (begin_tms.tms_utime + begin_tms.tms_stime);

  if (print_stats_on_exit)
    print_stats ();
  exit (code);
}

char *
ttl_version_string (void)
{
  return __turtle_version;
}

char *
ttl_malloc_c_string (ttl_value str)
{
  ttl_string s = TTL_VALUE_TO_OBJ (ttl_string, str);
  size_t len = TTL_SIZE (str);
  char * result = malloc ((len + 1) * sizeof (char));
  size_t i;

  if (!result)
    {
      fprintf (stderr, "turtle rt: out of virtual memory\n");
      ttl_exit (1);
    }
  for (i = 0; i < len; i++)
    result[i] = (char) (s->data[i] & 0xff);
  result[len] = '\0';
  return result;
}

char **
ttl_malloc_c_string_array (ttl_value s_arr)
{
  ttl_array a = TTL_VALUE_TO_OBJ (ttl_array, s_arr);
  size_t len = TTL_SIZE (s_arr);
  char ** arr = malloc ((len + 1) * sizeof (char *));
  size_t i;

  if (!arr)
    {
      fprintf (stderr, "turtle rt: out of virtual memory\n");
      abort ();
    }
  for (i = 0; i < len; i++)
    arr[i] = ttl_malloc_c_string (a->data[i]);
  arr[len] = NULL;
  return arr;
}


void
ttl_free_c_string_array (char ** arr)
{
  int idx = 0;
  
  while (arr[idx])
    {
      free (arr[idx]);
      idx++;
    }
  free (arr);
}


static void
short_info (ttl_value acc)
{
  if (TTL_IMMEDIATE_P (acc))
    fprintf (stderr, "Immediate");
  else if (TTL_OBJECT_P (acc))
    {
      fprintf (stderr, "Object, tc=%ld (%s), size=%ld",
	       TTL_TYPE_CODE (acc), tc_names[TTL_TYPE_CODE (acc)],
	       TTL_SIZE (acc));
    }
  else if (TTL_PAIR_P (acc))
    {
      fprintf (stderr, "Pair");
    }
  else
    fprintf (stderr, "Header");
}

void
ttl_examine (void)
{
  ttl_value acc = ttl_global_acc;

  fprintf (stderr, "%08lx, ", (ttl_word) acc);
  if (TTL_IMMEDIATE_P (acc))
    short_info (acc);
  else if (TTL_OBJECT_P (acc))
    {
      ttl_value * raw = (void *) TTL_VALUE_TO_OBJ (ttl_broken_heart, acc);
      short_info (acc);
      fprintf (stderr, ", ");
      if (raw >= from_space && raw < from_space_limit)
	fprintf (stderr, "from-space, ");
      else if (raw >= to_space && raw < to_space_limit)
	fprintf (stderr, "to-space, ");
      else
	fprintf (stderr, "outer-space, ");
      switch (TTL_TYPE_CODE (acc))
	{
	case TTL_TC_BROKEN_HEART:
	  fprintf (stderr, "forward: %8p, ",
		   TTL_VALUE_TO_OBJ (ttl_broken_heart, acc)->forward);
	  short_info (TTL_VALUE_TO_OBJ (ttl_broken_heart, acc)->forward);
	  break;
	default:
	  fprintf (stderr, "no more info");
	}
    }
  else if (TTL_PAIR_P (acc))
    {
      ttl_value * raw = (void *) TTL_VALUE_TO_OBJ (ttl_broken_heart, acc);
      short_info (acc);
      fprintf (stderr, ", ");
      if (raw >= from_space && raw < from_space_limit)
	fprintf (stderr, "from-space, ");
      else if (raw >= to_space && raw < to_space_limit)
	fprintf (stderr, "to-space, ");
      else
	fprintf (stderr, "outer-space, ");
      short_info (TTL_CAR (acc));
      fprintf (stderr, ", ");
      short_info (TTL_CDR (acc));
    }
  else
    fprintf (stderr, "Header");
  fprintf (stderr, "\n");
  
}

static idg_constraint_list all_constraints = NULL;

static idg_constraint_list
addc (idg_constraint_list ls, idg_constraint c)
{
  if (ls == NULL)
    return idg_constraint_cons (c, NULL);
  else
    return idg_constraint_cons (ls->constraint, addc (ls->next, c));
}

void
ttl_add_real_constraint (int type, int variable_count)
{
  idg_constraint constraint;
  enum constraint_kind kind;
  double constant;
  idg_variable * variables;
  double * coeffs;
  ttl_value val;
  int i;
  int strength;

  variables = malloc (variable_count * sizeof (idg_variable));
  coeffs = malloc (variable_count * sizeof (double));

  i = 0;
  while (i < variable_count)
    {
      val = ttl_stack[--ttl_global_sp];
      coeffs[i] = TTL_VALUE_TO_OBJ (ttl_real, val)->value;
      val = ttl_stack[--ttl_global_sp];
      variables[i] = (idg_variable) TTL_VALUE_TO_OBJ
	(ttl_constrainable_variable, val)->hook;
      i++;
    }
  val = ttl_stack[--ttl_global_sp];
  constant = TTL_VALUE_TO_OBJ (ttl_real, val)->value;
  strength = TTL_VALUE_TO_INT (ttl_stack[--ttl_global_sp]);

  switch (type)
    {
    case EQ_EQ:
      kind = constraint_eq;
      break;
    case EQ_LE:
      kind = constraint_le;
      break;
    case EQ_GE:
      kind = constraint_ge;
      break;
    default:
      kind = constraint_eq;
      abort ();
    }

  constraint = idg_new_constraint ("c<0>", strength, kind, constant,
				   variable_count, variables,
				   coeffs);
						       
/*   all_constraints = idg_constraint_cons (constraint, */
/* 					 all_constraints); */
  all_constraints = addc (all_constraints, constraint);
}

void
ttl_real_resolve (void)
{
  int i;
  idg_constraint_list l;
  idg_solve (all_constraints);
  l = all_constraints;
  while (l)
    {
      idg_constraint c = l->constraint;
      for (i = 0; i < c->var_count; i++)
	{
	  idg_variable v = c->variables[i];
#if 0
	  if (!v->value->low_inf && !v->value->high_inf &&
	      v->value->low == v->value->high)
#endif
	    {
	      TTL_VALUE_TO_OBJ
		(ttl_real, TTL_VALUE_TO_OBJ
		 (ttl_constrainable_variable,
		  c->variables[i]->variable)->value)->value =
		(c->variables[i]->value->low + c->variables[i]->value->high) /
		2.0;
	    }
#if 0
	  else
	    {
	      fprintf (stderr, "underconstrained problem.\n");
	      abort ();
	    }
#endif
	}
      l = l->next;
    }
}

static fd_constraint_list all_fd_constraints = NULL;

void
ttl_add_fd_constraint (int type, int variable_count)
{
  fd_constraint constraint;
  enum constraint_kind kind;
  int constant;
  fd_variable * variables;
  int * coeffs;
  ttl_value val;
  int i;
  int strength;

  variables = malloc (variable_count * sizeof (fd_variable));
  coeffs = malloc (variable_count * sizeof (int));

  i = 0;
  while (i < variable_count)
    {
      val = ttl_stack[--ttl_global_sp];
      coeffs[i] = TTL_VALUE_TO_INT (val);
      val = ttl_stack[--ttl_global_sp];
      variables[i] = (fd_variable) TTL_VALUE_TO_OBJ
	(ttl_constrainable_variable, val)->hook;
      i++;
    }
  val = ttl_stack[--ttl_global_sp];
  constant = TTL_VALUE_TO_INT (val);
  strength = TTL_VALUE_TO_INT (ttl_stack[--ttl_global_sp]);

  switch (type)
    {
    case EQ_EQ:
      kind = constraint_eq;
      break;
    case EQ_LE:
      kind = constraint_le;
      break;
    case EQ_GE:
      kind = constraint_ge;
      break;
    case EQ_LT:
      kind = constraint_lt;
      break;
    case EQ_GT:
      kind = constraint_gt;
      break;
    case EQ_NE:
      kind = constraint_ne;
      break;
    default:
      kind = constraint_eq;
      abort ();
    }

  constraint = fd_new_constraint ("c<0>", strength, kind, constant,
				   variable_count, variables,
				   coeffs);
/*   fd_print_constraint (constraint); */
						       
  all_fd_constraints = fd_constraint_cons (constraint, all_fd_constraints);
}

void
ttl_fd_resolve (void)
{
  int i;
  fd_constraint_list l;
  fd_solve (all_fd_constraints);
  l = all_fd_constraints;
  while (l)
    {
      fd_constraint c = l->constraint;
      for (i = 0; i < c->var_count; i++)
	{
	  fd_variable v = c->variables[i];
/* 	  fprintf (stdout, "value: %s: %d\n", v->name, v->value); */
	  TTL_VALUE_TO_OBJ (ttl_constrainable_variable, v->variable)->value =
	    TTL_INT_TO_VALUE (v->value);
	}
      l = l->next;
    }
}

/* End of libturtlert.c.  */
