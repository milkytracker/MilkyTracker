/*
 *  compression/DecompressorZIP.cpp
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
 *  DecompressorZIP.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 22.06.08.
 *
 */

#include "DecompressorZIP.h"
#include "XMFile.h"
#include "ZipExtractor.h"

// -- ZIP --------------------------------------------------------------------
DecompressorZIP::DecompressorZIP(const PPSystemString& filename) :
	DecompressorBase(filename)
{
}

bool DecompressorZIP::identify(XMFile& f)
{
	const PPSystemString filename(f.getFileName());
	PPSystemString ext = filename.getExtension();
	
	if ((ext.compareToNoCase(".ZIP") != 0) &&
		(ext.compareToNoCase(".MDZ") != 0))
		return false;

	ZipExtractor extractor(filename);
	
	pp_int32 error = 0;
	bool res = extractor.parseZip(error, false, NULL);
	return (res && error == 0);
}	
	
const PPSimpleVector<Descriptor>& DecompressorZIP::getDescriptors(Hints hint) const
{
	descriptors.clear();
	descriptors.add(new Descriptor("zip", "ZIP Archive")); 	

	if (hint == HintModules || hint == HintAll)
		descriptors.add(new Descriptor("mdz", "Zipped Module")); 	
		
	return descriptors;
}		
	
bool DecompressorZIP::decompress(const PPSystemString& outFilename, Hints hint)
{
	ZipExtractor extractor(fileName);
	
	pp_int32 error = 0;
	bool res = extractor.parseZip(error, true, &outFilename);
	return (res && error == 0);
}

DecompressorBase* DecompressorZIP::clone()
{
	return new DecompressorZIP(fileName);
}	

static Decompressor::RegisterDecompressor<DecompressorZIP> registerDecompressor;
