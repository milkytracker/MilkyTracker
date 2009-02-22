/*
 *  tracker/FileIdentificator.cpp
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
 *  FileIdentificator.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.05.05.
 *
 */

#include "FileIdentificator.h"
#include "XMFile.h"
#include "XModule.h"
#include "SampleLoaderGeneric.h"
#include "Decompressor.h"

FileIdentificator::FileIdentificator(const PPSystemString& fileName) :
	fileName(fileName)
{
	f = new XMFile(fileName);
	if (!f->isOpen())
	{
		delete f;
		f = NULL;
	}
}

FileIdentificator::~FileIdentificator() 
{
	delete f;
}

FileIdentificator::FileTypes FileIdentificator::getFileType()
{
	if (!isValid())
		return FileTypeUnknown;

	// Check instruments first = sure thing
	if (isInstrument())
		return FileTypeInstrument;

	// Check patterns and tracks second = sure thing
	if (isPattern())
		return FileTypePattern;
		
	if (isTrack())
		return FileTypeTrack;

	// Okay now check module, could go wrong because of several weak file type detections
	if (isModule())
		return FileTypeModule;

	// check for packed file types last
	if (isCompressed())
		return FileTypeCompressed;
			
	// Okay now check samples, if sample type can not be detected it will assume raw-sample and load
	if (isSample())
		return FileTypeSample;		

	return FileTypeUnknown;
}

bool FileIdentificator::isModule()
{
	mp_ubyte buffer[2048];
	
	f->seek(0);
	f->read(buffer, 1, 2048);
	
	return XModule::identifyModule(buffer) != NULL;
}

bool FileIdentificator::isInstrument()
{
	char sig[32];

	f->seek(0);
	f->read(sig, 1, 21);
	
	if (memcmp(sig,"GF1PATCH",8) == 0)
		return true;

	return memcmp(sig,"Extended Instrument: ",21) == 0;
}

bool FileIdentificator::isSample()
{
	XModule* module = NULL;

	SampleLoaderGeneric sampleLoader(fileName, *module);
		
	return sampleLoader.identifySample();
}

bool FileIdentificator::isPattern()
{
	if ((f->size() - 4) % (32*5) != 0)
		return false;
		
	f->seek(0);

	mp_uword wtf = f->readWord();
	if (wtf != 1)
		return false;

	mp_uword numRows = f->readWord();
	if (numRows > 256)
		return false;
	
	PPSystemString ext = fileName.getExtension();
	ext.toUpper();
	
	if (ext.compareTo(".XP") != 0)
		return false;
										
	return true;
}

bool FileIdentificator::isTrack()
{
	if ((f->size() - 4) % (32*5) != 0)
		return false;
		
	f->seek(0);

	mp_uword wtf = f->readWord();
	if (wtf != 1)
		return false;

	mp_uword numRows = f->readWord();
	if (numRows > 256)
		return false;
	
	PPSystemString ext = fileName.getExtension();
	ext.toUpper();
	
	if (ext.compareTo(".XT") != 0)
		return false;
										
	return true;
}

bool FileIdentificator::isCompressed()
{
	Decompressor decompressor(fileName);
	f->seek(0);
	return decompressor.identify(*f);
}



