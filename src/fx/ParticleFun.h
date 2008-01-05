#ifndef PARTICLEFUN__H
#define PARTICLEFUN__H

#include "ParticleFX.h"
#include "Math3d.h"

class ParticleFun : public ParticleFX
{
private:
	unsigned short* flareTexture;
	float phi;

public:
	ParticleFun(int width, int height, int FOV);
	virtual ~ParticleFun();

	virtual void update(float syncFrac);

};

#endif
