/////////////////////////////////////////////////////////////////
//
//	PPContainer control that looks like a Message box 
//  (can contain other controls)
//
/////////////////////////////////////////////////////////////////
#ifndef MESSAGEBOXCONTAINER__H
#define MESSAGEBOXCONTAINER__H

#include "BasicTypes.h"
#include "Container.h"

class PPButton;

class PPMessageBoxContainer : public PPContainer
{
private:
	PPString caption;

	PPButton* button;

public:
	PPMessageBoxContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, PPSize size, const PPString& caption);

	virtual ~PPMessageBoxContainer();

	virtual void paint(PPGraphicsAbstract* graphics);

	virtual void setSize(PPSize size);
	virtual void setLocation(PPPoint location);

	void setCaption(const PPString& caption) { this->caption = caption; }
};

#endif
