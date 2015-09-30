## Process this file with automake to produce Makefile.in
# 
# tests/Makefile.am
#

# The environment variable TURTLE_HACKING must be set to the directory
# containing the turtle runtime, because otherwise it tries to link
# against the installed version, which might not be available when
# building the tests.
#
TURTLE = TURTLE_HACKING=../libturtle/.libs ../turtle/turtle
TURTLEFLAGS = --module-path=../crawl
#TURTLEFLAGS = --pragma=static --module-path=../crawl

TESTFILES = overloading0.t overloading1.t overloading2.t tupletest.t\
 arraytest.t listtest.t stringtest.t inttest.t longtest.t booltest.t math0.t\
 fun0.t fun1.t array0.t array1.t rand0.t list0.t binary0.t stress0.t\
 stress1.t stress2.t\
 lex0.t parse0.t hashtab0.t exceptions0.t pairs0.t triples0.t trees0.t\
 bstrees0.t sys_users0.t sys_procs0.t filenames0.t sys_files0.t\
 listfold0.t listreduce0.t listzip0.t listindex0.t sys_dirs0.t\
 sys_sigs0.t internal_timeout0.t constraints0.t constraints1.t\
 stress3.t sys_times0.t suitetest.t foreign0.t import0.t

TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(top_builddir)/libturtle/.libs
TESTS = $(TESTFILES:%.t=%)

suitetest: testsuite.o

%: %.t
	$(TURTLE) $(TURTLEFLAGS) --main=$@ $<
%.o: %.t
	$(TURTLE) $(TURTLEFLAGS) $<

foreign0: foreign0.t
	$(TURTLE) $(TURTLEFLAGS) --pragma=foreign --main=$@ $<

extracheck: 
	$(MAKE) check TESTS=sys_net0

EXTRA_DIST = $(TESTFILES) test-template.t lex1.t parse1.t sys_net0.t\
 testsuite.t suitetest.t

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = *.ifc *.c *.h *.o $(TESTS) sys_net0

# End of Makefile.am.
