## This file is part of GNU Mailutils.
## Copyright (C) 2005, 2007, 2010-2012, 2014-2017 Free Software
## Foundation, Inc.
##
## GNU Mailutils is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 3, or (at
## your option) any later version.
##
## GNU Mailutils is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. 

SUBDIRS = tests

AM_CPPFLAGS = \
 @MU_APP_COMMON_INCLUDES@\
 -D_GNU_SOURCE=1\
 -DDEFAULT_CUPS_CONFDIR=\"@DEFAULT_CUPS_CONFDIR@\"

bin_PROGRAMS = mimeview
mimeview_SOURCES = \
 mimeview.c \
 grammar.y \
 lexer.l \
 mimeview.h\
 grammar.h

YLWRAP = @MU_YLWRAP@
AM_YFLAGS=-vtd
AM_LEXFLAGS=-d
EXTRA_DIST = gylwrap.conf

mimeview_LDADD = \
 ${MU_APP_LIBRARIES}\
 ${MU_LIB_MAILUTILS}\
 @LTLIBINTL@
 
