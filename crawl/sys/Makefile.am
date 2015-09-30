## Process this file with automake to produce Makefile.in
# 
# crawl/sys/Makefile.am
#

subpkglibdir = $(pkglibdir)/$(VERSION)/sys
subpkglib_DATA = $(LIBOBJS) $(LIBIFCS) $(LIBHDRS)

TURTLE = ../../turtle/turtle
TURTLEFLAGS = -I$(top_srcdir) -p$(top_builddir)/crawl

LIBOBJS =\
 errno.o\
 users.o\
 procs.o\
 files.o\
 dirs.o\
 net.o\
 sigs.o\
 times.o

LIBIFCS = $(LIBOBJS:%.o=%.ifc)

LIBHDRS = $(LIBOBJS:%.o=%.h)

LIBSRCS = $(LIBOBJS:%.o=%.t)

#BUILT_SOURCES = $(LIBOBJS)

# Do `make modules' to build the library modules which already
# can be compiled
#
modules: $(LIBOBJS)

users.o: users.t users.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

procs.o: procs.t procs.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

errno.o: errno.t errno.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

files.o: files.t files.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

dirs.o: dirs.t dirs.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

net.o: net.t net.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded --pragma=foreign $<

sigs.o: sigs.t sigs.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded --pragma=foreign $<

times.o: times.t times.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

%.o: %.t
	$(TURTLE) $(TURTLEFLAGS) $<

EXTRA_DIST = users.t users.t.i procs.t procs.t.i errno.t errno.t.i\
 net.t net.t.i files.t files.t.i dirs.t dirs.t.i sigs.t sigs.t.i\
 times.t times.t.i

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = *.ifc *.c *.h *.o

# End of Makefile.am.
