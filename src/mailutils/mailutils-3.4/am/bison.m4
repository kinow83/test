# bison.m4 serial 1
AC_DEFUN([MU_PROG_BISON],
[
  if test "x$ac_cv_prog_YACC" = x; then
    AC_PROG_YACC
    if ! $YACC --version 2>/dev/null | grep -q '^bison '; then
       YACC="$SHELL $missing_dir/missing bison"
    fi
  fi
])

