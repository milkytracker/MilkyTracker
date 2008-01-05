/*
 *  Zapper.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 26.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __ZAPPER_H__
#define __ZAPPER_H__

class Zapper
{
private:
	class Tracker& tracker;

public:
	Zapper(Tracker& tracker) :
		tracker(tracker)
	{
	}
	
	void zapAll();
	void zapSong();
	void zapPattern();
	void zapInstrument();	
};

#endif
