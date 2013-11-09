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

#include "DisplayDevice_Haiku.h"
#include "Graphics.h"
#include "MilkyView.h"
#include "MilkyWindow.h"
#include "WaitWindow.h"

#include <Application.h>
#include <Bitmap.h>
#include <Region.h>
#include <Screen.h>
#include <Window.h>

#include <stdio.h>

DisplayDevice_Haiku::DisplayDevice_Haiku(MilkyView* milkyView,
	pp_int32 scaleFactor)
	:
	PPDisplayDeviceBase(milkyView->Bounds().IntegerWidth(),
		milkyView->Bounds().IntegerHeight(), scaleFactor),
	fMilkyView(milkyView),
	fDirtyRegion(milkyView->DirtyRegion()),
	fBitmapRect(milkyView->Bitmap()->Bounds()),
	fBitmapLock(milkyView->BitmapLock()),
	fWaitWindow(NULL)
{
	// In general, the Tracker will hold the lock to the display buffer since
	// it could draw there anytime -- except when we block it, which we do in
	// the DisplayDevice_Haiku::update() method
	acquire_sem(fBitmapLock);

	PPGraphics_32bpp_generic* graphics = new PPGraphics_32bpp_generic(
		fMilkyView->Bounds().IntegerWidth(),
		fMilkyView->Bounds().IntegerHeight(),
		fMilkyView->Bitmap()->BytesPerRow(),
		fMilkyView->Bitmap()->Bits());
	graphics->setComponentBitpositions(16, 8, 0);

	currentGraphics = graphics;

	currentGraphics->lock = true;
}


DisplayDevice_Haiku::~DisplayDevice_Haiku()
{
	delete currentGraphics;
}


// #pragma mark - PPDisplayDeviceBase


PPGraphicsAbstract*
DisplayDevice_Haiku::open()
{
	if (!isEnabled())
		return NULL;

	currentGraphics->lock = false;

	return currentGraphics;
}


void
DisplayDevice_Haiku::close()
{
	currentGraphics->lock = true;
}


void
DisplayDevice_Haiku::update()
{
	fDirtyRegion->Include(fBitmapRect);
	release_sem(fBitmapLock);
	acquire_sem(fBitmapLock);
}


void
DisplayDevice_Haiku::update(const PPRect&r)
{
	// Translate coordinates: tracker -> view
	BRect updateRect(r.x1, r.y1, r.x2, r.y2);
	updateRect.OffsetBy(fMilkyView->TopLeft());

	fDirtyRegion->Include(updateRect);
	release_sem(fBitmapLock);
	acquire_sem(fBitmapLock);
}


void
DisplayDevice_Haiku::setSize(const PPSize& size)
{
	// It seems this method is never used, and I'm not sure what it's supposed
	// to be used for. The other platforms resize the window, so we do it here
	// as well, but resizing the window doesn't resize its bitmap which makes
	// it kind of useless IMO.
	fMilkyView->Window()->ResizeTo(size.width, size.height);

	// TODO: move window in fullscreen mode?

	this->size = size;
}


// #pragma mark - ex. PPWindow


void
DisplayDevice_Haiku::setTitle(const PPSystemString& title)
{
	fMilkyView->Window()->SetTitle(title);
}


bool
DisplayDevice_Haiku::goFullScreen(bool b)
{
	bool success = false;
	MilkyWindow* milkyWindow = (MilkyWindow*)fMilkyView->Window();

	if (milkyWindow->LockLooper()) {
		success = milkyWindow->SetFullScreen(b);
		milkyWindow->UnlockLooper();
	}
	else
		debugger("Failed to lock MilkyWindow looper");

	if (success)
		bFullScreen = b;
	else
		fprintf(stderr, "ERROR: setting fullscreen mode failed\n");

	return success;
}


PPSize
DisplayDevice_Haiku::getDisplayResolution() const
{
	BScreen screen;
	BRect frame = screen.Frame();
	return PPSize(frame.IntegerWidth(), frame.IntegerHeight());
}


void
DisplayDevice_Haiku::shutDown()
{
	fMilkyView->Window()->PostMessage(B_QUIT_REQUESTED);
}


void
DisplayDevice_Haiku::signalWaitState(bool b, const PPColor& color)
{
	if (b) {
		if (fWaitWindow != NULL)
			return;

		BWindow* window = fMilkyView->Window();
		BRect frame = window->Frame();

		fWaitWindow = new WaitWindow(frame.left + (frame.IntegerWidth() / 2),
			frame.top + (frame.IntegerHeight() / 2));
		fWaitWindow->Show();
	} else {
		if (fWaitWindow == NULL)
			return;

		fWaitWindow->LockLooper();
		fWaitWindow->Quit();
		fWaitWindow = NULL;
	}
}


void
DisplayDevice_Haiku::setMouseCursor(MouseCursorTypes type)
{
}
