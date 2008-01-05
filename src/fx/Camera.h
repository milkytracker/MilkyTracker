#ifndef CAMERA__H
#define CAMERA__H

#include "Math3d.h"

class Camera 
{
public:
	VectorFP	position,target;
	float		bank,lens;

	MatrixFP	trackPositionMatrix,trackTargetMatrix;

				Camera();
				Camera(VectorFP& pos, VectorFP& target);

	MatrixFP	getMatrix();

	MatrixFP	getMatrixInverse();
};

#endif
