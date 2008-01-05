/////////////////////////////////////////////////////////////////
//
//	PPScrollbar control class
//
/////////////////////////////////////////////////////////////////
#ifndef SCROLLBAR__H
#define SCROLLBAR__H

//#ifndef __LOWRES__
	#define SCROLLBUTTONSIZE 10
//#else
//	#define SCROLLBUTTONSIZE 8
//#endif

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"

// Forwards
class PPGraphicsAbstract;
class PPButton;

class PPScrollbar : public PPControl, public EventListenerInterface
{
private:
	PPColor backgroundColor;

	pp_uint32 oneDimSize;
	bool horizontal;

	PPButton* backgroundButton;
	PPButton* buttonUp;
	PPButton* buttonDown;
	PPButton* buttonBar;

	PPControl* caughtControl;
	bool controlCaughtByLMouseButton, controlCaughtByRMouseButton;
	PPPoint caughtMouseLocation, caughtControlLocation;

	pp_int32 currentBarSize, currentBarPosition;
	
	//bool pressed;
	void initButtons();

public:
	PPScrollbar(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, pp_int32 size, bool horizontal = false);
	~PPScrollbar();	

	void setBackgroundColor(const PPColor& color) { backgroundColor = color; }

	// set bar size [none:0 - full:65536]
	void setBarSize(pp_int32 size, bool repaint = false);
	// set bar position [0 - 65536]
	void setBarPosition(pp_int32 pos, bool repaint = false);

	pp_uint32 getBarSize() { return currentBarSize; }
	pp_uint32 getBarPosition() { return currentBarPosition; }

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 callEventListener(PPEvent* event);
	
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	virtual bool gainsFocus() { return false; }

	virtual bool isActive() { return true; }

	virtual void setLocation(PPPoint location);

	virtual void setSize(pp_uint32 size);
};

#endif
