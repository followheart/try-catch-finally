# Generated automatically from Makefile.in by configure.
# Build yase

SHELL = /bin/sh

top_srcdir = ..
srcdir = .

prefix = /home/dibyendu/yase
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
sampledir = $(prefix)/sample

INSTALLFILES_BIN = pdf2text.sh txt2xml.sh word2xml.sh
INSTALLFILES_SAMPLE = [1-6] yase.config alice13a.txt.gz
DISTFILES = $(INSTALLFILES_BIN) $(INSTALLFILES_SAMPLE) Makefile.in 

distdir = $(top_srcdir)/$(DISTNAME)/sample

clean:

dist:
	$(top_srcdir)/mkinstalldirs $(distdir) 
	cp $(DISTFILES) $(distdir)

install:
	$(top_srcdir)/mkinstalldirs $(sampledir) $(bindir)
	cp $(INSTALLFILES_SAMPLE) $(sampledir)
	cp $(INSTALLFILES_BIN) $(bindir)
	chmod +x $(bindir)/txt2xml.sh
	chmod +x $(bindir)/pdf2text.sh
	chmod +x $(bindir)/word2xml.sh
