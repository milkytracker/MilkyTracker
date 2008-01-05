#ifndef __ZZIP_INTERNAL_HINTS_H
#define __ZZIP_INTERNAL_HINTS_H
#include <zzip_conf.h>

#ifndef ZZIP_GNUC_ATLEAST
# if defined __GNUC__ && defined __GNUC_MINOR__
# define ZZIP_GNUC_ATLEAST(_M_,_N_) \
        ((__GNUC__ << 10) + __GNUC_MINOR__ >= ((_M_) << 10) + (_N_))
# elif defined __GNUC__
# define ZZIP_GNUC_ATLEAST(_M_,_N_) \
        ((__GNUC__ << 10) >= ((_M_) << 10))
# else
# define ZZIP_GNUC_ATLEAST(_M_, _N_) 0
# endif
#endif

#ifndef ZZIP_GNUC_EXTENSION
# if ZZIP_GNUC_ATLEAST(2,8)
# define ZZIP_GNUC_EXTENSION __extension__
# else
# define ZZIP_GNUC_EXTENSION 
# endif
#endif

/* func has no side effects, return value depends only on params and globals */
#ifndef ZZIP_GNUC_PURE
# if ZZIP_GNUC_ATLEAST(2,8)
# define ZZIP_GNUC_PURE __attribute__((__pure__))
# else
# define ZZIP_GNUC_PURE
# endif
#endif

/* func has no side effects, return value depends only on params */
#ifndef ZZIP_GNUC_CONST
# if ZZIP_GNUC_ATLEAST(2,4)
# define ZZIP_GNUC_CONST __attribute__((__const__))
# else
# define ZZIP_GNUC_CONST
# endif
#endif

/* typename / variable / function possibly unused */
#ifndef ZZIP_GNUC_UNUSED
# if ZZIP_GNUC_ATLEAST(2,4)
# define ZZIP_GNUC_UNUSED __attribute__((__unused__))
# else
# define ZZIP_GNUC_UNUSED
# endif
#endif

/* obvious. btw, a noreturn-func should return void */
#ifndef ZZIP_GNUC_NORETURN
# if ZZIP_GNUC_ATLEAST(2,5)
# define ZZIP_GNUC_NORETURN __attribute__((__noreturn__))
# else
# define ZZIP_GNUC_NORETURN
# endif
#endif

/* omit function from profiling with -finstrument-functions */
#ifndef ZZIP_GNUC_NO_INSTRUMENT
# if ZZIP_GNUC_ATLEAST(2,4)
# define ZZIP_GNUC_NO_INSTRUMENT __attribute__((__no_instrument_function__))
# else
# define ZZIP_GNUC_NO_INSTRUMENT
# endif
#endif

/* all pointer args must not be null, and allow optimiztons based on the fact*/
#ifndef ZZIP_GNUC_NONNULL
# if ZZIP_GNUC_ATLEAST(3,1)
# define ZZIP_GNUC_NONNULL __attribute__((nonnull))
# else
# define ZZIP_GNUC_NONNULL
# endif
#endif

/* the function can not throw - the libc function are usually nothrow */
#ifndef ZZIP_GNUC_NOTHROW
# if ZZIP_GNUC_ATLEAST(3,2)
# define ZZIP_GNUC_NOTHROW __attribute__((nothrow))
# else
# define ZZIP_GNUC_NOTHROW
# endif
#endif

/* typename / function / variable is obsolete but still listed in headers */
#ifndef ZZIP_GNUC_DEPRECATED
# if ZZIP_GNUC_ATLEAST(3,1)
# define ZZIP_GNUC_DEPRECATED __attribute__((deprecated))
# else
# define ZZIP_GNUC_DEPRECATED
# endif
#endif

/* resolve references to this function during pre-linking the libary */
#ifndef ZZIP_GNUC_LIB_PROTECTED
# if ZZIP_GNUC_ATLEAST(3,1)
# define ZZIP_GNUC_LIB_PROTECTED __attribute__((visiblity("protected")))
# else
# define ZZIP_GNUC_LIB_PROTECTED
# endif
#endif

/* func shall only be usable within the same lib (so, no entry in lib symtab)*/
#ifndef ZZIP_GNUC_LIB_PRIVATE
# if ZZIP_GNUC_ATLEAST(3,1)
# define ZZIP_GNUC_LIB_PRIVATE __attribute__((visiblity("hidden")))
# else
# define ZZIP_GNUC_LIB_PRIVATE
# endif
#endif

/* ... and not even passed as a function pointer reference to outside the lib*/
#ifndef ZZIP_GNUC_LIB_INTERNAL
# if ZZIP_GNUC_ATLEAST(3,1)
# define ZZIP_GNUC_LIB_INTERNAL __attribute__((visiblity("internal")))
# else
# define ZZIP_GNUC_LIB_INTERNAL
# endif
#endif


#ifndef ZZIP_GNUC_FORMAT
# if ZZIP_GNUC_ATLEAST(2,4)
# define ZZIP_GNUC_FORMAT(_X_) __attribute__((__format_arg__(_X_)))
# else
# define ZZIP_GNUC_FORMAT(_X_)
# endif
#endif

#ifndef ZZIP_GNUC_SCANF
# if ZZIP_GNUC_ATLEAST(2,4)
# define ZZIP_GNUC_SCANF(_S_,_X_) __attribute__((__scanf__(_S_,_X_)))
# else
# define ZZIP_GNUC_SCANF(_S_,_X_)
# endif
#endif

#ifndef ZZIP_GNUC_PRINTF
# if ZZIP_GNUC_ATLEAST(2,4)
# define ZZIP_GNUC_PRINTF(_S_,_X_) __attribute__((__printf__(_S_,_X_)))
# else
# define ZZIP_GNUC_PRINTF(_S_,_X_)
# endif
#endif

#ifndef ZZIP_FUNCTION
# if ZZIP_GNUC_ATLEAST(2,6)
# define ZZIP_FUNC             __FUNCTION__
# define ZZIP_FUNCTION         __FUNCTION__
# elif  defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
# define ZZIP_FUNC             __func__
# define ZZIP_FUNCTION         ""
# else   
# define ZZIP_FUNC             0
# define ZZIP_FUNCTION         ""
# endif  
#endif

#ifndef ZZIP_STRING
#define ZZIP_STRING(_X_)   ZZIP_STRING_(_X_)
#define ZZIP_STRING_(_Y_)  #_Y_
#endif

#ifndef ZZIP_DIM
#define ZZIP_DIM(_A_)  (sizeof(_A_) / sizeof ((_A_)[0]))
#endif

#if !(defined ZZIP_FOR1 && defined ZZIP_END1)
# if defined sun || defined __sun__
# define ZZIP_FOR1  if (1)
# define ZZIP_END1  else (void)0
# else
# define ZZIP_FOR1  do
# define ZZIP_END1  while (0)
# endif
#endif

#ifndef ZZIP_BRANCH_OVER
# if ZZIP_GNUC_ATLEAST(2,96)
# define ZZIP_BRANCH_OVER(_X_) __builtin_expect((_X_),0)
# else
# define ZZIP_BRANCH_OVER(_X_) (_X_)
# endif
#endif

#endif
