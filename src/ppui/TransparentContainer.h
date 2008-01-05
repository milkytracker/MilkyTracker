/////////////////////////////////////////////////////////////////
//
//	PPContainer control that can be used to group controls
//
/////////////////////////////////////////////////////////////////
#ifndef TRANSPARENTCONTAINER__H
#define TRANSPARENTCONTAINER__H

#include "BasicTypes.h"
#include "Container.h"

class PPButton;

class PPTransparentContainer : public PPContainer
{
public:
	PPTransparentContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, PPSize size);

	virtual ~PPTransparentContainer();

	virtual void paint(PPGraphicsAbstract* graphics);
};

#endif
