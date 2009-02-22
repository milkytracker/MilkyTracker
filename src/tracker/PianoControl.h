/*
 *  tracker/PianoControl.h
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

#ifndef PIANOCONTROL__H
#define PIANOCONTROL__H

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"

class PianoControl : public PPControl, public EventListenerInterface
{
public:
	enum Modes
	{
		ModeEdit = 0,
		ModePlay = 1
	};

private:
	pp_int32 XMAX();
	pp_int32 YMAX();
	pp_int32 KEYWIDTH();
	
	const pp_uint8 NUMNOTES;

	bool border;
	PPColor ourOwnBorderColor;
	const PPColor* borderColor;

	class PPScrollbar* hScrollbar;	

	PPControl* caughtControl;
	bool controlCaughtByLMouseButton, controlCaughtByRMouseButton;

	// extent
	pp_int32 xscale;
	pp_int32 yscale;

	pp_int32 xMax;
	pp_int32 yMax;

	pp_int32 startPos;
	pp_int32 visibleWidth;
	pp_int32 visibleHeight;

	pp_int32 sampleIndex;

	pp_uint8* nbu;

	struct KeyState
	{
		bool pressed;
		bool muted;
		
		KeyState() :
			pressed(false),
			muted(false)
		{
		}
	};
	
	KeyState* keyState;

	Modes mode;
	pp_int32 currentSelectedNote;
	
	class PianoBitmapBase* pianoBitmap;

public:
	PianoControl(pp_int32 id, 
				 PPScreen* parentScreen, 
				 EventListenerInterface* eventListener, 
				 const PPPoint& location, 
				 const PPSize& size, 
				 pp_uint8 numNotes,
				 bool border = true);

	~PianoControl();

	void setMode(Modes mode) { this->mode = mode; }
	Modes getMode() const { return mode; }

	void setBorderColor(const PPColor& color) { this->borderColor = &color; }

	void setxMax(pp_int32 xMax) { this->xMax = xMax; adjustScrollbars(); }
	pp_int32 getxMax() const { return xMax; }

	void setyMax(pp_int32 yMax) { this->yMax = yMax; adjustScrollbars(); }
	pp_int32 getyMax() const { return yMax; }

	void setxScale(pp_int32 scale);
	void setyScale(pp_int32 scale);

	pp_int32 getxScale() const { return xscale; }
	pp_int32 getyScale() const { return yscale; }

	void setSampleTable(const pp_uint8* nbu);

	void setSampleIndex(pp_int32 index) { sampleIndex = index; }

	void pressNote(pp_int32 note, bool pressed, bool muted = false);

	bool getNoteState(pp_int32 note) const;

	void assureNoteVisible(pp_int32 note);

	// from PPControl
	virtual void paint(PPGraphicsAbstract* graphics);	
	virtual pp_int32 dispatchEvent(PPEvent* event);
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	virtual void setLocation(const PPPoint& location);
	virtual void setSize(const PPSize& size);

private:
	pp_int32 getMaxWidth();

	void adjustScrollbars();

	pp_int32 positionToNote(PPPoint cp);
};


#endif
