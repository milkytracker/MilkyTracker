/////////////////////////////////////////////////////////////////
//
//	Our display device
//
/////////////////////////////////////////////////////////////////
#ifndef DISPLAYDEVICE__H
#define DISPLAYDEVICE__H

#include "BasicTypes.h"
#include "DisplayDeviceBase.h"

#include <windows.h>

// Forwards
class PPGraphicsAbstract;

class PPDisplayDevice : public PPDisplayDeviceBase
{
private:
	HWND hWnd;

public:
	PPDisplayDevice(HWND hWnd, pp_uint32 width, pp_uint32 height);
	virtual ~PPDisplayDevice();

	virtual PPGraphicsAbstract* open();
	virtual void close();

	void update();
	void update(const PPRect& r);

	// ----------------------------- ex. PPWindow ----------------------------
	virtual void setTitle(const PPSystemString& title);	
	virtual void shutDown();
	virtual void signalWaitState(bool b, const PPColor& color);
};

#endif