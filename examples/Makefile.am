## Process this file with automake to produce Makefile.in
# 
# examples/Makefile.am
#

# The environment variable TURTLE_HACKING must be set to the directory
# containing the turtle runtime, because otherwise it tries to link
# against the installed version, which might not be available when
# building the examples.
#
TURTLE = TURTLE_HACKING=../libturtle/.libs ../turtle/turtle
TURTLEFLAGS = 
#--pragma=static

EXAMPLES = hello minimal fib fac fac_iterative evenodd oddeven counter\
 higher_order overloading module_params queens hello2 constraints indigo\
 trees layout omaopa sendmory sendmory2 tak hanoi loop interpret\
 min_ex crypto

EXTRA_EXAMPLES = copy_file miniwget helloserver helloclient webserver

TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(top_builddir)/libturtle/.libs
TESTS = $(EXAMPLES)

examples: $(EXAMPLES) $(EXTRA_EXAMPLES)

minimal: minimal.t
	$(TURTLE) $(TURTLEFLAGS) --main=$@ $<
hello: hello.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
fib: fib.t
	$(TURTLE) $(TURTLEFLAGS) --main=$@ $<
fac: fac.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
fac_iterative: fac_iterative.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
evenodd: evenodd.t
	$(TURTLE) $(TURTLEFLAGS) --main=$@ $<
oddeven: odd_module.t even_module.t
	$(TURTLE) $(srcdir)/odd_module.t
	$(TURTLE) $(TURTLEFLAGS) --main=$@ $(srcdir)/even_module.t
counter: counter.t
	$(TURTLE) $(TURTLEFLAGS) --main=$@ $<
higher_order: higher_order.t
	$(TURTLE) $(TURTLEFLAGS) --main=$@ $<
overloading: overloading.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
module_params: module_params.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
queens: queens.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
hello2: hello2.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
trees: trees.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
copy_file: copy_file.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
indigo: indigo.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
layout: layout.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
constraints: constraints.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
omaopa: omaopa.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
sendmory: sendmory.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
sendmory2: sendmory2.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
crypto: crypto.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
min_ex: min_ex.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
tak: tak.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
hanoi: hanoi.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
loop: loop.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
interpret: interpret.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<

miniwget: miniwget.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
helloserver: helloserver.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
helloclient: helloclient.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<
webserver: webserver.t config.o html.o http.o dirlist.o wiki.o game.o
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl --main=$@ $<

%.o: %.t
	$(TURTLE) $(TURTLEFLAGS) --module-path=../crawl $<

EXTRA_DIST = even_module.t fac.t fib.t minimal.t hello.t queens.t counter.t\
 evenodd.t fac_iterative.t odd_module.t trees.t higher_order.t\
 overloading.t module_params.t queens_cip.t hello2.t copy_file.t\
 test_copy_file.sh miniwget.t helloserver.t helloclient.t webserver.t http.t\
 html.t dirlist.t config.t wiki.t game.t indigo.t layout.t constraints.t\
 omaopa.t sendmory.t sendmory2.t tak.t hanoi.t loop.t interpret.t min_ex.t

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = *.ifc *.c *.h *.o $(EXAMPLES) copy_file miniwget helloserver\
 helloclient webserver

# End of Makefile.am.
