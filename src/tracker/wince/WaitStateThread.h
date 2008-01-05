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