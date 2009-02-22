/*
 *  tracker/ColorExportImport.cpp
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
 *  ColorExportImport.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Mar 13 2005.
 *
 */

#include "ColorExportImport.h"
#include "TrackerSettingsDatabase.h"
#include "XMFile.h"

bool ColorExportImport::importColorPalette(TColorPalette& palette)
{
	TrackerSettingsDatabase colorDataBase(1);

	XMFile f(fileName);

	if (!f.isOpen())
	{
		// deal with error
		return false;
	}
	
	if (!colorDataBase.serialize(f))
		return false;
	
	PPDictionaryKey* key = colorDataBase.restore("ACTIVECOLORS");
	
	if (key)
	{
		palette = ColorPaletteContainer::decodePalette(key->getStringValue());			
	}	
	
	return key != NULL;
}

bool ColorExportImport::exportColorPalette(const TColorPalette& palette)
{
	TrackerSettingsDatabase colorDataBase(1);
	colorDataBase.store("ACTIVECOLORS", ColorPaletteContainer::encodePalette(palette));
		
	XMFile f(fileName, true);
	if (!f.isOpenForWriting())
	{
		// deal with error
		return false;
	}
		
	if (!colorDataBase.serialize(f))
		return false;
	
	return true;
}
