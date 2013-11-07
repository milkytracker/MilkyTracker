#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(MilkyTracker, 0.90.86, www.milkytracker.net)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src)
AM_INIT_AUTOMAKE([dist-bzip2 subdir-objects])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_MKDIR_P

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS_ONCE([fcntl.h inttypes.h limits.h malloc.h memory.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/time.h unistd.h],,AC_MSG_ERROR([One or more required header files are missing]))
# AC_CHECK_HEADER(zzip/lib.h,,AC_MSG_ERROR("Cannot find zzip/lib.h"))

# Checks for libraries.
AC_ARG_WITH([alsa],
	[AS_HELP_STRING([--with-alsa],
	[Enable native ALSA sound/midi support @<:@default=check@:>@])],
	[],
	[with_alsa=check])

if test "x$with_alsa" != xno; then
	AM_PATH_ALSA([1.0.0],[
		RTMIDI="../midi/libmidi.a"
		RTMIDI_DIR="midi"
		AC_SUBST(RTMIDI)
		AC_SUBST(RTMIDI_DIR)],
		[if test "x$with_alsa" != xcheck; then
			AC_MSG_FAILURE([--with-alsa was given, but test for alsa failed])
		else
			AC_MSG_WARN([ALSA not found - midi support disabled])
		fi])
fi

AC_SEARCH_LIBS(gzopen, z, AC_DEFINE([HAVE_LIBZ], [1], [Define to 1 if you have the 'libz' library.]),)
#AC_CHECK_LIB(zzip, zzip_file_open,,AC_MSG_ERROR("Cannot find -lzzip"))
#PKG_CHECK_MODULES([ZZIP],[zziplib >= 0.10.75])

SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,, AC_MSG_ERROR([Please install libSDL]))
CPPFLAGS="$CPPFLAGS -D__FORCE_SDL_AUDIO__"

# Check that SDL supports the X11 wm
AC_MSG_CHECKING([whether SDL supports X11])
OLDCPPFLAGS=$CPPFLAGS
CPPFLAGS=$SDL_CFLAGS
AC_COMPILE_IFELSE(
	[AC_LANG_SOURCE([#include<SDL_syswm.h>
	const int test = SDL_SYSWM_X11;])],
	[AC_MSG_RESULT([yes])]
	AC_DEFINE([HAVE_X11], [1], [SDL supports X11 features]),
	[AC_MSG_RESULT([no])])
CPPFLAGS=$OLDCPPFLAGS

# Jack is dynamically linked to ease binary portability (no longer required)
AC_ARG_WITH([jack],
	[AS_HELP_STRING([--with-jack],
	[Enable JACK support @<:@default=check@:>@])],
	[],
	[with_jack=check])

if test x$with_jack != xno; then
	AC_CHECK_HEADERS([jack/jack.h],
		[AC_SEARCH_LIBS(dlsym,dl,,[AC_MSG_ERROR([Dynamic library support not found - required for JACK support])])],
		[if test x$with_jack != xcheck; then
			AC_MSG_FAILURE([--with-jack was given, but JACK headers were not found])
		else
			AC_MSG_WARN([JACK headers not found, JACK support disabled])
		fi])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STAT
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_STRUCT_ST_BLOCKS
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FSEEKO
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_CHECK_FUNCS([atexit floor getcwd gettimeofday memmove memset munmap pow select sqrt strcasecmp strdup strerror strrchr])

AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/compression/Makefile
                 src/compression/zziplib/generic/Makefile
                 src/fx/Makefile
                 src/midi/Makefile
                 src/milkyplay/Makefile
                 src/ppui/Makefile
                 src/ppui/osinterface/Makefile
                 src/tracker/Makefile])
AC_OUTPUT
#([Makefile src/Makefile src/midi/Makefile src/milkyplay/Makefile src/ppui/Makefile src/fx/Makefile src/tracker/Makefile src/compression/Makefile])

