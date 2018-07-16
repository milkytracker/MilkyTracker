#ifndef _ZZIP__CONFIG_H
#define _ZZIP__CONFIG_H 1
 
/* zzip/_config.h. Generated automatically at end of configure. */
/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.ac by autoheader.  */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef _zzip_const */

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef ZZIP__FILE_OFFSET_BITS */

/* Define if you have the <dirent.h> header file, and it defines `DIR'. */
#ifndef ZZIP_HAVE_DIRENT_H 
#define ZZIP_HAVE_DIRENT_H  1 
#endif

#define _zzip_off64_t off_t

/* Define if you have the <dlfcn.h> header file. */
/* #undef ZZIP_HAVE_DLFCN_H */

/* Define if you have the <inttypes.h> header file. */
#define ZZIP_HAVE_INTTYPES_H 1

/* Define if you have the <io.h> header file. */
/* #undef ZZIP_HAVE_IO_H */

/* Define if you have the <memory.h> header file. */
/* #undef ZZIP_HAVE_MEMORY_H */

/* Define if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef ZZIP_HAVE_NDIR_H */

/* Define if you have the <stdint.h> header file. */
#ifndef ZZIP_HAVE_STDINT_H 
#define ZZIP_HAVE_STDINT_H  1 
#endif

/* Define if you have the <stdlib.h> header file. */
#ifndef ZZIP_HAVE_STDLIB_H 
#define ZZIP_HAVE_STDLIB_H  1 
#endif

/* Define if you have the `strcasecmp' function. */
#ifndef ZZIP_HAVE_STRCASECMP 
#define ZZIP_HAVE_STRCASECMP  1 
#endif

/* Define if you have the <string.h> header file. */
#ifndef ZZIP_HAVE_STRING_H 
#define ZZIP_HAVE_STRING_H  1 
#endif

/* Define if you have the <strings.h> header file. */
#ifndef ZZIP_HAVE_STRINGS_H 
#define ZZIP_HAVE_STRINGS_H  1 
#endif

/* Define if you have the <sys/dir.h> header file, and it defines `DIR'. */
/* #undef ZZIP_HAVE_SYS_DIR_H */

/* Define if you have the <sys/int_types.h> header file. */
/* #undef ZZIP_HAVE_SYS_INT_TYPES_H */

/* Define if you have the <sys/mman.h> header file. */
/* #undef ZZIP_HAVE_SYS_MMAN_H */

/* Define if you have the <sys/ndir.h> header file, and it defines `DIR'. */
/* #undef ZZIP_HAVE_SYS_NDIR_H */

/* Define if you have the <sys/param.h> header file. */
/* #undef ZZIP_HAVE_SYS_PARAM_H */

/* Define if you have the <sys/stat.h> header file. */
#ifndef ZZIP_HAVE_SYS_STAT_H 
#define ZZIP_HAVE_SYS_STAT_H  1 
#endif

/* Define if you have the <sys/types.h> header file. */
#ifndef ZZIP_HAVE_SYS_TYPES_H 
#define ZZIP_HAVE_SYS_TYPES_H  1 
#endif

/* Define if you have the <unistd.h> header file. */
#ifndef ZZIP_HAVE_UNISTD_H 
#define ZZIP_HAVE_UNISTD_H  1 
#endif

/* Define if you have the <winbase.h> header file. */
/* #undef ZZIP_HAVE_WINBASE_H */

/* Define if you have the <windows.h> header file. */
/* #undef ZZIP_HAVE_WINDOWS_H */

/* Define if you have the <winnt.h> header file. */
/* #undef ZZIP_HAVE_WINNT_H */

/* Define if you have the <zlib.h> header file. */
#ifndef ZZIP_HAVE_ZLIB_H 
#define ZZIP_HAVE_ZLIB_H  1 
#endif

/* Define as `__inline' if that's what the C compiler calls it, or to nothing
   if it is not supported. */
/* #undef _zzip_inline */

/* Define for large files, on AIX-style hosts. */
/* #undef ZZIP__LARGE_FILES */

/* whether the system defaults to 32bit off_t but can do 64bit when requested
   */
#define ZZIP_LARGEFILE_SENSITIVE 1

/* Define to `long' if <sys/types.h> does not define. */
/* #undef _zzip_off_t */

/* Name of package */
#ifndef ZZIP_PACKAGE 
#define ZZIP_PACKAGE  "zziplib" 
#endif

/* The number of bytes in type int */
#ifndef ZZIP_SIZEOF_INT 
#define ZZIP_SIZEOF_INT  4 
#endif

/* The number of bytes in type long */
#ifndef ZZIP_SIZEOF_LONG 
#define ZZIP_SIZEOF_LONG  4 
#endif

/* The number of bytes in type short */
#ifndef ZZIP_SIZEOF_SHORT 
#define ZZIP_SIZEOF_SHORT  2 
#endif

/* Define to `unsigned' if <sys/types.h> does not define. */
 #undef _zzip_size_t

/* Define to `int' if <sys/types.h> does not define. */
 #undef _zzip_ssize_t

/* Define if you have the ANSI C header files. */
#ifndef ZZIP_STDC_HEADERS 
#define ZZIP_STDC_HEADERS  1 
#endif

/* Version number of package */
#ifndef ZZIP_VERSION 
#define ZZIP_VERSION  "0.10.82" 
#endif

/* io-wrap needs to wrap systemcalls */
/* #undef ZZIP_WRAPWRAP */
 
/* once: _ZZIP__CONFIG_H */
#endif
