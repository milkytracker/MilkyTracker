/*
 *  ZipExtractor.h
 *  MilkyPlayer
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
