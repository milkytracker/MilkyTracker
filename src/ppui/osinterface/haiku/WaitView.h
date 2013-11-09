/*
 *  Copyright 2012 Julian Harnath
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

#ifndef __HAIKU_WAITVIEW_H__
#define __HAIKU_WAITVIEW_H__

#include <View.h>
#include <Polygon.h>

enum {
	kNumPolygons = 6
};

class WaitView : public BView
{
public:
						WaitView(BRect frame);
	virtual				~WaitView();

	virtual	void		Draw(BRect updateRect);
	virtual	void		Pulse();

private:
			BPolygon	fPolygon;
			int32		fPosition[kNumPolygons];
};

#endif // __HAIKU_WAITVIEW_H__
