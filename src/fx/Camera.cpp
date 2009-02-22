/*
 *  fx/Camera.cpp
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

#include "Camera.h"

Camera::Camera()
{
	position.x = position.y = position.z = 0;
	target.x = target.y = target.z = 0;

	bank = lens = 0.0f;

	trackPositionMatrix.setID();
	trackTargetMatrix.setID();
}

Camera::Camera(VectorFP& pos, VectorFP& target)
{
	trackPositionMatrix.setID();
	trackTargetMatrix.setID();

	bank = lens = 0.0f;
	
	position = pos;
	target = target;
}

////////////
// Camera //
////////////
static float getAngle(VectorFloat vec1,VectorFloat vec2)
{
	float cosPhi = (vec1*vec2)/(vec1.length()*vec2.length());
	
	if (vec1.y>=vec2.y)
		return (float)acos(cosPhi);
	else
		return -(float)acos(cosPhi);
}

MatrixFP Camera::getMatrix()
{
	VectorFloat target,target2,veca,vecb;
	float rotangle,rotx,roty,rotz;
	MatrixFP cm;

	// 3ds style vectors
	/*cm.form[0][0] = 65536; cm.form[0][1] = 0; cm.form[0][2] = 0; cm.form[0][3] = 0;
	cm.form[1][0] = 0; cm.form[1][1] = 0; cm.form[1][2] = 65536; cm.form[1][3] = 0;
	cm.form[2][0] = 0; cm.form[2][1] = 65536; cm.form[2][2] = 0; cm.form[2][3] = 0;
	cm.form[3][0] = 0; cm.form[3][1] = 0; cm.form[3][2] = 0; cm.form[3][3] = 65536;*/

	cm.setID();

	MatrixFP im = (cm*trackPositionMatrix);
	VectorFP pos=im*this->position;

	im = (cm*trackTargetMatrix);
	VectorFP targ=im*this->target;

	veca.x=1.0f;
	veca.y=0.0f;
	veca.z=0.0f;

	vecb.x=(targ.z-pos.z)/65536.0f;
	vecb.y=(targ.x-pos.x)/65536.0f;
	vecb.z=0.0f;
	roty=getAngle(veca,vecb);

	target.x=(targ.x-pos.x)/65536.0f;
	target.y=(targ.z-pos.z)/65536.0f;
	rotangle=-roty;

	//target2.x=(target.x*(float)cos(rotangle))-(target.y*(float)sin(rotangle));
	target2.y=(target.x*(float)sin(rotangle))+(target.y*(float)cos(rotangle));

	vecb.x=target2.y;
	vecb.y=(targ.y-pos.y)/65536.0f;
	vecb.z=0.0f;
	rotx=-getAngle(veca,vecb);

	rotz=(float)-(bank/180.0f*M_PI);
	
	MatrixFP rotX,rotY,rotZ,tm;
	rotX.setRotX(rotx);
	rotY.setRotY(roty);
	rotZ.setRotZ(rotz);

	tm.setSubtract(pos);

	/*MatrixFP m = rotZ*rotX*rotY;

	rotX.setRotX(-rotx);
	rotY.setRotY(-roty);
	rotZ.setRotZ(-rotz);
	
	MatrixFP m2 = rotY*rotX*rotZ;

	MatrixFP m3 = m*m2;*/

	im = tm*cm;
	im = rotY*im;
	im = rotX*im;
	im = rotZ*im;
	return im;
}

MatrixFP Camera::getMatrixInverse()
{
	VectorFloat target,target2,veca,vecb;
	float rotangle,rotx,roty,rotz;
	MatrixFP cm;

	// .3ds style vectors
	/*cm.form[0][0] = 65536; cm.form[0][1] = 0; cm.form[0][2] = 0; cm.form[0][3] = 0;
	cm.form[1][0] = 0; cm.form[1][1] = 0; cm.form[1][2] = 65536; cm.form[1][3] = 0;
	cm.form[2][0] = 0; cm.form[2][1] = 65536; cm.form[2][2] = 0; cm.form[2][3] = 0;
	cm.form[3][0] = 0; cm.form[3][1] = 0; cm.form[3][2] = 0; cm.form[3][3] = 65536;*/

	cm.setID();

	MatrixFP im = (cm*trackPositionMatrix);
	VectorFP pos=im*this->position;

	im = (cm*trackTargetMatrix);
	VectorFP targ=im*this->target;

	veca.x=1.0f;
	veca.y=0.0f;
	veca.z=0.0f;

	vecb.x=(targ.z-pos.z)/65536.0f;
	vecb.y=(targ.x-pos.x)/65536.0f;
	vecb.z=0.0f;
	roty=getAngle(veca,vecb);

	target.x=(targ.x-pos.x)/65536.0f;
	target.y=(targ.z-pos.z)/65536.0f;
	rotangle=-roty;

	//target2.x=(target.x*(float)cos(rotangle))-(target.y*(float)sin(rotangle));
	target2.y=(target.x*(float)sin(rotangle))+(target.y*(float)cos(rotangle));

	vecb.x=target2.y;
	vecb.y=(targ.y-pos.y)/65536.0f;
	vecb.z=0.0f;
	rotx=-getAngle(veca,vecb);

	rotz=(float)-(bank/180.0f*M_PI);
	
	MatrixFP rotX,rotY,rotZ,tm;
	//tm.setSubtract(pos);

	//MatrixFP m = rotZ*rotX*rotY;

	rotX.setRotX(-rotx);
	rotY.setRotY(-roty);
	rotZ.setRotZ(-rotz);
	
	im = rotX*rotZ;
	
	return rotY*im;
}

