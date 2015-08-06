/*
 *  compression/DecompressorPP20.cpp
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
 *  DecompressorPP20.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 22.06.08.
 *
 */

#include "DecompressorPP20.h"
#include "PP20.h"
#include "XMFile.h"

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
	
const PPSimpleVector<Descriptor>& DecompressorPP20::getDescriptors(Hints hint) const
{
	descriptors.clear();
	descriptors.add(new Descriptor("pp", "Powerpacker Archive")); 	
	return descriptors;
}	
	
bool DecompressorPP20::decompress(const PPSystemString& outFileName, Hints hint)
{
	XMFile f(fileName);	
	unsigned int size = f.size();
	unsigned char* buffer = new unsigned char[size];
	
	f.read(buffer, 1, size);
	
	PP20 pp20;
	
	if (!pp20.isCompressed(buffer, size))
	{
		delete[] buffer;
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

DecompressorBase* DecompressorPP20::clone()
{
	return new DecompressorPP20(fileName);
}	

static Decompressor::RegisterDecompressor<DecompressorPP20> registerDecompressor;

