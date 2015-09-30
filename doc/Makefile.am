## Process this file with automake to produce Makefile.in

SUBDIRS = da

info_TEXINFOS = turtle.texi

html:
	rm -rf html1
	mkdir html1
	(cd html1 && texi2html -menu -number -monolithic ../turtle.texi)
	rm -rf html2
	mkdir html2
	(cd html2 && texi2html -menu -number -split_chapter ../turtle.texi)
	rm -rf html3
	mkdir html3
	(cd html3 && texi2html -menu -number -split_node ../turtle.texi)

EXTRA_DIST =  tagging.text

#CLEANFILES = *.aux *.bbl *.blg *.log *.toc

MAINTAINERCLEANFILES = Makefile.in
