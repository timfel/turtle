## Process this file with automake to produce Makefile.in
#

SUBDIRS = doc libturtle turtle crawl tests examples emacs tools misc

EXTRA_DIST = autogen.sh HACKING

MAINTAINERCLEANFILES = config.h.in configure Makefile.in stamp-h.in

snap: clean
	(cd .. && tar czvf turtle-snap-`date +%Y-%m-%d_%H_%M`.tar.gz turtle)
