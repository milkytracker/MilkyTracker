/*
 * unlike in <zzip_conf.h> we are allowed to make up typedefs here,
 * while over there only #defines are allowed
 *
 * Author: 
 *	Guido Draheim <guidod@gmx.de>
 *
 *	Copyright (c) 2003,2004 Guido Draheim
 * 	    All rights reserved, 
 *          usage allowed under the restrictions of the
 *	    Lesser GNU General Public License 
 *          or alternatively the restrictions 
 *          of the Mozilla Public License 1.1
 *
 * if you see "unknown symbol" errors, check first that `-I ..` is part of
 * your compiler options - a special hint to VC/IDE users who tend to make up
 * their own workspace files. All includes look like #include <zzip|*.h>, so
 * you need to add an include path to the dir containing (!!) the ./zzip_ dir
 */

#ifndef _ZZIP_TYPES_H_
#define _ZZIP_TYPES_H_

#include <zzip_conf.h>
#include <fcntl.h>
#include <stddef.h> /* size_t and friends */
#ifdef ZZIP_HAVE_SYS_TYPES_H
#include <sys/types.h> /* bsd (mac) has size_t here */
#endif
/* msvc6 has neither ssize_t (we assume "int") nor off_t (assume "long") */

typedef unsigned char zzip_byte_t; // especially zlib decoding data

typedef       _zzip_off64_t     zzip_off64_t;
typedef       _zzip_off_t       zzip_off_t;
typedef       _zzip_size_t      zzip_size_t;
typedef       _zzip_ssize_t     zzip_ssize_t;

/* in <zzip_format.h> */
typedef struct zzip_disk64_trailer ZZIP_DISK64_TRAILER;
typedef struct zzip_disk_trailer ZZIP_DISK_TRAILER;
typedef struct zzip_file_trailer ZZIP_FILE_TRAILER;
typedef struct zzip_root_dirent  ZZIP_ROOT_DIRENT;
typedef struct zzip_file_header ZZIP_FILE_HEADER;
typedef struct zzip_disk_entry  ZZIP_DISK_ENTRY;
typedef struct zzip_extra_block ZZIP_EXTRA_BLOCK;



#endif

