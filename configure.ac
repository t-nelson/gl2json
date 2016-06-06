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

DEFAULT_GLROOT="/jail/glftpd"
AC_ARG_WITH([glroot],
            ["Specify path to glroot (default: $DEFAULT_GLROOT")],
            [with_glroot=$withval],
            [with_glroot=$DEFAULT_GLROOT])

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

