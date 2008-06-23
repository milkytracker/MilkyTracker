/*
 * slidehuf.h -- part of LHa for UNIX
 */
#ifndef __SLIDEHUF_H__
#define __SLIDEHUF_H__

#define CHAR_BIT	8
#define UCHAR_MAX	255
#define USHRT_MAX	65535
#define SHRT_MAX	32767
#define SHRT_MIN (SHRT_MAX-USHRT_MAX)


/* from slide.c */
#define MAX_DICBIT    13 
#define MAX_DICSIZ (1 << MAX_DICBIT)
#define MATCHBIT   8    /* bits for MAXMATCH - THRESHOLD */

/* from huf.c */
#define CBIT 9  /* $\lfloor \log_2 NC \rfloor + 1$ */
#define USHRT_BIT 16	/* (CHAR_BIT * sizeof(ushort)) */

#endif
