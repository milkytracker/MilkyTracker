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

#include "WaitView.h"

#include <Region.h>

enum {
	kPolygonSpacing = 40,
	kPolygonSpeed = 4
};


WaitView::WaitView(BRect frame)
	:
	BView(frame, "WaitView", B_FOLLOW_ALL, B_WILL_DRAW | B_PULSE_NEEDED)
{
	SetViewColor(220, 220, 255);

	BPoint pointList[4];
	pointList[0].Set(20,  0);
	pointList[1].Set(40,  0);
	pointList[2].Set(20, 30);
	pointList[3].Set( 0, 30);
	fPolygon.AddPoints(pointList, 4);

	for (int i = 0; i < kNumPolygons; i++)
		fPosition[i] = i * kPolygonSpacing;
}


WaitView::~WaitView()
{

}


void
WaitView::Draw(BRect updateRect)
{
	BRect bounds = Bounds();

	SetHighColor(100, 100, 255);

	for (int i = 0; i < kNumPolygons; i++) {
		SetOrigin(fPosition[i], 0);
		FillPolygon(&fPolygon);
	}
}


void
WaitView::Pulse()
{
	BRect bounds = Bounds();

	for (int i = 0; i < kNumPolygons; i++) {
		fPosition[i] += kPolygonSpeed;
		if (fPosition[i] > bounds.IntegerWidth())
			fPosition[i] -= bounds.IntegerWidth() + kPolygonSpacing;
	}

	Invalidate();
}
