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
	BITMAPINFOHEADER	m_BitmapInfo; 
	HBITMAP				m_hBitmap;
	LPBYTE				m_pBits;
	HWND				m_hWnd;
	RECT				m_lastRect;
	LONG				m_lastWindowStyle, m_lastWindowExStyle;
	HDC					m_hDC;

	bool				m_waitWindowVisible;
	HANDLE				m_hThread;
	DWORD				m_threadID;

	void blit(HWND hWnd, HDC pDC, pp_int32 x, pp_int32 y, pp_int32 width, pp_int32 height);

public:
	PPDisplayDevice(HWND hWnd, pp_int32 width, pp_int32 height);
	virtual ~PPDisplayDevice();

	virtual PPGraphicsAbstract* open();
	virtual void close();

	void update();
	void update(const PPRect& r);

	void adjustWindowSize();

	// ----------------------------- ex. PPWindow ----------------------------
public:
	virtual void setTitle(const PPSystemString& title);	
	virtual void setSize(const PPSize& size);
	virtual bool goFullScreen(bool b);
	virtual void shutDown();
	virtual void signalWaitState(bool b, const PPColor& color);
	virtual void setMouseCursor(MouseCursorTypes type);

	static DWORD WINAPI UpdateWindowThreadProc(LPVOID lpParameter);
};

#endif
