#ifndef PARTICLEEMITTER__H
#define PARTICLEEMITTER__H

#include "ParticleFX.h"
#include "Math3d.h"

class ParticleEmitter : public ParticleFX
{
private:
	unsigned short* flareTexture[3];
	unsigned short* temp;
	unsigned short*	buffer;

	unsigned int numEmitters;
	unsigned short* emitterTexture[6];
	Particle* emitters;
	int	currentParticleIndex;

	int maxParticles;

public:
	ParticleEmitter(int width, int height, int FOV, int numEmitters, int maxParticles);
	virtual ~ParticleEmitter();

	void setEmitter(unsigned int index, VectorFP& pos) { if (index < numEmitters) emitters[index].pos = pos; }

	virtual void update(float syncFrac);
	virtual void render(unsigned short* vscreen, unsigned int pitch);

};

#endif
