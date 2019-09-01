PACKAGE		= @PACKAGE@
VERSION		= @VERSION@

AM_CPPFLAGS	= -I$(top_builddir) -I$(top_srcdir)

includedir = $(prefix)/include/mbus
include_HEADERS = mbus.h mbus-protocol.h mbus-serial.h 

lib_LTLIBRARIES	   = libmbus.la
libmbus_la_SOURCES = mbus.c mbus-protocol.c mbus-serial.c 

