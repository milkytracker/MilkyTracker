#ifndef TMM_GLOBALS_H
#define TMM_GLOBALS_H
#pragma once

#define _USE_MATH_DEFINES

#include <time.h>
#include <math.h>

#ifdef __AMIGA__
#   define M_PI	        3.14159265358979323846
#   define M_SQRT2      1.41421356237309504880
#   define M_SQRT1_2    0.70710678118654752440
#endif

// For usage in my demo engine
#if defined(P_AMIGA)
#   include <math-68881.h>
#endif

#endif /* TMM_GLOBALS_H */
