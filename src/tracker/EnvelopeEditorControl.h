/*
 *  tracker/EnvelopeEditorControl.h
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

#ifndef ENVELOPEEDITORCONTROL__H
#define ENVELOPEEDITORCONTROL__H

#include "BasicTypes.h"
#include "EditorBase.h"
#include "Control.h"
#include "Event.h"

class EnvelopeEditorControl : public PPControl, public EventListenerInterface, public EditorBase::EditorNotificationListener
{
private:
	PPColor ourOwnBorderColor;
	PPColor backgroundColor;

	bool border;
	const PPColor* borderColor;

	class PPScrollbar* hScrollbar;	

	class PPControl* caughtControl;
	bool controlCaughtByLMouseButton, controlCaughtByRMouseButton;

	class PPContextMenu* editMenuControl;
	class PPContextMenu* subMenuAdvanced;

	class EnvelopeEditor* envelopeEditor;
	const struct TEnvelope* envelope;

	// extent
	pp_int32 xMax;
	pp_int32 yMax;
	PPPoint currentPosition;

	pp_int32 xScale;

	// panning envelopes show the zero axis 
	bool showVCenter;

	pp_int32 startPos;
	pp_int32 visibleWidth;
	pp_int32 visibleHeight;
	bool hasDragged;
	bool invertMWheelZoom;

	// selection
	pp_int32 selectionTicker;

	// Envelope counters
	struct ShowMark
	{
		pp_int32 pos;
		pp_int32 intensity;
		pp_int32 panning;
	} *showMarks;

	enum MenuCommandIDs
	{
		MenuCommandIDXScale = 99,
		MenuCommandIDYScale
	};

public:
	EnvelopeEditorControl(pp_int32 id, 
						  PPScreen* parentScreen, 
						  EventListenerInterface* eventListener, 
						  const PPPoint& location, 
						  const PPSize& size, 
						  bool border = true);

	~EnvelopeEditorControl();

	void setColor(const PPColor& color) { backgroundColor = color; }

	void setBorderColor(const PPColor& color) { borderColor = &color; }

	void setxMax(pp_int32 xMax) { this->xMax = xMax; adjustScrollbars(); }
	pp_int32 getxMax() { return xMax; }

	void setyMax(pp_int32 yMax) { this->yMax = yMax; adjustScrollbars(); }
	pp_int32 getyMax() { return yMax; }

	void attachEnvelopeEditor(EnvelopeEditor* envelopeEditor);
	
	const TEnvelope* getEnvelope() { return envelope; }
	EnvelopeEditor* getEnvelopeEditor() { return envelopeEditor; }
	
	void setShowVCenter(bool b) { showVCenter = b; }

	void setInvertMWheelZoom(bool invert) { invertMWheelZoom = invert; }

	void paintGrid(PPGraphicsAbstract* graphics, pp_int32 xOffset, pp_int32 yOffset);
	
	// from PPControl
	virtual void paint(PPGraphicsAbstract* graphics);	
	virtual pp_int32 dispatchEvent(PPEvent* event);
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	virtual void setSize(const PPSize& size);
	virtual void setLocation(const PPPoint& location);

protected:
	virtual void translateCoordinates(PPPoint& cp) 
	{
		PPControl::translateCoordinates(cp);
		cp.x -= 2;
		cp.y -= 2;
	}

public:
	void reset();
	
	// Showmarks
	void setShowMark(pp_int32 c, pp_int32 m, pp_int32 i = 256, pp_int32 p = 128) { showMarks[c].pos = m; showMarks[c].intensity = i; showMarks[c].panning = p; }
	pp_int32 getShowMarkPos(pp_int32 c) { return showMarks[c].pos; }

	void clearShowMarks();
	bool hasShowMarks();
	
	// set scale from 16 to 1024
	void setScale(pp_int32 scale);
	
	pp_int32 getScale() { return xScale; }
	
private:
	pp_int32 getMaxWidth();

	void adjustScrollbars();

	pp_int32 selectEnvelopePoint(pp_int32 x, pp_int32 y);

	void setEnvelopePoint(pp_int32 index, pp_int32 x, pp_int32 y);
	
	float calcXScale();
	float calcYScale();
	
	void updateCurrentPosition(const PPPoint& cp);
	
	void validate();

	void invokeContextMenu(PPPoint p);

	void executeMenuCommand(pp_int32 commandId);

public:
	enum EnvelopeToolTypes
	{
		EnvelopeToolTypeScaleX,
		EnvelopeToolTypeScaleY
	};

	bool invokeToolParameterDialog(EnvelopeToolTypes type);

private:
	// --- envelope tool responder
	class ToolHandlerResponder : public DialogResponder
	{
	
	private:
		EnvelopeEditorControl& envelopeEditorControl;
		EnvelopeToolTypes envelopeToolType;
		
	public:
		ToolHandlerResponder(EnvelopeEditorControl& theEnvelopeEditorControl);

		void setEnvelopeToolType(EnvelopeToolTypes type) { envelopeToolType = type; }
		EnvelopeToolTypes getEnvelopeToolType() { return envelopeToolType; }
				
		virtual pp_int32 ActionOkay(PPObject* sender);
		virtual pp_int32 ActionCancel(PPObject* sender);
	};

	friend class ToolHandlerResponder;

	class PPDialogBase* dialog;
	ToolHandlerResponder* toolHandlerResponder;
	
	bool invokeTool(EnvelopeToolTypes type);

	// Last values
	struct TLastValues
	{
		float scaleEnvelope;
	};
	
	TLastValues lastValues;
	
	void resetLastValues()
	{
		lastValues.scaleEnvelope = -1.0f;
	}	

	void notifyUpdate()
	{
		PPEvent e(eUpdated);						
		eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}
	
	virtual void editorNotification(EditorBase* sender, EditorBase::EditorNotifications notification);
};


#endif
