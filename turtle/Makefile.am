## Process this file with automake to produce Makefile.in
# 
# turtle/Makefile.am
#

bin_PROGRAMS = turtle

turtle_SOURCES = turtle.c

turtle_LDADD = $(top_builddir)/libturtle/libturtle.la

INCLUDES = -I$(top_srcdir)

TURTLE = TURTLE_HACKING=../libturtle/.libs ./turtle

lala: lala.t
	$(TURTLE) --module-path=../crawl --main=$@ $<

EXTRA_DIST = lala.t

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = *.ifc lala.c lala.h lala.o lala

AM_CPPFLAGS = $(CPPEXTRAFLAGS)

# End of Makefile.am.
