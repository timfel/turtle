// lala.t -- Test file for always.

module lala;

public fun main(argv: list of string): int
  var x: int := 1;
  var y: int := 2;

  x := y * x;
  return 0;
end;

// End of lala.t.
