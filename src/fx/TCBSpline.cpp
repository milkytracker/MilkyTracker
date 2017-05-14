/*
 *  fx/TCBSpline.cpp
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

#include "TCBSpline.h"

TCBSpline::TCBSpline(int numKeys)
{
	if (numKeys == 0)
		numKeys++;
	
	this->numKeys = numKeys;

	keys = new TKey[numKeys];

	for (int i = 0; i < numKeys; i++)
	{
		keys[i].v.set(0.0f,0.0f,0.0f);
		keys[i].t = keys[i].c = keys[i].b = 0.0f;
		keys[i].ti = 0;
	}
}

TCBSpline::~TCBSpline()
{
	
	delete[] keys;

}

void TCBSpline::setKey(int index, VectorFloat& v, int time, float tension, float continuity, float bias)
{
	if (index  < numKeys)
	{
	
		keys[index].v = v;
		keys[index].ti = time;
		keys[index].t = tension;
		keys[index].c = continuity;
		keys[index].b = bias;
	
	}
}

VectorFloat TCBSpline::getLastPos(int i)
{
	VectorFloat lastPos;

	if (i>0)
		lastPos = keys[i-1].v;
	else
		lastPos = keys[numKeys-1].v;

	return lastPos;
}

VectorFloat TCBSpline::getNextPos(int i)
{
	VectorFloat nextPos;

	if (i<numKeys-1)
		nextPos = keys[i+1].v;
	else
		nextPos = keys[0].v;

	return nextPos;
}

VectorFloat TCBSpline::calcTi(int i)
{
	//return 0.5f * (getNextPos(keys, i, numKeys) - getLastPos(keys, i, numKeys));

	float coeff1 = ((1.0f - keys[i].t) * (1.0f + keys[i].c) * (1.0f - keys[i].b))*0.5f;
	
	float coeff2 = ((1.0f - keys[i].t) * (1.0f - keys[i].c) * (1.0f + keys[i].b))*0.5f;

	return (coeff1 * (getNextPos(i) - keys[i].v)) + (coeff2 * (keys[i].v - getLastPos(i)));
}

VectorFloat TCBSpline::calcTo(int i)
{
	//return 0.5f * (getNextPos(keys, i, numKeys) - getLastPos(keys, i, numKeys));

	float coeff1 = ((1.0f - keys[i].t) * (1.0f - keys[i].c) * (1.0f - keys[i].b))*0.5f;
	
	float coeff2 = ((1.0f - keys[i].t) * (1.0f + keys[i].c) * (1.0f + keys[i].b))*0.5f;

	return (coeff1 * (getNextPos(i) - keys[i].v)) + (coeff2 * (keys[i].v - getLastPos(i)));
}

int TCBSpline::findKey(int curTime,int startIndex, int endIndex)
{

	if (curTime > keys[numKeys-1].ti)
		return -1;

	if (startIndex >= endIndex)
		return -1;

	if (curTime >= keys[startIndex].ti && curTime < keys[startIndex+1].ti)
		return startIndex;

	if (startIndex + 1 == endIndex)
		return startIndex;

	int mid = (startIndex+endIndex) >> 1;

	if (curTime >= keys[startIndex].ti && curTime < keys[mid].ti)
	{
		
		return findKey(curTime, startIndex, mid);

	}
	else if (curTime >= keys[mid].ti && curTime <= keys[endIndex].ti)
	{
	
		return findKey(curTime, mid, endIndex);

	}
	else
	{
		return keys[endIndex].ti;
	}

}

VectorFloat TCBSpline::getPos(float curTime)
{

	VectorFloat pos(0.0f,0.0f,0.0f);

	int i = findKey((int)curTime, 0, numKeys-1);

	if (i != -1)
	{

	/*for (int i = 0; i < numKeys-1; i++)
	{			
		// okay "i" must be the current key
		
		if (curTime >= keys[i].ti && 
			curTime < keys[i+1].ti)
		{*/
			
			float deltai = (float)(keys[i+1].ti - keys[i].ti);
			
			float t = (float)(curTime - keys[i].ti);
			
			float v = t/deltai;
			
			VectorFloat curPos = keys[i].v;
			VectorFloat nextPos = getNextPos(i);
//			VectorFloat lastPos = getLastPos(i);
			
			// incoming tangent vector for the current point
//			VectorFloat Ti = calcTi(i);
			// outgoing tangent vector for the current point
			VectorFloat To = calcTo(i);
			
			// incoming tangent vector for the next point
			VectorFloat Ti1 = calcTi(i+1);
			// outgoing tangent vector for the next point
//			VectorFloat To1 = calcTo(i+1);
			
			VectorFloat Ai = curPos;
			VectorFloat Bi = deltai * To;
			VectorFloat Ci = (3.0f * (nextPos - curPos)) - (deltai * ((2.0f * To) + Ti1));
			VectorFloat Di = (-2.0f * (nextPos - curPos)) + (deltai * (To + Ti1));
			
			float v2 = v*v;
			float v3 = v2*v;
			
			pos = Ai + (v*Bi) + (v2*Ci) + (v3*Di);
			
/*			break;
		}
		
	}*/

	}

	return pos;

}
