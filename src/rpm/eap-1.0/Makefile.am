# shared libraries
lib_LTLIBRARIES = libeap.la
libeap_la_SOURCES = eap.c
#libeap_la_LDFLAGS = -version-info 0


bin_PROGRAMS = testeap
testeap_SOURCES = test.c
testeap_LDADD = libeap.la
testeap_LDFLAGS = -lext -L.

testeapdir=/usr/lib64
testeap_SCRIPTS = libext.so
