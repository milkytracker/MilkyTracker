#ifndef WAITWINDOW__H
#define WAITWINDOW__H

#include <windows.h>
#include "tchar.h"
#include "BasicTypes.h"

class WaitWindow
{
private:
	static WaitWindow*		instance;

	// Windows stuff
	static TCHAR			szClassName[];
	
	HWND					hWnd;

	// Window callback
	static LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

	// Render my own window content
	BITMAPINFOHEADER		m_BitmapInfo; // <- this should be saved, eg make it a member variable
	HBITMAP					m_hBitmap;
	LPBYTE					m_pBits;
	HDC						m_hDC;

	void					init();

	// Blit my content into DC
	void					blit(HWND hWnd, HDC pDC, 
								 pp_int32 x, pp_int32 y, 
								 pp_int32 width, pp_int32 height);
	

	void					render();

	PPColor					color;

	WaitWindow();

public:
	static WaitWindow* getInstance();

	~WaitWindow();

	void show();
	void hide();

	void move(pp_int32 x, pp_int32 y);

	void update() { render(); }

	pp_int32 getWidth() { return 260; }
	pp_int32 getHeight() { return 80; }

	void setColor(const PPColor& color) { this->color = color; }
};

#endif
