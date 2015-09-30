/* libturtle/indigo.c -- Indigo constraint solver.
 
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

#include <stdio.h>
#include <math.h>

#include "version.h"
#include "indigo.h"

#define DEBUG 0

extern void yyparse (void);
char * filename = "(stdin)";

idg_interval
idg_new_interval (double low, double high)
{
  idg_interval iv = malloc (sizeof (struct idg_interval));
  iv->low = low;
  iv->high = high;
  iv->low_inf = 0;
  iv->high_inf = 0;
  return iv;
}

idg_interval
idg_new_interval_inf (void)
{
  idg_interval iv = malloc (sizeof (struct idg_interval));
  iv->low = 0.0;
  iv->high = 0.0;
  iv->low_inf = 1;
  iv->high_inf = 1;
  return iv;
}

idg_interval
idg_new_interval_full (double low, double high, int low_inf, int high_inf)
{
  idg_interval iv = malloc (sizeof (struct idg_interval));
  iv->low = low;
  iv->high = high;
  iv->low_inf = low_inf;
  iv->high_inf = high_inf;
  return iv;
}

idg_interval
idg_new_interval_low (double low)
{
  idg_interval iv = malloc (sizeof (struct idg_interval));
  iv->low = low;
  iv->high = 0.0;
  iv->low_inf = 0;
  iv->high_inf = 1;
  return iv;
}

idg_interval
idg_new_interval_high (double high)
{
  idg_interval iv = malloc (sizeof (struct idg_interval));
  iv->low = 0.0;
  iv->high = high;
  iv->low_inf = 1;
  iv->high_inf = 0;
  return iv;
}

idg_interval
idg_new_interval_singleton (double value)
{
  idg_interval iv = malloc (sizeof (struct idg_interval));
  iv->low = value;
  iv->high = value;
  iv->low_inf = 0;
  iv->high_inf = 0;
  return iv;
}

int
idg_interval_empty_p (idg_interval a)
{
  return (!a->low_inf && !a->high_inf && a->low > a->high);
}

int
idg_interval_singleton_p (idg_interval a)
{
  return (!a->low_inf && !a->high_inf && a->low == a->high);
}

int
idg_intervals_equal_p (idg_interval a, idg_interval b)
{
  int equal = 1;
  if (a->low_inf)
    {
      if (!b->low_inf)
	equal = 0;
    }
  else
    if (b->low_inf)
      equal = 0;
    else if (a->low != b->low)
      equal = 0;
      
  if (a->high_inf)
    {
      if (!b->high_inf)
	equal = 0;
    }
  else
    if (b->high_inf)
      equal = 0;
    else if (a->high != b->high)
      equal = 0;
  return equal;
}

idg_interval
idg_add_intervals (idg_interval a, idg_interval b)
{
  double lower, upper;
  int low_inf, high_inf;
  
  if (idg_interval_empty_p (a))
    return a;
  if (idg_interval_empty_p (b))
    return b;
  if (a->low_inf || b->low_inf)
    {
      low_inf = 1;
      lower = 0.0;
    }
  else
    {
      low_inf = 0;
      lower = a->low + b->low;
    }
  if (a->high_inf || b->high_inf)
    {
      high_inf = 1;
      upper = 0.0;
    }
  else
    {
      high_inf = 0;
      upper = a->high + b->high;
    }
  return idg_new_interval_full (lower, upper, low_inf, high_inf);
}

idg_interval
idg_sub_intervals (idg_interval a, idg_interval b)
{
  double lower, upper;
  int low_inf, high_inf;
  
  if (idg_interval_empty_p (a))
    return a;
  if (idg_interval_empty_p (b))
    return b;
  if (a->low_inf || b->high_inf)
    {
      low_inf = 1;
      lower = 0.0;
    }
  else
    {
      low_inf = 0;
      lower = a->low - b->high;
    }
  if (a->high_inf || b->low_inf)
    {
      high_inf = 1;
      upper = 0.0;
    }
  else
    {
      high_inf = 0;
      upper = a->high - b->low;
    }
  return idg_new_interval_full (lower, upper, low_inf, high_inf);
}

static double
dmin (double a, double b)
{
  return a < b ? a : b;
}

static double
dmax (double a, double b)
{
  return a > b ? a : b;
}

idg_interval
idg_mul_intervals (idg_interval a, idg_interval b)
{
  double lower = 0.0, upper = 0.0, temp;
  int not_set = 1;
  int upper_inf = 0, lower_inf = 0;

  if (idg_interval_empty_p (a))
    return a;
  if (idg_interval_empty_p (b))
    return b;
  if (!a->low_inf && !b->low_inf)
    {
      not_set = 0;
      upper = lower = a->low * b->low;
    }
  if (!a->low_inf && !b->high_inf)
    {
      temp = a->low * b->high;
      lower = not_set ? temp : dmin (lower, temp);
      upper = not_set ? temp : dmax (upper, temp);
    }
  if (!a->high_inf && !b->low_inf)
    {
      temp = a->high * b->low;
      lower = not_set ? temp : dmin (lower, temp);
      upper = not_set ? temp : dmax (upper, temp);
    }
  if ((a->low_inf && (b->high_inf || b->high > 0)) ||
      (b->low_inf && (a->high_inf || a->high > 0)) ||
      (a->high_inf && (b->low_inf || b->low < 0)) ||
      (b->high_inf && (a->low_inf || a->low < 0)))
    {
      lower_inf = 1;
    }
  if ((a->low_inf && (b->low_inf || b->low < 0)) ||
      (b->low_inf && (a->low_inf || a->low < 0)) ||
      (a->high_inf && (b->high_inf || b->high > 0)) ||
      (b->high_inf && (a->high_inf || a->high > 0)))
    {
      upper_inf = 1;
    }
  return idg_new_interval_full (lower, upper, lower_inf, upper_inf);
}

idg_interval
idg_div_intervals (idg_interval n, idg_interval d)
{
  double lower = 0.0, upper = 0.0, temp;
  int n_pos, n_neg, d_epsilon, d_neg_epsilon;
  int not_set = 1;
  int lower_inf = 0, upper_inf = 0;

  if (idg_interval_empty_p (n))
    return n;
  if (idg_interval_empty_p (d))
    return d;
  if (!n->low_inf && !d->low_inf)
    {
      not_set = 0;
      upper = lower = n->low / d->low;
    }
  if (!n->low_inf && !d->high_inf)
    {
      temp = n->low / d->high;
      lower = not_set ? temp : dmin (lower, temp);
      upper = not_set ? temp : dmax (upper, temp);
    }
  if (!n->high_inf && !d->low_inf)
    {
      temp = n->high / d->low;
      lower = not_set ? temp : dmin (lower, temp);
      upper = not_set ? temp : dmax (upper, temp);
    }
  if (!n->high_inf && !d->high_inf)
    {
      temp = n->high / d->high;
      lower = not_set ? temp : dmin (lower, temp);
      upper = not_set ? temp : dmax (upper, temp);
    }
  n_pos = n->high_inf || n->high > 0;
  n_neg = n->low_inf || n->low < 0;
  d_epsilon = (d->low_inf || d->low < 0) && (d->high_inf || d->high > 0);
  d_neg_epsilon = (d->low_inf || d->low < 0) && (d->high_inf || d->high >= 0);
  if ((n->low_inf && (d->high_inf || d->high > 0)) ||
      (n->high_inf && (d->low_inf || d->low < 0)) ||
      (n_neg && d_epsilon) ||
      (n_pos && d_neg_epsilon))
    {
      lower_inf = 1;
    }
  if ((n->low_inf && (d->low_inf || d->low < 0)) ||
      (n->high_inf && (d->high_inf || d->high > 0)) ||
      (n_neg && d_neg_epsilon) ||
      (n_pos && d_epsilon))
    {
      upper_inf = 1;
    }
  return idg_new_interval_full (lower, upper, lower_inf, upper_inf);
}

int
idg_intersect (idg_interval a, idg_interval b)
{
  if (idg_interval_empty_p (a) || idg_interval_empty_p (b))
    return 0;
  if (a->low_inf && a->high_inf)
    return 1;
  if (b->low_inf && b->high_inf)
    return 1;
  if (a->low_inf && b->low_inf)
    return 1;
  if (a->high_inf && b->high_inf)
    return 1;
  if (a->high_inf && b->low_inf)
    return a->low <= b->high;
  if (b->high_inf && a->low_inf)
    return b->low <= a->high;
  if (a->low_inf)
    return a->high >= b->low;
  if (b->low_inf)
    return b->high >= a->low;
  if (a->high_inf)
    return a->low <= b->high;
  if (b->high_inf)
    return b->low <= a->high;
  return (a->low <= b->low && a->high >= b->low) ||
    (b->low <= a->low && b->high >= a->low);
}

idg_interval
idg_intersection (idg_interval a, idg_interval b)
{
  double lower = 0.0, upper = 0.0;
  int lower_inf = 0, upper_inf = 0;
  if (a->low_inf && b->low_inf)
    lower_inf = 1;
  else if (a->low_inf)
    lower = b->low;
  else if (b->low_inf)
    lower = a->low;
  else
    lower = dmax (a->low, b->low);
  if (a->high_inf && b->high_inf)
    upper_inf = 1;
  else if (a->high_inf)
    upper = b->high;
  else if (b->high_inf)
    upper = a->high;
  else
    upper = dmin (a->high, b->high);
  return idg_new_interval_full (lower, upper, lower_inf, upper_inf);
}

idg_interval
idg_tighten (idg_interval var, idg_interval val, idg_variable variable)
{
  idg_interval intvl;
#if DEBUG
  printf ("tightening variable %s ", variable->name);
  idg_print (stdout, var);
  printf (" with value ");
  idg_print (stdout, val);
  printf (": ");
#endif
  if (idg_intersect (var, val))
    {
      intvl = idg_intersection (var, val);
    }
  else
    {
      if (var->high < val->low)
	intvl = idg_new_interval_singleton (var->high);
      else
	intvl = idg_new_interval_singleton (var->low);
    }
#if DEBUG
  idg_print (stdout, intvl);
  printf ("\n");
#endif
  return intvl;
}

void
idg_print (FILE * f, idg_interval i)
{
  if (idg_interval_singleton_p (i))
    fprintf (f, "%g", i->low);
  else
    {
      fprintf (f, "[");
      if (!idg_interval_empty_p (i))
	{
	  if (i->low_inf)
	    fprintf (f, "-INF, ");
	  else
	    fprintf (f, "%g, ", i->low);
	  if (i->high_inf)
	    fprintf (f, "+INF");
	  else
	    fprintf (f, "%g", i->high);
	}
      fprintf (f, "]");
    }
}

void
idg_print_variable (idg_variable v)
{
  printf ("%s = ", v->name);
  idg_print (stdout, v->value);
  printf ("\n");
}

void idg_print_constraint (idg_constraint c)
{
  char * strength_names[] =
    {
      "required   ",		/* 0 */
      "strong     ",		/* 1 */
      "medium     ",		/* 2 */
      "weak       ",		/* 3 */
      "abs_weakest"		/* 4 */
    };
  char * op_names[] =
    {
      "=",
      "<=",
      ">="
    };
  int i;
  printf ("%s:\t[%s]\t", c->name, strength_names[c->strength]);
  for (i = 0; i < c->var_count; i++)
    {
      if (c->coeffs[i] == 1)
	{
	  if (i > 0)
	    printf (" + ");
	}
      else if (c->coeffs[i] == -1)
	{
	  if (i > 0)
	    printf (" - ");
	}
      else if (c->coeffs[i] > 0)
	{
	  if (i > 0)
	    printf (" + %g * ", c->coeffs[i]);
	  else
	    printf ("%g * ", c->coeffs[i]);
	}
      else if (c->coeffs[i] < 0)
	{
	  if (i > 0)
	    printf (" - %g *", -c->coeffs[i]);
	  else
	    printf ("%g *", c->coeffs[i]);
	}
      printf ("%s", c->variables[i]->name);
    }
  if (c->constant == 0.0)
    ;
  else if (c->constant < 0.0)
    printf (" - %g", -c->constant);
  else
    printf (" + %g", c->constant);
  printf (" %s 0\n", op_names[(int) c->kind]);
  
}

idg_constraint_list
idg_constraint_cons (idg_constraint constraint, idg_constraint_list tail)
{
  idg_constraint_list l = malloc (sizeof (struct idg_constraint_list));
  l->next = tail;
  l->constraint = constraint;
  return l;
}

idg_variable_list
idg_variable_cons (idg_variable variable, idg_variable_list tail)
{
  idg_variable_list l = malloc (sizeof (struct idg_variable_list));
  l->next = tail;
  l->variable = variable;
  return l;
}

idg_constraint
idg_new_constraint (char * name, int strength, enum constraint_kind kind,
		    double constant, int var_count,
		    idg_variable * variables,
		    double * coeffs)
{
  int i;
  idg_constraint c = malloc (sizeof (struct idg_constraint));
  c->name = name;
  c->strength = strength;
  c->kind = kind;
  c->constant = -constant;
  c->var_count = var_count;
  c->variables = variables;
  c->coeffs = coeffs;
  for (i = 0; i < var_count; i++)
    {
      variables[i]->constraints =
	idg_constraint_cons (c, variables[i]->constraints);
    }
  return c;
}

idg_variable
idg_new_variable (char * name, void * store)
{
  idg_variable v = malloc (sizeof (struct idg_variable));
  v->name = name;
  v->value = NULL;
  v->constraints = NULL;
  v->variable = store;
  return v;
}

void
vqueue_add (idg_variable_list * queue, idg_variable variable)
{
  while (*queue)
    queue = &((*queue)->next);
  *queue = idg_variable_cons (variable, NULL);
}

void
queue_add (idg_constraint_list * queue, idg_constraint constraint)
{
  while (*queue)
    queue = &((*queue)->next);
  *queue = idg_constraint_cons (constraint, NULL);
}

void
queue_delete (idg_constraint_list * queue, idg_constraint constraint)
{
  while (*queue)
    {
      if ((*queue)->constraint == constraint)
	*queue = (*queue)->next;
      else
	queue = &((*queue)->next);
    }
}

void
dequeue (idg_constraint_list * queue)
{
  *queue = (*queue)->next;
}

int
var_search (idg_variable_list list, idg_variable var)
{
  while (list)
    {
      if (list->variable == var)
	return 1;
      list = list->next;
    }
  return 0;
}

int
constraint_search (idg_constraint_list list, idg_constraint c)
{
  while (list)
    {
      if (list->constraint == c)
	return 1;
      list = list->next;
    }
  return 0;
}

int
tighten_bounds_for (idg_constraint constraint,
		    idg_variable variable)
{
  int i;
  idg_interval val = idg_new_interval_singleton (0.0);
  idg_interval res;
  double var_coeff = 1.0;
  for (i = 0; i < constraint->var_count; i++)
    {
      if (constraint->variables[i] != variable)
	val = idg_add_intervals (val,
				 idg_mul_intervals
				 (constraint->variables[i]->value,
				  idg_new_interval_singleton
				  (constraint->coeffs[i])));
      else
	var_coeff = constraint->coeffs[i];
    }
  val = idg_add_intervals (val, idg_new_interval_singleton
			   (constraint->constant));
  val = idg_div_intervals (val, idg_new_interval_singleton (-var_coeff));
  if (constraint->kind == constraint_le)
    val->low_inf = 1;
  else if (constraint->kind == constraint_ge)
    val->high_inf = 1;
  res = idg_tighten (variable->value, val, variable);
  if (idg_intervals_equal_p (variable->value, res))
    return 0;
  else
    {
      variable->value = res;
      return 1;
    }
}

int
constraint_satisfied_p (idg_constraint c)
{
  int i;
  idg_interval val = idg_new_interval_singleton (0.0);
  for (i = 0; i < c->var_count; i++)
    {
      val = idg_add_intervals (val,
			       idg_mul_intervals
			       (c->variables[i]->value,
				idg_new_interval_singleton
				(c->coeffs[i])));
    }
  val = idg_add_intervals (val, idg_new_interval_singleton
			   (c->constant));
  if (idg_interval_empty_p (val))
    return 0;
  if (!val->low_inf && val->low > 0)
    return 0;
  if (!val->high_inf && val->high < 0)
    return 0;
  return 1;
}

void
check_constraint (idg_constraint constraint,
		  idg_constraint_list * active_constraints)
{
  if (constraint->var_count == 1)
    {
      if (constraint->strength == 0 &&
	  !constraint_satisfied_p (constraint))
	{
	  printf ("required constraint not satisfied\n");
	  idg_print_constraint (constraint);
	  abort ();
	}
      return;
    }
  {
    int i;
    for (i = 0; i < constraint->var_count; i++)
      {
	if (!idg_interval_singleton_p (constraint->variables[i]->value))
	  {
	    queue_add (active_constraints, constraint);
	    return;
	  }
      }
  }
  {
    int satisfied = 1;
    if (!constraint_satisfied_p (constraint))
      {
	satisfied = 0;
      }
    if (satisfied)
      queue_delete (active_constraints, constraint);
    else if (constraint->strength == 0)
      {
	  printf ("required constraint not satisfied\n");
	  idg_print_constraint (constraint);
	  abort ();
      }
    else
      {
	  printf ("constraints too difficult\n");
	  abort ();
      }
  }
}

void
tighten_bounds (idg_constraint cn,
		idg_constraint_list * queue,
		idg_variable_list * tight_variables,
		idg_constraint_list * active_constraints)
{
  int i;

  /* For each variable in the constraint, try to tighten its
     bounds.  */
  for (i = 0; i < cn->var_count; i++)
    {
      idg_variable v = cn->variables[i];

      /* Only variables which have not been processed for the current
	 root constraint are handled.  */
      if (!var_search (*tight_variables, v))
	{
	  /* Try to tighten the bounds of v, according to the current
	     constraint.  */
	  int tighten_flag = tighten_bounds_for (cn, v);
	  vqueue_add (tight_variables, v);
	  /* If any tightening was possible, add all constraints which
	     contain this variables to the queue for the current root
	     constraint.  */
	  if (tighten_flag)
	    {
	      idg_constraint_list cl = v->constraints;
	      while (cl)
		{
		  if (constraint_search (*active_constraints,
					 cl->constraint) &&
		      !constraint_search (*queue, cl->constraint))
		    queue_add (queue, cl->constraint);
		  cl = cl->next;
		}
	    }
	}
    }
}

/* Sort the constraint list `list' according to strength, so that
   stronger constraints come earlier in the list than weaker ones.  */
idg_constraint_list
sort_constraints (idg_constraint_list list)
{
  idg_constraint_list l = list;
  idg_constraint_list res = NULL, * resp = &res;
  int min_strength = 0;
  int count = 0;
  while (l)
    {
      count++;
      l = l->next;
    }
  while (count > 0)
    {
      l = list;
      while (l)
	{
	  if (l->constraint->strength == min_strength)
	    {
	      *resp = idg_constraint_cons (l->constraint, NULL);
	      resp = &((*resp)->next);
	    }
	  l = l->next;
	}
      min_strength++;
      count--;
    }
  return res;
}

void
idg_solve (idg_constraint_list all_constraints)
{
  idg_constraint_list cl;
  idg_constraint_list active_constraints = NULL;
  idg_constraint current_constraint;
  int count = 0;

  /* Sort the constraints, so that the strongest come first.  */
  all_constraints = sort_constraints (all_constraints);

  /* Initialize all variables to the interval of the reals.  */
  cl = all_constraints;
  while (cl)
    {
      int i;
      for (i = 0; i < cl->constraint->var_count; i++)
	{
	  cl->constraint->variables[i]->value = idg_new_interval_inf ();
	}
#if DEBUG
      idg_print_constraint (cl->constraint);
#endif
      cl = cl->next;
      count++;
    }

#if DEBUG
  fprintf (stderr, "%d constraints\n", count);
#endif

  /* Process all constraints, strongest to weakest.  */
  cl = all_constraints;
  while (cl)
    {
      idg_variable_list tight_variables = NULL;
      idg_constraint_list queue = NULL;

      current_constraint = cl->constraint;
      cl = cl->next;

      /* Add the current constraint to the queue of constraints, and
	 iterate until this constraint and all constraints triggered
	 by variable updates are processed.  */
      queue_add (&queue, current_constraint);
      while (queue)
	{
	  idg_constraint cn = queue->constraint;
	  tighten_bounds (cn, &queue, &tight_variables, &active_constraints);
	  check_constraint (cn, &active_constraints);
	  dequeue (&queue);
	}
    }
}

#if TEST_INDIGO

#define MAX_VARS 128
static int var_count;
static idg_variable var_tab[MAX_VARS];

idg_variable
lookup_var (char * c)
{
  int i = 0;
  while (i < var_count)
    {
      if (!strcmp (c, var_tab[i]->name))
	break;
      i++;
    }
  if (i < var_count)
    return var_tab[i];
  else
    {
      var_tab[var_count] = idg_new_variable (c);
      var_count++;
      return var_tab[var_count - 1];
    }
}

static int constr_count = 0;
static idg_constraint_list constraints = NULL;

void
add_constraint (double * coeffs, char ** names, int count, double rhs,
		int strength, int eq)
{
  char * buf = malloc (32 * sizeof (char));
  idg_variable * vars = malloc (count * sizeof (idg_variable));
  double * vals = malloc (count * sizeof (double));
  int i;
  enum constraint_kind eq_kind;
  idg_constraint constraint;

  sprintf (buf, "c%d", constr_count++);
  for (i = 0; i < count; i++)
    {
      vars[i] = lookup_var (names[i]);
      vals[i] = coeffs[i];
    }
  switch (eq)
    {
    case EQ_EQ:
      eq_kind = constraint_eq;
      break;
    case EQ_LE:
      eq_kind = constraint_le;
      break;
    case EQ_GE:
      eq_kind = constraint_ge;
      break;
    default:
      abort ();
    }
  constraint = idg_new_constraint (buf, strength, eq_kind, 
				   rhs, count, vars, vals);
  constraints = idg_constraint_cons (constraint, constraints);
}

int
main (int argc, char * argv[])
{
#if 1
  printf ("indigo " __turtle_version "\n");
  printf ("Copyright 2002 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>\n");
  printf ("indigo is free software, covered by the GNU General Public License, and you are\n");
  printf ("welcome to change it and/or distribute copies of it under certain conditions.\n");
  printf ("There is absolutely no warranty for indigo.\n");

#if 0
  printf ("indigo> ");
#endif
  yyparse ();
  solve (constraints);
  {
    int i;
    for (i = 0; i < var_count; i++)
      idg_print_variable (var_tab[i]);
  }
#else
  {
    idg_variable a = idg_new_variable ("A");
    idg_variable b = idg_new_variable ("B");
    idg_variable c = idg_new_variable ("C");
    idg_variable d = idg_new_variable ("D");
    idg_constraint c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    idg_variable c1vars[1], c2vars[1], c3vars[3], c4vars[2];
    idg_variable c5vars[1], c6vars[1], c7vars[1], c8vars[1];
    idg_variable c9vars[1], c10vars[1];
    double c1coeffs[1], c2coeffs[1], c3coeffs[3], c4coeffs[2];
    double c5coeffs[1], c6coeffs[1], c7coeffs[1], c8coeffs[1];
    double c9coeffs[1], c10coeffs[1];
    c1vars[0] = a; c1coeffs[0] = 1.0;

    c2vars[0] = b; c2coeffs[0] = 1.0;

    c3vars[0] = a; c3coeffs[0] = 1.0;
    c3vars[1] = b; c3coeffs[1] = 1.0;
    c3vars[2] = c; c3coeffs[2] = -1.0;

    c4vars[0] = c; c4coeffs[0] = 1.0;
    c4vars[1] = d; c4coeffs[1] = -1.0;

    c5vars[0] = d; c5coeffs[0] = 1.0;

    c6vars[0] = a; c6coeffs[0] = 1.0;

    c7vars[0] = a; c7coeffs[0] = 1.0;

    c8vars[0] = b; c8coeffs[0] = 1.0;

    c9vars[0] = c; c9coeffs[0] = 1.0;

    c10vars[0] = d; c10coeffs[0] = 1.0;

    c1 = idg_new_constraint ("c1", 0, constraint_ge, 10.0, 1, c1vars, c1coeffs);
    c2 = idg_new_constraint ("c2", 0, constraint_ge, 20.0, 1, c2vars, c2coeffs);
    c3 = idg_new_constraint ("c3", 0, constraint_eq, 0.0, 3, c3vars, c3coeffs);
    c4 = idg_new_constraint ("c4", 0, constraint_eq, -25.0, 2, c4vars, c4coeffs);
    c5 = idg_new_constraint ("c5", 1, constraint_le, 100.0, 1, c5vars, c5coeffs);
    c6 = idg_new_constraint ("c6", 2, constraint_eq, 50.0, 1, c6vars, c6coeffs);
    c7 = idg_new_constraint ("c7", 4, constraint_eq, 5.0, 1, c7vars, c7coeffs);
    c8 = idg_new_constraint ("c8", 4, constraint_eq, 5.0, 1, c8vars, c8coeffs);
    c9 = idg_new_constraint ("c9", 4, constraint_eq, 100.0, 1, c9vars, c9coeffs);
    c10 = idg_new_constraint ("c10", 4, constraint_eq, 200.0, 1, c10vars, c10coeffs);

    solve (idg_constraint_cons 
	   (c1, idg_constraint_cons
	    (c2, idg_constraint_cons
	     (c3, idg_constraint_cons
	      (c4, idg_constraint_cons
	       (c5, idg_constraint_cons
		(c6, idg_constraint_cons
		 (c7, idg_constraint_cons
		  (c8, idg_constraint_cons
		   (c9, idg_constraint_cons
		    (c10, NULL)))))))))));
    idg_print_variable (a);
    idg_print_variable (b);
    idg_print_variable (c);
    idg_print_variable (d);
  }
#endif
  return 0;
}
#endif

/* End of indigo.c.  */
