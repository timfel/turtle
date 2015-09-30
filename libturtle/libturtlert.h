/* libturtle/libturtlert.h -- Common declarations for the Turtle runtime.
 
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

#ifndef TTL_LIBTURTLERT_H
#define TTL_LIBTURTLERT_H


#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>


/* The Turtle runtime system collects various statistics while a
   Turtle program is running.  All these statistics are collected in a
   variable of the following structure.  */
struct ttl_statistics
{
  unsigned dispatch_call_count;	/* Dispatched function calls.  */
  unsigned direct_call_count;	/* Intra-module direct calls to functions. */
  unsigned local_call_count;	/* Intra-module indirec calls.  */
  unsigned closure_call_count;	/* Calls to closures.  */

  unsigned gc_checks;		/* Number of heap overflow checks.  */
  unsigned gc_calls;		/* Number of garbage collections.  */
  unsigned gc_grows;		/* Number of heap resizes.  */
  unsigned gc_retries;		/* Number of garbage collection iteratons.  */

  unsigned allocations;		/* Number of allocation operations.  */
  unsigned alloced_words;	/* Number of words allocated.  */
  unsigned forwarded_words;	/* Number of words forwarded.  */

  unsigned save_cont_count;	/* Number of contiuation saves.  */
  unsigned restore_cont_count;	/* Number of continuation restores.  */

  unsigned total_run_time;	/* Total runtime in clock ticks.  */
  unsigned total_gc_time;	/* Garbage collection in clock ticks.  */
  unsigned min_gc_time;		/* Minimum garbage collection duration.  */
  unsigned max_gc_time;		/* Maximum garbage collection duration.  */

  unsigned tick_count;		/* Number of tick timeouts.  */
  unsigned signal_count;	/* Number of signal handler calls.  */
};

extern struct ttl_statistics ttl_stats;

/* Basic data type.  A word must be at least as large as a pointer, so
   that pointers can be stored in words.  */
typedef unsigned long ttl_word;

/* `ttl_value' is the ubiquitous handle to all objects.  */
typedef ttl_word * ttl_value;

/* Mask to remove tags from descriptors.  */
#define TTL_MASK        3

/* Tags to distinguish the different object types.  */
#define TTL_IMM_TAG     0
#define TTL_OBJECT_TAG  1
#define TTL_PAIR_TAG    2
/* This is special and appears only in object headers.  */
#define TTL_HEADER_TAG  3

/* Predicate macros to distinguish between descriptor types.  */
#define TTL_IMMEDIATE_P(v)  ((((ttl_word) (v)) & TTL_MASK) == TTL_IMM_TAG)
#define TTL_OBJECT_P(v)     ((((ttl_word) (v)) & TTL_MASK) == TTL_OBJECT_TAG)
#define TTL_PAIR_P(v)       ((((ttl_word) (v)) & TTL_MASK) == TTL_PAIR_TAG)
#define TTL_HEADER_P(v)     ((((ttl_word) (v)) & TTL_MASK) == TTL_HEADER_TAG)

#define TTL_ALLOCATED_P(v)  (TTL_OBJECT_P(v) || TTL_PAIR_P(v))

/* Conversion macros integers <-> fixnums.  */
#define TTL_INT_TO_VALUE(i)    ((ttl_value) (((i) << 2) | TTL_IMM_TAG))
#define TTL_VALUE_TO_INT(v)    (((int) (v)) >> 2)

/* Maximum representable integer value.  */
#define TTL_MAX_INT (2147483647 >> 2)
/* Minimu representable integer value.  */
#define TTL_MIN_INT ((-TTL_MAX_INT) - 1)

/* Conversion macros char <-> character.  */
#define TTL_CHAR_TO_VALUE(c)   ((ttl_value) ((((ttl_word) (c)) << 16) | \
                                TTL_IMM_TAG))
#define TTL_VALUE_TO_CHAR(v)   (((ttl_word) (v)) >> 16)

/* Boolean constants.  */
#define TTL_FALSE ((ttl_value) ((0 << 2) | TTL_IMM_TAG))
#define TTL_TRUE  ((ttl_value) ((1 << 2) | TTL_IMM_TAG))

/* Conversion macros int <-> bool.  */
#define TTL_BOOL_TO_VALUE(b)   ((b) ? TTL_TRUE : TTL_FALSE)
#define TTL_VALUE_TO_BOOL(v)   (((ttl_word) (v)) >> 2)

/* The null pointer value, for terminating lists and uninitialized
   aggregate typed variables.  */
#define TTL_NULL               ((ttl_value) 0)

/* Return the header word of a stored object.  May only be called iff
   TTL_OBJECT_P (v) is true.  */
#define TTL_HEADER(v)    (*((ttl_value) (((char *) (v)) - TTL_OBJECT_TAG)))
/* Return the size stored in v's header.  May only be called iff
   TTL_OBJECT_P (v) is true.  */
#define TTL_SIZE(v)      ((TTL_HEADER(v)) >> 8)
/* Return the type code stored in v's header.  May only be called iff
   TTL_OBJECT_P (v) is true.  */
#define TTL_TYPE_CODE(v) (((TTL_HEADER(v)) >> 2) & 0x3f)

/* These are the type codes defined for various types of objects
   stored on the heap.  */
#define TTL_TC_BROKEN_HEART           0	/* To mark forwarded objects.  */
#define TTL_TC_CONTINUATION           1
#define TTL_TC_PROCEDURE              2
#define TTL_TC_CLOSURE                3
#define TTL_TC_STRING                 4
#define TTL_TC_REAL                   5
#define TTL_TC_ARRAY                  6
#define TTL_TC_NONTRACED_ARRAY        7
#define TTL_TC_BINARY_ARRAY           8
#define TTL_TC_ENVIRONMENT            9
#if 0
#define TTL_TC_IDG_VARIABLE           10
#endif
#define TTL_TC_CONSTRAINT             11
#define TTL_TC_LONG                   12
#define TTL_TC_VARIABLE               13
#define TTL_TC_METHOD                 14
#define TTL_TC_CONSTRAINABLE_VARIABLE 15

/* Given a type code and a size value, create a valid header word.  */
#define TTL_MAKE_HEADER(tc, size) ((((ttl_word) (size)) << 8) | \
                                   (((ttl_word) (tc)) << 2) | TTL_HEADER_TAG)

/* Convert a object pointer to a raw pointer and vice versa.  This may
   only be called iff TTL_OBJECT_P(v) is true.  */
#define TTL_VALUE_TO_OBJ(t, v) ((t) (((char *) (v)) - TTL_OBJECT_TAG))
#define TTL_OBJ_TO_VALUE(v)   ((ttl_value) (((ttl_word) (v)) + TTL_OBJECT_TAG))

/* Return the car/cdr of a pair.  This may only be called iff
   TTL_PAIR_P(v) is true.  */
#define TTL_CAR(v)   (((ttl_pair) (((char *) (v)) - TTL_PAIR_TAG))->car)
#define TTL_CDR(v)   (((ttl_pair) (((char *) (v)) - TTL_PAIR_TAG))->cdr)

/* Convert a object pointer to a raw pointer and vice versa.  This may
   only be called iff TTL_PAIR_P(v) is true.  */
#define TTL_PAIR_TO_VALUE(v) ((ttl_value) (((ttl_word) (v)) + TTL_PAIR_TAG))
#define TTL_VALUE_TO_PAIR(v) ((ttl_pair) (((char *) (v)) - TTL_PAIR_TAG))

/* This is the header field which is stored into the
   compiler-generated procedure descriptors.  */
#define TTL_DESCRIPTOR_HEADER TTL_MAKE_HEADER(TTL_TC_PROCEDURE, 3)

/* This structure represents a pair object.  Note that these are the
   only objects which do not have a header field.  */
typedef struct ttl_pair * ttl_pair;
struct ttl_pair
{
  ttl_value car;
  ttl_value cdr;
};

/* A broken heart object is stored into the place of forwarded objects
   while a garbage collection is in progress.  The only field (besides
   the header) points to the forwarded copy of the object.  Note:
   there is the invariant that there are never objects of this kind on
   the heap except during garbage collection, and then only in
   from-space.  */
typedef struct ttl_broken_heart * ttl_broken_heart;
struct ttl_broken_heart
{
  ttl_word header;
  ttl_value forward;
};

/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_REAL (3)

/* This object kind is for real numbers.  Because of alignment
   requirements on some architectures (and because it's faster on
   others), there is an usused word in the middle of the
   structure.  */
typedef struct ttl_real * ttl_real;
struct ttl_real
{
  ttl_word header;
  ttl_word unused;		/* For alignment.  */
  double value;
};

/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_LONG (3)

/* This object kind is for long numbers.  Because of alignment
   requirements on some architectures (and because it's faster on
   others), there is an usused word in the middle of the
   structure.  */
typedef struct ttl_long * ttl_long;
struct ttl_long
{
  ttl_word header;
  ttl_word unused;		/* For alignment.  */
  long value;
};

/* This holds strings.  The elements of strings are 16-bit values, so
   the Turtle runtime is prepared to be extended to use Unicode for
   character representation, if only the compiler could deal with it.
   The size stored in the header is counted in characters, not in
   words, so the size of a string object is ((TTL_SIZE (s) + 1) / 2) +
   1 words, where the last +1 is for the header.  */
typedef struct ttl_string * ttl_string;
struct ttl_string
{
  ttl_word header;
  unsigned short data[2];
};

/* Arrays store the size of the data in the header.  */
typedef struct ttl_array * ttl_array;
struct ttl_array
{
  ttl_word header;
  ttl_value data[1];
};

/* Untraced arrays are like the normal arrays above, but the data
   field is not traced during garbage collection and so should not
   contain any references to heap-allocated objects, or they might get
   lost during collection.  */
typedef struct ttl_untraced_array * ttl_untraced_array;
struct ttl_untraced_array
{
  ttl_word header;
  ttl_word data[1];
};

/* Binary arrays are arrays of bytes, and the size stored in the
   header is in bytes.  The length in words is thus calculated as
   ((TTL_SIZE(b) + 3) / 4) words +1 for the header.  */
typedef struct ttl_binary_array * ttl_binary_array;
struct ttl_binary_array
{
  ttl_word header;
  unsigned char data[4];
};

/* For every source code function, the compiler creates a structure of
   this type.  It is used for debugging purposes, such as
   backtraces.  */
struct ttl_function_info
{
  char * function;		/* Name of the function.  */
  char * module;		/* Name of the module, maybe qualified.  */
  char * filename;		/* Name of the source code file.  */
};

/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_DESCR 3

/* For functions, the compiler generates objects of this kind.  They
   are always statically allocated and thus don't need to get
   forwarded during collection.  `header' is a normal object header
   with type code TTL_TC_PROCEDURE, `host' is the function to call for
   executing the function described by this object, `function_info'
   points to the `struct ttl_function_info' associated to the function
   to which this descriptor belongs and `line' is the source code line
   associated with this descriptor.  */
typedef struct ttl_descr * ttl_descr;
struct ttl_descr
{
  ttl_word header;
  int (* host)(void);
  struct ttl_function_info * function_info;
  int line;
};

/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_CLOSURE 3

/* A closure is similar to a descriptor and it is important that the
   two fields at the beginning are the same for both structures.  When
   `host' is called with a closure in `ttl_global_pc', it notices that
   it is not a normal function descriptor but a closure and sets
   `ttl_global_gc' to the descriptor to be called, which is found in
   the field `code'.  `env' is the environment captured in the
   closure.  */
typedef struct ttl_closure * ttl_closure;
struct ttl_closure
{
  ttl_word header;
  int (* host)(void);
  ttl_value code;
  ttl_value env;
};

/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_CONTINUATION 4
typedef struct ttl_continuation * ttl_continuation;

/* A continuation captures the state of the virtual machine call for
   later resuming execution at a specified place, in the same state.
   Note that a continuation contains enough information to freeze a
   thread of execution and later resume it, so a multitasking feature
   could easily added to the current implementation.  */
struct ttl_continuation
{
  ttl_word header;
  ttl_value cont;
  ttl_descr pc;
  ttl_value env;
  int sp;
  ttl_value stack[1];
};

/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_ENVIRONMENT 1

/* An environment stores the parameters and local variables of a
   running function.  When entering a function, a new environment is
   created, its parameters are copied into the environment and can be
   accessed from there while the function is running.  Environment are
   captured in closures to implement higher-order functions.  */
typedef struct ttl_environment * ttl_environment;
struct ttl_environment
{
  ttl_word header;
  ttl_value parent;
  ttl_value locals[2];
};


#if 0
/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_IDG_VARIABLE (1 + 2 + 1)

typedef struct ttl_idg_variable * ttl_idg_variable;
struct ttl_idg_variable
{
  ttl_word header;
  ttl_value value;
  double val;
  idg_variable variable;
};
#endif

/* The internal representation for variables used by the individual
   solvers must match the following structures, that means the first
   word of the record must be a pointer to the location where the
   variable's value is stored.  This is necessary because the garbage
   collector must know how to update the pointers in the solvers' data
   structures.  */
typedef struct ttl_solver_variable * ttl_solver_variable;
struct ttl_solver_variable
{
  ttl_value variable;
};

/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_CONSTRAINABLE_VARIABLE (2)

typedef struct ttl_constrainable_variable * ttl_constrainable_variable;
struct ttl_constrainable_variable
{
  ttl_word header;
  ttl_value value;
  ttl_solver_variable hook;
};

#define TTL_WEAKEST_STRENGTH 10000

/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_VARIABLE 6

typedef struct ttl_variable * ttl_variable;
struct ttl_variable
{
  ttl_word header;
  ttl_value value;		/* Current value.  */
  ttl_value constraints;	/* List of constraints.  */
  ttl_value determined_by;	/* Constraint.  */
  int walk_strength;		/* Walkabout strength.  */
  int mark;			/* This variable's mark.  */
  int valid;			/* True if the variable is valid.  */
};

/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_CONSTRAINT 5

typedef struct ttl_constraint * ttl_constraint;
struct ttl_constraint
{
  ttl_word header;
  int strength;			/* Level in the constraint hierarchy.  */
  ttl_value variables;		/* List of variables.  */
  ttl_value methods;		/* List of methods.  */
  ttl_value selected_method;	/* Selected method.  */
  int mark;			/* This constraint's mark.  */
};

/* TTL_SIZEOF_* constants are without the header!  */
#define TTL_SIZEOF_METHOD 3

typedef struct ttl_method * ttl_method;
struct ttl_method
{
  ttl_word header;
  ttl_value code;		/* A descriptor or closure.  */
  ttl_value inputs;		/* List of input variables.  */
  ttl_value outputs;		/* List of output variables.  */
};

/* Registers for the Turtle machine.  */
extern ttl_value ttl_global_pc;
extern ttl_value ttl_global_acc;
extern ttl_value ttl_global_env;
extern ttl_value ttl_global_cont;
extern ttl_value * ttl_alloc_ptr;
extern ttl_value * ttl_alloc_limit;
extern int ttl_global_sp;
extern ttl_value ttl_stack[];

/* List of currently active exception handlers.  When an exception
   occurs, the first one is taken from the list and invoked.  */
extern ttl_value ttl_exception_handler;

/* When an exception occurs, the current chain of continuations is
   stored in this variable, so that it can later be examined, for
   example for printing a backtrace.  */
extern ttl_value ttl_saved_continuations;

/* These variables hold pre-defined exception names which might be
   raised by the runtime system of the virtual machine.  */
extern ttl_value ttl_null_pointer_exception;
extern ttl_value ttl_subscript_exception;
extern ttl_value ttl_out_of_range_exception;
extern ttl_value ttl_wrong_variant_exception;
extern ttl_value ttl_require_exception;


/* This macro stores all locally cached virtual machine registers to
   their global variables.  This is necessary when a host procedure is
   left or when a runtime function is called which might need the
   current values, for example for allocating memory or calling the
   garbage collector.  */
#define TTL_SAVE_REGISTERS			\
do {						\
  ttl_global_acc = acc;				\
  ttl_global_env = TTL_OBJ_TO_VALUE (env);	\
  ttl_global_pc = TTL_OBJ_TO_VALUE (pc);	\
  ttl_global_sp = sp - ttl_stack;				\
  ttl_alloc_ptr = alloc;			\
} while (0)
/*   ttl_global_sp = sp;				\ */


/* Fetch the locally cached virtual machine registers from their
   backing store.  */
#define TTL_RESTORE_REGISTERS					\
do {								\
  acc = ttl_global_acc;						\
  env = TTL_VALUE_TO_OBJ (ttl_environment, ttl_global_env);	\
  pc = TTL_VALUE_TO_OBJ (ttl_descr, ttl_global_pc);		\
  sp = ttl_stack + ttl_global_sp;				\
  alloc = ttl_alloc_ptr;					\
} while (0)
/*   sp = ttl_global_sp;						\ */


/* Check whether enough heap is free to allocate `words' words of
   memory.  Before checking, `words' is rounded to the next even
   value, because memory can only be allocated in multiples of 2
   words.  */
#define TTL_GC_CHECK(words)				\
do {							\
  ttl_stats.gc_checks++;				\
  if (alloc + (((words) + 1) & ~1) > ttl_alloc_limit)	\
    {							\
      TTL_SAVE_REGISTERS;				\
      ttl_garbage_collect (words);			\
      TTL_RESTORE_REGISTERS;				\
    }							\
} while (0)


/* Allocate `words' words on the heap and store a pointer to the
   beginning of the allocated area into `var'.  `Words' is rounded up
   to the next even value for the reasons described above.  */
#define TTL_ALLOC(var, words)				\
do {							\
  ttl_stats.allocations++;				\
  ttl_stats.alloced_words += ((words) + 1) & ~1;	\
  (var) = (void *) alloc;				\
  alloc += ((words) + 1) & ~1;				\
} while (0)


/* Create a pair on the heap and store a valid Turtle reference in the
   virtual machine register `acc'.  */
#define TTL_CONS				\
do {						\
  ttl_pair p;					\
  						\
  TTL_ALLOC (p, 2);				\
  p->car = *(--sp);				\
  p->cdr = acc;					\
  acc = TTL_PAIR_TO_VALUE (p);			\
} while (0)
/*   p->car = ttl_stack[--sp];			\ */


/* Create a real value with the double value `val' and store a
   reference to it in `acc'.  */
#define TTL_MAKE_REAL(val)					\
do {								\
  ttl_real _r;							\
  TTL_ALLOC (_r, TTL_SIZEOF_REAL + 1);				\
  _r->value = (val);						\
  _r->header = TTL_MAKE_HEADER (TTL_TC_REAL, TTL_SIZEOF_REAL);	\
  acc = TTL_OBJ_TO_VALUE (_r);					\
} while (0)


/* Create a long value `val' and store a reference to it in `acc'.  */
#define TTL_MAKE_LONG(val)					\
do {								\
  ttl_long _l;							\
  TTL_ALLOC (_l, TTL_SIZEOF_LONG + 1);				\
  _l->value = (val);						\
  _l->header = TTL_MAKE_HEADER (TTL_TC_LONG, TTL_SIZEOF_LONG);	\
  acc = TTL_OBJ_TO_VALUE (_l);					\
} while (0)


#define TTL_MAKE_UNINITIALIZED_ARRAY(len)		\
do {							\
  ttl_array a;						\
  TTL_ALLOC (a, (len) + 1);				\
  a->header = TTL_MAKE_HEADER (TTL_TC_ARRAY, (len));	\
  acc = TTL_OBJ_TO_VALUE (a);				\
} while (0)


/* Make a closure and store a reference in `acc'.  The descriptor to
   be called when the closure is invoked is given in `descriptor'.  */
#define TTL_MAKE_CLOSURE(descriptor)					\
do {									\
  ttl_closure c;							\
  TTL_ALLOC (c, TTL_SIZEOF_CLOSURE + 1);				\
  c->host = host_procedure;						\
  c->code = TTL_OBJ_TO_VALUE (descriptor);				\
  c->env = TTL_OBJ_TO_VALUE (env);					\
  c->header = TTL_MAKE_HEADER (TTL_TC_CLOSURE, TTL_SIZEOF_CLOSURE);	\
  acc = TTL_OBJ_TO_VALUE (c);						\
} while (0)


static char *
ttl_genname (void)
{
  static int counter = 0;
  char buf[32];
  sprintf (buf, "var%d", counter++);
  return strdup (buf);
}

/* Create a variable object and store it in `acc'.  */
#define TTL_MAKE_REAL_VARIABLE()					  \
do {									  \
  ttl_constrainable_variable v;						  \
  TTL_ALLOC (v, TTL_SIZEOF_CONSTRAINABLE_VARIABLE + 1);			  \
  v->hook = (ttl_solver_variable) idg_new_variable (ttl_genname (),	  \
						    TTL_OBJ_TO_VALUE(v)); \
  v->value = TTL_NULL;							  \
  v->header = TTL_MAKE_HEADER (TTL_TC_CONSTRAINABLE_VARIABLE,		  \
			       TTL_SIZEOF_CONSTRAINABLE_VARIABLE);	  \
  acc = TTL_OBJ_TO_VALUE (v);						  \
} while (0)


/* Create a variable object and store it in `acc'.  */
#define TTL_MAKE_FD_VARIABLE()						  \
do {									  \
  ttl_constrainable_variable v;						  \
  TTL_ALLOC (v, TTL_SIZEOF_CONSTRAINABLE_VARIABLE + 1);			  \
  v->hook = (ttl_solver_variable) fd_new_variable (ttl_genname (),	  \
						    TTL_OBJ_TO_VALUE(v)); \
  v->value = TTL_NULL;							  \
  v->header = TTL_MAKE_HEADER (TTL_TC_CONSTRAINABLE_VARIABLE,		  \
			       TTL_SIZEOF_CONSTRAINABLE_VARIABLE);	  \
  acc = TTL_OBJ_TO_VALUE (v);						  \
} while (0)


/* Make an enviroment with enough space for `params' parameter and
   `locs' local variables.  */
#define TTL_MAKE_ENV(params, locs)					   \
do {									   \
  ttl_environment e;							   \
  int i;								   \
									   \
  TTL_ALLOC (e, TTL_SIZEOF_ENVIRONMENT + 1 + (params) + (locs));	   \
  for (i = (params); i < (params) + (locs); i++)			   \
    e->locals[i] = NULL;						   \
  e->parent = TTL_OBJ_TO_VALUE (env);					   \
  e->header = TTL_MAKE_HEADER (TTL_TC_ENVIRONMENT, 			   \
			       TTL_SIZEOF_ENVIRONMENT +(params) + (locs)); \
  env = e;								   \
} while (0)


/* Create a continuation on the heap and save the machine state into
   it.  Make `next_pc' the descriptor at which execution should resume
   when the continuation is restored.  `sp_value' must be the current
   size of the evaluation stack and is used to save the stack into the
   continuation. */
#define TTL_SAVE_CONT(next_pc, sp_value)				\
do {									\
  ttl_continuation c;							\
  int s;								\
  TTL_ALLOC (c, TTL_SIZEOF_CONTINUATION + 1 + (sp_value));		\
  c->cont = ttl_global_cont;						\
  c->pc = (next_pc);							\
  c->env = TTL_OBJ_TO_VALUE (env);					\
  c->sp = sp - ttl_stack;						\
  s = c->sp;								\
  while (s > 0)								\
    {									\
      s--;								\
      c->stack[s] = *(--sp);						\
    }									\
  c->header = TTL_MAKE_HEADER (TTL_TC_CONTINUATION, 			\
			       TTL_SIZEOF_CONTINUATION + (sp_value));	\
  ttl_global_cont = TTL_OBJ_TO_VALUE (c);				\
  ttl_stats.save_cont_count++;						\
} while (0)

/*   if (sp_value != sp)							\ */
/*     {									\ */
/*       fprintf (stderr,							\ */
/* 	       "sp uncorrectly predicted (pred: %d, corr: %d)\n",	\ */
/* 	       (sp_value), sp);						\ */
/*       abort ();								\ */
/*     }									\ */

#define TTL_RESTORE_CONT goto restore_cont;

/* Take the current continuation from the stack of continuations and
   restore the machind state saved in it.  */
#define TTL_RESTORE_CONT_REALLY					\
do {								\
  int x;							\
  ttl_continuation c = TTL_VALUE_TO_OBJ (ttl_continuation,	\
					 ttl_global_cont);	\
  for (x = 0; x < c->sp; x++)					\
    ttl_stack[x] = c->stack[x];					\
  sp = c->sp + ttl_stack;					\
  pc = c->pc;							\
  env = TTL_VALUE_TO_OBJ (ttl_environment, c->env);		\
  ttl_global_cont = c->cont;					\
								\
  ttl_stats.restore_cont_count++;				\
} while (0)


/* Save the current continuation stack in the global variable
   `ttl_saved_continuation' (for later examination), push the
   exception name `exception' onto the stack and call the topmost
   exception handler.  */
#define TTL_RAISE(exception)						\
do {									\
  TTL_GC_CHECK (TTL_SIZEOF_CONTINUATION + 1 + (sp - ttl_stack));	\
  TTL_SAVE_CONT (pc, (sp - ttl_stack));					\
  ttl_saved_continuations = ttl_global_cont;				\
  *sp++ = (exception);					\
  pc = TTL_VALUE_TO_OBJ (ttl_descr,					\
                         TTL_CDR (TTL_CAR (ttl_exception_handler)));	\
  ttl_global_cont = TTL_CAR (TTL_CAR (ttl_exception_handler));		\
  ttl_exception_handler = TTL_CDR (ttl_exception_handler);		\
  goto save_regs_and_return;						\
} while (0)


/* Check whether the register `acc' contains the NULL pointer, and
   raise a `null-pointer' exception if it does.  */
#define TTL_NULL_CHECK				\
do {						\
  if (!acc)					\
    goto raise_null_pointer_exception;		\
} while (0)


/* Check wheterh `arr' is non-null and if `idx' is a valid index into
   the object `arr'.  Raise the appropriate exception if one of these
   is not the case.  */
#define TTL_RANGE_CHECK(idx, arr)		\
do {						\
  if (!arr)					\
    goto raise_null_pointer_exception;		\
  if (idx < 0 || idx >= TTL_SIZE (arr))		\
    goto raise_subscript_exception;		\
} while (0)


/* Various macros for handling the operand stack.  */
#if OLD_SP
#define TTL_PUSH() ttl_stack[sp++] = acc
#define TTL_POP()  acc = ttl_stack[--sp]
#define TTL_DUP()  ttl_stack[sp] = ttl_stack[sp - 1]; sp++
#define TTL_OVER() ttl_stack[sp] = ttl_stack[sp - 2]; sp++
#define TTL_DROP() --sp
#define TTL_TUPLE_REF(idx) \
 ttl_stack[sp++] = TTL_VALUE_TO_OBJ (ttl_array, acc)->data[idx]
#define TTL_TUPLE_SET(idx) \
 TTL_VALUE_TO_OBJ (ttl_array, ttl_stack[--sp])->data[idx] = acc
#else
#define TTL_PUSH() *sp++ = acc
#define TTL_POP()  acc = *(--sp)
#define TTL_DUP()  *sp = *(sp - 1); sp++
#define TTL_OVER() *sp = *(sp - 2); sp++
#define TTL_DROP() --sp
#define TTL_TUPLE_REF(idx) \
 *sp++ = TTL_VALUE_TO_OBJ (ttl_array, acc)->data[idx]
#define TTL_TUPLE_SET(idx) \
 TTL_VALUE_TO_OBJ (ttl_array, *(--sp))->data[idx] = acc
#endif

/* Initialize the Turtle runtime and save the command line arguments
   for later use in the program.  Also handle command line options for
   the runtime system, which start with `-:'.  */
void ttl_initialize (int argc, char * argv[]);

/* Call the Turtle procedure for which a descriptor is stored in
   `main', and never return.  */
void ttl_dispatcher (ttl_value main);
/* Call the Turtle procedure for which a descriptor is stored in
   `init', and return when this function returns.  This is used for
   calling the initialization functions of modules, hence the
   name.  */
void ttl_init_dispatcher (ttl_value init);

/* Perform a garbage collection.  At least `required' words must be
   freed, if not possible by a normal collection, the heap will be
   resized.  */
void ttl_garbage_collect (int required);

/* Register the address of a global variable as a root.  Values in
   this variable will be considered as garbage collection roots and
   never be freed during garbage collection.  Calls to this function
   are generated by the compiler for global variables which might hold
   non-immedieate values.  */
void ttl_register_root (ttl_value * root);

/* The following three functions do not check for heap overflow, so
   make sure that there is enough space before calling them.  */
ttl_value ttl_unsafe_string_to_value (char * str, int len);
ttl_value ttl_unsafe_real_to_value (double d);
ttl_value ttl_unsafe_long_to_value (long l);
/* This does not initialize the array elements.  */
ttl_value ttl_unsafe_alloc_array (unsigned size);

/* Similar to the functions above, but these call the garbage
   collector if necessary.  */
ttl_value ttl_string_to_value (char * str, int len);
ttl_value ttl_real_to_value (double d);
ttl_value ttl_long_to_value (long l);

/* Allocation functions for various types, which do not initialize the
   fields.  At least for `ttl_alloc_array' (and
   `ttl_unsafe_alloc_array'), this must happen immediately, or the
   program might crash at the next garbage collection.  */
ttl_value ttl_alloc_string (unsigned chars);
ttl_value ttl_alloc_binary_array (unsigned bytes);
ttl_value ttl_alloc_array (unsigned size);
ttl_value ttl_alloc_constrained_array (unsigned size);
ttl_value ttl_alloc_fd_variable (void);
ttl_value ttl_alloc_real_variable (void);
ttl_value ttl_alloc_constrainable_variable (void);

void ttl_coerce_to_constrained_array (void);
void ttl_coerce_to_constrained_list (void);

/* Use these functions for filling value `fill' into all slots of the
   given string or array values.  */
void ttl_fill_string (ttl_value string, unsigned short fill);
void ttl_fill_array (ttl_value array, ttl_value fill);
void ttl_fill_constrained_array (ttl_value array, ttl_value fill);

/* Append the strings `s1' and `s2' to form a new string.  This is
   called for the `+' operation on strings.  May call the garbage
   collector, if necessary.  */
ttl_value ttl_append_strings (ttl_value s1, ttl_value s2);

/* Create a list of `elems' elements, where the car of the elements
   will be initialized to `fill'.  May call the garbage collector.  */
ttl_value ttl_make_list (unsigned elems, ttl_value fill);

/* Return the version number of the Turtle implementation the runtime
   was taken from.  */
char * ttl_version_string (void);

/* Utility functions, mostly useful for handcoding.  The first creates
   a value null-terminated C string from a Turtle string (simply
   dropping the 8 upper bits).  The result must be free()d when not
   needed anymore, or a memory leak will occur.  The second takes a
   Turtle array of Turtle strings and returns a `char **' array, for
   example usable for C library functions such as `execve'.  Use the
   third functions to free such an array later.  */
char * ttl_malloc_c_string (ttl_value str);
char ** ttl_malloc_c_string_array (ttl_value s_arr);
void ttl_free_c_string_array (char ** arr);

/* The number of ticks remaining for this time slice.  This gets
   decremented at every function entry or at the top of loops, and
   when it falls below zero, a timer function will be called.  It is
   also used for signal handling, since a caught signal will set it to
   0 so that it will be handled at the next checkpoint.  */
extern int ttl_time_slice;

/* Install `handler' (a function descriptor or closure) as the signal
   handler for signal number `no'.  */
void ttl_install_signal_handler (int no, ttl_value handler);

/* Install `handler' (a function descriptor or closure) as the timer
   function, which gets called whenever a time slice expires.  */
void ttl_install_timer_handler (ttl_value handler);


void ttl_add_real_constraint (int type, int variables);
void ttl_real_resolve (void);
void ttl_add_fd_constraint (int type, int variables);
void ttl_fd_resolve (void);

ttl_value ttl_alloc_variable (ttl_value initial_value);
ttl_value ttl_alloc_constraint (int strength, ttl_value vars, ttl_value meths);
ttl_value ttl_alloc_method (ttl_value code, ttl_value inputs,
			    ttl_value outputs);


#endif /* not TTL_LIBTURTLERT_H */
