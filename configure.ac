AC_INIT(README)
AM_INIT_AUTOMAKE(harfbuzz, 0.1)
AM_MAINTAINER_MODE
AC_PROG_CC
AM_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG
AM_CONFIG_HEADER(config.h)

AC_PATH_PROG(ft_config,freetype-config,no)
if test "$ft_config" = "no"; then
    AC_MSG_ERROR([You must have freetype installed; see http://www.freetype.org/])
fi

FREETYPE_CFLAGS="`$ft_config --cflags`"
FREETYPE_LIBS="`$ft_config --libs`"

AC_SUBST(FREETYPE_LIBS)
AC_SUBST(FREETYPE_CFLAGS)

PKG_CHECK_MODULES([FONTCONFIG], [fontconfig])
AC_SUBST(FONTCONFIG_CFLAGS)
AC_SUBST(FONTCONFIG_LIBS)

AC_OUTPUT([
Makefile
src/Makefile
test/Makefile
])
