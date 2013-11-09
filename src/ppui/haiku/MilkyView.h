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
#ifndef __HAIKU_MILKYVIEW_H__
#define __HAIKU_MILKYVIEW_H__

#include <View.h>

#include <Locker.h>
#include <Region.h>

#include "Event.h"

struct ModifierData;


class MilkyView : public BView
{
public:
							MilkyView(BRect frame,  int32 scaleFactor, sem_id trackerLock);
	virtual					~MilkyView();

	virtual	void			AttachedToWindow();
			void			MessageReceived(BMessage* message);

// --- Graphics output --------------------------------------------------------
public:
	virtual void			Draw(BRect updateRect);
			void			DrawScreen();

			BBitmap*		Bitmap()
								{ return fBitmap; }
			sem_id			BitmapLock()
								{ return fBitmapLock; }
			BRegion*		DirtyRegion()
								{ return &fDirtyRegion; }
			void			SetTopLeft(int x, int y)
								{ fBitmapRect.OffsetTo(x, y); }
			BPoint			TopLeft()
								{ return fBitmapRect.LeftTop(); }
private:
	static	status_t		_DrawThread(void* data);

			thread_id		fDrawThread;
			BBitmap*		fBitmap;		// protected by fBitmapLock
			BRect			fBitmapRect;
			sem_id			fBitmapLock;
			BRegion			fDirtyRegion;	// protected by fBitmapLock

// --- Keyboard input handling ------------------------------------------------
public:
	virtual void	 		KeyDown(const char* bytes, int32 numBytes);
	virtual void 			KeyUp(const char* bytes, int32 numBytes);
			void			ModifiersChanged(int32 oldModifiers,
								int32 modifiers);
private:
			void			_FillKeyArray(int32 keyCode, const char* bytes,
								int32 numBytes, pp_uint16* keyArray);
			void			_UpdateModifier(ModifierData* modifierData,
								int32 oldModifiers, int32 modifiers);

// --- Mouse input handling ---------------------------------------------------
public:
	virtual void			MouseDown(BPoint point);
	virtual void			MouseUp(BPoint point);
	virtual void			MouseMoved(BPoint point, uint32 transit,
											const BMessage* message);
			void			MouseWheelChanged(float deltaX, float deltaY);
	virtual void			Pulse();
private:
			int32			fMouseDownButtons;
			bigtime_t		fMouseDownTime;
};


#endif // __HAIKU_MILKYVIEW_H__
