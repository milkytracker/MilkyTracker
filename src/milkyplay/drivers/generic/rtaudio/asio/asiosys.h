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
