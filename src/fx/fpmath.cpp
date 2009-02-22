/*
 *  fx/fpmath.cpp
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

#include "fpmath.h"

#ifdef _MIPS_
extern "C" { // Use extern "c" fopp_int32r C++ file only 
void __asm(char*, ...); 
}; 
#pragma intrinsic(__asm) // this is actually optional 
pp_int32 fpmul(pp_int32 a, pp_int32 b) { 
__asm("mult $4,$5"); 
__asm("mflo $11");
__asm("mfhi $12"); 
__asm("srl $13,$11,16");
__asm("sll $14,$12,16");
__asm("or  $2,$13,$14");
} 

pp_int32 fpdiv(pp_int32 n,pp_int32 d)
{
__asm("addu $10,$5,$0;"
	  "and $15,$10,-2147483648;"
	  "beq $15, 0, $34;"
	  "negu $5,$5;"
	  "$34:;"
	  "addu $10,$5,$0;"
	  "li $11,3;"
	  "and $12,$10,1073741824;"
	  "bne $12,0,$36;"
	  "$35:;"
	  "sll $10,$10,1;"
	  "sll $11,$11,1;"
	  "and $12,$10,1073741824;"
	  "beq $12,0,$35;"
	  "$36:;"
	  "addu $6,$11,$0;");

__asm("mult $5,$6"); 
__asm("mflo $11");
__asm("mfhi $12"); 
__asm("srl $13,$11,16");
__asm("sll $14,$12,16");
__asm("or  $8,$13,$14");

__asm("mult $4,$6"); 
__asm("mflo $11");
__asm("mfhi $12"); 
__asm("srl $13,$11,16");
__asm("sll $14,$12,16");
__asm("or  $9,$13,$14");
__asm("addiu $24,$0,2");
__asm("sll $24,$24,16");
__asm("subu $10,$24,$8");

__asm("mult $10,$8"); 
__asm("mflo $11");
__asm("mfhi $12"); 
__asm("srl $13,$11,16");
__asm("sll $14,$12,16");
__asm("or  $8,$13,$14");

__asm("mult $10,$9"); 
__asm("mflo $11");
__asm("mfhi $12"); 
__asm("srl $13,$11,16");
__asm("sll $14,$12,16");
__asm("or  $9,$13,$14");
__asm("subu $10,$24,$8");

__asm("mult $10,$8"); 
__asm("mflo $11");
__asm("mfhi $12"); 
__asm("srl $13,$11,16");
__asm("sll $14,$12,16");
__asm("or  $8,$13,$14");

__asm("mult $10,$9"); 
__asm("mflo $11");
__asm("mfhi $12"); 
__asm("srl $13,$11,16");
__asm("sll $14,$12,16");
__asm("or  $9,$13,$14");
__asm("subu $10,$24,$8");

__asm("mult $10,$9"); 
__asm("mflo $11");
__asm("mfhi $12"); 
__asm("srl $13,$11,16");
__asm("sll $14,$12,16");
__asm("or  $2,$13,$14");


__asm("beq $15, 0, $37;"
	  "negu $2,$2;"
	  "$37:");
}

#else

pp_int32 fpmul(pp_int32 a,pp_int32 b)
{
	return ((pp_int32)(((pp_int64)(a)*(pp_int64)(b))>>16));
}

pp_int32 fpdiv(pp_int32 n,pp_int32 d)
{
	/*if (d==0)
	{
		return 0;
	}*/
	
	pp_int32 g,t0,t1,t2;
    pp_int32 sign=d&0x80000000;
    if(sign)
        d=-d;

    //we need a good guess to start the Newton-Raphson interation
    //the logic that follows is from a suggestion by    David Seal (dseal@acorn.co.uk) found at:
    //http://www.poppyfields.net/acorn/tech/division.shtml (toward the bottom)

    g=3;
    pp_int32 td=d;
    while((td&0x40000000)==0)
    {
        td<<=1;
        g<<=1;
    }

    //now we have our guess for the modified Newton-Raphson iteration
    //the following iterative convergence algorithm is from:
    //Cavanagh, J. 1984. Digital Computer Arithmetic. McGraw-Hill. Page 284.

    t1=fpmul(g,d);     //t1=g*d
    t2=fpmul(g,n);     //t2=g*n
    t0=0x20000-t1;      //t0=2-t1

    t1=fpmul(t0,t1);   //t1=(2-(g*d))*(g*d)
    t2=fpmul(t0,t2);   //t2=(2-(g*d))*(g*n)
    t0=0x20000-t1;      //t0=2-((2-(g*d))*(g*d))

    t1=fpmul(t0,t1);   //t1=(2-((2-(g*d))*(g*d)))*(2-(g*d))*(g*d)
    t2=fpmul(t0,t2);   //t2=(2-((2-(g*d))*(g*d)))*(2-(g*d))*(g*n)
    t0=0x20000-t1;      //t0=2-((2-((2-(g*d))*(g*d)))*(2-(g*d))*(g*d))

    t0=fpmul(t0,t2);   //t0=(2-((2-((2-(g*d))*(g*d)))*(2-(g*d))*(g*d)))*
                        //   (2-((2-(g*d))*(g*d)))*(2-(g*d))*(g*n)*/
    if(sign)
        t0=-t0;

	//divisionprofilecounter++;

    return t0;
}

#endif

#define step(shift) \
    if((0x40000000l >> shift) + root <= value)          \
    {                                                   \
        value -= (0x40000000l >> shift) + root;         \
        root = (root >> 1) | (0x40000000l >> shift);    \
    }                                                   \
    else                                                \
    {                                                   \
        root = root >> 1;                               \
    }

pp_int32 fpsqrt(pp_int32 value)
{
    pp_int32 root = 0;

    /*step( 0);
    step( 2);
    step( 4);
    step( 6);
    step( 8);
    step(10);
    step(12);
    step(14);
    step(16);
    step(18);
    step(20);
    step(22);
    step(24);
    step(26);
    step(28);
    step(30);*/

	for (pp_int32 i = 0; i <= 30; i+=2)
		step(i);

    // round to the nearest integer, cuts max error in half

    if(root < value)
    {
        ++root;
    }

    root <<= 8;

    return root;
}
