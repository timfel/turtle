/* libturtle/indigo.h -- Indigo constraint solver.
 
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

#ifndef TTL_INDIGO_H
#define TTL_INDIGO_H

#include <stdlib.h>

typedef struct idg_interval * idg_interval;
struct idg_interval
{
  double low;
  double high;
  short low_inf;
  short high_inf;
};

idg_interval
idg_new_interval_full (double low, double high, int low_inf, int high_inf);
idg_interval
idg_new_interval (double low, double high);
idg_interval
idg_new_interval_low (double low);
idg_interval
idg_new_interval_high (double high);

void idg_print (FILE * f, idg_interval i);

typedef struct idg_constraint_list * idg_constraint_list;

typedef struct idg_variable * idg_variable;
struct idg_variable
{
  void * variable;		/* Pointer to run-time variable.  */
  char * name;			/* This variable's name.  */
  idg_interval value;
  idg_constraint_list constraints;
};

typedef struct idg_variable_list * idg_variable_list;
struct idg_variable_list
{
  idg_variable_list next;
  idg_variable variable;
};

enum constraint_kind
  {
    constraint_eq,
    constraint_le,
    constraint_ge,
    constraint_lt,
    constraint_gt,
    constraint_ne
  };

typedef struct idg_constraint * idg_constraint;
struct idg_constraint
{
  char * name;
  int strength;
  enum constraint_kind kind;
  double constant;
  int var_count;
  idg_variable * variables;
  double * coeffs;
};

struct idg_constraint_list
{
  idg_constraint_list next;
  idg_constraint constraint;
};

idg_constraint_list idg_constraint_cons (idg_constraint constraint,
					 idg_constraint_list tail);
idg_variable_list idg_variable_cons (idg_variable variable,
				     idg_variable_list tail);
idg_constraint idg_new_constraint (char * name,
				   int strength, enum constraint_kind kind,
				   double constant,
				   int var_count,
				   idg_variable * variables,
				   double * coeffs);
idg_variable idg_new_variable (char * name, void * store);


#define EQ_EQ 1
#define EQ_LE 2
#define EQ_GE 3
#define EQ_LT 4
#define EQ_GT 5
#define EQ_NE 6

void add_constraint (double * coeffs, char ** names, int count, double rhs,
		     int strength, int eq);

void idg_solve (idg_constraint_list all_constraints);

#endif /* not TTL_INDIGO_H */
