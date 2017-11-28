pkglibexec_PROGRAMS=\
 mailutils-acl\
 mailutils-cflags\
 mailutils-ldflags\
 mailutils-filter\
 mailutils-flt2047\
 mailutils-info\
 mailutils-logger\
 mailutils-query\
 mailutils-send\
 mailutils-smtp\
 mailutils-stat\
 mailutils-wicket

noinst_HEADERS = mu.h
noinst_LIBRARIES = libmutool.a
libmutool_a_SOURCES = \
 getopt.c\
 getans.c\
 getarg.c\
 getyn.c\
 shell.c\
 util.c\
 verbose.c

mailutils_acl_SOURCES = acl.c
mailutils_filter_SOURCES = filter.c
mailutils_flt2047_SOURCES = flt2047.c
mailutils_info_SOURCES = info.c
mailutils_logger_SOURCES = logger.c
mailutils_query_SOURCES = query.c
mailutils_wicket_SOURCES = wicket.c

mailutils_cflags_SOURCES=cflags.c
mailutils_cflags_CPPFLAGS = $(AM_CPPFLAGS)\
  -DCOMPILE_FLAGS="\"-I$(includedir)\""

mailutils_ldflags_SOURCES=ldflags.c
mailutils_ldflags_CPPFLAGS = $(AM_CPPFLAGS)\
  -DLINK_FLAGS="\"-L$(libdir)\"" \
  -DLINK_POSTFLAGS="\"$(MU_LINK_POSTFLAGS)\"" \
  -DAUTHLIBS="\"$(MU_AUTHLIBS)\"" \
  -DGUILE_LIBS="\"$(GUILE_LIBS)\"" \
  -DPYTHON_LIBS="\"$(PYTHON_LIBS)\"" \
  -DI18NLIBS="\"$(LIBINTL)\"" \
  -DDBMLIBS="\"$(DBMLIBS)\""

if MU_COND_DBM
 pkglibexec_PROGRAMS += mailutils-dbm
 mailutils_dbm_SOURCES = dbm.c
 mailutils_dbm_LDFLAGS = \
 ${MU_APP_LIBRARIES}\
 ../../libmu_dbm/libmu_dbm.la\
 @DBMLIBS@\
 $(MUTOOL_LIBRARIES_TAIL)
endif

if MU_COND_SUPPORT_POP
 pkglibexec_PROGRAMS += mailutils-pop
 mailutils_pop_SOURCES = pop.c
 mailutils_pop_CPPFLAGS = \
 $(AM_CPPFLAGS)\
 @MU_AUTHINCS@
 mailutils_pop_LDADD =\
 ${MU_APP_LIBRARIES}\
 ${MU_LIB_POP}\
 ${MU_LIB_AUTH}\
 @MU_AUTHLIBS@\
 $(MUTOOL_LIBRARIES_TAIL)
endif

if MU_COND_SUPPORT_IMAP
 pkglibexec_PROGRAMS += mailutils-imap
 mailutils_imap_SOURCES = imap.c
 mailutils_imap_CPPFLAGS = \
 $(AM_CPPFLAGS)\
 @MU_AUTHINCS@
 mailutils_imap_LDADD =\
 ${MU_APP_LIBRARIES}\
 ${MU_LIB_IMAP}\
 ${MU_LIB_AUTH}\
 @MU_AUTHLIBS@\
 $(MUTOOL_LIBRARIES_TAIL)
endif

mailutils_send_SOURCES = send.c
mailutils_send_CPPFLAGS = \
 $(AM_CPPFLAGS)\
 @MU_AUTHINCS@
mailutils_send_LDADD = \
 ${MU_APP_LIBRARIES}\
 ${MU_LIB_MAILER}\
 ${MU_LIB_AUTH}\
 @MU_AUTHLIBS@\
 $(MUTOOL_LIBRARIES_TAIL)

mailutils_stat_SOURCES = stat.c
mailutils_stat_LDADD = \
 ${MU_APP_LIBRARIES}\
 ${MU_LIB_MBOX}\
 ${MU_LIB_IMAP}\
 ${MU_LIB_POP}\
 ${MU_LIB_NNTP}\
 ${MU_LIB_MH}\
 ${MU_LIB_MAILDIR}\
 ${MU_LIB_AUTH}\
 @MU_AUTHLIBS@\
 $(MUTOOL_LIBRARIES_TAIL)

mailutils_smtp_SOURCES = smtp.c
mailutils_smtp_CPPFLAGS = \
 $(AM_CPPFLAGS)\
 @MU_AUTHINCS@
mailutils_smtp_LDADD = \
 ${MU_APP_LIBRARIES}\
 ${MU_LIB_MAILER}\
 ${MU_LIB_AUTH}\
 @MU_AUTHLIBS@\
 $(MUTOOL_LIBRARIES_TAIL)

MUTOOL_LIBRARIES_TAIL = \
 ./libmutool.a\
 ${MU_APP_LIBRARIES}\
 ${MU_LIB_MAILUTILS}\
 @READLINE_LIBS@\
 @MU_COMMON_LIBRARIES@

LDADD = ${MU_APP_LIBRARIES} $(MUTOOL_LIBRARIES_TAIL)

AM_CPPFLAGS = \
 @MU_APP_COMMON_INCLUDES@
