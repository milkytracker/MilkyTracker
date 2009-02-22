/*
 *  fx/TCBSpline.h
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

#ifndef TCBSPLINE__H
#define TCBSPLINE__H

#include "Math3d.h"

class TCBSpline
{
public:
	struct TKey
	{
		int			ti;
		VectorFloat v;
		float		t,c,b;
	};

private:
	TKey*	keys;

	int		numKeys;

	VectorFloat getLastPos(int i);
	VectorFloat getNextPos(int i);
	VectorFloat calcTi(int i);
	VectorFloat calcTo(int i);
	int			findKey(int curTime,int startIndex, int endIndex);

public:

	TCBSpline(int numKeys);
	~TCBSpline();

	void		setKey(int index, VectorFloat& v, int time, float tension = 0, float continuity = 0, float bias = 0);
	void		setKey(int index, TKey& key) { if (index < numKeys) keys[index] = key; }
	TKey*		getKey(int index) { if (index < numKeys) return &keys[index]; else return &keys[0];}
	VectorFloat	getPos(float curTime);
		
};

#endif

