/*
 *  fx/ParticleEmitter.cpp
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

#include "BasicTypes.h"
#include "ParticleEmitter.h"
#include "Math3d.h"
#include "Filter.h"
#include "Texture.h"

#undef LENSE_REFLECTION

// emitter size is hardcoded at 64 pixels ;)
#define TEXTURESIZE_PARTICLE	16

#ifdef LENSE_REFLECTION
void createRingTexture(unsigned char* texture, int size, int outer, int inner, int r, int g, int b, float innerShade = 0.0f, float outerShade = 0.0f)
{
	
	float hx = size>>1;
	float hy = size>>1;

	float mid = (outer+inner)>>1;
	float c = (outer-inner)*0.5f;

	for (int y = 0; y < size; y++)
		for (int x = 0; x < size; x++)
		{

			float px = x-hx;
			float py = y-hy;

			float dist = sqrt(px*px+py*py);

			float s = 1.0f-(abs(mid-dist)/c);

			s = (float)pow(sin(s*(M_PI/2.0f)),4.0f);
			
			if (mid-dist >= 0)
			{
				s+=innerShade;
			}
			else
				s+=outerShade;

			if (s >= 1.0f)
				s = 1.0f;

			if (dist >= inner && dist <= outer)
			{
				texture[(y*size+x)*3] = r*s;
				texture[(y*size+x)*3+1] = g*s;
				texture[(y*size+x)*3+2] = b*s;
			}
			else if (dist < inner)
			{
				texture[(y*size+x)*3] = r*innerShade;
				texture[(y*size+x)*3+1] = g*innerShade;
				texture[(y*size+x)*3+2] = b*innerShade;
			}
			else
			{
				texture[(y*size+x)*3] = r*outerShade;
				texture[(y*size+x)*3+1] = g*outerShade;
				texture[(y*size+x)*3+2] = b*outerShade;
			}
		}

}
#endif

ParticleEmitter::ParticleEmitter(int width, int height, int FOV, int numEmitters, int maxParticles) : 
	ParticleFX(width, height, maxParticles+numEmitters, FOV)
{

	this->maxParticles = maxParticles;

	setZOffset(0);
	
	//temp = new unsigned short[width*height];
	//buffer = new unsigned short[width*height];

	int i;
	
	unsigned char *texture = new unsigned char[TEXTURESIZE_PARTICLE*TEXTURESIZE_PARTICLE*3];

	Texture::createFlareTexture(texture, 224, 191, 64, 4.0f, TEXTURESIZE_PARTICLE);
	
	flareTexture[0] = new unsigned short[TEXTURESIZE_PARTICLE*TEXTURESIZE_PARTICLE];
	flareTexture[1] = new unsigned short[TEXTURESIZE_PARTICLE*TEXTURESIZE_PARTICLE];
	flareTexture[2] = new unsigned short[TEXTURESIZE_PARTICLE*TEXTURESIZE_PARTICLE];

	for (i = 0; i < TEXTURESIZE_PARTICLE*TEXTURESIZE_PARTICLE*3; i++)
		texture[i]>>=1;
	
	for (i = 0; i < TEXTURESIZE_PARTICLE*TEXTURESIZE_PARTICLE; i++) {
		//texture[i*3] = texture[i*3+1] = 0;
		
		flareTexture[0][i] = RGB2SHORT(texture[i*3],texture[i*3+1],texture[i*3+2]);
	
		flareTexture[1][i] = RGB2SHORT(texture[i*3+2],texture[i*3+1],texture[i*3]);

		flareTexture[2][i] = RGB2SHORT(texture[i*3+1],texture[i*3+2],texture[i*3]);
	}

	delete[] texture;

	emitterTexture[0] = new unsigned short[64*64];
#ifdef LENSE_REFLECTION
	emitterTexture[1] = new unsigned short[64*64];
	emitterTexture[2] = new unsigned short[64*64];
	emitterTexture[3] = new unsigned short[64*64];
	emitterTexture[4] = new unsigned short[64*64];
	emitterTexture[5] = new unsigned short[64*64];
#endif

	texture = new unsigned char[64*64*3];

	Texture::createFlareTexture(texture, 224, 191, 64, 4.0f, 64);

	float len = (float)sqrt((float)32*32*2);

	for (i = 0; i < 64; i++)
	{

		float dist = (float)pow((float)(len-sqrt((double)((i-32)*(i-32)*2)))/len,2);

		int r = texture[(i*64+i)*3];
		int g = texture[(i*64+i)*3+1];
		int b = texture[(i*64+i)*3+2];
		r+=(int)(255*dist); if (r>255) r = 255;
		g+=(int)(191*dist); if (g>255) g = 255;
		b+=(int)(64*dist); if (b>255) b = 255;

		texture[(i*64+i)*3] = r;
		texture[(i*64+i)*3+1] = g;
		texture[(i*64+i)*3+2] = b;

		r = texture[(i*64+63-i)*3];
		g = texture[(i*64+63-i)*3+1];
		b = texture[(i*64+63-i)*3+2];
		r+=(int)(255*dist); if (r>255) r = 255;
		g+=(int)(191*dist); if (g>255) g = 255;
		b+=(int)(64*dist); if (b>255) b = 255;		

		texture[(i*64+63-i)*3] = r;
		texture[(i*64+63-i)*3+1] = g;
		texture[(i*64+63-i)*3+2] = b;
		
		r = texture[(i*64+31)*3];
		g = texture[(i*64+31)*3+1];
		b = texture[(i*64+31)*3+2];
		r+=(int)(255*dist); if (r>255) r = 255;
		g+=(int)(191*dist); if (g>255) g = 255;
		b+=(int)(64*dist); if (b>255) b = 255;		

		texture[(i*64+31)*3] = r;
		texture[(i*64+31)*3+1] = g;
		texture[(i*64+31)*3+2] = b;
		
		r = texture[(31*64+i)*3];
		g = texture[(31*64+i)*3+1];
		b = texture[(31*64+i)*3+2];
		r+=(int)(255*dist); if (r>255) r = 255;
		g+=(int)(191*dist); if (g>255) g = 255;
		b+=(int)(64*dist); if (b>255) b = 255;		

		texture[(32*64+i)*3] = r;
		texture[(32*64+i)*3+1] = g;
		texture[(32*64+i)*3+2] = b;
	}

	// spikey
	Texture::blur24(texture, 64, 64, 2);
	Texture::convert24to16(emitterTexture[0], texture, 64*64, 0);

#ifdef LENSE_REFLECTION
	// green ring
	createRingTexture(texture, 64, 28, 19, 64>>1, 224>>1, 96>>1, 0.0f);
	Texture::convert24to16(emitterTexture[1], texture, 64*64, 0);

	// blue ring
	createRingTexture(texture, 64, 28, 19, 0, 96>>1, 224>>1, 0.4f);
	Texture::convert24to16(emitterTexture[2], texture, 64*64, 0);

	// orange ring
	createRingTexture(texture, 64, 28, 19, 224>>1, 128>>1, 32>>1, 0.4f);
	Texture::convert24to16(emitterTexture[3], texture, 64*64, 0);

	// blue ball
	Texture::createFlareTexture(texture, 0, 96>>1, 224>>1, 1.0f, 64);
	Texture::convert24to16(emitterTexture[4], texture, 64*64, 0);

	// red ball
	Texture::createFlareTexture(texture, 255>>1, 96>>1, 32>>1, 1.0f, 64);
	Texture::convert24to16(emitterTexture[5], texture, 64*64, 0);
#endif

	delete[] texture;

	this->numEmitters = numEmitters;

	emitters = new Particle[numEmitters];

	// create emitters
	for (i = 0; i < numEmitters; i++)
	{
		emitters[i].pos.x = 0;
		emitters[i].pos.y = 0;
		emitters[i].pos.z = 0;
		emitters[i].size = 65536*4;
		emitters[i].texture = emitterTexture[0];
		emitters[i].textureHeight = emitters[i].textureWidth = 64;
	}

	for (i = 0; i < maxParticles+numEmitters; i++)
	{
		particles[i].pos.x = particles[i].pos.y = particles[i].pos.z = 0;
		particles[i].texture = flareTexture[0];
		particles[i].size = 0;
		particles[i].grow = 0;
	}

	currentParticleIndex = numEmitters;
}

ParticleEmitter::~ParticleEmitter()
{
	for (int i = 0; i < 3; i++)
		delete flareTexture[i];

	//delete buffer;
	//delete temp;

	delete emitterTexture[0];
#ifdef LENSE_REFLECTION
	delete emitterTexture[1];
	delete emitterTexture[2];
	delete emitterTexture[3];
	delete emitterTexture[4];
	delete emitterTexture[5];
#endif

	delete[] emitters;
}

void ParticleEmitter::update(float syncFrac)
{
	int syncFracFP = (int)(syncFrac*65536.0f)<<1;

	/*int speed = fpmul(syncFracFP, 16384*4);

	int i;

	rotMatrix.setID();

	particles[0] = emitter;

	static int j = 1;

	for (i = 0; i < 10; i++)
	{
		particles[j].pos = emitter.pos;

		particles[j].pos.x += (((rand()&255)-127)<<11);
		particles[j].pos.z += (((rand()&255)-127)<<11);

		particles[j].size = 32768;
		particles[j].direction.x = 0;
		particles[j].direction.y = -fpmul(((rand()&255)<<8),speed);
		particles[j].direction.z = 0;
		particles[j].texture = flareTexture[rand()%3];
		particles[j].textureHeight = particles[j].textureWidth = TEXTURESIZE_PARTICLE;
		
		j++;
		if (j>NUMPARTICLES)
			j = 1;
	}

	for (i = 1; i < NUMPARTICLES+1; i++)
	{
		particles[i].pos = particles[i].pos+particles[i].direction;
		//if (particles[i].size >= 256)
		//{
		//	particles[i].size -= (syncFracFP>>8);
		//}
		//else
		//	particles[i].size = 0;

	}*/

	int speed = fpmul(syncFracFP, 16384);

	unsigned int i;

	rotMatrix.setID();

	particles[0] = *emitters;

	unsigned int j = currentParticleIndex;

	for (unsigned int k = 0; k < numEmitters; k++)
	{

		for (i = 0; i < 10; i++)
		{
			particles[j].pos = emitters[k].pos;
			
			particles[j].size = 32768*2;
			particles[j].grow = 256;
			particles[j].direction.x = fpmul(((rand()&255)<<8)-32768,speed);
			particles[j].direction.y = -fpmul(((rand()&255)<<8)-32768,speed);
			particles[j].direction.z = fpmul(((rand()&255)<<8)-32768,speed);
			particles[j].texture = flareTexture[rand()%3];
			particles[j].textureHeight = particles[j].textureWidth = TEXTURESIZE_PARTICLE;	
			
			j++;
			if (j >= maxParticles+numEmitters)
				j = numEmitters;
		}

	}

	currentParticleIndex = j;

	for (i = numEmitters; i < maxParticles+numEmitters; i++)
	{
		particles[i].pos = particles[i].pos+particles[i].direction;
		if (particles[i].size >= particles[i].grow)
		{
			particles[i].size -= fpmul(particles[i].grow,syncFracFP<<2);
		}
		else
			particles[i].size = 0;

	}


}

void ParticleEmitter::render(unsigned short* vscreen, unsigned int pitch)
{
	
	ParticleFX::render(vscreen, pitch);

#ifdef LENSE_REFLECTION
	for (int i = 0; i < numEmitters; i++)
	{
		// draw lens-flare reflections
		VectorFP v = rotMatrix*emitters[i].pos;
		
		int z = v.z+zOffset;
		
		if (z>65536)
		{
			if (emitters[i].size)
			{
				
				int rz = fpdiv(65536,z);
				int ix = (fpmul((v.x),rz)*FOV)+(width>>1)*65536;
				int iy = (fpmul((v.y),rz)*FOV)+(height>>1)*65536;
				
				int size = fpmul((emitters[i].size>>1)*FOV,rz);
				
				int ix2 = (width-1)*65536-ix; 
				int iy2 = (height-1)*65536-iy; 
				
				int dx = ix2-ix;
				int dy = iy2-iy;
				
				int ix3 = (ix+ix2)>>1; 
				int iy3 = (iy+iy2)>>1; 
				
				Sprite spr;
				spr.tx = (ix+fpmul(dx,65536*3));
				spr.ty = (iy+fpmul(dy,65536*3));
				spr.xsize = size<<2;
				spr.ysize = size<<2;
				spr.xres = emitters[i].textureWidth-1;
				spr.yres = emitters[i].textureHeight-1;
				spr.xoffset = 0;
				spr.yoffset = 0;
				spr.texturexres = emitters[i].textureWidth;
				spr.flags = 0;
				spr.texture = emitterTexture[4];
				drawSprite(vscreen,width, height, pitch, &spr);
				
				spr.tx = (ix+fpmul(dx,65536));
				spr.ty = (iy+fpmul(dy,65536));
				spr.xsize = fpmul(size,65536+32768);
				spr.ysize = fpmul(size,65536+32768);
				spr.texture = emitterTexture[1];
				drawSprite(vscreen,width, height, pitch, &spr);
				
				spr.tx = ix3;
				spr.ty = iy3;
				spr.xsize = size>>2;
				spr.ysize = size>>2;
				spr.texture = emitterTexture[5];
				drawSprite(vscreen,width, height, pitch, &spr);
				
				spr.tx = (ix+ix3)>>1;
				spr.ty = (iy+iy3)>>1;
				spr.xsize = size;
				spr.ysize = size;
				spr.texture = emitterTexture[2];
				drawSprite(vscreen,width, height, pitch, &spr);
				
				spr.tx = (ix+fpmul(dx,65536*2));
				spr.ty = (iy+fpmul(dy,65536*2));
				spr.xsize = size;
				spr.ysize = size;
				spr.texture = emitterTexture[3];
				drawSprite(vscreen,width, height, pitch, &spr);
				
				spr.tx = (ix+fpmul(dx,65536*3+32768));
				spr.ty = (iy+fpmul(dy,65536*3+32768));
				spr.xsize = size>>1;
				spr.ysize = size>>1;
				spr.texture = emitterTexture[5];
				drawSprite(vscreen,width, height, pitch, &spr);
			}
		}
	}
#endif

}
