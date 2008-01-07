/*
 *  milkyplay/drivers/generic/rtaudio/asio/asiosys.h
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

#ifndef __asiosys__
	#define __asiosys__

	#ifdef WIN32
		#undef MAC 
		#define PPC 0
		#define WINDOWS 1
		#define SGI 0
		#define SUN 0
		#define LINUX 0
		#define BEOS 0

		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1
	
	#elif BEOS
		#define MAC 0
		#define PPC 0
		#define WINDOWS 0
		#define PC 0
		#define SGI 0
		#define SUN 0
		#define LINUX 0
		
		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1
		
		#ifndef DEBUG
			#define DEBUG 0
		 	#if DEBUG
		 		void DEBUGGERMESSAGE(char *string);
		 	#else
		  		#define DEBUGGERMESSAGE(a)
			#endif
		#endif

	#elif SGI
		#define MAC 0
		#define PPC 0
		#define WINDOWS 0
		#define PC 0
		#define SUN 0
		#define LINUX 0
		#define BEOS 0
		
		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1
		
		#ifndef DEBUG
			#define DEBUG 0
		 	#if DEBUG
		 		void DEBUGGERMESSAGE(char *string);
		 	#else
		  		#define DEBUGGERMESSAGE(a)
			#endif
		#endif

	#else	// MAC

		#define MAC 1
		#define PPC 1
		#define WINDOWS 0
		#define PC 0
		#define SGI 0
		#define SUN 0
		#define LINUX 0
		#define BEOS 0

		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1

		#ifndef DEBUG
			#define DEBUG 0
			#if DEBUG
				void DEBUGGERMESSAGE(char *string);
			#else
				#define DEBUGGERMESSAGE(a)
			#endif
		#endif
	#endif

#endif
