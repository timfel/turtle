// sys/procs.t -- Process handling.
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
//* Module for accessing process information and dealing with processes.

module sys.procs;

import sys.errno;


//* Return the process identifier of the current process.
//
public fun getpid (): int;

//* Return the process identifier of the parent of the current
//* process.
//
public fun getppid (): int;


//* Make the current process sleep until @var{sec} seconds have
//* elapsed or a signal arrives which is not ignored.
//* 
//* The first version returns zero if the requested time has
//* elapsed, or the number of seconds left to sleep.  The second
//* version does not return anything.
//
public fun sleep (sec: int): int;
//* ""
public fun sleep (sec: int)
  var i: int := sleep (sec);
end;

//* Terminate the current process normally and return the value of
//* @var{status} to the parent.
//
public fun exit (status: int);

//* @code{kill} can be used to send any signal to any process group
//* or process.
//* 
//* If @var{pid} is positive, then signal @var{sig} is sent to
//* @var{pid}.  If @var{pid} equals 0, then @var{sig} is sent to
//* every process in the process group of the current process.  If
//* @var{pid} equals -1, then @var{sig} is sent to every process
//* except for the first one, from higher numbers in the process
//* table to lower.  If @var{pid} is less than -1, then @var{sig}
//* is sent to every process in the process group @var{-pid}.
//* 
//* If @var{sig} is 0, then no signal is sent, but error checking
//* is still performed.
//* 
//* On sucess, zero is returned, On error, -1 is returned and
//* @code{errno} is set appropriately.  The second version of the
//* @code{kill} function does not return anything and ignores any
//* errors.
//
public fun kill (pid: int, sig: int): int;
//* ""
public fun kill (pid: int, sig: int)
  var i: int := kill (pid, sig);
end;


//* Create a child process that differs from the parent process
//* only in its PID and PPID, and in the fact that resource
//* utilizations are set to 0.  File locks and pending signals are
//* not inherited.
//* 
//* On success, the process identifier of the child process is
//* returned in the parent's thread of execution, and a 0 is
//* returned in the child's thread of execution.  On failure, -1
//* will be returned in the parent's context, no child process will
//* be created, and the variable @code{errno.errno} will be set
//* appropriately.
//
public fun fork (): int;

//* Suspend execution of the current process until a child has
//* exited, or until a signal is delivered whose action is to
//* terminate the current process or to call a signal handling
//* function.  If a child has already exited by the time of the
//* call (a so-called "zombie" process), the function returns
//* immediately.  Any system resources used by the child are freed.
//*
//* 
//* The return value is a pair of the process ID of the exited
//* child (or -1 on error) and the status of the exited child.
//
public fun wait (): (int, int);

//* Constants to be used as the options argument to @code{waitpid}.
//
public const WNOHANG: int := 1;
//* ""
public const WUNTRACED: int := 2;

//* Suspend execution of the current process until a child as
//* specified by the @var{pid} argument, or until a signal is
//* delivered whose action is to terminate the current process or
//* to call a signal handling function.  If a child as requested by
//* @var{pid} has already exited by the time of the call (a
//* so-called "zombie" process", the function returns immediately.
//* Any system resources used by the child are freed.
//* 
//* The value of @var{pid} can be one of
//* 
//* @table @asis
//* @item < -1
//* which means to wait for any child process whose process group
//* ID is equal to the absolute value of @var{pid}.
//* 
//* @item -1
//* which means to wait for any child process; this is the same
//* behaviour which @code{wait} exhibits.
//* 
//* @item 0
//* which means to wait for any child process whose process group
//* ID is equal to that of the calling process.
//* 
//* @item > 0
//* which means to wait for the child whose process ID is equal to
//* the value of @var{pid}.
//* @end table
//* 
//* The value of @var{options} is an OR of zero or more of the
//* following constants:
//* 
//* @table @code
//* @item WNOHANG
//* which means to return immediately if no child has exited.
//* 
//* @item WUNTRACED
//* which means to also return for children which are stopped, and
//* whose status has not been reported.
//* @end table
//* 
//* The return value is a pair of the process ID of the exited
//* child (or -1 on error) and the status of the exited child.
//* 
//* If @code{WNOHANG} was given as an option, and no child has
//* exited, 0 is returned as the process ID.
//
public fun waitpid (pid: int, options: int): (int, int);

//* Return @code{true}, if the status code indicates that the
//* process has exited normally.
//
public fun WIFEXITED (status: int): bool;

//* Return the exit status of the exited process.  This may only be
//* called if @code{WIFEXITED} returned true for @var{status}.
//
public fun WEXITSTATUS (status: int): int;

//* Return @code{true}, if the status code indicates that the
//* process was terminated by a signal.
//
public fun WIFSIGNALED (status: int): bool;

//* Return the number of the signal which terminated the process.
//* This may only be called if @code{WIFSIGNALED} returned true for
//* @var{status}.
//
public fun WTERMSIG (status: int): int;

//* Return @code{true}, if the status code indicates that the
//* process was stopped by a signal.
//
public fun WIFSTOPPED (status: int): bool;

//* Return the number of the signal which stopped the process.
//* This may only be called if @code{WIFSTOPPED} returned true for
//* @var{status}.
//
public fun WSTOPSIG (status: int): int;


//* Execute the program called @var{filename}.  @var{argv} is an
//* array or list of argument strings passed to the new program.
//* @var{envp} is an array of string, conventionally of the form
//* @code{key=value}, which are passed as environment to the new
//* program.
//* 
//* Normally these functions do not return, on error, the value -1
//* is returned and the appropriate error code is placed in
//* the variable @code{sys.errno.errno}.
//
public fun execve (filename: string, argv: array of string, 
            envp: array of string): int;
//* ""
public fun execve (filename: string, argv: array of string, 
            envp: array of string)
  var i: int := execve (filename, argv, envp);
end;
//* ""
public fun execve (filename: string, argv: list of string, envp: list of string): int
  var argva: array of string;
  var envpa: array of string;
  var l: list of string;
  var len: int;

  len := 0;
  l := argv;
  while l <> null do
    len := len + 1;
    l := tl l;
  end;

  argva := array len of "";

  len := 0;
  l := argv;
  while l <> null do
    argva[len] := hd l;
    len := len + 1;
    l := tl l;
  end;

  len := 0;
  l := envp;
  while l <> null do
    len := len + 1;
    l := tl l;
  end;

  envpa := array len of "";

  len := 0;
  l := envp;
  while l <> null do
    envpa[len] := hd l;
    len := len + 1;
    l := tl l;
  end;

  return execve (filename, argva, envpa);
end;
//* ""
public fun execve (filename: string, argv: list of string, 
                   envp: list of string)
  var i: int := execve (filename, argv, envp);
end;
//* ""
public fun execve (filename: string, argv: array of string): int
  return execve (filename, argv, {});
end;
//* ""
public fun execve (filename: string, argv: array of string)
  var i: int := execve (filename, argv, {});
end;
//* ""
public fun execve (filename: string, argv: list of string): int
  return execve (filename, argv, []);
end;
//* ""
public fun execve (filename: string, argv: list of string)
  var i: int := execve (filename, argv, []);
end;


//* Return the value of the environment variable called
//* @var{varname}
//
public fun getenv (varname: string): string = "ttl_getenv";

// End of sys/procs.t.
