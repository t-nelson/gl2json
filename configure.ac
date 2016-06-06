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

GL_SOURCE_DIR="$with_glroot/bin/sources"
CPPFLAGS="$CPPFLAGS -I$GL_SOURCE_DIR"
AC_CHECK_HEADER([glconf.h], [], AC_MSG_ERROR([Missing glconf.h. See --with-glroot]))

AC_CHECK_LIB([json-c], [main],, AC_MSG_ERROR([Missing a required library!]))

AC_CONFIG_FILES([Makefile]
                [src/Makefile]
               )

AM_CPPFLAGS="-Wall -Werror"

AC_SUBST(AM_CPPFLAGS)

AC_OUTPUT

