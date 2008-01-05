#ifndef TEXTUREDGRID__H
#define TEXTUREDGRID__H

#include "FXInterface.h"

struct VectorFP;

class TexturedGrid : public FXInterface
{
protected:
	int				fov;
	int				gridshift;
	int				gridsize;		// must be 1 << GRIDSHIFT

	VectorFP*		grid;
	int				gridWidth, gridHeight;

	unsigned short* texture;
	unsigned short* buffer;

	int				sintab[1024];

public:
	TexturedGrid(int width, int height, int gridshift);
	virtual ~TexturedGrid();

	void setTexture(unsigned short* texture) { this->texture = texture; }

	// from FXInterface
	virtual void render(unsigned short* vscreen, unsigned int pitch);
	virtual void update(float syncFrac);

};

#endif
