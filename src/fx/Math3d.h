/*
 *  fx/Math3d.h
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

#ifndef MATH3D_H
#define MATH3D_H

// because of "FILE"
#include <stdio.h>
#include <math.h>
#include "fpmath.h"

#ifndef M_PI
#define M_PI 3.1415
#endif

struct VectorFP
{
	pp_int32 x,y,z;
};

#define setVectorFP(v,x,y,z) \
	v.x = x; \
	v.y = y; \
	v.z = z;

VectorFP operator*(pp_int32  s,VectorFP v2);
VectorFP operator+ (VectorFP v1,VectorFP v2);
VectorFP operator- (VectorFP v1,VectorFP v2);
VectorFP operator- (VectorFP v);
pp_int32  operator* (VectorFP v1,VectorFP v2);

// floating point based 3-dimensional vector class
class VectorFloat
{
public:
	float		x,y,z;
				
				VectorFloat();
				VectorFloat(float x,float y,float z);
				VectorFloat(VectorFP vector);

	void		set(float nx,float ny,float nz);
	void		set(VectorFP vector);

	float		length();
	void		normalize();

	VectorFP	convertToFixedPoint();

	void		readFromFile(FILE *f);
};

// multiply vector with scalar
// result is a new vector
VectorFloat operator* (float s,VectorFloat v2);

// add two vectors
// result is a new vector
VectorFloat operator+ (VectorFloat v1,VectorFloat v2);

// subtract two vectors
// result is a new vector
VectorFloat operator- (VectorFloat v1,VectorFloat v2);

// dot product of two vectors
// result is a scalar
float		operator* (VectorFloat v1,VectorFloat v2);

// vector product of two vectors
// result is a vector
VectorFloat operator^ (VectorFloat v1,VectorFloat v2);


// floating point based 4x4 matrix class
class MatrixFloat
{
public:
	float   form[4][4];
	
	void	setID();
	void	setRotX(float phi);
	void	setRotY(float phi);
	void	setRotZ(float phi);
	void	setScale(VectorFloat v);
	void	setTranslate(VectorFloat v);
	void	setSubtract(VectorFloat v);
};

// "multiply" two matrices
// result is a new 4x4 matrix
MatrixFloat operator* (MatrixFloat m1,MatrixFloat m2);

// transform vector by matrix
// result is a new vector
VectorFloat operator* (MatrixFloat m,VectorFloat v);


// fixed point based 4x4 matrix class
class MatrixFP
{
public:
	pp_int32 		form[4][4];
	
			MatrixFP();
			MatrixFP(MatrixFloat matrix);

	void	setID();
	void	setRotX(float phi);
	void	setRotY(float phi);
	void	setRotZ(float phi);
	void	setRotXYZ(float phiX,float phiY,float phiZ);
	void	setScale(VectorFloat v);
	void	setScale(VectorFP v);
	void	setTranslate(VectorFP v);
	void	setSubtract(VectorFP v);
	void	stripTranslation();
};

// "multiply" two matrices
// result is a new 4x4 matrix
MatrixFP operator* (MatrixFP &m1,MatrixFP &m2);

// transform fixed point vector by matrix
// result is a new vector
VectorFP operator* (MatrixFP &m,VectorFP &v);


struct QuaternionFP
{
	pp_int32 			w;
	VectorFP	v;
};

class QuaternionFloat
{
public:
	float		w;
	VectorFloat	v;

	void			normalize();

	QuaternionFP	convertToFixedPoint();
};

QuaternionFloat operator *(QuaternionFloat q1,float s);
QuaternionFloat operator +(QuaternionFloat q1,QuaternionFloat q2);
QuaternionFloat operator -(QuaternionFloat q1,QuaternionFloat q2);
QuaternionFloat operator *(QuaternionFloat q1,QuaternionFloat q2);

MatrixFP quaternionToMatrixFP(QuaternionFloat q);


#define Quaternion QuaternionFP

Quaternion normalize(Quaternion &q1);

Quaternion operator *(Quaternion &q1,pp_int32  s);
Quaternion operator +(Quaternion &q1,Quaternion &q2);
Quaternion operator -(Quaternion &q1,Quaternion &q2);
Quaternion operator *(Quaternion &q1,Quaternion &q2);

MatrixFP quaternionToMatrixFP(Quaternion &q);

#endif
