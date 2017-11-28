## This file is part of GNU Mailutils.
## Copyright (C) 2001-2002, 2006-2007, 2009-2012, 2014-2017 Free
## Software Foundation, Inc.
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

bin_SCRIPTS = guimb

guimb: guimb.scmi package.sed
	$(AM_V_GEN)sed -f package.sed $(srcdir)/guimb.scmi > guimb
	$(AM_V_at)chmod +x guimb

package.sed: Makefile
	$(AM_V_GEN)echo 's,%GUILE_BINDIR%,@GUILE_BINDIR@,g' > package.sed
	$(AM_V_at)echo 's,%BINDIR%,$(bindir),g' >> package.sed
	$(AM_V_at)echo 's,%GUILE_SITE%,$(GUILE_SITE),g' >> package.sed
	$(AM_V_at)echo 's,%PACKAGE%,$(PACKAGE),g' >> package.sed
	$(AM_V_at)echo 's,%VERSION%,$(VERSION),g' >> package.sed

CLEANFILES = guimb package.sed

EXTRA_DIST=\
 guimb.scmi

installcheck-binSCRIPTS: $(bin_SCRIPTS)
	bad=0; pid=$$$$; list="$(bin_SCRIPTS)"; for p in $$list; do \
	  case ' $(AM_INSTALLCHECK_STD_OPTIONS_EXEMPT) ' in \
	   *" $$p "* | *" $(srcdir)/$$p "*) continue;; \
	  esac; \
	  f=`echo "$$p" | sed 's,^.*/,,;$(transform)'`; \
	  for opt in --help --version; do \
	    if GUILE_AUTO_COMPILE=0 "$(DESTDIR)$(bindir)/$$f" $$opt >c$${pid}_.out \
	         2>c$${pid}_.err </dev/null \
		 && test -n "`cat c$${pid}_.out`" \
		 && test -z "`cat c$${pid}_.err`"; then :; \
	    else echo "$$f does not support $$opt" 1>&2; bad=1; fi; \
	  done; \
	done; rm -f c$${pid}_.???; exit $$bad
