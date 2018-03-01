/*
 *  compression/DecompressorUMX.cpp
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
 *  DecompressorUMX.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 22.06.08.
 *
 */

#include "DecompressorUMX.h"
#include "XMFile.h"
#include "LittleEndian.h"

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
	
const PPSimpleVector<Descriptor>& DecompressorUMX::getDescriptors(Hints hint) const
{
	descriptors.clear();
	descriptors.add(new Descriptor("umx", "Unreal Data File")); 	
	return descriptors;
}	
	
#define TEST_SIZE 1500

#define MAGIC4(a,b,c,d) \
    (((pp_uint32)(a)<<24)|((pp_uint32)(b)<<16)|((pp_uint32)(c)<<8)|(d))

#define MAGIC_IMPM	MAGIC4('I','M','P','M')
#define MAGIC_SCRM	MAGIC4('S','C','R','M')
#define MAGIC_M_K_	MAGIC4('M','.','K','.')
	
bool DecompressorUMX::decompress(const PPSystemString& outFileName, Hints hint)
{
	// If client requests something else than a module we can't deal we that
	if (hint != HintAll &&
		hint != HintModules)
		return false;

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

DecompressorBase* DecompressorUMX::clone()
{
	return new DecompressorUMX(fileName);
}	

static Decompressor::RegisterDecompressor<DecompressorUMX> registerDecompressor;
