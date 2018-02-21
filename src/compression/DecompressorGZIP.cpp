/*
 *  compression/DecompressorGZIP.cpp
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
 *  DecompressorGZIP.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 22.06.08.
 *
 */

#include "DecompressorGZIP.h"
#include "XMFile.h"
#include "LittleEndian.h"
#include "zlib.h"

// -- GZIP --------------------------------------------------------------------
DecompressorGZIP::DecompressorGZIP(const PPSystemString& fileName) :
	DecompressorBase(fileName)
{
}

bool DecompressorGZIP::identify(XMFile& f)
{
	f.seek(0);
	mp_dword id = f.readDword();

	// GZIP ID
	return (id == 0x08088B1F);
}

const PPSimpleVector<Descriptor>& DecompressorGZIP::getDescriptors(Hints hint) const
{
	descriptors.clear();
	descriptors.add(new Descriptor("gz", "GZIP Archive"));
	return descriptors;
}

bool DecompressorGZIP::decompress(const PPSystemString& outFileName, Hints hint)
{
	gzFile gz_input_file = NULL;
	int len = 0;
	pp_uint8 *buf;

	if ((gz_input_file = gzopen (fileName.getStrBuffer(), "r")) == NULL)
		return false;

	if ((buf = new pp_uint8[0x10000]) == NULL)
		return false;

	XMFile fOut(outFileName, true);

	while (true)
	{
		len = gzread (gz_input_file, buf, 0x10000);

		if (len < 0)
		{
			delete[] buf;
			return false;
		}

		if (len == 0) break;

		fOut.write(buf, 1, len);
	}

	if (gzclose (gz_input_file) != Z_OK)
	{
		delete[] buf;
		return false;
	}

	delete[] buf;

	return true;
}

DecompressorBase* DecompressorGZIP::clone()
{
	return new DecompressorGZIP(fileName);
}

static Decompressor::RegisterDecompressor<DecompressorGZIP> registerDecompressor;
