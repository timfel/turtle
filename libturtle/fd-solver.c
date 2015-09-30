/* libturtle/fd-solver.c -- Simple finite-domain constraint solver.
 
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
#include <limits.h>

#include "fd-solver.h"

#define DEBUG 0
#define DEBUG_PRINT 0

fd_variable
fd_new_variable (char * name, void * store)
{
  fd_variable fd = malloc (sizeof (struct fd_variable));
  fd->variable = store;
  fd->name = name;
  fd->min_value = INT_MIN;
  fd->max_value = INT_MAX;
  fd->mask = NULL;
  fd->value = 0;
  fd->next = NULL;
  fd->mark = 0;
  return fd;
}

fd_constraint_list
fd_constraint_cons (fd_constraint constraint, fd_constraint_list tail)
{
  fd_constraint_list l = malloc (sizeof (struct fd_constraint_list));
  l->next = tail;
  l->constraint = constraint;
  return l;
}

fd_constraint
fd_new_constraint (char * name, int strength, enum constraint_kind kind,
		   int constant, int var_count, fd_variable * variables,
		   int * coeffs)
{
  int i;
  fd_constraint c = malloc (sizeof (struct fd_constraint));
  c->name = name;
  c->strength = strength;
  c->kind = kind;
  c->constant = -constant;
  c->var_count = var_count;
  c->variables = variables;
  c->coeffs = coeffs;
  return c;
}

void
fd_print_constraint (fd_constraint c)
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
      ">=",
      "<",
      ">",
      "<>"
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
	    printf (" + %d * ", c->coeffs[i]);
	  else
	    printf ("%d * ", c->coeffs[i]);
	}
      else if (c->coeffs[i] < 0)
	{
	  if (i > 0)
	    printf (" - %d *", -c->coeffs[i]);
	  else
	    printf ("%d *", c->coeffs[i]);
	}
      printf ("%s", c->variables[i]->name);
    }
  if (c->constant == 0.0)
    ;
  else if (c->constant < 0.0)
    printf (" - %d", -c->constant);
  else
    printf (" + %d", c->constant);
  printf (" %s 0\n", op_names[(int) c->kind]);
  
}

static int
check_constraint (fd_constraint c)
{
  char * op_names[] =
    {
      "=",
      "<=",
      ">=",
      "<",
      ">",
      "<>"
    };
  int res = 0;
  int i;
  int sum = c->constant;
  for (i = 0; i < c->var_count; i++)
    sum += c->variables[i]->value * c->coeffs[i];
#if DEBUG_PRINT
  fprintf (stdout, "%d %s %d => ", sum, op_names[c->kind], 0);
#endif
  switch (c->kind)
    {
    case constraint_eq:
      res = (sum == 0);
      break;
    case constraint_le:
      res = (sum <= 0);
      break;
    case constraint_ge:
      res = (sum >= 0);
      break;
    case constraint_lt:
      res = (sum < 0);
      break;
    case constraint_gt:
      res = (sum > 0);
      break;
    case constraint_ne:
      res =  (sum != 0);
      break;
    }
#if DEBUG_PRINT
  fprintf (stdout, "%d\n", res);
#endif
  return res;
}

static int
check_constraints (fd_constraint_list all_constraints)
{
  fd_constraint_list l = all_constraints;
  while (l)
    {
      fd_constraint c = l->constraint;
      if (!check_constraint (c))
	return 0;
      l = l->next;
    }
  return 1;
}

static int
try (fd_variable var, fd_variable rest, fd_variable all_vars,
     fd_constraint_list all_constraints)
{
  int ok;
  int save;
  fd_variable l;
  int i = var->min_value;
  while (i <= var->max_value)
    {
      fd_variable v = all_vars;
      save = var->value;
      var->value = i;
      if (rest)
	{
	  ok = try (rest, rest->next, all_vars, all_constraints);
	  if (ok)
	    return 1;
	}
      else
	{
#if DEBUG_PRINT
	  while (v)
	    {
	      fprintf (stdout, " %s = %d", v->name, v->value);
	      v = v->next;
	    }
	  fprintf (stdout, "\n");
#endif
	  if (check_constraints (all_constraints))
	    {
#if DEBUG_PRINT
	      fprintf (stdout, ">>> success\n");
#endif
	      return 1;
	    }
	}
      /*       var->value = save; */
      i++;
    }
  return 0;
}

void
fd_solve (fd_constraint_list all_constraints)
{
  fd_constraint_list l = all_constraints;
  fd_variable vl = NULL;
  fd_variable vll;
  static int i = 0;
  while (l)
    {
      fd_constraint c = l->constraint;
      int i;
/*       fd_print_constraint (l->constraint); */
      for (i = 0; i < c->var_count; i++)
	{
#define FASTER 1
#if FASTER
	  c->variables[i]->min_value = -1000000;
	  c->variables[i]->max_value =  1000000;
#else
	  c->variables[i]->min_value = INT_MIN;
	  c->variables[i]->max_value = INT_MAX;
#endif
	  if (!c->variables[i]->mark)
	    {
	      c->variables[i]->next = vl;
	      vl = c->variables[i];
	      c->variables[i]->mark = 1;
	    }
	}
      l = l->next;
    }
  l = all_constraints;
  while (l)
    {
      fd_constraint c = l->constraint;
      int i;
      if (c->var_count == 1)
	{
	  fd_variable v = c->variables[0];
	  switch (c->kind)
	    {
	    case constraint_eq:
	      v->min_value = -c->constant;
	      v->max_value = -c->constant;
	      break;
	    case constraint_le:
	      v->max_value = -c->constant;
	      break;
	    case constraint_ge:
	      v->min_value = -c->constant;
	      break;
	    case constraint_lt:
	      v->max_value = -c->constant + 1;
	      break;
	    case constraint_gt:
	      v->min_value = -c->constant + 1;
	      break;
	    case constraint_ne:
	      break;
	    }
	}
      l = l->next;
    }

  try (vl, vl->next, vl, all_constraints);

  vll = vl;
  while (vll)
    {
      fd_variable next = vll->next;
      vll->mark = 0;
      vll->next = NULL;
/*       vll->value = (vll->min_value + vll->max_value) / 2; */
#if DEBUG_PRINT
      fprintf (stdout, "name: %s, min: %d, max: %d, value: %d\n",
	       vll->name, vll->min_value, vll->max_value, vll->value);
#endif
      vll = next;
    }
#if DEBUG_PRINT
  printf ("\n");
#endif
}

#if DEBUG

int
main (int argc, char * argv[])
{
  return 0;
}

#endif

/* End of fd-solver.c.  */
