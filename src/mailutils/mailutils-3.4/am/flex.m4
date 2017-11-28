# flex.m4 serial 1
AC_DEFUN([MU_PROG_FLEX],
[
 if test "x$LEX" = x; then
   AC_PROG_LEX
   if ! $LEX --version 2>/dev/null | grep -q '^flex '; then
      LEX="$SHELL $missing_dir/missing flex"
      AC_SUBST([LEX_OUTPUT_ROOT], [lex.yy])
      AC_SUBST([LEXLIB], [''])
   fi
 fi
])
   
