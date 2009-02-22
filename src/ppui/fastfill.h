/*
 *  ppui/fastfill.h
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

/*
 *  fastfill.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.12.07.
 *
 */

#ifdef __GNUC__
static __attribute__((noinline)) void fill_dword(pp_uint32* buff, pp_uint32 dw, pp_uint32 len)
#else
static inline void fill_dword(pp_uint32* buff, pp_uint32 dw, pp_uint32 len)
#endif
{
#if defined(__ppc__) && defined(__GNUC__)
	// PPC assembly FTW!!1!
	// r3 = buff
	// r4 = dw
	// r5 = len
	asm volatile("li r9, 0\n"
				 "srawi r10, r5, 2\n"
				 "nop\n"				// align loop start to 16 byte boundary
				 "cmpw cr7,r10,r9\n"
				 "nop\n"				// see above
				 "beq cr7,$+36\n"
				 "2:\n"
				 "stw r4,0(r3)\n"
				 "stw r4,4(r3)\n"
				 "stw r4,8(r3)\n"
				 "stw r4,12(r3)\n"
				 "addi r10,r10,-1\n"
				 "addi r3,r3,16\n"		// advance by 16
				 "cmpw cr7,r10,r9\n"
				 "bne cr7,2b\n"				 				
				 "clrlwi r11, r5, 30\n"
				 "nop\n"				// align loop start to 16 byte boundary
				 "cmpw cr7,r11,r9\n"
				 "beq cr7,$+24\n"
				 "1:\n"
				 "stw r4,0(r3)\n"
				 "addi r11,r11,-1\n"
				 "addi r3,r3,4\n"		// advance by 4
				 "cmpw cr7,r11,r9\n"
				 "bne cr7,1b"); 
#else
	pp_uint32 newlen = len >> 2;
	pp_uint32 remlen = len & 3;
	if (newlen)
	{
		do
		{
			*buff = dw;
			*(buff+1) = dw;
			*(buff+2) = dw;
			*(buff+3) = dw;
			buff+=4;
		} while (--newlen);
	}
	if (remlen)
	{
		do
		{
			*buff++ = dw;
		} while (--remlen);
	}
#endif
}

#ifdef __GNUC__
static __attribute__((noinline)) void fill_dword_vertical(pp_uint32* buff, pp_uint32 dw, pp_uint32 len, pp_uint32 pitch)
#else
static inline void fill_dword_vertical(pp_uint32* buff, pp_uint32 dw, pp_uint32 len, pp_uint32 pitch)
#endif
{
#if defined(__ppc__) && defined(__GNUC__)
	asm volatile("nop\n" // align loop start to 16 byte boundary
				 "nop\n" // same
				 "nop\n" // same
				 "li r9,0\n"
				 "1:\n"
				 "stw r4,0(r3)\n"
				 "addi r5,r5,-1\n"
				 "add r3,r3,r6\n"
				 "cmpw cr7,r5,r9\n"
				 "bne cr7,1b"); 
#else
	do
	{
		*buff = dw;
		buff+=(pitch>>2);
	} while (--len);
#endif
}
