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
