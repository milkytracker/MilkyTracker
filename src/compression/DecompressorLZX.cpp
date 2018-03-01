/*
 *  compression/DecompressorLZX.cpp
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
 *  DecompressorLZX.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 17.04.09.
 *
 */


#include "DecompressorLZX.h"
#include "XMFile.h"
#include "XModule.h"
#include "unlzx.h"

struct ModuleIdentificator : public Unlzx::FileIdentificator 
{
	virtual bool identify(const PPSystemString& filename) const
	{
		XMFile file(filename);
		
		mp_ubyte buff[XModule::IdentificationBufferSize];
		memset(buff, 0, sizeof(buff));

		file.read(buff, 1, sizeof(buff));
		
		return XModule::identifyModule(buff) != NULL;
	}
};

DecompressorLZX::DecompressorLZX(const PPSystemString& filename) :
	DecompressorBase(filename)
{
}

bool DecompressorLZX::identify(XMFile& f)
{
	f.seek(0);	

	char buffer[3];

	f.read(buffer, 1, 3);
	
	return (buffer[0] == 76) && (buffer[1] == 90) && (buffer[2] == 88);
}	
	
const PPSimpleVector<Descriptor>& DecompressorLZX::getDescriptors(Hints hint) const
{
	descriptors.clear();
	descriptors.add(new Descriptor("lzx", "LZX Archive")); 	
	return descriptors;
}		
	
bool DecompressorLZX::decompress(const PPSystemString& outFilename, Hints hint)
{
	// If client requests something else than a module we can't deal we that
	if (hint != HintAll &&
		hint != HintModules)
		return false;

	ModuleIdentificator identificator;
	Unlzx unlzx(fileName, &identificator);
	
	return unlzx.extractFile(true, &outFilename);
}

DecompressorBase* DecompressorLZX::clone()
{
	return new DecompressorLZX(fileName);
}	

static Decompressor::RegisterDecompressor<DecompressorLZX> registerDecompressor;
