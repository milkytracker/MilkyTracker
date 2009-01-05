/*
 *  MyIO.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.12.07.
 *
 */

#ifndef __MYIO_H__
#define __MYIO_H__

#include "zzip_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

__zzipfd		Myopen(const zzip_char_t* name, int flags, ...);
int				Myclose(__zzipfd fd);
zzip_ssize_t	Myread(__zzipfd fd, void *buffer, zzip_size_t count);
zzip_off_t		Mylseek(__zzipfd fd, zzip_off_t offset, int origin);
zzip_off_t		Myfsize(__zzipfd fd);

#ifdef __cplusplus
};
#endif

#endif
