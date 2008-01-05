/*
 *  FileIdentificator.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.05.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
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
