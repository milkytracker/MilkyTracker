/*
 *  milkyplay/drivers/generic/rtaudio/asio/ginclude.h
 *
 *  Copyright 2008 Peter Barth
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

#ifndef __gInclude__
#define __gInclude__

#if SGI 
	#undef BEOS 
	#undef MAC 
	#undef WINDOWS
	//
	#define ASIO_BIG_ENDIAN 1
	#define ASIO_CPU_MIPS 1
#elif defined WIN32
	#undef BEOS 
	#undef MAC 
	#undef SGI
	#define WINDOWS 1
	#define ASIO_LITTLE_ENDIAN 1
	#define ASIO_CPU_X86 1
#elif BEOS
	#undef MAC 
	#undef SGI
	#undef WINDOWS
	#define ASIO_LITTLE_ENDIAN 1
	#define ASIO_CPU_X86 1
	//
#else
	#define MAC 1
	#undef BEOS 
	#undef WINDOWS
	#undef SGI
	#define ASIO_BIG_ENDIAN 1
	#define ASIO_CPU_PPC 1
#endif

// always
#define NATIVE_INT64 0
#define IEEE754_64FLOAT 1

#endif	// __gInclude__
