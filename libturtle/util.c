/* libturtle/util.c - Utility routines.
 
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

#include "util.h"

char *
ttl_basename (ttl_pool pool, char * filename)
{
  char * result;
  char * p = filename;
  char * endp;

/*   ttl_check_mem (filename, strlen (filename) + 1, __FILE__, __LINE__); */

  while (*p)
    p++;
  endp = p;
  while (p >= filename && *p != '/')
    p--;
  if (p < filename)
    return filename;
  else
    {
      result = ttl_malloc (pool, (endp - p) + 1);
      memcpy (result, p + 1, endp - p);
      result[endp - p] = '\0';
      ttl_check_mem (result, (endp - p) + 1, __FILE__, __LINE__);
      return result;
    }
}

char *
ttl_replace_file_ext (ttl_pool pool, char * filename, char * ext)
{
  char * path;
  char * p;

  /* Search back from the end to the last dot of slash.  */
  p = filename + strlen (filename);
  while (p >= filename && *p != '.' && *p != '/')
    p--;
  if (p >= filename && *p == '.')
    {
      path = ttl_malloc (pool, ((p - filename) + strlen (ext) + 1) *
			 sizeof (char));
      strncpy (path, filename, p - filename);
      path[p - filename] = '\0';
    }
  else
    {
      path = ttl_malloc (pool, (strlen (filename) + strlen (ext) + 1) *
			 sizeof (char));
      strcpy (path, filename);
    }
  strcat (path, ext);
  return path;
}


size_t
ttl_qualident_length (ttl_ast_node ident)
{
  if (ident->kind == ast_identifier)
    {
      size_t len = ident->d.identifier.symbol->length;
      if (len > 0 && (ident->d.identifier.symbol->text[len - 1] == '?' ||
		      ident->d.identifier.symbol->text[len - 1] == '!'))
	return len + 1;
      else
	return len;
    }
  else if (ident->kind == ast_annotated_identifier)
    return ttl_qualident_length (ident->d.annotated_identifier.identifier);
  else if (ident->kind == ast_qualident)
    return ttl_qualident_length (ident->d.qualident.module) + 1
      + ttl_qualident_length (ident->d.qualident.identifier);
  else
    abort ();
}


size_t
ttl_qualident_to_string (char * s, ttl_ast_node ident, char sep)
{
  if (ident->kind == ast_identifier)
    {
      size_t len = ident->d.identifier.symbol->length;

      memmove (s, ident->d.identifier.symbol->text, len);
      if (len > 0 && ident->d.identifier.symbol->text[len - 1] == '?')
	{
	  s[len - 1] = 'A';
	  s[len] = 'P';
	  len++;
	}
      else if (len > 0 && ident->d.identifier.symbol->text[len - 1] == '!')
	{
	  s[len - 1] = 'A';
	  s[len] = 'S';
	  len++;
	}
      s[len] = '\0';
      return len;
    }
  else if (ident->kind == ast_annotated_identifier)
    return ttl_qualident_to_string (s,
				    ident->d.annotated_identifier.identifier,
				    sep);
  else if (ident->kind == ast_qualident)
    {
      size_t len = ttl_qualident_to_string (s, ident->d.qualident.module, sep);
      s[len] = sep;
      len += 1 + ttl_qualident_to_string (s + len + 1,
					  ident->d.qualident.identifier, sep);
      return len;

    }
  else
    abort ();
}


char *
ttl_qualident_to_filename (ttl_pool pool, ttl_ast_node ident, char * suffix)
{
  char * s = ttl_malloc (pool, ttl_qualident_length (ident) + 1 +
			 strlen (suffix));
  ttl_qualident_to_string (s, ident, '/');
  strcat (s, suffix);

  return s;
}


char *
ttl_qualident_to_c_ident (ttl_pool pool, ttl_ast_node ident)
{
  char * s = ttl_malloc (pool, ttl_qualident_length (ident) + 1);
  ttl_qualident_to_string (s, ident, '_');

  return s;
}


/* Compute the length of a mangled identifier consisting of the module
   name `mod_name', the identifier `name' and the string
   representation of its data type `type'.  */
static size_t
mangled_name_length (ttl_ast_node mod_name, ttl_ast_node name, ttl_type type)
{
  return ttl_qualident_length (mod_name) + 1 +
    ttl_qualident_length (name) + 1 + ttl_type_string_length (type);
}


/* Put the mangled string representation of the identifier consisting
   of the module name `mod_name', the identifier `name' and the data
   type `type' into the string `s'.  `s' must be allocated before
   calling this function and large enough so that the string
   representation fits into the string.  The required size of `s' can
   be calculated using the function `mangled_name_length()' above.  */
static void
mangle_name_to_string (char * s, ttl_ast_node mod_name, ttl_ast_node name,
		       ttl_type type)
{
  ttl_qualident_to_string (s, mod_name, '_');
  strcat (s, "_");
  ttl_qualident_to_string (s + ttl_qualident_length (mod_name) + 1, name, '_');
  strcat (s, "_");
  ttl_type_to_string (s + 2 + ttl_qualident_length (mod_name)
		      + ttl_qualident_length (name), type);
}


ttl_symbol
ttl_mangle_name (ttl_compile_state state, ttl_ast_node mod_name,
		 ttl_ast_node name, ttl_type type)
{
  size_t len = mangled_name_length (mod_name, name, type);
  char * s = ttl_malloc (state->pool, (len + 1) * sizeof (char));
  mangle_name_to_string (s, mod_name, name, type);
  s[len] = '\0';
  return ttl_symbol_enter (state->symbol_table, s, len);
}


ttl_symbol
ttl_uniquify_name (ttl_compile_state state, ttl_ast_node name)
{
  static int i = 0;
  char buf[32];
  char * s = ttl_qualident_to_c_ident (state->pool, name);
  ttl_symbol sym = ttl_symbol_enter (state->symbol_table,
				     s, strlen (s));

  sprintf (buf, "_%d", i);
  i++;
  
  s = ttl_malloc (state->pool,
		  (strlen (buf) + sym->length + 1) * sizeof (char));
  memmove (s, sym->text, sym->length);
  s[sym->length] = '\0';
  strcat (s, buf);
  return ttl_symbol_enter (state->symbol_table, s, strlen (buf) + sym->length);
}


void
ttl_print_escaped_char (FILE * f, int c)
{
  if (c >= 32 && c < 128)
    {
      switch (c)
	{
	case '"':
	  fprintf (f, "\\\"");
	  break;
	case '\'':
	  fprintf (f, "\\\'");
	  break;
	case '\\':
	  fprintf (f, "\\\\");
	  break;
	default:
	  fputc (c, f);
	  break;
	}
    }
  else
    switch (c)
      {
      case '\n':
	fprintf (f, "\\n");
	break;
      case '\r':
	fprintf (f, "\\r");
	break;
      case '\t':
	fprintf (f, "\\t");
	break;
      case '\f':
	fprintf (f, "\\f");
	break;
      case '\b':
	fprintf (f, "\\b");
	break;
      case '\v':
	fprintf (f, "\\v");
	break;
      case '\a':
	fprintf (f, "\\a");
	break;
      default:
	fprintf (f, "\\%o", c);
      }
}

char *
ttl_find_file (ttl_pool pool, char * fname, char * search_path)
{
  FILE * f;
  char * sp;

  f = fopen (fname, "r");
  if (f)
    {
      fclose (f);
      return fname;
    }

  while (*search_path)
    {
      sp = search_path;
      while (*sp && *sp != ':')
	sp++;
      if (sp > search_path)
	{
	  char * name;

	  name = ttl_malloc (pool, (sp - search_path) + strlen (fname) + 2);
	  memmove (name, search_path, (sp - search_path));
	  name[sp - search_path] = '\0';
	  if (name[(sp - search_path) - 1] != '/')
	    strcat (name, "/");
	  strcat (name, fname);
	  f = fopen (name, "r");
	  if (f)
	    {
	      fclose (f);
	      return name;
	    }
	}
      if (*sp)
	search_path = sp + 1;
      else
	search_path = sp;
    }
  return NULL;
}


char *
ttl_string_append (ttl_pool pool, char * s1, char * s2)
{
  size_t len1 = strlen (s1);
  size_t len2 = strlen (s2);
  char * res = ttl_malloc (pool, len1 + len2 + 1);
  strcpy (res, s1);
  strcat (res, s2);
  return res;
}

/* End of util.c.  */
