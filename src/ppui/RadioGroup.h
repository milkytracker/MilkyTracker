/////////////////////////////////////////////////////////////////
//
//	PPRadioGroup control class
//
/////////////////////////////////////////////////////////////////
#ifndef RADIOGROUP__H
#define RADIOGROUP__H

#include "BasicTypes.h"
#include "Control.h"
#include "SimpleVector.h"

// Forwards
class PPGraphicsAbstract;
class PPFont;
class PPButton;

class PPRadioGroup : public PPControl
{
private:
	const PPColor* radioButtonColor;
	const PPColor* textColor;

	PPSimpleVector<PPString> items;

	pp_uint32 spacerHeight;

	pp_uint32 choice;

	PPFont* font;

	bool horizontal;

	pp_int32 maxWidth;

public:
	enum DEFAULTEXTENTS {
		eDefaultSpacerHeight = 5,
		eDefaultRadioWidth = 14
	};

	PPRadioGroup(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, PPSize size, pp_uint32 spacerHeight = eDefaultSpacerHeight);
	~PPRadioGroup();

	void setColor(const PPColor& color) { this->radioButtonColor = &color; }

	void addItem(const PPString& item);
	const PPString& getItem(pp_int32 index);

	void setSpacerHeight(pp_uint32 spacerHeight) { this->spacerHeight = spacerHeight; }

	void setFont(PPFont* font) { this->font = font; }

	void setHorizontal(bool b) { horizontal = b; }

	void setChoice(pp_uint32 choice) { this->choice = choice; }

	pp_uint32 getChoice() { return choice; }

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 callEventListener(PPEvent* event);

	virtual bool gainsFocus() { return false; }

	virtual bool isActive() { return true; }

	void fitSize(); 
};

#endif
