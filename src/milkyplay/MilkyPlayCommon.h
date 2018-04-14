/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  MilkyPlayCommon.h
 *  MilkyPlay 
 *
 *  Created by Peter Barth on 23.12.04.
 *
 */
#ifndef __MILKYPLAYCOMMON_H__
#define __MILKYPLAYCOMMON_H__

#include "MilkyPlayTypes.h"
#include "MilkyPlayResults.h"

#if defined WIN32 && !defined _WIN32_WCE
#include <assert.h>
#define ASSERT assert
#endif

#if defined WIN32 || defined _WIN32_WCE
	#include <windows.h>
#elif defined __PSP__
	#include <assert.h>
	#define ASSERT assert

	#include <malloc.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
#else
	#include <assert.h>
	#define ASSERT assert
	
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
#endif

#if defined WIN32 || defined _WIN32_WCE
	typedef TCHAR SYSCHAR;
	typedef HANDLE FHANDLE;
#ifdef __GNUWIN32__
	typedef long long mp_int64;
#else
	typedef __int64 mp_int64;
#endif
#else
	typedef long long mp_int64;
	typedef char SYSCHAR;
	typedef FILE* FHANDLE;
#endif

#ifdef MILKYTRACKER
	#define MP_MAXSAMPLES 256*16
#else
	#define MP_MAXSAMPLES 256
#endif

#define MP_NUMEFFECTS 4

#if (defined(WIN32) || defined(_WIN32_WCE)) && !defined(__FORCE_SDL_AUDIO__)
	#define DRIVER_WIN32
#elif defined(__APPLE__) && !defined(__FORCE_SDL_AUDIO__)
	#define DRIVER_OSX
#elif defined(__MILKYPLAY_UNIX__) && !defined(__FORCE_SDL_AUDIO__)
	#define DRIVER_UNIX
#elif defined(__PSP__) && !defined(__FORCE_SDL_AUDIO__)
	#define DRIVER_PSP
#elif defined(__HAIKU__) && !defined(__FORCE_SDL_AUDIO__)
	#define DRIVER_HAIKU
#else
	#define DRIVER_SDL
#endif

#endif

