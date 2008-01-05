#ifndef _ZZIP_AUTOCONF_H_
#define _ZZIP_AUTOCONF_H_ 1

/*
 * This file is trying to override configure time checks of zzip with
 * definitions at compile time. This is not used by zzip sources themselves
 * but it may be really helpful with thirdparty software that happens to
 * include zzip headers from a central place but running on a different host.
 */

#include "conf.h" /* <zzip_conf.h> : <zzip__config.h> */

#if   defined HAVE_ENDIAN_H          || defined ZZIP_HAVE_ENDIAN_H
#include <endian.h>     /* glibc */
#elif defined HAVE_SYS_PARAM_H       || defined ZZIP_HAVE_SYS_PARAM_H
#include <sys/param.h>  /* solaris */
#endif

#if             defined __BYTE_ORDER
#define ZZIP_BYTE_ORDER __BYTE_ORDER
#elif           defined BYTE_ORDER
#define ZZIP_BYTE_ORDER BYTE_ORDER
#elif           defined _LITTLE_ENDIAN
#define ZZIP_BYTE_ORDER 1234
#elif           defined _BIG_ENDIAN
#define ZZIP_BYTE_ORDER 4321
#elif           defined __i386__
#define ZZIP_BYTE_ORDER 1234
#elif           defined WORDS_BIGENDIAN || defined ZZIP_WORDS_BIGENDIAN
#define ZZIP_BYTE_ORDER 4321
#else
#define ZZIP_BYTE_ORDER 1234
#endif

/* override ZZIP_WORDS_BIGENDIAN : macros ZZIP_GET16 / ZZIP_GET32 */ 
#ifdef ZZIP_BYTE_ORDER+0 == 1234
#undef ZZIP_WORDS_BIGENDIAN
#endif
#ifdef ZZIP_BYTE_ORDER+0 == 4321
#ifndef ZZIP_WORDS_BIGENDIAN
#define ZZIP_WORDS_BIGENDIAN 1
#endif
#endif

#endif
