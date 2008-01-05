#ifndef PARTICLEFX__H
#define PARTICLEFX__H

#include "FXInterface.h"
#include "Math3d.h"

class ParticleFX : public FXInterface
{
protected:
	struct Particle 
	{
		VectorFP		pos;
		VectorFP		direction;
		int				size;
		int				grow;
		unsigned short*	texture;
		int				textureWidth, textureHeight;
	};

	struct Sprite
	{
		int				tx,ty;
		int				xsize,ysize;
		int				xres,yres;
		int				xoffset,yoffset;
		unsigned short	zval;
		int				flags;
		int				shade;
		unsigned int	texturexres;
		unsigned short	*texture;
	};

	int				FOV;
	int				zOffset;

	Particle*		particles;
	unsigned int	numParticles;

	MatrixFP		rotMatrix;

public:
	ParticleFX(int width, int height, int numParticles, int FOV = -1);
	virtual ~ParticleFX();

	virtual void render(unsigned short* vscreen, unsigned int pitch);
	virtual void update(float syncFrac) = 0;

	void setFOV(int FOV) { this->FOV = FOV; }
	void setZOffset(int zOffset) { this->zOffset = zOffset; }
	void setMatrix(MatrixFP& matrix) { rotMatrix = matrix; }
	MatrixFP getMatrix() { return rotMatrix; }

	static void	drawSprite(unsigned short *buffer, int XMAX, int YMAX, int PITCH, Sprite* spr);

};

#endif
