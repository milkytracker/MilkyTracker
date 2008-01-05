/*
 * Author: 
 *	Guido Draheim <guidod@gmx.de>
 *
 * Copyright (c) 2002,2003 Guido Draheim
 * 	    All rights reserved
 *	    use under the restrictions of the
 *	    Lesser GNU General Public License
 *          or alternatively the restrictions 
 *          of the Mozilla Public License 1.1
 *
 *  the interfaces for the plugin_io system
 *
 * Using the following you can provide your own file I/O functions to
 * e.g. read data directly from memory, provide simple
 * "encryption"/"decryption" of on-disk .zip-files...
 * Note that this currently only provides a subset of the functionality
 * in zziplib. It does not attempt to provide any directory functions,
 * but if your program 1) only uses ordinary on-disk files and you
 * just want this for file obfuscation, or 2) you only access your
 * .zip archives using zzip_open & co., this is sufficient.
 *
 * Currently the default io are the POSIX functions, except
 * for 'filesize' that is zziplibs own provided zzip_filesize function,
 * using standard POSIX fd's. You are however free to replace this with
 * whatever data type you need, so long as you provide implementations
 * for all the functions, and the data type fits an int.
 *
 * all functions receiving ext_io are able to cope with both arguments
 * set to zero which will let them default to a ZIP ext and posix io.
 */
#ifndef _ZZIP_PLUGIN_H /* zzip-io.h */
#define _ZZIP_PLUGIN_H 1

#include <zzip.h>

#ifdef __cplusplus
extern "C" {
#endif

/* we have renamed zzip_plugin_io.use_mmap to zzip_plugin_io.sys */
#define ZZIP_PLUGIN_IO_SYS 1

struct zzip_plugin_io { /* use "zzip_plugin_io_handlers" in applications !! */
    size_t       (*open)(zzip_char_t* name, int flags, ...);
    int          (*close)(__zzipfd fd);
    zzip_ssize_t (*read)(__zzipfd fd, void* buf, zzip_size_t len);
    zzip_off_t   (*seeks)(__zzipfd fd, zzip_off_t offset, int whence);
    zzip_off_t   (*filesize)(__zzipfd fd);
    long         sys;
    long         type;
    zzip_ssize_t (*write)(__zzipfd fd, _zzip_const void* buf, zzip_size_t len);
};

typedef union _zzip_plugin_io
{
    struct zzip_plugin_io fd;
    struct { void* padding[8]; } ptr;
} zzip_plugin_io_handlers;

#define _zzip_plugin_io_handlers zzip_plugin_io_handlers
/* for backward compatibility, and the following to your application code:
 * #ifndef _zzip_plugin_io_handlers
 * #define _zzip_plugin_io_handlers struct zzip_plugin_io
 */
typedef zzip_plugin_io_handlers* zzip_plugin_io_handlers_t;

#ifdef ZZIP_LARGEFILE_RENAME
#define zzip_filesize        zzip_filesize64
#define zzip_get_default_io  zzip_get_default_io64
#define zzip_init_io         zzip_init_io64
#endif

_zzip_export zzip_off_t
zzip_filesize(__zzipfd fd);

/* get the default file I/O functions */
_zzip_export zzip_plugin_io_t zzip_get_default_io(void);

/*
 * Initializes a zzip_plugin_io_t to the zziplib default io.
 * This is useful if you only want to override e.g. the 'read' function.
 * all zzip functions that can receive a zzip_plugin_io_t can
 * handle a zero pointer in that place and default to posix io.
 */
_zzip_export
int zzip_init_io(zzip_plugin_io_handlers_t io, int flags);

/* zzip_init_io flags : */
# define ZZIP_IO_USE_MMAP 1

#ifdef __cplusplus
};
#endif

#endif
