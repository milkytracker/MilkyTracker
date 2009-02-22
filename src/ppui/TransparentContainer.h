/*
 *  ppui/TransparentContainer.h
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
	PPTransparentContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
						   const PPPoint& location, const PPSize& size);

	virtual ~PPTransparentContainer();

	virtual void paint(PPGraphicsAbstract* graphics);
};

#endif
