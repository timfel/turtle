## Process this file with automake to produce Makefile.in
# 
# tools/Makefile.am
#

TURTLE = TURTLE_HACKING=../libturtle/.libs ../turtle/turtle
TURTLEFLAGS = --module-path=../crawl --optimize=d

tools: turtledoc analyze

turtledoc: turtledoc.t ast.o scanner.o parser.o
	$(TURTLE) $(TURTLEFLAGS) --main=$@ $<

analyze: analyze.t ast.o scanner.o parser.o env.o types.o
	$(TURTLE) $(TURTLEFLAGS) --main=$@ $<

parser.o: parser.t scanner.o ast.o
scanner.o: scanner.t
ast.o: ast.t
env.o: env.t ast.o types.o
types.o: types.t

%.o: %.t
	$(TURTLE) $(TURTLEFLAGS) $<

EXTRA_DIST = ast.t scanner.t parser.t turtledoc.t make-assembler.sh\
 env.t analyze.t types.t

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = *.ifc *.c *.h *.o turtledoc

# End of Makefile.am.
