// testsuite.t -- Common functions for all test programs.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

//* This module provides infrastructure for Turtle test programs.
//* It displays nice progress indicators while the tests are
//* running, counts successful tests, failures and exceptions and
//* gives a summary of the tests at the end.
//* 
//* See the file @code{suitetest.t} for an example of this module's
//* usage.

module testsuite;

import io, strings, exceptions;

//* Statistics.
//
var test_count: int := 0;
var fail_count: int := 0;
var expected_fail_count: int := 0;
var exception_count: int := 0;
var expected_exception_count: int := 0;
var success_count: int := 0;

//* Program state.  Needed for displaying nice section numbers.
//
var test_name: string := "";
var level: list of int := [];


//* Print the numbers in the list backwards, separated by dots.
//
fun wr (l: list of int)
  if l <> null then
    if tl l <> null then
      wr (tl l);
      io.put (".");
    end;
    io.put (hd l);
  end;
end;


//* This function must be called when a test file begins.
//* @var{file} should be the name of the test file.
//
public fun start (file: string)
  io.put ("Testing ");
  io.put (file);
  io.nl ();
  test_name := file;
  level := [0];
end;


//* Open a new level of tests.
//
public fun open (topic: string)
  level := hd level + 1 :: tl level;
  wr (level);
  level := 0 :: level;
  io.put (" ");
  io.put (topic);
  io.nl ();
end;


//* Close the current level of tests.
//
public fun close ()
  level := tl level;
end;

//* - Do a test called @var{name}.  @var{f} is the function to be
//* called.  If the function returns @code{true}, the test is
//* regarded successful, if it returns @code{false}, the test
//* failed.  If an exception occurs while @var{f} is executing,
//* this is counted as an exception.  The remaining arguments
//* specify how failures and exception should be counted (as
//* expected or unexpected).
//
fun test (name: string, f: fun (): bool, expect_fail: bool,
          expect_exception: bool)
  var result: bool := false;
  var except: bool := false;
  var except_name: string;

  level := hd level + 1 :: tl level;
  wr (level);
  io.put ("\t");
  io.put (strings.rpad (name + " ", 30, '.'));

  test_count := test_count + 1;

  exceptions.handle (fun () result := f (); end,
                     fun (s: string) except_name := s; except := true; end);
  if except then
    io.put (" EXCEPTION: ");
    io.put (except_name);
    io.nl ();
    if expect_exception then
      expected_exception_count := expected_exception_count + 1;
    else
      exception_count := exception_count + 1;
    end;
  elsif result then
    io.put (" ok.\n");
    success_count := success_count + 1;
  else
    io.put (" FAILURE.\n");
    if expect_fail then
      expected_fail_count := expected_fail_count + 1;
    else
      fail_count := fail_count + 1;
    end;
  end;
end;


//* This function executes a normal test.
//
public fun test (name: string, f: fun (): bool)
  test (name, f, false, false);
end;


//* This function executes a test which is expected to fail.
//
public fun fail (name: string, f: fun (): bool)
  test (name, f, true, false);
end;


//* This function executes a test which is expected to raise an
//* exception.
//
public fun exception (name: string, f: fun (): bool)
  test (name, f, false, true);
end;


//* Summarise the test results in a human-readable way.
//
public fun summary (): int
  io.put ("\nTest summary for ");
  io.put (test_name);
  io.put ("\n-----------------");
  io.put (strings.lpad ("", sizeof test_name, '-'));
  io.nl ();
  io.put ("tests ................. "); io.put (test_count); io.nl ();
  io.put ("successful ............ "); io.put (success_count); io.nl ();
  io.put ("failed ................ "); io.put (fail_count); io.nl ();
  io.put ("failed (expected) ..... "); io.put (expected_fail_count); io.nl ();
  io.put ("exception ............. "); io.put (exception_count); io.nl ();
  io.put ("exception (expected) .. "); io.put (expected_exception_count); 
  io.nl ();
  io.put ("unexpected failures ... "); io.put (fail_count + exception_count);
  io.nl ();
  if fail_count + exception_count = 0 then
    io.put ("\n--- Test successful.\n");
    return 0;
  else
    io.put ("\n*** THERE WERE UNEXPECTED FAILURES ***\n");
    return 1;
  end;
end;

// End of testsuite.t.
