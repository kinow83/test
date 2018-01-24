#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

AC_PROG_RANLIB
AC_PROG_LIBTOOL

# target, ...
AC_CANONICAL_SYSTEM
case "$target" in
#	x86_64-pc-linux-gnu
	x86_64-*-linux*)
		DIRS="linux";
		AC_SUBST(DIRS)
		;;
	*solaris*)
		DIRS="solaris";
		AC_SUBST(DIRS)
		;;
	*)
		echo $target
		;;
esac

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile
                 linux/Makefile
                 linux/aus/Makefile
                 linux/aus/common/Makefile
                 linux/aus/libs/Makefile
                 linux/aus/src/Makefile
                 linux/kernel/Makefile
                 linux/wips/Makefile
                 linux/wips/bin/Makefile
                 linux/wips/etc/Makefile
                 linux/wips/libs/Makefile
                 linux/wips/script/Makefile
                 linux/wips/src/Makefile])
AC_OUTPUT
