#ifndef PARTICLEBLOBS__H
#define PARTICLEBLOBS__H

#include "ParticleFX.h"
#include "Math3d.h"


class ParticleBlobs : public ParticleFX
{
	unsigned short* flareTexture[3];
	unsigned short* temp;
	unsigned short*	buffer;


public:
	ParticleBlobs(int width, int height);
	virtual ~ParticleBlobs();

	virtual void render(unsigned short* vscreen, unsigned int pitch);
	virtual void update(float syncFrac);

};

#endif
