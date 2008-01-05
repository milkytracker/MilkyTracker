#ifndef FPMATH_H
#define FPMATH_H

#include "BasicTypes.h"

#define fpceil(x) ((x+65535)>>16)

// 16.16 fixed point multiply
pp_int32 fpmul(pp_int32 a, pp_int32 b);

// 16.16 fixed point division
pp_int32 fpdiv(pp_int32 n, pp_int32 d);

// 16.16 fixed point squareroot
pp_int32 fpsqrt(pp_int32 value);

#endif
