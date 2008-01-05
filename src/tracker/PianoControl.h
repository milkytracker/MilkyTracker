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

	bool* pressed;

	Modes mode;
	pp_int32 currentSelectedNote;
	
	class PianoBitmapBase* pianoBitmap;

public:
	PianoControl(pp_int32 id, 
				 PPScreen* parentScreen, 
				 EventListenerInterface* eventListener, 
				 PPPoint location, 
				 PPSize size, 
				 pp_uint8 numNotes,
				 bool border = true);

	~PianoControl();

	void setMode(Modes mode) { this->mode = mode; }
	Modes getMode() { return mode; }

	void setBorderColor(const PPColor& color) { this->borderColor = &color; }

	void setxMax(pp_int32 xMax) { this->xMax = xMax; adjustScrollbars(); }
	pp_int32 getxMax() { return xMax; }

	void setyMax(pp_int32 yMax) { this->yMax = yMax; adjustScrollbars(); }
	pp_int32 getyMax() { return yMax; }

	void setxScale(pp_int32 scale);
	void setyScale(pp_int32 scale);

	pp_int32 getxScale() { return xscale; }
	pp_int32 getyScale() { return yscale; }

	void setSampleTable(const pp_uint8* nbu);

	void setSampleIndex(pp_int32 index) { sampleIndex = index; }

	void pressNote(pp_int32 note, bool pressed);

	bool getNoteState(pp_int32 note);

	void assureNoteVisible(pp_int32 note);

	// from PPControl
	virtual void paint(PPGraphicsAbstract* graphics);	
	virtual bool gainsFocus() { return false; }
	virtual bool isActive() { return true; }
	virtual pp_int32 callEventListener(PPEvent* event);
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	virtual void setLocation(PPPoint location);
	virtual void setSize(PPSize size);

private:
	pp_int32 getMaxWidth();

	void adjustScrollbars();

	pp_int32 positionToNote(PPPoint cp);
};


#endif
