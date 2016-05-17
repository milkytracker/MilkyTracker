/*
 *  MyIO.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.12.07.
 *
 */

#ifndef __MYIO_H__
#define __MYIO_H__

#include <zzip/plugin.h>

#ifdef __cplusplus
extern "C" {
#endif

int     		Myopen(const zzip_char_t* name, int flags, ...);
int				Myclose(int fd);
zzip_ssize_t	Myread(int fd, void *buffer, zzip_size_t count);
zzip_off_t		Mylseek(int fd, zzip_off_t offset, int origin);
zzip_off_t		Myfsize(int fd);

#ifdef __cplusplus
};
#endif

#endif
