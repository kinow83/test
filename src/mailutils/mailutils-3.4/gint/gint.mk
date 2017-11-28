# This file is part of Gint
# Copyright (C) 2010, 2013 Sergey Poznyakoff
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

if GINT_COND_GUILE_OK
AM_CPPFLAGS += @GUILE_INCLUDES@ 

if GINT_COND_DOC_DISTRIB
 EXTRA_DIST += $(DOT_X_FILES)
 MAINTAINERCLEANFILES += $(DOT_X_FILES)
else
 DISTCLEANFILES += $(DOT_X_FILES)
endif
BUILT_SOURCES += $(DOT_X_FILES)


ETAGS_ARGS = --regex='/SCM_\(GLOBAL_\)?\(G?PROC\|G?PROC1\|SYMBOL\|VCELL\|CONST_LONG\).*\"\([^\"]\)*\"/\3/' \
   --regex='/[ \t]*SCM_[G]?DEFINE1?[ \t]*(\([^,]*\),[^,]*/\1/'

SUFFIXES += .x

snarfcppopts = $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) 

if GINT_COND_DOC

if GINT_COND_DOC_DISTRIB
 EXTRA_DIST += guile-procedures.texi guile-procedures.txt $(DOT_DOC_FILES)
 MAINTAINERCLEANFILES += \
  $(DOT_DOC_FILES)\
  guile-procedures.texi\
  guile-procedures.txt
else
 DISTCLEANFILES += \
  $(DOT_DOC_FILES)\
  guile-procedures.texi\
  guile-procedures.txt
endif

site_DATA += guile-procedures.txt

BUILT_SOURCES += $(DOT_DOC_FILES) guile-procedures.texi

SUFFIXES += .doc

if GINT_COND_SNARF_DOC_FILTER
GUILE_DOC_SNARF=$(top_builddir)/$(GINT_MODULE_DIR)/snarf-doc-filter --snarfer
else
GUILE_DOC_SNARF=$(top_builddir)/$(GINT_MODULE_DIR)/clexer --snarfer
endif

.c.doc:
	$(AM_V_GEN)$(CC) -DSCM_MAGIC_SNARF_DOCS $(snarfcppopts) -E $< | \
	$(GUILE_DOC_SNARF) -o $@ || { rm $@; false; }

guile-procedures.texi: $(DOT_DOC_FILES)
	$(AM_V_GEN)cat $(DOT_DOC_FILES) | \
         $(GUILE_TOOLS) snarf-check-and-output-texi > $@

guile-procedures.txt: guile-procedures.texi
	$(AM_V_GEN) rm -f $@; \
	  $(MAKEINFO) --force -o $@ guile-procedures.texi || test -f $@
endif

.c.x:
	$(AM_V_GEN)AWK="$(AWK)" \
	CPP="$(CPP)" \
	$(GUILE_SNARF) -o $@ \
                           $< $(snarfcppopts)

if GINT_COND_INC
SUFFIXES += .inc
CLEANFILES += *.inc

.doc.inc:
	$(AM_V_GEN)$(top_srcdir)/$(GINT_MODULE_DIR)/extract-exports -o $@ $<
endif

## Add -MG to make the .x magic work with auto-dep code.
MKDEP = $(CC) -M -MG $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(CFLAGS)
endif

