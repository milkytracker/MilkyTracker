/*
 *  tracker/FileIdentificator.h
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
 *  FileIdentificator.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.05.05.
 *
 */

#ifndef __FILEINDENTIFICATOR_H__
#define __FILEINDENTIFICATOR_H__

#include "BasicTypes.h"

class XMFile;

class FileIdentificator
{
public:
	enum FileTypes
	{
		FileTypeUnknown,
		FileTypeModule,
		FileTypeInstrument,
		FileTypeSample,
		FileTypePattern,
		FileTypeTrack,
		FileTypeCompressed
	};
	
private:
	PPSystemString fileName;
	XMFile* f;
	
	bool isModule();
	bool isInstrument();
	bool isSample();
	bool isPattern();
	bool isTrack();
	bool isCompressed();
	
public:
	FileIdentificator(const PPSystemString& fileName);
	~FileIdentificator();
	
	bool isValid() { return f != NULL; }
	
	FileTypes getFileType();
};

#endif
