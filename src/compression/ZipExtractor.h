/*
 *  compression/ZipExtractor.h
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
 *  ZipExtractor.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 31.01.07.
 *
 */
 
#ifndef __ARCHIVEEXTRACTOR_H__
#define __ARCHIVEEXTRACTOR_H__

#include "BasicTypes.h"

class ZipExtractor
{
private:
	PPSystemString archivePath;

public:
	ZipExtractor(const PPSystemString& archivePath);

	bool parseZip(pp_int32& err, bool extract, const PPSystemString* outFile);
};

#endif
