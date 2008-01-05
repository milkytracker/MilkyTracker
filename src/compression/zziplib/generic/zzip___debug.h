#ifndef __ZZIP_INTERNAL_DEBUG_H
#define __ZZIP_INTERNAL_DEBUG_H

#include <zzip_conf.h>
#include <zzip___hints.h>

/* perhaps want to show on syslog(3) ?? */

#ifdef DEBUG
#include <stdio.h>
#define DBG1(X1) ZZIP_FOR1 { \
        fprintf(stderr,"\n%s:%i:"X1"\n", ZZIP_FUNC,__LINE__ \
	    ); } ZZIP_END1
#define DBG2(X1,X2) ZZIP_FOR1 { \
        fprintf(stderr,"\n%s:%i:"X1"\n", ZZIP_FUNC,__LINE__ \
		,X2);} ZZIP_END1
#define DBG3(X1,X2,X3) ZZIP_FOR1 { \
        fprintf(stderr,"\n%s:%i:"X1"\n", ZZIP_FUNC,__LINE__ \
		 ,X2,X3); } ZZIP_END1
#define DBG4(X1,X2,X3,X4)   ZZIP_FOR1 { \
        fprintf(stderr,"\n%s:%i:"X1"\n", ZZIP_FUNC,__LINE__ \
		 ,X2,X3,X4); } ZZIP_END1
#define DBG5(X1,X2,X3,X4,X5)   ZZIP_FOR1 { \
        fprintf(stderr,"\n%s:%i:"X1"\n", ZZIP_FUNC,__LINE__ \
		 ,X2,X3,X4,X5); } ZZIP_END1
#define DBG6(X1,X2,X3,X4,X5,X6)   ZZIP_FOR1 { \
        fprintf(stderr,"\n%s:%i:"X1"\n", ZZIP_FUNC,__LINE__ \
		 ,X2,X3,X4,X5,X6); } ZZIP_END1

#else
#define DBG1(X1) {}
#define DBG2(X1,X2) {}
#define DBG3(X1,X2,X3) {}
#define DBG4(X1,X2,X3,X4) {}
#define DBG5(X1,X2,X3,X4,X5) {}
#define DBG6(X1,X2,X3,X4,X5,X6) {}
#endif

#define HINT1(X1)                     DBG1("HINT: " X1) 
#define HINT2(X1,X2)                  DBG2("HINT: " X1,X2) 
#define HINT3(X1,X2,X3)               DBG3("HINT: " X1,X2,X3) 
#define HINT4(X1,X2,X3,X4)            DBG4("HINT: " X1,X2,X3,X4) 
#define HINT5(X1,X2,X3,X4,X5)         DBG5("HINT: " X1,X2,X3,X4,X5) 
#define HINT6(X1,X2,X3,X4,X5,X6)      DBG6("HINT: " X1,X2,X3,X4,X5,X6) 

#define NOTE1(X1)                     DBG1("NOTE: " X1) 
#define NOTE2(X1,X2)                  DBG2("NOTE: " X1,X2) 
#define NOTE3(X1,X2,X3)               DBG3("NOTE: " X1,X2,X3) 
#define NOTE4(X1,X2,X3,X4)            DBG4("NOTE: " X1,X2,X3,X4) 
#define NOTE5(X1,X2,X3,X4,X5)         DBG5("NOTE: " X1,X2,X3,X4,X5) 
#define NOTE6(X1,X2,X3,X4,X5,X6)      DBG6("NOTE: " X1,X2,X3,X4,X5,X6) 

#define WARN1(X1)                     DBG1("WARN: " X1) 
#define WARN2(X1,X2)                  DBG2("WARN: " X1,X2) 
#define WARN3(X1,X2,X3)               DBG3("WARN: " X1,X2,X3) 
#define WARN4(X1,X2,X3,X4)            DBG4("WARN: " X1,X2,X3,X4) 
#define WARN5(X1,X2,X3,X4,X5)         DBG5("WARN: " X1,X2,X3,X4,X5) 
#define WARN6(X1,X2,X3,X4,X5,X6)      DBG6("WARN: " X1,X2,X3,X4,X5,X6) 

#define FAIL1(X1)                     DBG1("FAIL: " X1) 
#define FAIL2(X1,X2)                  DBG2("FAIL: " X1,X2) 
#define FAIL3(X1,X2,X3)               DBG3("FAIL: " X1,X2,X3) 
#define FAIL4(X1,X2,X3,X4)            DBG4("FAIL: " X1,X2,X3,X4) 
#define FAIL5(X1,X2,X3,X4,X5)         DBG5("FAIL: " X1,X2,X3,X4,X5) 
#define FAIL6(X1,X2,X3,X4,X5,X6)      DBG6("FAIL: " X1,X2,X3,X4,X5,X6) 



#ifdef DEBUG
_zzip_inline static void zzip_debug_xbuf (unsigned char* p, int l)
    /* ZZIP_GNUC_UNUSED */
{
#   define q(a) ((a&0x7F)<32?32:(a&0x7F))
    while (l > 0)
    {
        fprintf (stderr, 
		 "%02x %02x %02x %02x  "
		 "%02x %02x %02x %02x  "
		 "%c%c%c%c %c%c%c%c\n",
		 p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
		 q(p[0]), q(p[1]), q(p[2]), q(p[3]), 
		 q(p[4]), q(p[5]), q(p[6]), q(p[7]));
        p += 8; l -= 8;
    }
#   undef q
}
#endif

#endif
