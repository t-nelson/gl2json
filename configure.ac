AC_INIT([gl2json], [0.0.1], [])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

# Work around stupid autoconf default cflags. pt 1
SAVE_CFLAGS="x${CFLAGS}"

AC_PROG_CC
AC_PROG_RANLIB

AM_PROG_CC_C_O

# Work around stupid autoconf default cflags. pt 2
if test "$SAVE_CFLAGS" = "x"; then
  CFLAGS=""
fi

AC_ARG_WITH([glroot],
            [AC_HELP_STRING([--with-glroot], [Specify path to glroot (default: /jail/glftpd)])],
            [with_glroot=$withval],
            [with_glroot=/jail/glftpd])

if ! test -d "$with_glroot"; then
  AC_MSG_ERROR([$with_glroot: No such directory])
fi

AC_CHECK_LIB([json],   [main],, AC_MSG_ERROR([Missing a required library!]))

AC_CONFIG_FILES([Makefile]
                [src/Makefile]
               )

AM_CPPFLAGS="-Wall -Werror -I$with_glroot/bin/sources"

AC_SUBST(AM_CPPFLAGS)

AC_OUTPUT

