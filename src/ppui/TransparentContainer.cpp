#include "GraphicsAbstract.h"
#include "TransparentContainer.h"
#include "BasicTypes.h"

PPTransparentContainer::PPTransparentContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, PPSize size) :
	PPContainer(id, parentScreen, eventListener, location, size)
{
}

PPTransparentContainer::~PPTransparentContainer()
{
}

void PPTransparentContainer::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;
	
	paintControls(g);
}
