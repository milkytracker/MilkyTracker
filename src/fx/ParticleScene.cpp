/*
 *  fx/ParticleScene.cpp
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
#include "Math3d.h"
#include "ParticleScene.h"
#include "ParticleEmitter.h"
#include "ParticleFun.h"
#include "TexturedPlane.h"
#include "TCBSpline.h"
#include "Camera.h"

#define MAXKEYS 1000
#define TIME_SCALE 20

static TCBSpline* cameraPosTrackSpline = new TCBSpline(MAXKEYS);
static TCBSpline* cameraTargetTrackSpline = new TCBSpline(MAXKEYS);
static TCBSpline* emitterTrackSpline = new TCBSpline(MAXKEYS);
static TCBSpline* cameraRollTrackSpline = new TCBSpline(MAXKEYS);

static int Time = 0;

TCBSpline::TKey genPosKey(int Time)
{

	int offset = (rand()&31)-15;

	float t = (((Time-offset)/99.0f)*512*16.0f);

	float radius = (float)(rand()&31)+20.0f;
		
	float py = (rand()%60)-30.0f;

	float px = (float)sin((float)t/128.0f*M_PI)*radius;
	float pz = (float)cos((float)t/128.0f*M_PI)*radius;

	VectorFloat v(px, py, pz);

	TCBSpline::TKey k;

	k.ti = Time*TIME_SCALE;
	k.t = 0.90f;
	k.c = 0.0f;
	k.b = 0.0f;
	k.v = v;

	return k;
}

TCBSpline::TKey genTargetKey(int Time)
{
	return genPosKey(Time);
	
	/*float t = (Time/99.0f)*512*32.0f;

	float radius = 5.0f; //(float)(rand()&3);
		
	float py = (rand()&7)-4.0f;

	float px = (float)sin((float)t/128.0f*M_PI)*radius;
	float pz = (float)cos((float)t/128.0f*M_PI)*radius;

	VectorFloat v(px, py, pz);

	TCBSpline::TKey k;

	k.ti = Time*TIME_SCALE;
	k.t = 1.0f;
	k.c = 0.0f;
	k.b = 0.0f;
	k.v = v;

	return k;*/
}

TCBSpline::TKey genRollKey(int Time)
{
	float px = (rand()%360)-180.0f;

	float py = 0.0f;
	float pz = 0.0f;

	VectorFloat v(px, py, pz);

	TCBSpline::TKey k;

	k.ti = Time*TIME_SCALE;
	k.t = 1.0f;
	k.c = 0.0f;
	k.b = 0.0f;
	k.v = v;

	return k;
}

ParticleScene::ParticleScene(int width, int height, int gridshift)
{
	this->width = width;
	this->height = height;
	
	particleFX = new ParticleEmitter(width, height, 192, 1, 1000);
	particleFX->setZOffset(0);

	particleFun = new ParticleFun(width, height, 192);

	texturedGrid = new TexturedPlane(width, height, gridshift);

	for (Time = 0; Time < MAXKEYS; Time++)
	{
		TCBSpline::TKey k = genPosKey(Time); 
		
		cameraPosTrackSpline->setKey(Time, k);

		k = genTargetKey(Time); 

		emitterTrackSpline->setKey(Time, k);
		
		float xo = (rand()&15)-7.0f;
		float yo = (rand()&15)-7.0f;
		float zo = (rand()&15)-7.0f;

		k.v.x+=xo;
		k.v.y+=yo;
		k.v.z+=zo;

		cameraTargetTrackSpline->setKey(Time, k);
	
		k = genRollKey(Time);

		cameraRollTrackSpline->setKey(Time, k);
	}

	time = 0.0f;

}

ParticleScene::~ParticleScene()
{

	delete texturedGrid;

	delete particleFX;

	delete particleFun;
}

void ParticleScene::update(float syncFrac)
{
	Camera cam;

	cam.position = cameraPosTrackSpline->getPos(time).convertToFixedPoint();
	cam.target = cameraTargetTrackSpline->getPos(time).convertToFixedPoint();

	cam.bank = 0.0f;
	//cam.bank = cameraRollTrackSpline->getPos(time).x;

	time+=syncFrac*0.15f;

	if (time > cameraPosTrackSpline->getKey(MAXKEYS-1)->ti)
	{
		
		time = 0.0f;

	}

	// update particle emitter stuff
	VectorFP emitterPos = emitterTrackSpline->getPos(time).convertToFixedPoint();
	
	reinterpret_cast<ParticleEmitter*>(particleFX)->setEmitter(0, emitterPos);
	
	particleFX->update(syncFrac);
	
	MatrixFP m1 =cam.getMatrix(), m2 = particleFX->getMatrix();
	MatrixFP m = m1*m2;
	
	particleFX->setMatrix(m);

	// update particle fun
	particleFun->update(syncFrac);
	
	m2 = particleFun->getMatrix();
	m = m1*m2;

	particleFun->setMatrix(m);

	// update grid raytracer
	m = cam.getMatrixInverse();

	reinterpret_cast<TexturedPlane*>(texturedGrid)->setMatrix(m);

	reinterpret_cast<TexturedPlane*>(texturedGrid)->setCOP(cam.position);

	reinterpret_cast<TexturedPlane*>(texturedGrid)->setLight(cam.target);

	texturedGrid->update(syncFrac);
}

void drawInt(int i, unsigned short* vScreen, int pitch, int x, unsigned int y, unsigned short col);

void ParticleScene::render(unsigned short* vscreen, unsigned int pitch)
{
	texturedGrid->render(vscreen, pitch);

	particleFX->render(vscreen, pitch);

	particleFun->render(vscreen, pitch);

	//drawInt((int)time, vscreen, pitch, 0, 239-8, 31);
	
}
