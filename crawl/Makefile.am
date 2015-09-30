## Process this file with automake to produce Makefile.in
# 
# crawl/Makefile.am
#

SUBDIRS = internal sys

subpkglibdir = $(pkglibdir)/$(VERSION)
subpkglib_DATA = $(LIBOBJS) $(LIBIFCS) $(LIBHDRS)

TURTLE = ../turtle/turtle
TURTLEFLAGS =

LIBOBJS =\
 turtle0.o\
 math.o\
 core.o\
 exceptions.o\
 bools.o\
 chars.o\
 union.o\
 ints.o\
 longs.o\
 reals.o\
 compare.o\
 arrays.o\
 arraysearch.o\
 binary.o\
 identity.o\
 lists.o\
 listfold.o\
 listreduce.o\
 listzip.o\
 listindex.o\
 strings.o\
 strformat.o\
 listmap.o\
 arraysort.o\
 compose.o\
 random.o\
 arraymap.o\
 listsort.o\
 listsearch.o\
 io.o\
 option.o\
 bintree.o\
 cmdline.o\
 hash.o\
 hashtab.o\
 pairs.o\
 triples.o\
 trees.o\
 bstrees.o\
 filenames.o

LIBIFCS = $(LIBOBJS:%.o=%.ifc)

LIBHDRS = $(LIBOBJS:%.o=%.h)

LIBSRCS = $(LIBOBJS:%.o=%.t)

#BUILT_SOURCES = $(LIBOBJS)

# Do `make modules' to build the library modules which already
# can be compiled
#
modules: $(LIBOBJS)

core.o: core.t core.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

math.o: math.t math.t.i
	$(TURTLE) $(TURTLEFLAGS) --pragma=handcoded $<

%.o: %.t
	$(TURTLE) $(TURTLEFLAGS) $<

modules.texi: $(LIBSRCS)
	echo "@c ==========================================================="\
 > $@;\
	for f in $(LIBSRCS) ; do\
	  echo Handling $$f;\
	  $(TURTLE) $(TURTLEFLAGS) --pragma=turtledoc $$f >> $@;\
	echo "@c ==========================================================="\
 >> $@;\
	done
EXTRA_DIST = turtle0.t arrays.t bintree.t compare.t compose.t\
 hashtab.t hash.t identity.t listmap.t lists.t io.t strings.t\
 option.t\
 core.t core.t.i arraysort.t arraysearch.t math.t math.t.i random.t\
 arraymap.t listsort.t listsearch.t cmdline.t\
 ints.t longs.t reals.t chars.t bools.t binary.t exceptions.t\
 pairs.t triples.t trees.t bstrees.t filenames.t\
 listfold.t listreduce.t listzip.t listindex.t strformat.t union.t

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = *.ifc *.c *.h *.o

# End of Makefile.am.
