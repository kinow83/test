bin_PROGRAMS = wips ha

wips_SOURCES = wips.c
ha_SOURCES = ha.c

wips_CPPFLAGS = -I@COMMON_DIR@ -I@LIBWIPS_DIR@
wips_LDADD = -lcommon -L@COMMON_DIR@ -lwips -L@LIBWIPS_DIR@
