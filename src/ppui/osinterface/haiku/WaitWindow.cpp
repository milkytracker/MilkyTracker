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

#include "WaitWindow.h"
#include "WaitView.h"


WaitWindow::WaitWindow(int centerX, int centerY)
	:
	BWindow(BRect(centerX - 100, centerY - 15, centerX + 100, centerY + 15),
		"Working...", B_MODAL_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	fWaitView = new WaitView(Bounds());
	AddChild(fWaitView);
	SetPulseRate(20000);
}


WaitWindow::~WaitWindow()
{
}
