/*
 *  tracker/Decompressor.cpp
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
 *  Decompressor.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 19.10.07.
 *
 */

#include "Decompressor.h"
#include "PP20.h"
#include "ZipExtractor.h"
#include "XMFile.h"
#include "LittleEndian.h"

void DecompressorBase::removeFile(const PPSystemString& fileName)
{
	XMFile::remove(fileName);
}

bool DecompressorBase::identify()
{
	XMFile f(fileName);
	return identify(f);
}

// -- PP20 -------------------------------------------------------------------
DecompressorPP20::DecompressorPP20(const PPSystemString& fileName) :
	DecompressorBase(fileName)
{
}

bool DecompressorPP20::identify(XMFile& f)
{
	f.seek(0);
	mp_dword id = f.readDword();
	
	// PP20
	return (id == 0x30325050);
}
	
bool DecompressorPP20::decompress(const PPSystemString& outFileName)
{
	XMFile f(fileName);	
	unsigned int size = f.size();
	unsigned char* buffer = new unsigned char[size];
	
	f.read(buffer, 1, size);
	
	PP20 pp20;
	
	if (!pp20.isCompressed(buffer, size))
	{
		delete buffer;
		return false;
	}
	
	XMFile fOut(outFileName, true);

	pp_uint8* outBuffer = NULL;
	 
	unsigned resultSize = pp20.decompress(buffer, size, &outBuffer);

	// delete this, it was allocated
	delete[] buffer;

	// if resultSize is 0 there is nothing more to deallocate
	if (resultSize == 0)
		return false;

	fOut.write(outBuffer, 1, resultSize);

	delete[] outBuffer;

	return true;
}

// -- UMX --------------------------------------------------------------------
DecompressorUMX::DecompressorUMX(const PPSystemString& fileName) :
	DecompressorBase(fileName)
{
}

bool DecompressorUMX::identify(XMFile& f)
{
	f.seek(0);
	mp_dword id = f.readDword();
	
	// UMX ID
	return (id == 0x9E2A83C1);
}
	
	
#define TEST_SIZE 1500

#define MAGIC4(a,b,c,d) \
    (((pp_uint32)(a)<<24)|((pp_uint32)(b)<<16)|((pp_uint32)(c)<<8)|(d))

#define MAGIC_IMPM	MAGIC4('I','M','P','M')
#define MAGIC_SCRM	MAGIC4('S','C','R','M')
#define MAGIC_M_K_	MAGIC4('M','.','K','.')
	
bool DecompressorUMX::decompress(const PPSystemString& outFileName)
{
	XMFile f(fileName);	

	int i;
	pp_uint8 *buf, *b;
	int len, offset = -1;

	if ((b = buf = new pp_uint8[0x10000]) == NULL)
		return false;

	f.read(buf, 1, TEST_SIZE);
	for (i = 0; i < TEST_SIZE; i++, b++) {
		pp_uint32 id;

		id = BigEndian::GET_DWORD(b);

		if (!memcmp(b, "Extended Module:", 16)) {
			offset = i;
			break;
		}
		if (id == MAGIC_IMPM) { 
			offset = i;
			break;
		}
		if (i > 44 && id == MAGIC_SCRM) { 
			offset = i - 44;
			break;
		}
		if (i > 1080 && id == MAGIC_M_K_) { 
			offset = i - 1080;
			break;
		}
	}
	
	if (offset < 0) {
		delete[] buf;
		return false;
	}

	f.seek(offset);
	
	XMFile fOut(outFileName, true);

	do {
		len = f.read(buf, 1, 0x10000);
		fOut.write(buf, 1, len);
	} while (len == 0x10000);

	delete[] buf;

	return true;
}

#ifndef __EXCLUDE_ZIPPEDMODULES__
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
	
bool DecompressorZIP::decompress(const PPSystemString& outFilename)
{
	ZipExtractor extractor(fileName);
	
	pp_int32 error = 0;
	bool res = extractor.parseZip(error, true, &outFilename);
	return (res && error == 0);
}
#endif

Decompressor::Decompressor(const PPSystemString& fileName) :
	DecompressorBase(fileName)
{
	decompressorList.add(new DecompressorPP20(fileName));
	decompressorList.add(new DecompressorUMX(fileName));
#ifndef __EXCLUDE_ZIPPEDMODULES__
	decompressorList.add(new DecompressorZIP(fileName));		
#endif
}
	
bool Decompressor::identify(XMFile& f)
{
	for (pp_int32 i = 0; i < decompressorList.size(); i++)
	{
		if (decompressorList.get(i)->identify(f))
		{
			return true;
		}
	}
	
	return false;
}	
	
bool Decompressor::decompress(const PPSystemString& outFileName)
{
	bool result = false;
	for (pp_int32 i = 0; i < decompressorList.size(); i++)
	{
		if (decompressorList.get(i)->identify())
		{
			result = decompressorList.get(i)->decompress(outFileName);
			break;
		}
	}
	
	if (!result)
		removeFile(outFileName);
	
	return result;
}
