/*
 *  tracker/ColorExportImport.h
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

/*
 *  ColorExportImport.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Mar 13 2005.
 *
 */
 
#ifndef __COLOREXPORTIMPORT_H__
#define __COLOREXPORTIMPORT_H__

#include "BasicTypes.h"
#include "ColorPaletteContainer.h"

class ColorExportImport
{
private:
	PPSystemString fileName;

public:
	ColorExportImport(const PPSystemString& fileName) :
		fileName(fileName)
	{
	}
	
	bool importColorPalette(TColorPalette& palette);
	bool exportColorPalette(const TColorPalette& palette);	
};

#endif
