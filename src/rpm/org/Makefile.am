# shared libraries
lib_LTLIBRARIES = libeap.la
libeap_la_SOURCES = eap.c
libeap_la_LDFLAGS = -version-info 1:0:0


bin_PROGRAMS = testeap
testeap_SOURCES = test.c
testeap_LDADD = libeap.la
