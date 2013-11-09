/*
 *  Copyright 2012 Julian Harnath
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
#ifndef __HAIKU_DISPLAYDEVICE_HAIKU_H__
#define __HAIKU_DISPLAYDEVICE_HAIKU_H__

#include "BasicTypes.h"
#include "DisplayDeviceBase.h"

#include <OS.h>
#include <Rect.h>

class BRegion;
class MilkyView;
class WaitWindow;


class DisplayDevice_Haiku : public PPDisplayDeviceBase
{
public:
							DisplayDevice_Haiku(MilkyView* milkyWindow,
								pp_int32 scaleFactor);
	virtual					~DisplayDevice_Haiku();

private:
			MilkyView*		fMilkyView;
			BRegion*		fDirtyRegion;
			BRect			fBitmapRect;
			sem_id			fBitmapLock;
			WaitWindow*		fWaitWindow;

	// --- PPDisplayDeviceBase ------------------------------------------------
public:
	virtual	PPGraphicsAbstract*	open();
	virtual	void			close();

	virtual	void			update();
	virtual	void			update(const PPRect&r);

	virtual	void			setSize(const PPSize& size);

	virtual	bool 			supportsScaling() const { return false; }

	// --- ex. PPWindow -------------------------------------------------------
public:
	virtual	void			setTitle(const PPSystemString& title);

	virtual	bool			goFullScreen(bool b);

	virtual	PPSize			getDisplayResolution() const;

	virtual	void			shutDown();
	virtual	void			signalWaitState(bool b, const PPColor& color);

	virtual	void			setMouseCursor(MouseCursorTypes type);
};

#endif // __DISPLAYDEVICE_HAIKU_H__
