/////////////////////////////////////////////////////////////////
//
//	PPButton control class
//
/////////////////////////////////////////////////////////////////
#ifndef CHECKBOX__H
#define CHECKBOX__H

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"

// Forwards
class PPGraphicsAbstract;
class PPFont;
class PPButton;

class PPCheckBox : public PPControl, public EventListenerInterface
{
private:
	PPButton* button;

public:
	PPCheckBox(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, bool checked = true);
	
	~PPCheckBox();

	bool isChecked();

	void checkIt(bool checked);

	// from control
	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 callEventListener(PPEvent* event);

	virtual bool gainsFocus() { return false; }

	virtual bool isActive() { return true; }

	virtual void enable(bool b);
	
	virtual void setSize(PPSize size);
	virtual void setLocation(PPPoint location);

	// from EventListenerInterface
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);
};

#endif
