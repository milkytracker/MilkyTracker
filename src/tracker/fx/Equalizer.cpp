/*
 *  tracker/Equalizer.cpp
 *
 *  Copyright 2009 David Ross (david_ross@hotmail.co.uk)
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

#include "Equalizer.h"

Equalizer::Equalizer(void) : xL1(0), xL2(0), xR1(0), xR2(0),
							 yL1(0), yL2(0), yR1(0), yR2(0),
							 b0(0), b1(0), b2(0), a0(0), a1(0), a2(0)
{
}

Equalizer::~Equalizer(void)
{
}

void Equalizer::CalcCoeffs(float centre, float width, float rate, float gain)
{
	const double twopi = 6.283185307179586476925286766559;

	double w0 = twopi * centre / rate;
	double dw = twopi * width / rate;

	double m1 = tan(dw/2) / 1 + tan(dw/2);
	double m2 = (1 - pow(tan(w0/2), 2)) / (1 + pow(tan(w0/2), 2));

	b0 = 1 + (gain - 1) * m1;
	b1 = 2 * m2 * (m1 - 1);
	b2 = 1 - m1 * (1 + gain);

	a0 = 1;
	a1 = b1;
	a2 = 1 - 2 * m1;
}
void Equalizer::Filter(double xL, double xR, double &yL, double &yR)
{
	const double denorm 	= 1e-24f;

	yL = denorm + (b0*xL + b1*xL1 + b2*xL2 - a1*yL1 - a2*yL2);
	yR = denorm + (b0*xR + b1*xR1 + b2*xR2 - a1*yR1 - a2*yR2);

	xL2 = xL1;
	xL1 = xL;
	xR2 = xR1;
	xR1 = xR;

	yL2 = yL1;
	yL1 = yL;
	yR2 = yR1;
	yR1 = yR;
}
