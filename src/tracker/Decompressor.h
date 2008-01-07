/*
 *  tracker/Decompressor.h
 *
 *  Copyright 2008 Peter Barth
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
 *  Decompressor.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 19.10.07.
 *
 */

#ifndef __DECOMPRESSOR_H__
#define __DECOMPRESSOR_H__

#include "BasicTypes.h"
#include "SimpleVector.h"

class XMFile;

class DecompressorBase
{
public:
	DecompressorBase(const PPSystemString& fileName) :
		fileName(fileName)
	{
	}
	
	virtual ~DecompressorBase()
	{
	}
	
	virtual bool identify(XMFile& f) = 0;

	virtual bool identify();
	
	virtual bool decompress(const PPSystemString& outFileName) = 0;
	
	static void removeFile(const PPSystemString& fileName);
	
	void setFilename(const PPSystemString& filename) { this->fileName = fileName; }
	
protected:
	PPSystemString fileName;
	
};

/*****************************************************************************
 * PowerPacker decompressor
 *****************************************************************************/
class DecompressorPP20 : public DecompressorBase
{
public:
	DecompressorPP20(const PPSystemString& fileName);

	virtual bool identify(XMFile& f);
	
	virtual bool decompress(const PPSystemString& outFileName);
};

/*****************************************************************************
 * Unreal Music (UMX) "decompressor" 
 *****************************************************************************/
class DecompressorUMX : public DecompressorBase
{
public:
	DecompressorUMX(const PPSystemString& fileName);

	virtual bool identify(XMFile& f);
	
	virtual bool decompress(const PPSystemString& outFileName);
};

#ifndef __EXCLUDE_ZIPPEDMODULES__
/*****************************************************************************
 * ZIP decompressor
 *****************************************************************************/
class DecompressorZIP : public DecompressorBase
{
public:
	DecompressorZIP(const PPSystemString& filename);

	virtual bool identify(XMFile& f);
	
	virtual bool decompress(const PPSystemString& outFilename);
};
#endif

/*****************************************************************************
 * Generic decompressor
 *****************************************************************************/
class Decompressor : public DecompressorBase
{
public:
	Decompressor(const PPSystemString& fileName);

	virtual bool identify(XMFile& f);
	
	virtual bool decompress(const PPSystemString& outFileName);

private:
	PPSimpleVector<DecompressorBase> decompressorList;
};

#endif

