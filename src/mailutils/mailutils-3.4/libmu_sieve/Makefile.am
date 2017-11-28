## This file is part of GNU Mailutils.
## Copyright (C) 2002-2005, 2007, 2009-2012, 2014-2017 Free Software
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

YLWRAP = @MU_YLWRAP@
AM_YFLAGS = -dtv
#AM_LEXFLAGS=-dvp
AM_CPPFLAGS =\
 @MU_LIB_COMMON_INCLUDES@\
 -DMU_SIEVE_MODDIR=\"@MU_SIEVE_MODDIR@\"

SUBDIRS = . extensions

lib_LTLIBRARIES = libmu_sieve.la

libmu_sieve_la_SOURCES = \
 actions.c\
 conf.c\
 comparator.c\
 encoded.c\
 environment.c\
 load.c\
 mem.c\
 prog.c\
 registry.c\
 relational.c\
 require.c\
 runtime.c\
 sieve-gram.y\
 sieve-lex.l\
 strexp.c\
 string.c\
 tests.c\
 util.c\
 variables.c
libmu_sieve_la_LIBADD = ${MU_LIB_MAILUTILS} @LTDL_LIB@
libmu_sieve_la_LDFLAGS = -version-info @VI_CURRENT@:@VI_REVISION@:@VI_AGE@

noinst_HEADERS = sieve-priv.h sieve-gram.h

BUILT_SOURCES = \
  sieve-gram.c sieve-gram.h \
  sieve-lex.c

EXTRA_DIST=gylwrap.conf

MAINTAINERCLEANFILES=$(BUILT_SOURCES)


