dnl Copyright (C) 2006-2007, 2010-2012, 2014-2017 Free Software
dnl Foundation, Inc.
dnl
dnl GNU Mailutils is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 3, or (at
dnl your option) any later version.
dnl
dnl GNU Mailutils is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>.

m4_define([am_mu_vercmp],[
m4_pushdef([_ver_A_],m4_car($1))dnl
m4_pushdef([_ver_B_],m4_car($2))dnl
m4_if(_ver_B_,,:,[if test m4_if(_ver_A_,,0,_ver_A_) -lt _ver_B_; then
  $3  
elif test m4_if(_ver_A_,,0,_ver_A_) -eq _ver_B_; then  
am_mu_vercmp(m4_cdr($1),m4_cdr($2),[$3])
fi
m4_popdef([_ver_A_])dnl
m4_popdef([_ver_B_])dnl])])

dnl AM_GNU_MAILUTILS(minversion, link-req, [act-if-found], [ac-if-not-found])
dnl                      $1         $2           $3              $4
dnl Verify if GNU Mailutils is installed and if its version is `minversion'
dnl or newer.  If not installed, execute `ac-if-not-found' or, if it is not
dnl given, spit out an error message.
dnl
dnl If Mailutils is found, set:
dnl     MAILUTILS_CONFIG to the full name of the mailutils-config program;
dnl     MAILUTILS_VERSION to the Mailutils version (string);
dnl     MAILUTILS_VERSION_MAJOR  Mailutils version: major number
dnl     MAILUTILS_VERSION_MINOR  Mailutils version: minor number
dnl     MAILUTILS_VERSION_PATCH  Mailutils version: patchlevel number (or 0,
dnl                              if not defined)
dnl     MAILUTILS_LIBS to the list of cc(1) flags needed to link in the
dnl         libraries requested by `link-req';
dnl     MAILUTILS_INCLUDES to the list of cc(1) flags needed to set include
dnl         paths to the Mailutils headers.
dnl
dnl Finally, if `act-if-found' is given, execute it.  Otherwise, append the
dnl value of $MAILUTILS_LIBS to LIBS. 
dnl
AC_DEFUN([AM_GNU_MAILUTILS],
 [AC_PATH_PROG(MAILUTILS_CONFIG, mailutils-config, none, $PATH)
  if test "$MAILUTILS_CONFIG" = "none"; then
    m4_if($4,,[AC_MSG_ERROR(cannot find GNU Mailutils)], [$4])
  fi
  AC_SUBST(MAILUTILS_CONFIG)
  AC_SUBST(MAILUTILS_VERSION)
  AC_SUBST(MAILUTILS_INCLUDES)
  AC_SUBST(MAILUTILS_LIBS)

  m4_ifndef([MU_VERSION_PARSE_DEFINED],[[
mu_version_parse() {
    set -- `echo "@S|@1" | sed 's/^\([0-9\.][0-9\.]*\).*/\1/;s/\./ /g'`
    major=@S|@{1:-0}
    minor=@S|@{2:-0}
    patch=@S|@{3:-0}
}]
  m4_pushdef([MU_VERSION_PARSE_DEFINED])])

  MAILUTILS_VERSION=`$MAILUTILS_CONFIG --info version|sed 's/VERSION=//'`
  mu_version_parse $MAILUTILS_VERSION
  AC_DEFINE_UNQUOTED([MAILUTILS_VERSION_MAJOR], $major, [Mailutils version major number])
  AC_DEFINE_UNQUOTED([MAILUTILS_VERSION_MINOR], $minor, [Mailutils version minor number])
  AC_DEFINE_UNQUOTED([MAILUTILS_VERSION_PATCH], $patch, [Mailutils version patchlevel number])
  
  AC_DEFINE_UNQUOTED(MAILUTILS_VERSION, "$MAILUTILS_VERSION", [Mailutils version number]) 
  m4_if($1,,,[
   am_mu_vercmp(m4_quote($major, $minor, $patch),
                m4_dquote(m4_bpatsubst($1, [\.],[,])),
      [AC_MSG_ERROR([Mailutils v. $MAILUTILS_VERSION is too old; required is at least ]$1)])
  ])
  req=""
  for x in $2
  do
    case $x in
    mailer)   test $MAILUTILS_VERSION_NUMBER -ge 1200 && req="$req $x";;
    *)        req="$req $x"
    esac
  done
  MAILUTILS_LIBS=`$MAILUTILS_CONFIG --link $req`
  MAILUTILS_INCLUDES=`$MAILUTILS_CONFIG --compile`
  m4_if($3,,[LIBS="$LIBS $MAILUTILS_LIBS"], [$3])
])  
  
