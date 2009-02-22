/*
 *  fx/fpmath.h
 *
 *  Copyright 2009 Peter Barth
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
