/*
 *  tracker/wince/WaitStateThread.h
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

#ifndef WAITSTATETHREAD__H
#define WAITSTATETHREAD__H

#include <windows.h>

class WaitStateThread
{
private:
	static WaitStateThread* instance;
	
	BOOL		bActivated;
	
	DWORD		nSleepTime;
	DWORD		threadID;
	HANDLE		hThread;

	WORD*		saveBuffer;

	// display width, height, pitch in 16 bit words
	int			xres, yres, pitch;

	int			UpperLeftX, UpperLeftY;
	int			LowerRightX, LowerRightY;
	// area width, height
	int			Width, Height;

	static DWORD WINAPI MyThreadProc(LPVOID lpParameter);

	WaitStateThread();

public:
	static WaitStateThread* getInstance()
	{
		if (instance == NULL)
			instance = new WaitStateThread();
		
		return instance;
	}
	
	void setDisplayResolution(int width, int height);

	void activate(BOOL bActivate, BOOL bDarken = TRUE, BOOL bPutText = TRUE);
};

#endif
