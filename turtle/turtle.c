/* turtle/turtle.c - Turtle compiler.
 
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

#if HAVE_CONFIG_H
# include <config.h>
#endif 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if HAVE_GETOPT_H
# include <getopt.h>
#endif

#include <version.h>

#include <libturtle/libturtle.h>


static char program_name[] = "turtle";

static char version_string[] = __turtle_version;

static char system_init_file_name[] = "init";
static char user_init_file_name[] = ".turtle";

static char * program_path = NULL;


/* Defining here the struct and #define's for getopt_long() if it is
   in libiberty.a but could not be found in getopt.h

   [This replacement for getopt_long() is due to Stefan Jahn
    <ela@lkcc.org>] 
 */
#if defined(HAVE_GETOPT_LONG) && !defined(DECLARED_GETOPT_LONG)

struct option
{
  const char *name;
  int has_arg;
  int *flag;
  int val;
};

#define no_argument       0
#define required_argument 1
#define optional_argument 2

extern int getopt_long(int argc, 
		       char * const argv[], 
		       const char *optstring,
		       const struct option *longopts, 
		       int *longindex);

extern int optind;
extern char * optarg;
#endif /* DECLARED_GETOPT_LONG */

#ifndef HAVE_GETOPT
char *optarg = NULL;
int optind = 1;
int opterr = 0;
int optopt = 0;

int 
getopt (int argc, char * const argv[], const char *optstring) 
{
  static int current_arg = 1, current_opt = 0;
  int n;

  while (current_arg < argc) 
    {
      if (current_arg != 0 || argv[current_arg][0] == '-') 
        {
          if (current_opt == 0)
            current_opt = 1;
          while (argv[current_arg][current_opt] != '\0')
	    {
	      n = 0;
	      /* go through all option characters */
	      while (optstring[n]) 
		{
		  if (optstring[n] == argv[current_arg][current_opt])
		    {
		      current_opt++;
		      if (optstring[n + 1] == ':') 
			optarg = argv[current_arg + 1];
		      else
			optarg = NULL;
		      if (argv[current_arg][current_opt] == '\0')
			{ 
			  current_arg++;
			  current_opt = 0;
			} 
		      optind = current_arg;
		      return optstring[n];
		    }
		  n++;
		}
            }
	  current_opt++;
	}
      current_arg++;
    }

  current_arg = 1;
  current_opt = 0;
  return EOF;
}
#endif /* not HAVE_GETOPT */


/* Print a usage help message to standard output.  */
static void 
usage (void)
{
#if HAVE_GETOPT_LONG
  printf ("usage: %s [OPTION...] FILENAME...\n\
  -h, --help                 display this help and exit\n\
  -v, --version              display version information and exit\n\
  -V, --verbose              increase verbosity level\n\
  -m, --main=NAME            compile a main program and output as NAME\n\
  -p, --module-path=PATH     set the search path for modules\n\
  -I, --include-path=PATH    set the search path for runtime header files\n\
  -z, --pragma=PRAGMA        set compilation pragma\n\
    where PRAGMA is one of\n\
      handcoded              handcoded module\n\
      foreign                foreign expressions\n\
      compile-only           do not run the C compiler\n\
      static                 produce a static executable\n\
      turtledoc              generate Texinfo documentation from comments\n\
      deps                   write dependency information to .P file\n\
      deps-stdout            write dependency information to standard output\n\
  -O, --optimize=FLAGS       set optimization flags\n\
    where FLAGS is one or more of\n\
      C                      optimize module-local calls\n\
      c                      do not optimize module-local calls\n\
      J                      convert module-local calls to jumps\n\
      j                      do not convert module-local calls to jumps\n\
      G                      optimize GC checks over basic blocks\n\
      g                      do not optimize GC checks over basic blocks\n\
      D                      inline data constructors etc.\n\
      d                      do not inline data constructors etc.\n\
      0-6                    set optimization level for C compiler\n\
  -d, --debug=MODIFIER       set debugging options\n\
    where MODIFIER is one or more of\n\
      a                      print AST after parsing\n\
      e                      print top-level environment of module\n\
      i                      dump the HIL representation to a .il file\n\
      b                      print bytecode after code generation\n\
Report bugs to mgrabmue@cs.tu-berlin.de\n",
	  program_name);
#else /* !HAVE_GETOPT_LONG */
  printf ("usage: %s [OPTION...] FILENAME...\n\
  -h                 display this help and exit\n\
  -v                 display version information and exit\n\
  -V                 increase verbosity level\n\
  -m NAME            compile a main program and output as NAME\n\
  -p PATH            set the search path for modules\n\
  -I PATH            set the search path for runtime header files\n\
  -z PRAGMA          set compilation pragma\n\
    where PRAGMA is one of\n\
      handcoded      handcoded module\n\
      foreign        foreign expressions\n\
      compile-only   do not run the C compiler\n\
      static         produce a static executable\n\
      turtledoc      generate Texinfo documentation from comments\n\
      deps           write dependency information to .P file\n\
      deps-stdout    write dependency information to standard output\n\
  -O FLAGS           set optimization flags\n\
    where FLAGS is one or more of\n\
      C              optimize module-local calls\n\
      c              do not optimize module-local calls\n\
      J              convert module-local calls to jumps\n\
      j              do not convert module-local calls to jumps\n\
      G              optimize GC checks over basic blocks\n\
      g              do not optimize GC checks over basic blocks\n\
      D              inline data constructors etc.\n\
      d              do not inline data constructors etc.\n\
      0-6            set optimization level for C compiler\n\
  -d MODIFIER        set debugging options\n\
    where MODIFIER is one or more of the letters\n\
      a              print AST after parsing\n\
      e              print top-level environment of module\n\
      i              dump the HIL representation to a .il file\n\
      b              print bytecode after code generation\n\
Report bugs to mgrabmue@cs.tu-berlin.de\n",
	  program_name);
#endif /* !HAVE_GETOPT_LONG */
}


/* Print version information to standard output.  */
static void 
version (void)
{
  printf ("%s %s\n", program_name, version_string);
}


int
main (int argc, char * argv[])
{
  int arg;
  int index;
  int exit_code = 0;
  char home_init_file[12];

  struct ttl_compile_options options;

#if HAVE_GETOPT_LONG
  static struct option command_line_options[] =
  {
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {"verbose", no_argument, NULL, 'V'},
    {"debug", required_argument, NULL, 'd'},
    {"pragma", required_argument, NULL, 'z'},
    {"optimize", required_argument, NULL, 'O'},
    {"main", required_argument, NULL, 'm'},
    {"module-path", required_argument, NULL, 'p'},
    {"include-path", required_argument, NULL, 'I'},
    {NULL, 0, NULL, 0}
  };
#endif /* HAVE_GETOPT_LONG */

  program_path = argv[0];


  ttl_init_compile_options (&options);

#if HAVE_GETOPT_LONG
  while ((arg = getopt_long (argc, argv, "+hvVm:d:I:z:O:p:",
			     command_line_options, &index)) != EOF)
#else /* !HAVE_GETOPT_LONG */
  while ((arg = getopt (argc, argv, "hvVm:d:I:z:O:p:")) != EOF)
#endif /* !HAVE_GETOPT_LONG */
    {
      switch (arg)
	{
	case 'h':
	  usage ();
	  exit (0);
	  break;

	case 'v':
	  version ();
	  exit (0);
	  break;

	case 'V':
	  options.verbose++;;
	  break;

	case 'm':
	  options.main = 1;
	  options.program_name = optarg;
	  break;

	case 'p':
	  options.module_path = optarg;
	  break;

	case 'I':
	  options.include_path = optarg;
	  break;

	case 'd':
	  {
	    char * p = optarg;
	    while (*p)
	      {
		switch (*p)
		  {
		  case 'e':
		    options.debug_dump_env = 1;
		    break;
		  case 'a':
		    options.debug_dump_ast = 1;
		    break;
		  case 'i':
		    options.debug_dump_il_file = 1;
		    break;
		  case 'b':
		    options.debug_dump_bytecode = 1;
		    break;
		  }
		p++;
	      }
	    break;
	  }

	case 'z':
	  {
	    if (!strcmp (optarg, "handcoded"))
	      options.pragma_handcoded = 1;
	    else if (!strcmp (optarg, "foreign"))
	      options.pragma_foreign = 1;
	    else if (!strcmp (optarg, "compile-only"))
	      options.pragma_compile_only = 1;
	    else if (!strcmp (optarg, "turtledoc"))
	      options.pragma_turtledoc = 1;
	    else if (!strcmp (optarg, "deps"))
	      options.pragma_printdeps = 1;
	    else if (!strcmp (optarg, "deps-stdout"))
	      options.pragma_printdepsstdout = 1;
	    else if (!strcmp (optarg, "static"))
	      options.link_static = 1;
	    else
	      {
		fprintf (stderr, "turtle: invalid pragma: %s\n", optarg);
		usage ();
		exit (1);
	      }
	    break;
	  }

	case 'O':
	  {
	    char * p = optarg;
	    while (*p)
	      {
		switch (*p)
		  {
		  case 'C':
		    options.opt_local_calls = 1;
		    break;
		  case 'c':
		    options.opt_local_calls = 0;
		    break;
		  case 'J':
		    options.opt_local_jumps = 1;
		    break;
		  case 'j':
		    options.opt_local_jumps = 0;
		    break;
		  case 'G':
		    options.opt_merge_gc_checks = 1;
		    break;
		  case 'g':
		    options.opt_merge_gc_checks = 0;
		    break;
		  case 'D':
		    options.opt_inline_constructors = 1;
		    break;
		  case 'd':
		    options.opt_inline_constructors = 0;
		    break;
		  case '0':
		  case '1':
		  case '2':
		  case '3':
		  case '4':
		  case '5':
		  case '6':
		    options.opt_gcc_level = *p - '0';
		    break;
		  }
		p++;
	      }
	    break;
	  }

	default:
	  usage ();
	  exit (1);
	  break;
	}
    }
 parsing_end:

  if (optind >= argc)
    {
      printf ("%s: No input files.\n", program_name);
      exit (1);
    }

  if (options.main && argc - optind > 1)
    {
      printf ("%s: Only one file can be compiled with --main=NAME\n",
	      program_name);
      exit (1);
    }

  {
    int i;

    i = optind;
    while (argv[i])
      {
	if (ttl_compile (argv[i], &options) > 0)
	  exit_code = 1;
	i++;
      }
  }

  if (ttl_allocated_bytes != 0)
    fprintf (stderr, "%d bytes leaked.\n", ttl_allocated_bytes);

  return exit_code;
}

/* End of turtle.c.  */
