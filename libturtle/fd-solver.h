/* libturtle/fd-solver.h -- Simple finite-domain constraint solver.
 
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

#ifndef TTL_FD_SOLVER_H
#define TTL_FD_SOLVER_H

#include <stdlib.h>

#include "indigo.h"

typedef struct fd_variable * fd_variable;
struct fd_variable
{
  void * variable;		/* Pointer to run-time variable.  */
  char * name;			/* This variable's name.  */
  int min_value;
  int max_value;
  int * mask;
  int value;
  fd_variable next;
  int mark;
};


typedef struct fd_constraint_list * fd_constraint_list;

typedef struct fd_constraint * fd_constraint;
struct fd_constraint
{
  char * name;
  int strength;
  enum constraint_kind kind;
  int constant;
  int var_count;
  fd_variable * variables;
  int * coeffs;
};

struct fd_constraint_list
{
  fd_constraint_list next;
  fd_constraint constraint;
};

fd_constraint_list fd_constraint_cons (fd_constraint constraint,
				       fd_constraint_list tail);

fd_variable fd_new_variable (char * name, void * store);
fd_constraint fd_new_constraint (char * name, int strength,
				 enum constraint_kind kind,
				 int constant,
				 int var_count,
				 fd_variable * variables,
				 int * coeffs);

void fd_solve (fd_constraint_list all_constraints);

void fd_print_constraint (fd_constraint c);

#endif /* not TTL_FD_SOLVER_H */
