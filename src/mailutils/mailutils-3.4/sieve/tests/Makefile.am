# This file is part of GNU Mailutils.
# Copyright (C) 2007-2012, 2014-2017 Free Software Foundation, Inc.
#
# GNU Mailutils is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.
#
# GNU Mailutils is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>.

EXTRA_DIST = $(TESTSUITE_AT) testsuite package.m4 moderator.mbox
DISTCLEANFILES       = atconfig $(check_SCRIPTS)
MAINTAINERCLEANFILES = Makefile.in $(TESTSUITE)
#SUBDIRS = etc

## ------------ ##
## package.m4.  ##
## ------------ ##

$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	$(AM_V_GEN) {                                      \
	  echo '# Signature of the current package.'; \
	  echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	  echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >$(srcdir)/package.m4

#

## ------------ ##
## Test suite.  ##
## ------------ ##

TESTSUITE_AT = \
  action.at\
  addheader.at\
  address.at\
  allof.at\
  anyof.at\
  delheader.at\
  compile.at\
  enc-char.at\
  envelope.at\
  environment.at\
  exists.at\
  ext.at\
  false.at\
  header.at\
  i-casemap.at\
  i-numeric.at\
  i-octet.at\
  list.at\
  moderator.at\
  mul-addr.at\
  not.at\
  pipeact.at\
  pipetest.at\
  redirect.at\
  reject.at\
  relational.at\
  size.at\
  true.at\
  testsuite.at\
  vacation.at\
  variables.at\
  version.at

TESTSUITE = $(srcdir)/testsuite
M4=m4

AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): package.m4 $(TESTSUITE_AT) $(top_srcdir)/testsuite/testsuite.inc
	$(AM_V_GEN)$(AUTOTEST) -I $(srcdir) -I $(top_srcdir)/testsuite testsuite.at -o $@.tmp
	$(AM_V_at)mv $@.tmp $@

atconfig: $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status tests/$@

clean-local:
	@test ! -f $(TESTSUITE) || $(SHELL) $(TESTSUITE) --clean

check-local: atconfig atlocal $(TESTSUITE) 
	@$(SHELL) $(TESTSUITE)

# Run the test suite on the *installed* tree.
#installcheck-local:
#	$(SHELL) $(TESTSUITE) AUTOTEST_PATH=$(exec_prefix)/bin


