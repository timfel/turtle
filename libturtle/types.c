/* libturtle/types.c - data type representations
 
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


#include <string.h>

#include "types.h"

int ttl_permissive_checking = 0;

ttl_type
ttl_make_type (ttl_pool pool, enum ttl_type_kind kind)
{
  ttl_type type = ttl_malloc (pool, sizeof (struct ttl_type));
  type->kind = kind;
  type->name = NULL;
  type->exported = 0;
  type->next = NULL;
  return type;
}

ttl_type
ttl_make_array_type (ttl_pool pool, ttl_type element)
{
  ttl_type type = ttl_make_type (pool, type_array);
  type->d.array.element = element;
  return type;
}

ttl_type
ttl_make_list_type (ttl_pool pool, ttl_type element)
{
  ttl_type type = ttl_make_type (pool, type_list);
  type->d.list.element = element;
  return type;
}

ttl_type
ttl_make_tuple_type (ttl_pool pool, size_t elem_type_count,
		     ttl_type * elem_types)
{
  ttl_type type = ttl_make_type (pool, type_tuple);
  type->d.tuple.elem_type_count = elem_type_count;
  type->d.tuple.elem_types = elem_types;
  return type;
}

ttl_type
ttl_make_function_type (ttl_pool pool, size_t param_type_count,
			ttl_type * param_types,
			ttl_type return_type)
{
  ttl_type type = ttl_make_type (pool, type_function);
  type->d.function.param_type_count = param_type_count;
  type->d.function.param_types = param_types;
  type->d.function.return_type = return_type;
  return type;
}

ttl_type
ttl_make_sum_type (ttl_pool pool, ttl_symbol name, ttl_ast_node full_name,
		   size_t type_count, ttl_type * types)
{
  ttl_type type = ttl_make_type (pool, type_sum);
  type->d.sum.name = name;
  type->d.sum.full_name = full_name;
  type->d.sum.type_count = type_count;
  type->d.sum.types = types;
  return type;
}

ttl_field
ttl_make_record_field (ttl_pool pool, ttl_symbol name, ttl_type type)
{
  ttl_field field = ttl_malloc (pool, sizeof (struct ttl_field));
  field->name = name;
  field->type = type;
  return field;
}

ttl_variant
ttl_make_record_variant (ttl_pool pool, ttl_symbol name, size_t field_count,
			 ttl_field * fields)
{
  ttl_variant variant = ttl_malloc (pool, sizeof (struct ttl_variant));
  variant->name = name;
  variant->field_count = field_count;
  variant->fields = fields;
  return variant;
}

ttl_type
ttl_make_record_type (ttl_pool pool, size_t variant_count,
		      ttl_variant * variants)
{
  ttl_type type = ttl_make_type (pool, type_record);
  type->d.record.variant_count = variant_count;
  type->d.record.variants = variants;
  return type;
}

ttl_type
ttl_make_alias_type (ttl_pool pool, ttl_symbol name, ttl_type base_type)
{
  ttl_type type = ttl_make_type (pool, type_alias);
  type->d.alias.name = name;
  type->d.alias.type = base_type;
  return type;
}

#if 0
ttl_type
ttl_make_variable_type (ttl_pool pool, ttl_symbol name)
{
  ttl_type type = ttl_make_type (pool, type_variable);
  type->d.variable.name = name;
  return type;
}
#endif

ttl_type
ttl_make_constrained_type (ttl_pool pool, ttl_type base)
{
  ttl_type type = ttl_make_type (pool, type_constrained);
  type->d.constrained.base = base;
  return type;
}

ttl_type
ttl_make_basic_type (ttl_pool pool, enum ttl_type_kind kind)
{
  return ttl_make_type (pool, kind);
}

void
ttl_print_type (FILE * f, ttl_type type)
{
  if (!type)
    return;
/*   fprintf (f, "%x:", (unsigned)type); */
  switch (type->kind)
    {
    case type_error:
      fprintf (f, "<!ERROR!>");
      break;
    case type_void:
      fprintf (f, "()");
      break;
    case type_integer:
      fprintf (f, "int");
      break;
    case type_long:
      fprintf (f, "long");
      break;
    case type_real:
      fprintf (f, "real");
      break;
    case type_bool:
      fprintf (f, "bool");
      break;
    case type_char:
      fprintf (f, "char");
      break;
    case type_string:
      fprintf (f, "string");
      break;
    case type_tuple:
      if (type->d.tuple.elem_type_count == 1)
	ttl_print_type (f, type->d.tuple.elem_types[0]);
      else
	{
	  fprintf (f, "(");
	  if (type->d.tuple.elem_type_count > 1)
	    {
	      unsigned i;

	      ttl_print_type (f, type->d.tuple.elem_types[0]);
	      for (i = 1; i < type->d.tuple.elem_type_count; i++)
		{
		  fprintf (f, ", ");
		  ttl_print_type (f, type->d.tuple.elem_types[i]);
		}
	    }
	  fprintf (f, ")");
	}
      break;
    case type_list:
      fprintf (f, "list of ");
      ttl_print_type (f, type->d.list.element);
      break;
    case type_array:
      fprintf (f, "array of ");
      ttl_print_type (f, type->d.array.element);
      break;
    case type_function:
      {
	unsigned i;

	fprintf (f, "fun(");
	for (i = 0; i < type->d.function.param_type_count; i++)
	  {
	    ttl_print_type (f, type->d.function.param_types[i]);
	    if (i < type->d.function.param_type_count - 1)
	      fprintf (f, ", ");
	  }
	fprintf (f, "): ");
	ttl_print_type (f, type->d.function.return_type);
      }
      break;
    case type_constrained:
      fprintf (f, "! ");
      ttl_print_type (f, type->d.constrained.base);
      break;
    case type_sum:
      ttl_ast_print (f, type->d.sum.full_name, 0);
      if (type->d.sum.type_count > 0)
	{
	  size_t i;

	  fprintf (f, "<");
	  ttl_print_type (f, type->d.sum.types[0]);
	  for (i = 1; i < type->d.sum.type_count; i++)
	    {
	      fprintf (f, ", ");
	      ttl_print_type (f, type->d.sum.types[i]);
	    }
	  fprintf (f, ">");
	}
/*       ttl_symbol_print (f, type->d.sum.name); */
      break;
    case type_record:
      {
	size_t vcount;

	for (vcount = 0; vcount < type->d.record.variant_count; vcount++)
	  {
	    size_t fcount;

	    ttl_symbol_print (f, type->d.record.variants[vcount]->name);
	    fprintf (f, "(");
	    for (fcount = 0;
		 fcount < type->d.record.variants[vcount]->field_count;
		 fcount++)
	      {
		ttl_symbol_print
		  (f,
		   type->d.record.variants[vcount]->fields[fcount]->name);
		fprintf (f, ": ");
		ttl_print_type
		  (f,
		   type->d.record.variants[vcount]->fields[fcount]->type);
	      }
	    fprintf (f, ")");
	    if (vcount < type->d.record.variant_count - 1)
	      fprintf (f, " or ");
	  }
      }
      break;
    case type_alias:
      if (type->d.alias.type)
	ttl_print_type (f, type->d.alias.type);
      else
	ttl_symbol_print (f, type->d.alias.name);
      break;
#if 0
    case type_variable:
      ttl_symbol_print (f, type->d.variable.name);
      break;
#endif
    case type_any:
      fprintf (f, "any");
      break;
    case type_nil:
      fprintf (f, "nil");
      break;
    }
}

void
ttl_print_c_type (FILE * f, ttl_type type)
{
  unsigned i;

/*   fprintf (f, "%x:", (unsigned)type); */
  switch (type->kind)
    {
    case type_error:
      fprintf (f, "<!ERROR!>\n");
      abort ();
      break;
    case type_void:
      fprintf (f, "void");
      break;
    case type_integer:
      fprintf (f, "int");
      break;
    case type_long:
      fprintf (f, "long long");
      break;
    case type_real:
      fprintf (f, "double");
      break;
    case type_bool:
      fprintf (f, "int");
      break;
    case type_char:
      fprintf (f, "short");
      break;
    case type_alias:
      ttl_symbol_print (f, type->d.alias.name);
      break;
#if 0
    case type_variable:
      ttl_symbol_print (f, type->d.variable.name);
      break;
#endif
    default:
      fprintf (f, "ttl_value");
      break;
    }
}

/* Return zero if the given types are not compatible, non-zero
   otherwise.  */
int
ttl_types_equal (ttl_type type0, ttl_type type1)
{
  if (type0->kind == type_error || type1->kind == type_error)
    return 1;
  else if (type0 == type1)
    return 1;
  else
    {
      if (type0->kind == type_alias)
	{
	  if (type0->d.alias.type)
	    return ttl_types_equal (type1, type0->d.alias.type);
	  else
	    return 0;
	}
      else if (type1->kind == type_alias)
	{
	  if (type1->d.alias.type)
	    return ttl_types_equal (type0, type1->d.alias.type);
	  else
	    return 0;
	}
      else if (type0->kind == type_any)
	{
	  return type1->kind != type_void;
	}
      else if (type1->kind == type_any)
	{
	  return type0->kind != type_void;
	}
      else if (type0->kind == type_void)
	{
	  return type1->kind == type_void;
	}
      else if (type0->kind == type_tuple)
	{
	  if (type1->kind == type_tuple)
	    {
	      if (type0->d.tuple.elem_type_count !=
		  type1->d.tuple.elem_type_count)
		return 0;
	      else
		{
		  unsigned i = 0;

		  for (i = 0; i < type0->d.tuple.elem_type_count; i++)
		    {
		      if (!ttl_types_equal (type0->d.tuple.elem_types[i],
					    type1->d.tuple.elem_types[i]))
			return 0;
		    }
		  return 1;
		}
	    }
	  else
	    return 0;
	}
      else if (type0->kind == type_nil)
	{
	  if (type1->kind == type_nil || type1->kind == type_list ||
	      type1->kind == type_string || type1->kind == type_sum)
	    return 1;
	  else
	    return 0;
	}
      else if (type0->kind == type_list)
	{
	  if (type1->kind == type_nil)
	    return 1;
	  else if (type1->kind == type_list)
	    return ttl_types_equal (type0->d.list.element,
				    type1->d.list.element);
	  else
	    return 0;
	}
      else if (type0->kind == type_array)
	{
	  if (type1->kind == type_array)
	    return ttl_types_equal (type0->d.array.element,
				    type1->d.array.element);
	  else
	    return 0;
	}
      else if (type0->kind == type_constrained)
	{
	  if (ttl_permissive_checking)
	    {
	  if (type1->kind == type_constrained)
	    return ttl_types_equal (type0->d.constrained.base,
				    type1->d.constrained.base);
	  else
	    return ttl_types_equal (type0->d.constrained.base,
				    type1);
	    }
	  else
	    {
	  if (type1->kind == type_constrained)
	    return ttl_types_equal (type0->d.constrained.base,
				    type1->d.constrained.base);
	  else
	    return 0;
	    }
	}
      else if (type1->kind == type_constrained &&
	       ttl_permissive_checking)
	{
	  return ttl_types_equal (type0, type1->d.constrained.base);
	}
      else if (type0->kind == type_sum)
	{
	  if (type1->kind == type_sum)
	    {
	      if (type0->d.sum.type_count == type1->d.sum.type_count &&
		  ttl_compare_identifiers (type0->d.sum.full_name,
					   type1->d.sum.full_name))
		{
		  size_t i = 0;
		  for (i = 0; i < type0->d.sum.type_count; i++)
		    {
		      if (!ttl_types_equal (type0->d.sum.types[i],
					    type1->d.sum.types[i]))
			return 0;
		    }
		  return 1;
		}
	      else
		return 0;
	    }
	  else if (type1->kind == type_nil)
	    {
	      return 1;
	    }
	  else
	    return 0;
	}
      else if (type0->kind == type_string)
	{
	  if (type1->kind == type_string)
	    return 1;
	  else if (type1->kind == type_nil)
	    return 1;
	  else
	    return 0;
	}
      else if (type0->kind == type_function)
	{
	  if (type1->kind == type_function)
	    {
	      if (type0->d.function.param_type_count ==
		  type1->d.function.param_type_count)
		{
		  unsigned i = 0;

		  for (i = 0; i < type0->d.function.param_type_count; i++)
		    {
		      if (!ttl_types_equal (type0->d.function.param_types[i],
					    type1->d.function.param_types[i]))
			return 0;
		    }
		  return ttl_types_equal (type0->d.function.return_type,
					  type1->d.function.return_type);
		}
	      else
		return 0;
	    }
	  else
	    return 0;
	}
      else
	return 0;
    }
}

ttl_type
ttl_constrained_type_base (ttl_type type)
{
  switch (type->kind)
    {
    case type_constrained:
      return type->d.constrained.base;
    case type_array:
      return ttl_constrained_type_base (type->d.array.element);
    case type_list:
      return ttl_constrained_type_base (type->d.list.element);
    default:
      return type;
    }
}

int
ttl_types_assignable (ttl_type type0, ttl_type type1)
{
  if (ttl_types_equal (type0, type1))
    return 1;
  else if (type0->kind == type_constrained)
    {
      return ttl_types_equal (type0->d.constrained.base, type1);
    }
  else if (type1->kind == type_constrained)
    {
      return ttl_types_equal (type0, type1->d.constrained.base);
    }
  else
    return 0;
}

/* Return the number of character the string representation of the
   type `type' will require.  */
size_t
ttl_type_string_length (ttl_type type)
{
  switch (type->kind)
    {
    case type_error:
      return 9;
    case type_integer:
      return 2;
    case type_bool:
      return 2;
    case type_char:
      return 2;
    case type_string:
      return 2;
    case type_real:
      return 2;
    case type_long:
      return 2;
    case type_any:
      return 2;
    case type_void:
      return 2;
    case type_list:
      return 2 + ttl_type_string_length (type->d.list.element);
    case type_array:
      return 2 + ttl_type_string_length (type->d.array.element);
    case type_tuple:
      {
	size_t len = 2;
	unsigned i = 0;
	char buf[32];

	sprintf (buf, "%d", type->d.tuple.elem_type_count);
	len += strlen (buf);
	if (type->d.tuple.elem_type_count > 1)
	  {
	    while (i < type->d.tuple.elem_type_count)
	      {
		len += ttl_type_string_length (type->d.tuple.elem_types[i]);
		i++;
	      }
	  }
	return len;
      }
    case type_function:
      {
	size_t len = 3;
	unsigned i;
	char buf[32];

	sprintf (buf, "%d", type->d.function.param_type_count);
	len += strlen (buf);
	for (i = 0; i < type->d.function.param_type_count; i++)
	  {
	    len += ttl_type_string_length (type->d.function.param_types[i]);
	  }
	len += ttl_type_string_length (type->d.function.return_type);
	return len;
      }
    case type_constrained:
      return 2 + ttl_type_string_length (type->d.constrained.base);
    case type_alias:
      if (type->d.alias.type)
	return ttl_type_string_length (type->d.alias.type);
      else
	return type->d.alias.name->length;
#if 0
    case type_variable:
      return type->d.variable.name->length;
#endif
    case type_sum:
      return 1 + type->d.sum.name->length;
    case type_record:
      {
	size_t len = 2;
	char buf[32];
	size_t vcount;

	len += sprintf (buf, "%d", type->d.record.variant_count);
	for (vcount = 0; vcount < type->d.record.variant_count; vcount++)
	  {
	    size_t fcount;

	    len +=  type->d.record.variants[vcount]->name->length;
	    len += sprintf (buf, "%d",
			    type->d.record.variants[vcount]->field_count);
	    for (fcount = 0;
		 fcount < type->d.record.variants[vcount]->field_count;
		 fcount++)
	      {
		len += type->d.record.variants[vcount]->fields[fcount]->name->length;
		len += ttl_type_string_length
		  (type->d.record.variants[vcount]->fields[fcount]->type);
	      }
	  }
	return len;
      }
    case type_nil:
      abort ();
    }
  abort ();
  return 0;
}


void
ttl_type_to_string (char * s, ttl_type type)
{
  switch (type->kind)
    {
    case type_error:
      strcpy (s, "<!ERROR!>");
      break;
    case type_integer:
      strcpy (s, "pI");
      break;
    case type_bool:
      strcpy (s, "pB");
      break;
    case type_char:
      strcpy (s, "pC");
      break;
    case type_string:
      strcpy (s, "pS");
      break;
    case type_long:
      strcpy (s, "pL");
      break;
    case type_real:
      strcpy (s, "pR");
      break;
    case type_any:
      strcpy (s, "pY");
      break;
    case type_void:
      strcpy (s, "pV");
      break;
    case type_list:
      strcpy (s, "pL");
      ttl_type_to_string (s + 2, type->d.list.element);
      break;
    case type_array:
      strcpy (s, "pA");
      ttl_type_to_string (s + 2, type->d.array.element);
      break;
    case type_tuple:
      if (type->d.tuple.elem_type_count == 1)
	ttl_type_to_string (s, type->d.tuple.elem_types[0]);
      else
	{
	  unsigned i = 0;
	  size_t ofs = 0;
	  char buf[32];

	  sprintf (buf, "pT%d", type->d.function.param_type_count);
	  strcpy (s, buf);
	  ofs += strlen (buf);
	  if (type->d.tuple.elem_type_count > 1)
	    {
	      while (i < type->d.tuple.elem_type_count)
		{
		  ttl_type_to_string (s + ofs, type->d.tuple.elem_types[i]);
		  ofs += ttl_type_string_length (type->d.tuple.elem_types[i]);
		  i++;
		}
	    }
	}
      break;
    case type_function:
      {
	unsigned i;
	size_t ofs = 0;
	char buf[32];

	sprintf (buf, "pF%d", type->d.function.param_type_count);
	strcpy (s, buf);
	ofs += strlen (buf);
	for (i = 0; i < type->d.function.param_type_count; i++)
	  {
	    ttl_type_to_string (s + ofs, type->d.function.param_types[i]);
	    ofs += ttl_type_string_length (type->d.function.param_types[i]);
	  }
	strcpy (s + ofs, "_");
	ofs += 1;
	ttl_type_to_string (s + ofs, type->d.function.return_type);
      }
      break;
    case type_constrained:
      strcpy (s, "pX");
      ttl_type_to_string (s + 2, type->d.constrained.base);
      break;
    case type_alias:
      if (type->d.alias.type)
	ttl_type_to_string (s, type->d.alias.type);
      else
	{
	  memmove (s, type->d.alias.name->text, type->d.alias.name->length);
	  s[type->d.alias.name->length] = '\0';
	}
      break;
#if 0
    case type_variable:
      memmove (s, type->d.variable.name->text, type->d.variable.name->length);
      s[type->d.variable.name->length] = '\0';
      break;
#endif
    case type_sum:
      s[0] = 'u';
      memmove (s + 1, type->d.sum.name->text, type->d.sum.name->length);
      s[type->d.sum.name->length + 1] = '\0';
      break;
    case type_record:
      {
	size_t vcount;

	sprintf (s, "pR%d", type->d.record.variant_count);
	s += strlen (s);
	for (vcount = 0; vcount < type->d.record.variant_count; vcount++)
	  {
	    size_t fcount;

	    memmove (s, type->d.record.variants[vcount]->name->text,
		     type->d.record.variants[vcount]->name->length);
	    s[type->d.record.variants[vcount]->name->length] = '\0';
	    s += strlen (s);
	    sprintf (s, "%d", type->d.record.variants[vcount]->field_count);
	    s += strlen (s);
	    for (fcount = 0;
		 fcount < type->d.record.variants[vcount]->field_count;
		 fcount++)
	      {
		memmove
		  (s,
		   type->d.record.variants[vcount]->fields[fcount]->name->text,
		   type->d.record.variants[vcount]->fields[fcount]->name->length);
		s[type->d.record.variants[vcount]->fields[fcount]->name->length]
		  = '\0';
		s += strlen (s);
		ttl_type_to_string
		  (s,
		   type->d.record.variants[vcount]->fields[fcount]->type);
		s += strlen (s);
	      }
	  }
      }
      break;
    case type_nil:
      abort ();
      break;
    }
}


/* End of types.c.  */
