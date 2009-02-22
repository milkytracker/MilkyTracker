/*
 *  fx/Twister.h
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

#ifndef TWISTER__H
#define TWISTER__H

#include "FXInterface.h"
#include "Math3d.h"

#define NUMPOINTS 4

class Twister : public FXInterface
{
private:
	int				center;
	
	unsigned short* texture;
	int*			zbuffer;

	struct MyVertex
	{
		VectorFP	p;
		int			u;
	};

	MyVertex sourcePoints[NUMPOINTS+1];
	MyVertex newPoints[NUMPOINTS+1];

	int sinTable[1024];
	int cosTable[1024];

	int mySin(int phi)
	{
		int t = (phi&255);
		
		int s1 = sinTable[(phi>>8)&1023];
		int s2 = sinTable[((phi>>8)+1)&1023];
		
		return (s1*(255-t)+t*s2)>>8;
	}

	int myCos(int phi)
	{
		int t = (phi&255);
		
		int s1 = cosTable[(phi>>8)&1023];
		int s2 = cosTable[((phi>>8)+1)&1023];
		
		return (s1*(255-t)+t*s2)>>8;
	}

public:
	Twister(int width, int height, int center = -1);
	~Twister();

	void render(unsigned short* vscreen, unsigned int pitch);
	void update(float syncFrac);

};

#endif
