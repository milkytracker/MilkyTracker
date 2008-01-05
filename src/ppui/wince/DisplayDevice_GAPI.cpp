#include "DisplayDevice_GAPI.h"
#include "Graphics.h"
#include "gx.h"
#include "WaitStateThread.h"

// virtual screen comes from outside
extern unsigned short *vScreen;

// Also the Update functions
void UpdateScreen(unsigned short* vScreen);
void UpdateScreenRegion(unsigned short* vScreen, const PPRect& r);

PPDisplayDevice::PPDisplayDevice(HWND hWnd, pp_uint32 width, pp_uint32 height) :
	PPDisplayDeviceBase(width, height)  
{
	currentGraphics = new PPGraphics_16BIT(width, height, (width * 16) / 8, (pp_uint8*)vScreen);

	this->hWnd = hWnd;
}

PPDisplayDevice::~PPDisplayDevice()
{
	
	delete currentGraphics;

}

PPGraphicsAbstract* PPDisplayDevice::open()
{
	if (!isEnabled())
		return NULL;

	currentGraphics->lock = false;

	return currentGraphics;
}

void PPDisplayDevice::close()
{
	currentGraphics->lock = true;
}

void PPDisplayDevice::update()
{
	if (!isUpdateAllowed() || !isEnabled())
		return;
	
	::UpdateScreen(vScreen);
}

void PPDisplayDevice::update(const PPRect& r)
{
	if (!isUpdateAllowed() || !isEnabled())
		return;
	
	::UpdateScreenRegion(vScreen, r);
}

void PPDisplayDevice::setTitle(const PPSystemString& title)
{
	::SetWindowText(hWnd, title);
}

void PPDisplayDevice::shutDown()
{
	::PostMessage(hWnd, WM_CLOSE, 0, 0);
}

void PPDisplayDevice::signalWaitState(bool b, const PPColor& color)
{
	WaitStateThread::getInstance()->activate(b);
}
