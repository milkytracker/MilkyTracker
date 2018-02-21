/*
 *  tracker/FileTypes.h
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
 *  FileTypes.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 12.07.08.
 *
 */

#ifndef __FILETYPES_H__
#define __FILETYPES_H__

#include "BasicTypes.h"

struct FileTypes
{
	enum Types
	{
		FileTypeAllFiles,
		FileTypeSongAllModules,
		FileTypeSongMOD,
		FileTypeSongXM,
		FileTypeSongWAV,
		FileTypePatternXP,
		FileTypeTrackXT,
		FileTypeSongAllInstruments,
		FileTypeInstrumentXI,
		FileTypeSongAllSamples,
		FileTypeSampleWAV,
		FileTypeSampleIFF
	};
	
	Types type;

	FileTypes() : 
		type(FileTypeAllFiles)
	{
	}
	
	FileTypes(Types type) : 
		type(type)
	{
	}

	FileTypes(pp_int32 type) : 
		type((Types)type)
	{
	}
	
	operator pp_int32() const
	{
		return (pp_int32)type;
	}
};

#endif
