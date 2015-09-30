## Process this file with automake to produce Makefile.in
# 
# crawl/internal/Makefile.am
#

subpkglibdir = $(pkglibdir)/$(VERSION)/internal
subpkglib_DATA = $(LIBOBJS) $(LIBIFCS) $(LIBHDRS)

TURTLE = ../../turtle/turtle
TURTLEFLAGS = -I$(top_srcdir)

LIBOBJS =\
 gc.o\
 stats.o\
 ex.o\
 limits.o\
 binary.o\
 version.o\
 random.o\
 timeout.o\
 debug.o

LIBIFCS = $(LIBOBJS:%.o=%.ifc)

LIBHDRS = $(LIBOBJS:%.o=%.h)

LIBSRCS = $(LIBOBJS:%.o=%.t)

#BUILT_SOURCES = $(LIBOBJS)

# Do `make modules' to build the library modules which already
# can be compiled
#
modules: $(LIBOBJS)

gc.o: gc.t gc.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

stats.o: stats.t stats.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

ex.o: ex.t ex.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

binary.o: binary.t binary.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

version.o: version.t version.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

random.o: random.t random.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

timeout.o: timeout.t timeout.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

debug.o: debug.t debug.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

%.o: %.t
	$(TURTLE) $(TURTLEFLAGS) $<

EXTRA_DIST = gc.t gc.t.i ex.t ex.t.i limits.t binary.t binary.t.i\
 version.t version.t.i random.t random.t.i stats.t stats.t.i\
 timeout.t timeout.t.i debug.t debug.t.i

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = *.ifc *.c *.h *.o

# End of Makefile.am.
