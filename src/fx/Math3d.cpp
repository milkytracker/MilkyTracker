/*
 *  fx/Math3d.cpp
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

#include "Math3d.h"

/////////////////////
// VectorFP struct //
/////////////////////

VectorFP operator*(pp_int32 s,VectorFP v2)
{
	VectorFP v = {fpmul(v2.x,s),fpmul(v2.y,s),fpmul(v2.z,s)};
	return v;
}

VectorFP operator+ (VectorFP v1,VectorFP v2)
{
	VectorFP v = {v1.x+v2.x,v1.y+v2.y,v1.z+v2.z};
	return v;	
}

VectorFP operator- (VectorFP v1,VectorFP v2)
{
	VectorFP v = {v1.x-v2.x,v1.y-v2.y,v1.z-v2.z};
	return v;	
}

VectorFP operator- (VectorFP v)
{
	VectorFP vr = {-v.x,-v.y,-v.z};
	return vr;	
}

pp_int32 operator* (VectorFP v1,VectorFP v2)
{
	return fpmul(v1.x,v2.x)+fpmul(v1.y,v2.y)+fpmul(v1.z,v2.z);
}

// vector product
VectorFP operator ^(VectorFP v1,VectorFP v2)
{
	VectorFP v;
	
	v.x=fpmul(v1.y,v2.z)-fpmul(v1.z,v2.y);
	v.y=fpmul(v1.z,v2.x)-fpmul(v1.x,v2.z);
	v.z=fpmul(v1.x,v2.y)-fpmul(v1.y,v2.x);
	
	return v;
}

///////////////////////
// VectorFloat class //
///////////////////////

VectorFloat::VectorFloat()
{
}

VectorFloat::VectorFloat(float nx,float ny,float nz)
{
	x = nx; y = ny; z = nz;	
}

VectorFloat::VectorFloat(VectorFP vector)
{
	x = vector.x*(1.0f/65536.0f);
	y = vector.y*(1.0f/65536.0f);
	z = vector.z*(1.0f/65536.0f);
}

void VectorFloat::set(float nx,float ny,float nz)
{
	x = nx; y = ny; z = nz;	
}

void VectorFloat::set(VectorFP vector)
{
	x = vector.x*(1.0f/65536.0f);
	y = vector.y*(1.0f/65536.0f);
	z = vector.z*(1.0f/65536.0f);
}

float VectorFloat::length()
{
	return (float)sqrt(x*x+y*y+z*z);
}

void VectorFloat::normalize()
{
	float len = (float)sqrt(x*x+y*y+z*z);
	if (len>0.0f)
	{
		x/=len;
		y/=len;
		z/=len;
	}
}

void VectorFloat::readFromFile(FILE *f)
{
	fread(&x,4,1,f);	
	fread(&y,4,1,f);	
	fread(&z,4,1,f);	
}

VectorFP VectorFloat::convertToFixedPoint()
{
	VectorFP v;
	v.x = (pp_int32 )(x*65536.0f);
	v.y = (pp_int32 )(y*65536.0f);
	v.z = (pp_int32 )(z*65536.0f);
	return v;
}

VectorFloat operator*(float s,VectorFloat v2)
{
	VectorFloat v(v2.x*s,v2.y*s,v2.z*s);
	return v;
}

VectorFloat operator+ (VectorFloat v1,VectorFloat v2)
{
	VectorFloat v(v1.x+v2.x,v1.y+v2.y,v1.z+v2.z);
	return v;	
}

VectorFloat operator- (VectorFloat v1,VectorFloat v2)
{
	VectorFloat v(v1.x-v2.x,v1.y-v2.y,v1.z-v2.z);
	return v;	
}

VectorFloat operator- (VectorFloat v)
{
	VectorFloat vr(-v.x,-v.y,-v.z);
	return vr;	
}

float operator* (VectorFloat v1,VectorFloat v2)
{
	return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z;
}

// vector product
VectorFloat operator ^(VectorFloat v1,VectorFloat v2)
{
	VectorFloat v;
	
	v.x=(v1.y*v2.z)-(v1.z*v2.y);
	v.y=(v1.z*v2.x)-(v1.x*v2.z);
	v.z=(v1.x*v2.y)-(v1.y*v2.x);
	
	return v;
}


///////////////////////
// MatrixFloat class //
///////////////////////

void MatrixFloat::setID()
{
	form[0][0] = 1.0f; form[0][1] = 0.0f; form[0][2] = 0.0f; form[0][3] = 0.0f;
	form[1][0] = 0.0f; form[1][1] = 1.0f; form[1][2] = 0.0f; form[1][3] = 0.0f;
	form[2][0] = 0.0f; form[2][1] = 0.0f; form[2][2] = 1.0f; form[2][3] = 0.0f;
	form[3][0] = 0.0f; form[3][1] = 0.0f; form[3][2] = 0.0f; form[3][3] = 1.0f;
}

void MatrixFloat::setRotX(float phi)
{
	form[0][0] = 1.0f; form[0][1] = 0.0f; form[0][2] = 0.0f; form[0][3] = 0.0f;
	form[1][0] = 0.0f; form[1][1] = (float)cos(phi); form[1][2] = -(float)sin(phi); form[1][3] = 0.0f;
	form[2][0] = 0.0f; form[2][1] = (float)sin(phi); form[2][2] = (float)cos(phi); form[2][3] = 0.0f;
	form[3][0] = 0.0f; form[3][1] = 0.0f; form[3][2] = 0.0f; form[3][3] = 1.0f;
}

void MatrixFloat::setRotY(float phi)
{
	form[0][0] = (float)cos(phi); form[0][1] = 0.0f; form[0][2] = (float)sin(phi); form[0][3] = 0.0f;
	form[1][0] = 0.0f; form[1][1] = 1.0f; form[1][2] = 0.0f; form[1][3] = 0.0f;
	form[2][0] = -(float)sin(phi); form[2][1] = 0.0f; form[2][2] = (float)cos(phi); form[2][3] = 0.0f;
	form[3][0] = 0.0f; form[3][1] = 0.0f; form[3][2] = 0.0f; form[3][3] = 1.0f;
}

void MatrixFloat::setRotZ(float phi)
{
	form[0][0] = (float)cos(phi); form[0][1] = -(float)sin(phi); form[0][2] = 0.0f; form[0][3] = 0.0f;
	form[1][0] = (float)sin(phi); form[1][1] = (float)cos(phi); form[1][2] = 0.0f; form[1][3] = 0.0f;
	form[2][0] = 0.0f; form[2][1] = 0.0f; form[2][2] = 1.0f; form[2][3] = 0.0f;
	form[3][0] = 0.0f; form[3][1] = 0.0f; form[3][2] = 0.0f; form[3][3] = 1.0f;
}

void MatrixFloat::setScale(VectorFloat v)
{
	form[0][0] = v.x; form[0][1] = 0.0f; form[0][2] = 0.0f; form[0][3] = 0.0f;
	form[1][0] = 0.0f; form[1][1] = v.y; form[1][2] = 0.0f; form[1][3] = 0.0f;
	form[2][0] = 0.0f; form[2][1] = 0.0f; form[2][2] = v.z; form[2][3] = 0.0f;
	form[3][0] = 0.0f; form[3][1] = 0.0f; form[3][2] = 0.0f; form[3][3] = 1.0f;
}

void MatrixFloat::setTranslate(VectorFloat v)
{
	form[0][0] = 1.0f; form[0][1] = 0.0f; form[0][2] = 0.0f; form[0][3] = v.x;
	form[1][0] = 0.0f; form[1][1] = 1.0f; form[1][2] = 0.0f; form[1][3] = v.y;
	form[2][0] = 0.0f; form[2][1] = 0.0f; form[2][2] = 1.0f; form[2][3] = v.z;
	form[3][0] = 0.0f; form[3][1] = 0.0f; form[3][2] = 0.0f; form[3][3] = 1.0f;
}

void MatrixFloat::setSubtract(VectorFloat v)
{
	form[0][0] = 1.0f; form[0][1] = 0.0f; form[0][2] = 0.0f; form[0][3] = -v.x;
	form[1][0] = 0.0f; form[1][1] = 1.0f; form[1][2] = 0.0f; form[1][3] = -v.y;
	form[2][0] = 0.0f; form[2][1] = 0.0f; form[2][2] = 1.0f; form[2][3] = -v.z;
	form[3][0] = 0.0f; form[3][1] = 0.0f; form[3][2] = 0.0f; form[3][3] = 1.0f;
}

MatrixFloat operator* (MatrixFloat m1,MatrixFloat m2)
{
	pp_int32 i,j,k;
	
	MatrixFloat mr;
	
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			mr.form[i][j] = 0.0f;
			for (k = 0; k < 4; k++)
				mr.form[i][j] += m1.form[i][k]*m2.form[k][j];
		}
		
	return mr;

}

VectorFloat operator* (MatrixFloat m,VectorFloat v)
{
	VectorFloat vr;
	
	vr.x = (m.form[0][0]*v.x)+(m.form[0][1]*v.y)+(m.form[0][2]*v.z)+(m.form[0][3]*1.0f);
	vr.y = (m.form[1][0]*v.x)+(m.form[1][1]*v.y)+(m.form[1][2]*v.z)+(m.form[1][3]*1.0f);
	vr.z = (m.form[2][0]*v.x)+(m.form[2][1]*v.y)+(m.form[2][2]*v.z)+(m.form[2][3]*1.0f);

	return vr;
}

////////////////////
// MatrixFP class //
////////////////////

// default constructor
MatrixFP::MatrixFP()
{
}

// convert floating point matrix into fixed point matrix
MatrixFP::MatrixFP(MatrixFloat matrix)
{
	for (pp_int32 i = 0; i < 4; i++)
		for (pp_int32 j = 0; j < 4; j++)
			form[i][j] = (pp_int32 )(matrix.form[i][j]*65536.0f);
}

void MatrixFP::setID()
{
	form[0][0] = 65536; form[0][1] = 0; form[0][2] = 0; form[0][3] = 0;
	form[1][0] = 0; form[1][1] = 65536; form[1][2] = 0; form[1][3] = 0;
	form[2][0] = 0; form[2][1] = 0; form[2][2] = 65536; form[2][3] = 0;
	form[3][0] = 0; form[3][1] = 0; form[3][2] = 0; form[3][3] = 65536;
}

void MatrixFP::setRotX(float phi)
{
	form[0][0] = 65536; form[0][1] = 0; form[0][2] = 0; form[0][3] = 0;
	form[1][0] = 0; form[1][1] = (pp_int32 )(cos(phi)*65536.0f); form[1][2] = (pp_int32 )(-sin(phi)*65536.0f); form[1][3] = 0;
	form[2][0] = 0; form[2][1] = (pp_int32 )(sin(phi)*65536.0f); form[2][2] = (pp_int32 )(cos(phi)*65536); form[2][3] = 0;
	form[3][0] = 0; form[3][1] = 0; form[3][2] = 0; form[3][3] = 65536;
}

void MatrixFP::setRotY(float phi)
{
	form[0][0] = (pp_int32 )(cos(phi)*65536.0f); form[0][1] = 0; form[0][2] = (pp_int32 )(sin(phi)*65536.0f); form[0][3] = 0;
	form[1][0] = 0; form[1][1] = 65536; form[1][2] = 0; form[1][3] = 0;
	form[2][0] = (pp_int32 )(-sin(phi)*65536.0f); form[2][1] = 0; form[2][2] = (pp_int32 )(cos(phi)*65536.0f); form[2][3] = 0;
	form[3][0] = 0; form[3][1] = 0; form[3][2] = 0; form[3][3] = 65536;
}

void MatrixFP::setRotZ(float phi)
{
	form[0][0] = (pp_int32 )(cos(phi)*65536.0f); form[0][1] = (pp_int32 )(-sin(phi)*65536.0f); form[0][2] = 0; form[0][3] = 0;
	form[1][0] = (pp_int32 )(sin(phi)*65536.0f); form[1][1] = (pp_int32 )(cos(phi)*65536.0f); form[1][2] = 0; form[1][3] = 0;
	form[2][0] = 0; form[2][1] = 0; form[2][2] = 65536; form[2][3] = 0;
	form[3][0] = 0; form[3][1] = 0; form[3][2] = 0; form[3][3] = 65536;
}

void MatrixFP::setRotXYZ(float phiX,float phiY,float phiZ)
{
	MatrixFP m1,m2,m3,mr;
	m1.setRotX(phiX);
	m2.setRotY(phiY);
	m3.setRotZ(phiZ);
	mr = m2*m1;
	mr = mr*m3;
	memcpy(&form,&mr.form,sizeof(form));
}

void MatrixFP::setScale(VectorFloat v)
{
	form[0][0] = (pp_int32 )(v.x*65536.0f); form[0][1] = 0; form[0][2] = 0; form[0][3] = 0;
	form[1][0] = 0; form[1][1] = (pp_int32 )(v.y*65536.0f); form[1][2] = 0; form[1][3] = 0;
	form[2][0] = 0; form[2][1] = 0; form[2][2] = (pp_int32 )(v.z*65536.0f); form[2][3] = 0;
	form[3][0] = 0; form[3][1] = 0; form[3][2] = 0; form[3][3] = 65536;
}

void MatrixFP::setScale(VectorFP v)
{
	form[0][0] = v.x; form[0][1] = 0; form[0][2] = 0; form[0][3] = 0;
	form[1][0] = 0; form[1][1] = v.y; form[1][2] = 0; form[1][3] = 0;
	form[2][0] = 0; form[2][1] = 0; form[2][2] = v.z; form[2][3] = 0;
	form[3][0] = 0; form[3][1] = 0; form[3][2] = 0; form[3][3] = 65536;
}

void MatrixFP::setTranslate(VectorFP v)
{
	form[0][0] = 65536; form[0][1] = 0; form[0][2] = 0; form[0][3] = v.x;
	form[1][0] = 0; form[1][1] = 65536; form[1][2] = 0; form[1][3] = v.y;
	form[2][0] = 0; form[2][1] = 0; form[2][2] = 65536; form[2][3] = v.z;
	form[3][0] = 0; form[3][1] = 0; form[3][2] = 0; form[3][3] = 65536;
}

void MatrixFP::setSubtract(VectorFP v)
{
	form[0][0] = 65536; form[0][1] = 0; form[0][2] = 0; form[0][3] = -v.x;
	form[1][0] = 0; form[1][1] = 65536; form[1][2] = 0; form[1][3] = -v.y;
	form[2][0] = 0; form[2][1] = 0; form[2][2] = 65536; form[2][3] = -v.z;
	form[3][0] = 0; form[3][1] = 0; form[3][2] = 0; form[3][3] = 65536;
}

void MatrixFP::stripTranslation()
{
	form[0][3] = 0;
	form[1][3] = 0;
	form[2][3] = 0;
	form[3][3] = 65536;

	form[3][0] = 0;
	form[3][1] = 0;
	form[3][2] = 0;
}
MatrixFP operator* (MatrixFP &m1,MatrixFP &m2)
{
	pp_int32 i,j,k;
	
	MatrixFP mr;
	
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			mr.form[i][j] = 0;
			for (k = 0; k < 4; k++)
				mr.form[i][j] += fpmul(m1.form[i][k],m2.form[k][j]);
		}
		
	return mr;

}

VectorFP operator* (MatrixFP &m,VectorFP &v)
{
	VectorFP vr;
	
	vr.x = fpmul(m.form[0][0],v.x)+fpmul(m.form[0][1],v.y)+fpmul(m.form[0][2],v.z)+m.form[0][3];
	vr.y = fpmul(m.form[1][0],v.x)+fpmul(m.form[1][1],v.y)+fpmul(m.form[1][2],v.z)+m.form[1][3];
	vr.z = fpmul(m.form[2][0],v.x)+fpmul(m.form[2][1],v.y)+fpmul(m.form[2][2],v.z)+m.form[2][3];

	return vr;
}

////////////////////////////////
// floating point quaternions //
////////////////////////////////
void QuaternionFloat::normalize()
{
	float len1 = (w*w)+(v.x*v.x)+(v.y*v.y)+(v.z*v.z);
	
	len1 = 1.0f/(float)sqrt(len1);
	
	w*=len1;
	v.x*=len1;
	v.y*=len1;
	v.z*=len1;
}

QuaternionFP QuaternionFloat::convertToFixedPoint()
{
	QuaternionFP q;

	q.v = v.convertToFixedPoint();
	q.w = (pp_int32 )(w*65536.0f);

	return q;
}

QuaternionFloat operator *(QuaternionFloat q1,float s)
{
	QuaternionFloat q;
	
	q.w=s*q1.w;
	q.v.x=s*q1.v.x;
	q.v.y=s*q1.v.y;
	q.v.z=s*q1.v.z;
	
	return q;
};

QuaternionFloat operator +(QuaternionFloat q1,QuaternionFloat q2)
{
	QuaternionFloat q;
	
	q.w=q1.w+q2.w;
	q.v.x=q1.v.x+q2.v.x;
	q.v.y=q1.v.y+q2.v.y;
	q.v.z=q1.v.z+q2.v.z;
	
	return q;
};

QuaternionFloat operator -(QuaternionFloat q1,QuaternionFloat q2)
{
	QuaternionFloat q;
	
	q.w=q1.w-q2.w;
	q.v.x=q1.v.x-q2.v.x;
	q.v.y=q1.v.y-q2.v.y;
	q.v.z=q1.v.z-q2.v.z;
	
	return q;
}

QuaternionFloat operator *(QuaternionFloat q1,QuaternionFloat q2)
{
	QuaternionFloat q;
	
	q.w=(q1.w*q2.w)-(q1.v*q2.v);
	
	q.v=(q1.w*q2.v)+(q2.w*q1.v)+(q1.v^q2.v);
	
	return q;
}

MatrixFP quaternionToMatrixFP(QuaternionFloat q)
{
	MatrixFP m;
	
	m.form[0][0]=(pp_int32 )((1.0f-(2.0f*q.v.y*q.v.y)-(2.0f*q.v.z*q.v.z))*65536.0f);
	m.form[0][1]=(pp_int32 )((2.0f*q.v.x*q.v.y+2.0f*q.w*q.v.z)*65536.0f);
	m.form[0][2]=(pp_int32 )((2.0f*q.v.x*q.v.z-2.0f*q.w*q.v.y)*65536.0f);
	m.form[0][3]=0;
	
	m.form[1][0]=(pp_int32 )((2.0f*q.v.x*q.v.y-2.0f*q.w*q.v.z)*65536.0f);
	m.form[1][1]=(pp_int32 )((1.0f-(2.0f*q.v.x*q.v.x)-(2.0f*q.v.z*q.v.z))*65536.0f);
	m.form[1][2]=(pp_int32 )((2.0f*q.v.y*q.v.z+2.0f*q.w*q.v.x)*65536.0f);
	m.form[1][3]=0;
	
	m.form[2][0]=(pp_int32 )((2.0f*q.v.x*q.v.z+2.0f*q.w*q.v.y)*65536.0f);
	m.form[2][1]=(pp_int32 )((2.0f*q.v.y*q.v.z-2.0f*q.w*q.v.x)*65536.0f);
	m.form[2][2]=(pp_int32 )((1.0f-(2.0f*q.v.x*q.v.x)-(2.0f*q.v.y*q.v.y))*65536.0f);
	m.form[2][3]=0;
	
	m.form[3][0] = m.form[3][1] = m.form[3][2] = 0; m.form[3][3] = 65536;
	
	return m;
}

/////////////////////////////
// Fixed Point Quaternions //
/////////////////////////////
QuaternionFP operator *(QuaternionFP &q1,pp_int32 s)
{
	QuaternionFP q;
	
	q.w=fpmul(s,q1.w);
	q.v.x=fpmul(s,q1.v.x);
	q.v.y=fpmul(s,q1.v.y);
	q.v.z=fpmul(s,q1.v.z);
	
	return q;
};

QuaternionFP operator +(QuaternionFP &q1,QuaternionFP &q2)
{
	QuaternionFP q;
	
	q.w=q1.w+q2.w;
	q.v.x=q1.v.x+q2.v.x;
	q.v.y=q1.v.y+q2.v.y;
	q.v.z=q1.v.z+q2.v.z;
	
	return q;
};

QuaternionFP operator -(QuaternionFP &q1,QuaternionFP &q2)
{
	QuaternionFP q;
	
	q.w=q1.w-q2.w;
	q.v.x=q1.v.x-q2.v.x;
	q.v.y=q1.v.y-q2.v.y;
	q.v.z=q1.v.z-q2.v.z;
	
	return q;
}

QuaternionFP operator *(QuaternionFP &q1,QuaternionFP &q2)
{
	QuaternionFP q;
	
	q.w=fpmul(q1.w,q2.w)-q1.v*q2.v;
	
	q.v=(q1.w*q2.v)+(q2.w*q1.v)+(q1.v^q2.v);
	
	return q;
}

MatrixFP quaternionToMatrixFP(QuaternionFP &q)
{
	MatrixFP m;
	
	m.form[0][0]=(pp_int32 )((65536-(2*fpmul(q.v.y,q.v.y))-(2*fpmul(q.v.z,q.v.z))));
	m.form[0][1]=(pp_int32 )((2*fpmul(q.v.x,q.v.y)+2*fpmul(q.w,q.v.z)));
	m.form[0][2]=(pp_int32 )((2*fpmul(q.v.x,q.v.z)-2*fpmul(q.w,q.v.y)));
	m.form[0][3]=0;
	
	m.form[1][0]=(pp_int32 )((2*fpmul(q.v.x,q.v.y)-2*fpmul(q.w,q.v.z)));
	m.form[1][1]=(pp_int32 )((65536-(2*fpmul(q.v.x,q.v.x))-(2*fpmul(q.v.z,q.v.z))));
	m.form[1][2]=(pp_int32 )((2*fpmul(q.v.y,q.v.z)+2*fpmul(q.w,q.v.x)));
	m.form[1][3]=0;
	
	m.form[2][0]=(pp_int32 )((2*fpmul(q.v.x,q.v.z)+2*fpmul(q.w,q.v.y)));
	m.form[2][1]=(pp_int32 )((2*fpmul(q.v.y,q.v.z)-2*fpmul(q.w,q.v.x)));
	m.form[2][2]=(pp_int32 )((65536-(2*fpmul(q.v.x,q.v.x))-(2*fpmul(q.v.y,q.v.y))));
	m.form[2][3]=0;
	
	m.form[3][0] = m.form[3][1] = m.form[3][2] = 0; m.form[3][3] = 65536;
	
	return m;
}
