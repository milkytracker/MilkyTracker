/////////////////////////////////////////////////////////////////
//
//	PPSlider control class
//
/////////////////////////////////////////////////////////////////
#ifndef SLIDER__H
#define SLIDER__H

#define SLIDERBUTTONSIZE 12
#define SLIDERBUTTONHEIGHT 10
#define SLIDERBUTTONWIDTH 12

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"

// Forwards
class PPGraphicsAbstract;
class PPButton;

class PPSlider : public PPControl, public EventListenerInterface
{
private:
	PPColor backgroundColor;

	pp_uint32 oneDimSize;
	bool horizontal;
	bool buttonSwap;

	PPButton* backgroundButton;
	PPButton* buttonUp;
	PPButton* buttonDown;
	PPButton* buttonBar;

	PPControl* caughtControl;
	bool controlCaughtByLMouseButton, controlCaughtByRMouseButton;
	PPPoint caughtMouseLocation, caughtControlLocation;

	pp_int32 currentBarSize, currentBarPosition;

	pp_uint32 minValue;
	pp_uint32 maxValue;
	pp_uint32 currentValue;

	//bool pressed;

	void initButtons();

public:
	PPSlider(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, pp_int32 size, bool horizontal = false, bool buttonSwap = false);
	~PPSlider();	

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 callEventListener(PPEvent* event);
	
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	virtual bool gainsFocus() { return false; }

	virtual bool isActive() { return true; }

	virtual void setLocation(PPPoint location);
	
	virtual void setSize(pp_uint32 size);

	void setMinValue(pp_uint32 min) { minValue = min; }
	void setMaxValue(pp_uint32 max) { maxValue = max; }
	void setCurrentValue(pp_uint32 newValue)
	{
		currentValue = newValue;
		float f = (float)(currentValue - minValue)/(maxValue - minValue);

		setBarPosition((pp_int32)(f*65536.0f));
	}
	pp_uint32 getCurrentValue() { return currentValue; }

	void setBackgroundColor(const PPColor& color) { backgroundColor = color; }

	// set bar size [none:0 - full:65536]
	void setBarSize(pp_int32 size, bool repaint = false);
	// set bar position [0 - 65536]
	void setBarPosition(pp_int32 pos, bool repaint = false);

	pp_uint32 getBarSize() { return currentBarSize; }
	pp_uint32 getBarPosition() { return currentBarPosition; }
};

#endif
