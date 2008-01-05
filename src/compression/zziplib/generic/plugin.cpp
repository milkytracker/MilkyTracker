/*
 * Author: 
 *	Guido Draheim <guidod@gmx.de>
 *      Mike Nordell <tamlin-@-algonet-se>
 *
 * Copyright (c) 2002,2003 Guido Draheim
 * 	    All rights reserved,
 *	    use under the restrictions of the
 *	    Lesser GNU General Public License
 *          or alternatively the restrictions 
 *          of the Mozilla Public License 1.1
 */

#include <zzip_lib.h>
#include <zzip_plugin.h>

#include "MyIO.h"

#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include <zzip_file.h>
#include <zzip_format.h>

zzip_off_t
zzip_filesize(__zzipfd fd)
{
  struct stat st;

  if (fstat(fd, &st) < 0)
    return -1;

# if defined DEBUG && ! defined _WIN32
  if (! st.st_size && st.st_blocks > 1) /* seen on some darwin 10.1 machines */
      fprintf(stderr, "broken fstat(2) ?? st_size=%ld st_blocks=%ld\n", 
	      (long) st.st_size, (long) st.st_blocks);
# endif

  return st.st_size;
}

static const struct zzip_plugin_io default_io =
{
    &Myopen,
    &Myclose,
    &Myread,
    &Mylseek,
    &Myfsize,
    1, 1,
    /*&_zzip_write*/NULL
};

/** => zzip_init_io
 * This function returns a zzip_plugin_io_t handle to static defaults
 * wrapping the posix io file functions for actual file access.
 */
zzip_plugin_io_t
zzip_get_default_io()
{
    return (zzip_plugin_io_t) &default_io;
}

/**
 * This function initializes the users handler struct to default values 
 * being the posix io functions in default configured environments.
 */
int zzip_init_io(zzip_plugin_io_handlers_t io, int flags)
{
    if (!io) {
        return ZZIP_ERROR;
    }
    memcpy(io, &default_io, sizeof(default_io));
    io->fd.sys = flags;
    return 0;
}

/* 
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
