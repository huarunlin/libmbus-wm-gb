#
#
#
PACKAGE		= @PACKAGE@
VERSION		= @VERSION@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmbus-wm-gb.pc


docdir = $(datadir)/doc/$(PACKAGE)-$(VERSION)
dist_docdir = $(DESTDIR)$(docdir)
doc_DATA = README.md \
           COPYING 

SUBDIRS		= mbus 
ACLOCAL		= aclocal -I .
ACLOCAL_AMFLAGS = -Werror -I m4
