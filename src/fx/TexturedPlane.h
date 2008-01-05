#ifndef TEXTUREDPLANE__H
#define TEXTUREDPLANE__H

#include "TexturedGrid.h"
#include "Math3d.h"

class TexturedPlane : public TexturedGrid
{
private:
	MatrixFP	rotMatrix;
	VectorFP	cop;
	VectorFP	light;

public:
	TexturedPlane(int width, int height, int gridshift);

	void setMatrix(MatrixFP& matrix) { rotMatrix = matrix; }
	void setCOP(VectorFP& cop) { this->cop = cop; }
	void setLight(VectorFP& light) { this->light = light; }

	virtual void update(float syncFrac);
};

#endif
