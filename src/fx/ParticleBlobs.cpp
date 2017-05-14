/*
 *  fx/ParticleBlobs.cpp
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

#include "ParticleBlobs.h"
#include "Math3d.h"
#include "Filter.h"
#include "Texture.h"

#define GRIDSIZE 8
#define NUMPARTICLES (GRIDSIZE*GRIDSIZE*GRIDSIZE)

struct Blob
{
	int x,y,z;
	int strength;
};

static Blob blobs[] = {{128,128,0,65536/8},{100,100,0,65536/8},{100,100,0,65536/8}};

#define MAXCLAMP 65536
#define TEXTURESIZE 64

ParticleBlobs::ParticleBlobs(int width, int height) :		
	ParticleFX(width, height, NUMPARTICLES)
{

	setZOffset(30*65536);
	
	temp = new unsigned short[width*height];
	
	//buffer = new unsigned short[pitch*height];

	int i;
	
	unsigned char *texture = new unsigned char[TEXTURESIZE*TEXTURESIZE*3];

    /*FILE* f = fopen("y:/stuff/demo/flare.tga","rb");
    fseek(f,18,SEEK_SET);
    fread(texture,TEXTURESIZE*TEXTURESIZE,3,f);
    fclose(f);*/

	Texture::createFlareTexture(texture, 224, 191, 64, 4.0f, TEXTURESIZE);
	
	flareTexture[0] = new unsigned short[TEXTURESIZE*TEXTURESIZE];
	flareTexture[1] = new unsigned short[TEXTURESIZE*TEXTURESIZE];
	flareTexture[2] = new unsigned short[TEXTURESIZE*TEXTURESIZE];

	for (i = 0; i < TEXTURESIZE*TEXTURESIZE*3; i++)
		texture[i]>>=1;
	
	for (i = 0; i < TEXTURESIZE*TEXTURESIZE; i++) {
		//texture[i*3] = texture[i*3+1] = 0;
		
		flareTexture[0][i] = RGB2SHORT(texture[i*3],texture[i*3+1],texture[i*3+2]);
	
		flareTexture[1][i] = RGB2SHORT(texture[i*3+2],texture[i*3+1],texture[i*3]);

		flareTexture[2][i] = RGB2SHORT(texture[i*3+1],texture[i*3+2],texture[i*3]);
	}

	delete[] texture;

	i = 0;
	for (int z = 0; z < GRIDSIZE; z++)
		for (int y = 0; y < GRIDSIZE; y++)
			for (int x = 0; x < GRIDSIZE; x++) {
				
				float fx = (x-((GRIDSIZE-1)*0.5f))*2.0f;
				float fy = (y-((GRIDSIZE-1)*0.5f))*2.0f;
				float fz = (z-((GRIDSIZE-1)*0.5f))*2.0f;
				
				int texture = rand()%3;

				particles[i].pos.x = int(fx*65536.0f);
				particles[i].pos.y = int(fy*65536.0f);
				particles[i].pos.z = int(fz*65536.0f);
				particles[i].texture = flareTexture[texture]; 
				particles[i].textureWidth = particles[i].textureHeight = TEXTURESIZE;
				i++;
			}
}

ParticleBlobs::~ParticleBlobs()
{
	for (int i = 0; i < 3; i++)
		delete flareTexture[i];

	//delete buffer;
	delete temp;
}

void ParticleBlobs::update(float SyncFrac)
{

	double phi = ::PPGetTickCount()*0.001f*0.5f;;

	blobs[0].x = (int)(sin(phi*4.0f)*4.0f*65536.0f);
	blobs[0].y = (int)(cos(phi*4.0f)*4.0f*65536.0f);
	blobs[0].z = (int)(sin(phi*4.0f)*4.0f*65536.0f);

	blobs[1].x = (int)(cos(-phi*2.0f)*4.0f*65536.0f);
	blobs[1].y = (int)(sin(phi*2.0f)*4.0f*65536.0f);
	blobs[1].z = (int)(cos(phi*2.0f)*4.0f*65536.0f);

	blobs[2].x = (int)(cos(-phi*3.0f)*6.0f*65536.0f);
	blobs[2].y = (int)(cos(phi*3.0f)*3.0f*65536.0f);
	blobs[2].z = (int)(sin(phi*3.0f)*5.0f*65536.0f);

	MatrixFP rotMatrix,mx,my,mz;
	mx.setRotX((float)(phi*0.75f));
	my.setRotY((float)phi);
	mz.setRotZ((float)(phi*0.5f));
	rotMatrix = (mx*my);
	rotMatrix = mz*rotMatrix;

	this->rotMatrix = rotMatrix;

	int numBlobs = sizeof(blobs)/sizeof(Blob);
	
	for (int i = 0; i < NUMPARTICLES; i++) {
		int scale = 32768;

		for (int j = 0; j < numBlobs; j++) {
			int dx = fpmul((blobs[j].x-particles[i].pos.x),blobs[j].strength);
			int dy = fpmul((blobs[j].y-particles[i].pos.y),blobs[j].strength);
			int dz = fpmul((blobs[j].z-particles[i].pos.z),blobs[j].strength);
				
			int sd = fpmul(dx,dx)+fpmul(dy,dy)+fpmul(dz,dz);
				
			if (sd<(0.707f*0.707f*65536))
				scale += (fpmul(sd,sd)-sd+16384)*4;
		}
		
		if (scale>=MAXCLAMP) scale = MAXCLAMP;
		scale = fpmul(scale,scale);
		scale = fpmul(scale,scale);
	
		particles[i].size = scale;
	}

}

void ParticleBlobs::render(unsigned short* vscreen, unsigned int pitch)
{
	int height = this->height;
	int width = this->width>>1;
	unsigned int bgColor = 0;
	unsigned int fill = (bgColor<<16) | bgColor;
	for (int j = 0; j < height; j++)
	{
		unsigned int* vPtr = (unsigned int*)(vscreen+j*pitch);
		for (int i = 0; i < width; i++)
			*vPtr++ = fill;
	}
	
	ParticleFX::render(vscreen, pitch);

	Filter::stylize(vscreen, temp, this->width, this->height, pitch, this->width, 100, 70, 60, 255, 192, 100);

	//Filter::applyRadial(vscreen, this->width, this->height, pitch, this->width>>1, this->height>>1, 8192, 1);

}
