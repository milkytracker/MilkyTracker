#ifndef PARTICLESCENE__H
#define PARTICLESCENE__H

#include "FXInterface.h"

class ParticleFX;
class TexturedGrid;

class ParticleScene : public FXInterface
{
private:
	float			time;
	ParticleFX		*particleFX;
	ParticleFX		*particleFun;
	TexturedGrid	*texturedGrid;

public:
	ParticleScene(int width, int height, int gridshift);
	~ParticleScene();

	void render(unsigned short* vscreen, unsigned int pitch);
	void update(float syncFrac);

};

#endif
