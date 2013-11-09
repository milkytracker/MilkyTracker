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

#include "MilkyView.h"
#include "MilkyWindow.h"

#include "Event.h"
#include "KeyCodeMap.h"

#include <Application.h>
#include <Bitmap.h>
#include <Entry.h>
#include <Region.h>
#include <UTF8.h>


MilkyView::MilkyView(BRect frame, int32 scaleFactor, sem_id trackerLock)
	:
	BView(frame, "MilkyView", B_FOLLOW_ALL, B_PULSE_NEEDED | B_WILL_DRAW),
	fMouseDownButtons(0),
	fMouseDownTime(0),
	fBitmap(new BBitmap(frame, B_RGBA32)),
	fBitmapRect(frame)
{
	SetViewColor(B_TRANSPARENT_32_BIT);

	fBitmapLock = create_sem(1, "Bitmap lock");
}


MilkyView::~MilkyView()
{
	// Stop drawing
	kill_thread(fDrawThread);

	// Release bitmap
	delete fBitmap;
	delete_sem(fBitmapLock);
}


void
MilkyView::AttachedToWindow()
{
	// Start drawing thread ---------------------------------------------------
	fDrawThread = spawn_thread(&_DrawThread, "Milky drawing",
		B_URGENT_DISPLAY_PRIORITY, this);

	if (fDrawThread < B_OK)
		debugger("Could not setup drawing thread");

	status_t status = resume_thread(fDrawThread);

	if (status < B_OK)
		debugger("Could not start drawing thread");

	// Various things ---------------------------------------------------------
	SetEventMask(0, B_NO_POINTER_HISTORY); // Receive less mouse move events
	MakeFocus(); // Get keyboard input focus
}


void
MilkyView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_SIMPLE_DATA: {
			// We received a drag & drop message, probably from Tracker
			// (the file manager in this case)... Check if there is a file
			// reference in it
			entry_ref ref;
			status_t status = message->FindRef("refs", &ref);
			if (status == B_OK) {
				// Forward message to the application which will check the file
				// type and try to open it
				be_app->RefsReceived(message);
			}
		}	break;

		default:
			BView::MessageReceived(message);
			break;
	}
}


// #pragma mark - Graphics output


void
MilkyView::Draw(BRect updateRect)
{
	// If our top left was moved, fill the border around the screen with black
	if (fBitmapRect.left != 0 || fBitmapRect.top != 0) {
		BRegion fillRegion(updateRect);
		fillRegion.Exclude(fBitmapRect);
		SetHighColor(0, 0, 0);
		FillRegion(&fillRegion);
	}

	// Blit the tracker screen bitmap
	BRect targetRect = updateRect & fBitmapRect;
	BRect sourceRect = targetRect.OffsetByCopy(-fBitmapRect.left,
		-fBitmapRect.top);
	DrawBitmap(fBitmap, sourceRect, targetRect, B_WAIT_FOR_RETRACE);
}


void
MilkyView::DrawScreen()
{
	bigtime_t beginTime = 0;
	MilkyWindow* milkyWindow = (MilkyWindow*)Window();

	for(;;) {
		beginTime = system_time();

		acquire_sem(fBitmapLock);
		milkyWindow->LockLooper();

		const uint32 dirtyRectCount = fDirtyRegion.CountRects();
		for (uint32 index = 0; index < dirtyRectCount; index++)
			Draw(fDirtyRegion.RectAt(index));
//		Sync();

		milkyWindow->UnlockLooper();
		fDirtyRegion.MakeEmpty();
		release_sem(fBitmapLock);

		// Limit framerate to 50 fps
		snooze_until(beginTime + 20000, B_SYSTEM_TIMEBASE);
	}
}


status_t
MilkyView::_DrawThread(void* data)
{
	MilkyView* milkyView = (MilkyView*)data;
	milkyView->DrawScreen();
	return B_OK;
}


// #pragma mark - Keyboard input handling


void
MilkyView::KeyDown(const char* bytes, int32 numBytes)
{
	MilkyWindow* milkyWindow = (MilkyWindow*)Window();

	BMessage* message = Window()->CurrentMessage();
	int32 key = message->FindInt32("key");

	pp_uint16 keyArray[3];
	_FillKeyArray(key, bytes, numBytes, keyArray);

	PPEvent event(eKeyDown, &keyArray, sizeof(keyArray));
	milkyWindow->RaiseEvent(&event);

	// On Command+Enter, toggle fullscreen mode
	if (key == 0x47 && (modifiers() & B_COMMAND_KEY)) {
		PPEvent event(eFullScreen);
		milkyWindow->RaiseEvent(&event);
	}

	// For printable characters, also raise an eKeyChar event
	uint16 character = bytes[0];
	if (character >= 32 && character <= 127) {
		PPEvent event(eKeyChar, &character, sizeof(character));
		milkyWindow->RaiseEvent(&event);
	}
}


void
MilkyView::KeyUp(const char* bytes, int32 numBytes)
{
	MilkyWindow* milkyWindow = (MilkyWindow*)Window();

	BMessage* message = Window()->CurrentMessage();
	int32 key = message->FindInt32("key");

	pp_uint16 keyArray[3];
	_FillKeyArray(key, bytes, numBytes, keyArray);

	PPEvent event(eKeyUp, &keyArray, sizeof(keyArray));
	milkyWindow->RaiseEvent(&event);
}


void
MilkyView::ModifiersChanged(int32 oldModifiers, int32 modifiers)
{
	_UpdateModifier(&gModifierDataShift,   oldModifiers, modifiers);
	_UpdateModifier(&gModifierDataControl, oldModifiers, modifiers);
	_UpdateModifier(&gModifierDataCommand, oldModifiers, modifiers);
}


void
MilkyView::_FillKeyArray(int32 keyCode, const char* bytes, int32 numBytes,
	pp_uint16* keyArray)
{
	keyArray[0] = KeyCodeToVK(keyCode, *bytes);	// virtualkey
	keyArray[1] = KeyCodeToSC(keyCode);			// scancode

	uint8* charBytes = (uint8*)&keyArray[2];	// character
	charBytes[0] = bytes[0];
	if (numBytes > 1)
		charBytes[1] = bytes[1];
}


void
MilkyView::_UpdateModifier(ModifierData* modifierData, int32 oldModifiers,
	int32 modifiers)
{
	MilkyWindow* milkyWindow = (MilkyWindow*)Window();

	bool oldState = (oldModifiers & modifierData->modifier) != 0;
	bool state    = (modifiers & modifierData->modifier) != 0;

	pp_uint16 keyArray[3] = { modifierData->vk, 0, 0 };

	if (!oldState && state) {
		// Key pressed ----------------------------------------------
		setKeyModifier(modifierData->keyModifiers);

		// Send generic key event
		PPEvent event(eKeyDown, &keyArray, sizeof(keyArray));
		milkyWindow->RaiseEvent(&event);

		// Send another key event for left or right key version
		if ((modifiers & modifierData->modifierLeft) != 0) {
			keyArray[0] = modifierData->vkLeft;
			PPEvent leftEvent(eKeyDown, &keyArray, sizeof(keyArray));
			milkyWindow->RaiseEvent(&leftEvent);
		} else {
			keyArray[0] = modifierData->vkRight;
			PPEvent rightEvent(eKeyDown, &keyArray, sizeof(keyArray));
			milkyWindow->RaiseEvent(&rightEvent);
		}
	} else if (oldState && !state) {
		// Key lifted -----------------------------------------------
		clearKeyModifier(modifierData->keyModifiers);

		// Send generic key event
		PPEvent event(eKeyUp, &keyArray, sizeof(keyArray));
		milkyWindow->RaiseEvent(&event);

		// Send another key event for left or right key version
		if ((oldModifiers & modifierData->modifierLeft) != 0) {
			keyArray[0] = modifierData->vkLeft;
			PPEvent leftEvent(eKeyDown, &keyArray, sizeof(keyArray));
			milkyWindow->RaiseEvent(&leftEvent);
		} else {
			keyArray[0] = modifierData->vkRight;
			PPEvent rightEvent(eKeyDown, &keyArray, sizeof(keyArray));
			milkyWindow->RaiseEvent(&rightEvent);
		}
	}
}


// #pragma mark - Mouse input handling


void
MilkyView::MouseDown(BPoint point)
{
	// Ignore mouse events outside the bitmap area
	if (!fBitmapRect.Contains(point))
		return;

	BMessage* message = Window()->CurrentMessage();
	int32 buttons = message->FindInt32("buttons");

	// Translate coordinates: view -> tracker
	point -= fBitmapRect.LeftTop();
	PPPoint pPoint(point.x, point.y);

	// Raise event(s)
	EEventDescriptor singleClickEventDescriptor;
	EEventDescriptor doubleClickEventDescriptor;
	if (buttons & B_PRIMARY_MOUSE_BUTTON) {
		singleClickEventDescriptor = eLMouseDown;
		doubleClickEventDescriptor = eLMouseDoubleClick;
	} else if (buttons & B_SECONDARY_MOUSE_BUTTON) {
		singleClickEventDescriptor = eRMouseDown;
		doubleClickEventDescriptor = eRMouseDoubleClick;
	} else
		return;

	MilkyWindow* milkyWindow = (MilkyWindow*)Window();

	PPEvent event(singleClickEventDescriptor, &pPoint, sizeof(PPPoint));
	milkyWindow->RaiseEvent(&event);

	if (message->FindInt32("clicks") == 2) {
		PPEvent event(doubleClickEventDescriptor, &pPoint, sizeof(PPPoint));
		milkyWindow->RaiseEvent(&event);
	}

	// Activate pulses which will be used to repeat click events when a mouse
	// button is held for longer than 500ms
	fMouseDownButtons = buttons &
		(B_PRIMARY_MOUSE_BUTTON | B_SECONDARY_MOUSE_BUTTON);
	fMouseDownTime = system_time();
	Window()->SetPulseRate(20000);

	// We want to keep receive the following mouse up event even when it's
	// outside the window
	SetMouseEventMask(B_POINTER_EVENTS);
}


void
MilkyView::MouseUp(BPoint point)
{
	BMessage* message = Window()->CurrentMessage();
	int32 buttons = message->FindInt32("buttons");

	// Translate coordinates: view -> tracker
	point -= fBitmapRect.LeftTop();
	PPPoint pPoint(point.x, point.y);

	// Raise event
	EEventDescriptor eventDescriptor;
	if (fMouseDownButtons & B_PRIMARY_MOUSE_BUTTON)
		eventDescriptor = eLMouseUp;
	else if (fMouseDownButtons & B_SECONDARY_MOUSE_BUTTON)
		eventDescriptor = eRMouseUp;
	else
		return;

	MilkyWindow* milkyWindow = (MilkyWindow*)Window();

	PPEvent event(eventDescriptor, &pPoint, sizeof(PPPoint));
	milkyWindow->RaiseEvent(&event);

	// Disable pulses when no more mouse button is depresseed
	fMouseDownButtons = buttons &
		(B_PRIMARY_MOUSE_BUTTON | B_SECONDARY_MOUSE_BUTTON);
	if (fMouseDownButtons == 0)
		Window()->SetPulseRate(0);
}


void
MilkyView::MouseMoved(BPoint point, uint32 transit, const BMessage* message)
{
	static bigtime_t sMouseMoveTime = 0;

	// Ignore mouse events outside the bitmap area
	if (!fBitmapRect.Contains(point))
		return;

	BMessage* moveMessage = Window()->CurrentMessage();
	int32 buttons = moveMessage->FindInt32("buttons");

	// Translate coordinates: view -> tracker
	point -= fBitmapRect.LeftTop();
	PPPoint pPoint(point.x, point.y);

	// Raise event (if a mouse button is depressed, make it a drag event)
	EEventDescriptor eventDescriptor = eMouseMoved;

	if (transit == B_INSIDE_VIEW) {
		if (buttons & B_PRIMARY_MOUSE_BUTTON)
			eventDescriptor = eLMouseDrag;
		else if (buttons & B_SECONDARY_MOUSE_BUTTON)
			eventDescriptor = eRMouseDrag;
	}

	// Limit the number of mouse move events to max 20 per second
	// Without the limit, dragging scrollbars is lagging because it can't
	// keep up with drawing
	bigtime_t currentTime = system_time();
	if (currentTime > (sMouseMoveTime + 50000)) {
		sMouseMoveTime = currentTime;

		MilkyWindow* milkyWindow = (MilkyWindow*)Window();

		PPEvent event(eventDescriptor, &pPoint, sizeof(PPPoint));
		milkyWindow->RaiseEvent(&event);
	}
}


void
MilkyView::MouseWheelChanged(float deltaX, float deltaY)
{
	BPoint point;
	uint32 buttons;
	GetMouse(&point, &buttons);

	// Ignore mouse events outside the bitmap area
	if (!fBitmapRect.Contains(point))
		return;

	// Translate coordinates: view -> tracker
	point -= fBitmapRect.LeftTop();

	// Raise event
	TMouseWheelEventParams wheelEventParams;
	wheelEventParams.pos.x = (pp_int32)point.x;
	wheelEventParams.pos.y = (pp_int32)point.y;
	wheelEventParams.delta = (pp_int32)-deltaY;

	MilkyWindow* milkyWindow = (MilkyWindow*)Window();

	PPEvent event(eMouseWheelMoved, &wheelEventParams,
		sizeof(wheelEventParams));
	milkyWindow->RaiseEvent(&event);
}


void
MilkyView::Pulse()
{
	if (fMouseDownButtons == 0) {
		// This shouldn't happen, but it's not fatal, so print a warning.
		// If you see this warning, find and fix the bug :-)
		fprintf(stderr, "Warning: MilkyView::Pulse() called without pressed "
		                "mouse buttons\n");
		Window()->SetPulseRate(0);
		return;
	}

	// Do nothing if the button press was less than 500ms ago
	if (system_time() < (fMouseDownTime + 500000))
		return;

	BPoint point;
	uint32 buttons;
	GetMouse(&point, &buttons);

	// Ignore mouse events in the black border in fullscreen mode
	if (!fBitmapRect.Contains(point))
		return;

	// Translate coordinates: view -> tracker
	point -= fBitmapRect.LeftTop();

	// Raise event
	EEventDescriptor eventDescriptor;
	if (buttons & B_PRIMARY_MOUSE_BUTTON)
		eventDescriptor = eLMouseRepeat;
	else if (buttons & B_SECONDARY_MOUSE_BUTTON)
		eventDescriptor = eRMouseRepeat;
	else
		return;

	MilkyWindow* milkyWindow = (MilkyWindow*)Window();

	PPPoint pPoint(point.x, point.y);
	PPEvent event(eventDescriptor, &pPoint, sizeof(PPPoint));
	milkyWindow->RaiseEvent(&event);
}
