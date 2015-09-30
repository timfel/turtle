// cmdline.t -- Command line parsing.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//  
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this package; see the file COPYING.  If not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.

// Commentary:
//
//* The function @code{getopt}, exported by this module, deconstructs a
//* command line into options, option parameters and other parameters.

module cmdline;

import strings, io;


//* Values of type @code{optspec} define the possibilities of command line
//* flags (parameterless options) and options (which have parameters).
//
public datatype optspec = flag (flag_char: char, flag_string: string) or
                          option (option_char: char, option_string: string);


//* Calls to the @code{getopt} function return lists of this type.
//* Each element of the list stands for one of three different
//* argument types:
//*
//* @table @asis
//* @item Flag
//* A flag is a parameterless option, which acts as a switch.
//*
//* @item Option
//* An option has an associated argument, which gives the program
//* additional information about the option.
//*
//* @item Parameter
//* Everything not beginning with one or two dashes is considered
//* a normal parameter, such as a filename to act on.  Everything
//* following the special option @code{--} will also be treated as
//* a parameter, even if it starts with a dash.  Note that a
//* single dash is also a normal parameter.
//* @end table
//
public datatype option = flag (flag_char: char) or
                         option (option_char: char, argument: string) or
			 parameter (param: string);


//* Given a list of option specifications and a list of command line
//* arguments (as passed to the @code{main} function, for example),
//* return a list of options, classified and deconstructed as flags,
//* options or parameters.
//
public fun getopt (specs: list of optspec, args: list of string): 
                   list of option

  // Reverse the list of options
  fun rev (l: list of option, r: list of option): list of option
    if l = null then
      return r;
    else
      return rev (tl l, hd l :: r);
    end;
  end;

  var result: list of option := null;
  var idx: int;
  var s: list of optspec;
  var found: bool;
  var progname: string := hd args;

  idx := strings.rindex (progname, '/');
  if idx >= 0 then
    progname := strings.substring (progname, idx + 1, sizeof progname);
  end;
  args := tl args;		// Skip program name.
  while args <> null do
    var arg: string := hd args;

    // Check whether we have an option...
    if (sizeof arg > 0) and (arg[0] = '-') then

      // Check whether we have a long option...
      if (sizeof arg > 1) and (arg[1] = '-') then

	// `--' means: stop processing, rest of command line are
        // normal parameters.
	//
	if sizeof arg = 2 then
	  args := tl args;
	  while args <> null do
	    result := parameter (hd args) :: result;
	    args := tl args;
	  end;
	  return rev (result, null);
	end;

	// Now we look for an `=', which separates the option name from
	// its value.
	//
	idx := strings.index (arg, '=');
	if idx >= 0 then
	  var p1: string := strings.substring (arg, 2, idx);
	  var p2: string := strings.substring (arg, idx + 1, sizeof arg);
	  found := false;
	  s := specs;
	  while (s <> null) and (not found) do
	    if flag? (hd s) then
	      if strings.eq (flag_string (hd s), p1) then
		io.put (progname);
		io.put (": flag option used with parameter: ");
		io.put (arg);
		io.nl ();
		found := true;
	      end;
	    else
	      if strings.eq (option_string (hd s), p1) then
		result := option (option_char (hd s), p2) :: result;
		found := true;
	      end;
	    end;
	    s := tl s;
	  end;
	  if not found then
	    io.put (progname);
	    io.put (": unknown parameter: ");
	    io.put (p1); io.nl ();
	    result := flag ('?') :: result;
	  end;
	else
	  // Handle a long option without an `='.
	  //
	  p1 := strings.substring (arg, 2, sizeof arg);
	  found := false;
	  s := specs;
	  while (s <> null) and (not found) do
	    if flag? (hd s) then
	      if strings.eq (flag_string (hd s), p1) then
		result := flag (flag_char (hd s)) :: result;
		found := true;
	      end;
	    else
	      if strings.eq (option_string (hd s), p1) then
		if tl args <> null then
		  p2 := hd tl args;
		  args := tl args;
		  result := option (option_char (hd s), p2) :: result;
		else
		  io.put (progname);
		  io.put (": option expects parameter: ");
		  io.put (arg);
		  io.nl ();
		  result := flag ('?') :: result;
		end;
		found := true;
	      end;
	    end;
	    s := tl s;
	  end;
	  if not found then
	    io.put (progname);
	    io.put (": unknown parameter: ");
	    io.put (p1); io.nl ();
	    result := flag ('?') :: result;
	  end;
	end;
      else
	// Handle a short option, which must consist at least of a dash
	// and a character.
	//
	if sizeof arg > 1 then
	  found := false;
	  s := specs;
	  while (s <> null) and (not found) do
	    if flag? (hd s) then
	      if flag_char (hd s) = arg[1] then
		// Handle the case of more than one short option in a
		// row, like in `-hv'.
		//
		if sizeof arg > 2 then
		  args := arg :: strings.append
		    ("-", strings.substring (arg, 2, sizeof arg)) :: tl args;
		end;
		result := flag (flag_char (hd s)) :: result;
		found := true;
	      end;
	    else
	      if option_char (hd s) = arg[1] then
		// Either take the rest of the current argument as the
		// option value, or take the next argument.
		if sizeof arg > 2 then
		  result := option (option_char (hd s),
		              strings.substring (arg, 2, sizeof arg))
			    :: result;
	        else
		  if tl args <> null then
		    result := option (option_char (hd s),
		                      hd tl args) :: result;
		    args := tl args;
                  else
		    io.put (progname);
		    io.put (": option expects parameter: -");
		    io.put (arg[1]);
		    io.nl ();
		    result := flag ('?') :: result;
		  end;
		end;
		found := true;
	      end;
	    end;
	    s := tl s;
	  end;
	  if not found then
	    io.put (progname);
	    io.put (": unknown parameter: -");
	    io.put (arg[1]); io.nl ();
	    result := flag ('?') :: result;
	  end;
	else
	  // A dash of its own is a normal parameter.  This is often used
	  // to indicate stdin or stdout as input/output files.
	  //
	  result := parameter (arg) :: result;
	end;
      end;
    else
      // Normal parameter, that is everything that does not belong to
      // a long or short option.
      //
      result := parameter (arg) :: result;
    end;
    if args <> null then
      args := tl args;
    end;
  end;

  return rev (result, null);
end;

// End of cmdline.t.
