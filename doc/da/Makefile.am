## Process this file with automake to produce Makefile.in

da: da.dvi

da.ps: da.dvi
	dvips -o $@ $<

da.dvi: da.tex cover-text.tex cip-literature.bib appendix.tex	\
 constraint-imperative.tex constraint-programming.tex		\
 introduction.tex summary.tex turtle.tex turtle-impl.tex
	if latex da|grep erun; then		\
	  bibtex da;				\
	  makeindex da;				\
	  latex da;				\
	  latex da;				\
	  latex da;				\
	fi

EXTRA_DIST = cip-literature.bib					\
 da.tex thema.tex cover-text.tex da.dvi temperature.fig		\
 runtime-schema.epic compiler.epic				\
 runtime-schema.fig compiler.fig				\
 temperature.epic appendix.tex constraint-imperative.tex	\
 constraint-programming.tex  introduction.tex summary.tex	\
 turtle.tex turtle-impl.tex diploma.cls float.sty		\
 binary-constraint.fig binary-constraint.epic			\
 compiling.fig compiling.epic					\
 dispatcher.fig dispatcher.epic					\
 data-rep.fig data-rep.epic					\
 variables.fig variables.epic					\
 constraint.fig constraint.epic					\
 splitting.fig splitting.epic					\
 constrainable-vars.fig constrainable-vars.epic			\
 data-header.fig data-header.epic


CLEANFILES = *.aux *.bbl *.blg *.log *.toc *.idx *.ind *.ilg *.lof	\
 *.dvi *.lot *.lotp

MAINTAINERCLEANFILES = Makefile.in
