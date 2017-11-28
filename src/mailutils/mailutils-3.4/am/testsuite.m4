# Initialize the (autotest) test suite.
AC_DEFUN([MU_CONFIG_TESTSUITE],
[AC_CONFIG_TESTDIR([$1/tests])
AC_CONFIG_FILES([$1/tests/Makefile $1/tests/atlocal])
])
