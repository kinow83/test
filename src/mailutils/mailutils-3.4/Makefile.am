## This file is part of GNU Mailutils
## Copyright (C) 2000-2012, 2014-2017 Free Software Foundation, Inc.
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

ACLOCAL_AMFLAGS = -I m4 -I am -I gint -I doc/imprimatur

if MU_COND_PYTHON
  PYTHON_DIR = python
endif

if MU_COND_MH
  MH_DIR = mh
endif

if MU_COND_FRM
  FRM_DIR = frm
endif

if MU_COND_POP3D
  POP3D_DIR = pop3d
endif

if MU_COND_IMAP4D
  IMAP4D_DIR = imap4d
endif

if MU_COND_MAIDAG
  MAIDAG_DIR = maidag
endif

if MU_COND_MAIL
  MAIL_DIR = mail
endif

if MU_COND_SIEVE
  SIEVE_DIR = sieve
endif

if MU_COND_MESSAGES
  MESSAGES_DIR = messages
endif

if MU_COND_COMSAT
  COMSAT_DIR = comsat
endif

if MU_COND_READMSG
  READMSG_DIR = readmsg
endif

if MU_COND_DOTLOCK
  DOTLOCK_DIR = dotlock
endif

if MU_COND_MOVEMAIL
  MOVEMAIL_DIR = movemail
endif

if MU_COND_MIMEVIEW
  MIMEVIEW_DIR = mimeview
endif

if MU_COND_LIBMU_SCM
  GINT_DIR = gint
  LIBMU_SCM_DIR = libmu_scm
  SCHEME_DIR = scheme
endif

if MU_COND_SUPPORT_CXX
  LIBMU_CPP_DIR = libmu_cpp
endif

if MU_COND_DBM
  LIBMU_DBM_DIR = libmu_dbm
endif

SUBDIRS = . \
 mu-aux\
 include\
 po\
 libmailutils\
 sql\
 libmu_auth\
 libproto\
 testsuite\
 lib\
 $(LIBMU_CPP_DIR)\
 $(GINT_DIR)\
 $(LIBMU_SCM_DIR)\
 $(LIBMU_DBM_DIR)\
 libmu_sieve\
 $(PYTHON_DIR)\
 doc\
 examples\
 mu\
 $(SCHEME_DIR)\
 $(FRM_DIR)\
 $(POP3D_DIR)\
 $(IMAP4D_DIR)\
 $(MAIDAG_DIR)\
 $(MAIL_DIR)\
 $(SIEVE_DIR)\
 $(MESSAGES_DIR)\
 $(COMSAT_DIR)\
 $(READMSG_DIR)\
 $(DOTLOCK_DIR)\
 $(MH_DIR)\
 $(MOVEMAIL_DIR)\
 $(MIMEVIEW_DIR)

EXTRA_DIST = COPYING.LESSER paths
DISTCLEANFILES = pathdefs.h

gen_start_date = "2008-12-08"
prev_change_log = "doc/ChangeLog.CVS"
amend_file = ChangeLog.amend

# Trigger creation of the ChangeLog
noinst_DATA = ChangeLog
.PHONY: ChangeLog
ChangeLog: 
	$(AM_V_at)if test -d .git; then					\
	    $(top_srcdir)/mu-aux/gencl --verbose --append-dot           \
                    --email=$(PACKAGE_BUGREPORT)                        \
		    --append=$(prev_change_log)     		        \
                    --amend=$(amend_file)                               \
	            --since=$(gen_start_date);    			\
	elif test "$(builddir)" = "$(srcdir)" && test ! -e ChangeLog; then \
	    echo "This file is a placeholder." > ChangeLog;             \
	fi

dist-hook: ChangeLog
	@if test -f ChangeLog && test -f NEWS; then			\
	  PATCHLEV=`echo "$(PACKAGE_VERSION)" | sed -r "s/[0-9]+\.[0-9]+\.?//"`;\
	  if test $${PATCHLEV:-0} -lt 50; then				     \
	    if head -n 6 ChangeLog | grep -q 'Uncommitted changes' ;	     \
	    then							     \
	      echo >&2 "*** Source tree contains uncommitted changes";	     \
	      echo >&2 "*** Aborting";					     \
	      exit 1;							     \
	    fi;								     \
	    if grep -q FIXME NEWS; then					     \
	      echo >&2 "*** NEWS file contains FIXMEs";			     \
	      echo >&2 "*** Aborting";					     \
	      exit 1;							     \
	    fi;								     \
	  fi;                                                                \
	fi

alpha:
	version=`$(GITINFO) -H'$$refversion{?$$refdist>0??-$$refdist?}'`;\
	if test -n "$$version"; then\
	  $(MAKE) dist distdir=$(PACKAGE)-$$version; \
	else \
	  $(MAKE) dist; \
	fi


alphacheck:
	version=`$(GITINFO) -H'$$refversion{?$$refdist>0??-$$refdist?}'`;\
	if test -n "$$version"; then\
	  $(MAKE) distcheck distdir=$(PACKAGE)-$$version; \
	else \
	  $(MAKE) distcheck; \
	fi

