/////////////////////////////////////////////////////////////////
//
//	PPContainer control class (can contain other controls)
//
/////////////////////////////////////////////////////////////////
#ifndef CONTAINER__H
#define CONTAINER__H

#include "BasicTypes.h"
#include "Control.h"
#include "Button.h"
#include "SimpleVector.h"

class PPContainer : public PPControl
{
protected:
	const PPColor* color;
	bool border;

private:
	PPButton* backgroundButton;

	PPSimpleVector<PPControl> controls;
	PPSimpleVector<PPControl>* timerEventControls;

	PPControl* focusedControl;

	PPPoint lastMousePoint;
	PPControl* lastMouseOverControl;

	// Control caught by mouse button press (left & right)
	PPControl* caughtControl;
	pp_int32 currentlyPressedMouseButtons;

public:
	PPContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, PPSize size, bool border = true);
	virtual ~PPContainer();

	virtual void setSize(PPSize size);
	virtual void setLocation(PPPoint location);

	//void setColor(pp_int32 r,pp_int32 g,pp_int32 b) { color.r = r; color.g = g; color.b = b; backgroundButton->setColor(color); }
	void setColor(const PPColor& color) { this->color = &color; backgroundButton->setColor(color); }

	const PPColor& getColor() { return *color; }

	void addControl(PPControl* control);

	bool removeControl(PPControl* control);

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 callEventListener(PPEvent* event);

	virtual bool gainsFocus();
	virtual bool gainedFocusByMouse();

	virtual void show(bool visible);
	virtual void hide(bool hidden);

	virtual bool isContainer() { return true; }

	PPControl* getControlByID(pp_int32 id);

	PPControl* getFocusedControl() { return focusedControl; }

	PPSimpleVector<PPControl>& getControls() { return controls; }

	void setFocus(PPControl* control, bool repaint = true);
	bool hasFocus(PPControl* control);
	
	void move(const PPPoint& offset);
	void adjustContainerSize();

protected:
	void paintControls(PPGraphicsAbstract* g)
	{
		for (pp_int32 i = 0; i < controls.size(); i++)
		{
			PPControl* ctrl = controls.get(i);
			if (ctrl->isVisible())
				ctrl->paint(g);
		}
	}
	
	friend class PPScreen;
};

#endif
