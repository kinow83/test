bin_PROGRAMS = aus logmon

AM_CPPFLAGS = -I$(top_srcdir)/linux/aus/libs

# /usr/local/share/doc/full-package-name/aus.conf
ausdir = $(datadir)/doc/@PACKAGE@
aus_SOURCES = aus.c
aus_LDADD = $(top_srcdir)/linux/aus/libs/libtls.a
aus_SCRIPTS = aus
aus_DATA = aus.conf

logmon_SOURCES = logmon.c
logmon_LDADD = $(top_srcdir)/linux/aus/libs/libeap.la


