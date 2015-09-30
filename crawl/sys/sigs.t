// sys/sigs.t -- Signal function installation.
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
//* Module for installing a signal function, that is a function
//* which gets called whenever the process receives an operating
//* system signal.
//* 
//* In Turtle, signals are handled synchroneously, that means that
//* a Turtle process repeatedly checks whether a signal has arrived
//* and then calls the signal handler function.  

module sys.sigs;

//* These are the signals defined in POSIX.1.
//
public const SIGHUP: int := foreign "TTL_INT_TO_VALUE (SIGHUP)";
//* ""
public const SIGINT: int := foreign "TTL_INT_TO_VALUE (SIGINT)";
//* ""
public const SIGQUIT: int := foreign "TTL_INT_TO_VALUE (SIGQUIT)";
//* ""
public const SIGILL: int := foreign "TTL_INT_TO_VALUE (SIGILL)";
//* ""
public const SIGABRT: int := foreign "TTL_INT_TO_VALUE (SIGABRT)";
//* ""
public const SIGFPE: int := foreign "TTL_INT_TO_VALUE (SIGFPE)";
//* ""
public const SIGKILL: int := foreign "TTL_INT_TO_VALUE (SIGKILL)";
//* ""
public const SIGSEGV: int := foreign "TTL_INT_TO_VALUE (SIGSEGV)";
//* ""
public const SIGPIPE: int := foreign "TTL_INT_TO_VALUE (SIGPIPE)";
//* ""
public const SIGALRM: int := foreign "TTL_INT_TO_VALUE (SIGALRM)";
//* ""
public const SIGTERM: int := foreign "TTL_INT_TO_VALUE (SIGTERM)";
//* ""
public const SIGUSR1: int := foreign "TTL_INT_TO_VALUE (SIGUSR1)";
//* ""
public const SIGUSR2: int := foreign "TTL_INT_TO_VALUE (SIGUSR2)";
//* ""
public const SIGCHLD: int := foreign "TTL_INT_TO_VALUE (SIGCHLD)";
//* ""
public const SIGCONT: int := foreign "TTL_INT_TO_VALUE (SIGCONT)";
//* ""
public const SIGSTOP: int := foreign "TTL_INT_TO_VALUE (SIGSTOP)";
//* ""
public const SIGTSTP: int := foreign "TTL_INT_TO_VALUE (SIGTSTP)";
//* ""
public const SIGTTIN: int := foreign "TTL_INT_TO_VALUE (SIGTTIN)";
//* ""
public const SIGTTOU: int := foreign "TTL_INT_TO_VALUE (SIGTTOU)";


//* Install a signal handler for signal number @var{no}.  Whenever
//* a signal is received, the process will stop at the next safe
//* point and call the handler function. with the signal number as
//* the argument.
//
public fun signal (no: int, handler: fun (int));

// End of sys/sigs.t.
