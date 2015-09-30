// suitetest.t -- Main test suite file.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

module suitetest;

import testsuite, ints, longs;

fun test_integer_constants ()
  testsuite.open ("constants");
  testsuite.test ("negative", fun (): bool return -1 = -1; end);
  testsuite.test ("positive", fun (): bool return 1 = 1; end);
  testsuite.test ("max", fun (): bool return ints.max = ints.max; end);
  testsuite.test ("min", fun (): bool return ints.min = ints.min; end);
  testsuite.close ();
end;

fun test_integer_arithmetic ()
  testsuite.open ("arithmetic");
  testsuite.test ("addition", fun (): bool return 1 + 1 = 2; end);
  testsuite.test ("subtraction", fun (): bool return 1 - 1 = 0; end);
  testsuite.test ("multiplication", fun (): bool return 1 * 2 = 2; end);
  testsuite.test ("division", fun (): bool return 2 / 2 = 1; end);
  testsuite.test ("modulo", fun (): bool return 2 % 2 = 0; end);
  testsuite.test ("negation", fun (): bool return -(-2) = 2; end);
  testsuite.close ();
end;

fun test_integer_comparison ()
  testsuite.open ("comparison");
  testsuite.test ("less than", fun (): bool return 1 < 2; end);
  testsuite.test ("greater than", fun (): bool return 2 > 1; end);
  testsuite.test ("less than or equal", fun (): bool return 1 <= 2; end);
  testsuite.test ("greater than or equal", fun (): bool return 2 >= 1; end);
  testsuite.test ("less than or equal (2)", fun (): bool return 2 <= 2; end);
  testsuite.test ("greater than or equal (2)", fun (): bool return 2 >= 2; end);
  testsuite.test ("equal", fun (): bool return 1 = 1; end);
  testsuite.test ("not equal", fun (): bool return 1 <> 2; end);
  testsuite.close ();
end;

fun test_integers ()
  testsuite.open ("integers");
  test_integer_constants ();
  test_integer_arithmetic ();
  test_integer_comparison ();
  testsuite.close ();
end;

fun test_long_constants ()
  testsuite.open ("constants");
  testsuite.test ("negative", fun (): bool return -1L = -1L; end);
  testsuite.test ("positive", fun (): bool return 1L = 1L; end);
  testsuite.test ("max", fun (): bool return longs.max = longs.max; end);
  testsuite.test ("min", fun (): bool return longs.min = longs.min; end);
  testsuite.close ();
end;

fun test_long_arithmetic ()
  testsuite.open ("arithmetic");
  testsuite.test ("addition", fun (): bool return 1L + 1L = 2L; end);
  testsuite.test ("subtraction", fun (): bool return 1L - 1L = 0L; end);
  testsuite.test ("multiplication", fun (): bool return 1L * 2L = 2L; end);
  testsuite.test ("division", fun (): bool return 2L / 2L = 1L; end);
  testsuite.test ("modulo", fun (): bool return 2L % 2L = 0L; end);
  testsuite.test ("negation", fun (): bool return -(-2L) = 2L; end);
  testsuite.close ();
end;

fun test_long_comparison ()
  testsuite.open ("comparison");
  testsuite.test ("less than", fun (): bool return 1L < 2L; end);
  testsuite.test ("greater than", fun (): bool return 2L > 1L; end);
  testsuite.test ("less than or equal", fun (): bool return 1L <= 2L; end);
  testsuite.test ("greater than or equal", fun (): bool return 2L >= 1L; end);
  testsuite.test ("less than or equal (2)", fun (): bool return 2L <= 2L; end);
  testsuite.test ("greater than or equal (2)", fun (): bool return 2L >= 2L; end);
  testsuite.test ("equal", fun (): bool return 1L = 1L; end);
  testsuite.test ("not equal", fun (): bool return 1L <> 2L; end);
  testsuite.close ();
end;

fun test_longs ()
  testsuite.open ("longs");
  test_long_constants ();
  test_long_arithmetic ();
  test_long_comparison ();
  testsuite.close ();
end;

fun test_real_constants ()
  testsuite.open ("constants");
  testsuite.test ("negative", fun (): bool return -1.0 = -1.0; end);
  testsuite.test ("positive", fun (): bool return 1.0 = 1.0; end);
  testsuite.close ();
end;

fun test_real_arithmetic ()
  testsuite.open ("arithmetic");
  testsuite.test ("addition", fun (): bool return 1.0 + 1.0 = 2.0; end);
  testsuite.test ("subtraction", fun (): bool return 1.0 - 1.0 = 0.0; end);
  testsuite.test ("multiplication", fun (): bool return 1.0 * 2.0 = 2.0; end);
  testsuite.test ("division", fun (): bool return 2.0 / 2.0 = 1.0; end);
  testsuite.test ("negation", fun (): bool return -(-2.0) = 2.0; end);
  testsuite.close ();
end;

fun test_real_comparison ()
  testsuite.open ("comparison");
  testsuite.test ("less than", fun (): bool return 1.0 < 2.0; end);
  testsuite.test ("greater than", fun (): bool return 2.0 > 1.0; end);
  testsuite.test ("less than or equal", fun (): bool return 1.0 <= 2.0; end);
  testsuite.test ("greater than or equal", fun (): bool return 2.0 >= 1.0; end);
  testsuite.test ("less than or equal (2)", fun (): bool return 2.0 <= 2.0; end);
  testsuite.test ("greater than or equal (2)", fun (): bool return 2.0 >= 2.0; end);
  testsuite.test ("equal", fun (): bool return 1.0 = 1.0; end);
  testsuite.test ("not equal", fun (): bool return 1.0 <> 2.0; end);
  testsuite.close ();
end;

fun test_reals ()
  testsuite.open ("reals");
  test_real_constants ();
  test_real_arithmetic ();
  test_real_comparison ();
  testsuite.close ();
end;

fun test_char_comparison ()
  testsuite.open ("comparison");
  testsuite.test ("less than", fun (): bool return 'a' < 'b'; end);
  testsuite.test ("greater than", fun (): bool return 'b' > 'a'; end);
  testsuite.test ("less than or equal", fun (): bool return 'a' <= 'b'; end);
  testsuite.test ("greater than or equal", fun (): bool return 'b' >= 'a'; end);
  testsuite.test ("less than or equal (2)", fun (): bool return 'b' <= 'b'; end);
  testsuite.test ("greater than or equal (2)", fun (): bool return 'b' >= 'b'; end);
  testsuite.test ("equal", fun (): bool return 1.0 = 1.0; end);
  testsuite.test ("not equal", fun (): bool return 1.0 <> 2.0; end);
  testsuite.close ();
end;

fun test_chars ()
  testsuite.open ("chars");
  test_char_comparison ();
  testsuite.close ();
end;

fun test_bool_comparison ()
  testsuite.open ("comparison");
  testsuite.test ("less than", fun (): bool return false < true; end);
  testsuite.test ("greater than", fun (): bool return true > false; end);
  testsuite.test ("less than or equal", fun (): bool return false <= true; end);
  testsuite.test ("greater than or equal", fun (): bool return true >= false; end);
  testsuite.test ("less than or equal (2)", fun (): bool return true <= true; end);
  testsuite.test ("greater than or equal (2)", fun (): bool return true >= true; end);
  testsuite.test ("equal", fun (): bool return false = false; end);
  testsuite.test ("not equal", fun (): bool return false <> true; end);
  testsuite.close ();
end;

fun test_bools ()
  testsuite.open ("bools");
  test_bool_comparison ();
  testsuite.close ();
end;

fun test_strings ()
  testsuite.open ("strings");
  testsuite.close ();
end;

fun test_list_constants ()
  testsuite.open ("constants");
  testsuite.test ("empty", fun (): bool return [] = null; end);
  testsuite.test ("singleton", fun (): bool return hd [1] = hd (1 :: null); end);
  testsuite.test ("hd", fun (): bool return hd [1, 2, 3] = 1; end);
  testsuite.exception ("hd", fun (): bool return hd tl [1] = 1; end);
  testsuite.test ("tl", fun (): bool return hd tl [1, 2, 3] = 2; end);
  testsuite.exception ("hd", fun (): bool return tl tl [1] = []; end);
  testsuite.close ();
end;

fun test_lists ()
  testsuite.open ("lists");
  test_list_constants ();
  testsuite.close ();
end;

fun main(args: list of string): int
  testsuite.start ("suitetest");
  test_integers ();
  test_longs ();
  test_reals ();
  test_chars ();
  test_bools ();
  test_lists ();
  return testsuite.summary ();
end;

// End of suitetest.t.
