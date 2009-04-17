/*
 *  compression/DecompressorLHA.cpp
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
 *  DecompressorLHA.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 22.06.08.
 *
 */

#include "DecompressorLHA.h"
#include "XMFile.h"
#include "XModule.h"
#include "unlha32.h"

// -- LHA --------------------------------------------------------------------
class XMFileStreamer : public CLhaArchive::StreamerBase
{
private:
	XMFile& f;

	pp_uint32 currentPos;

protected:
	virtual pp_uint8 get(pp_uint32 pos)
	{
		pp_uint8 result;
		if (currentPos != pos)
		{
			f.seek(pos);
			result = f.readByte();
			currentPos = f.pos();
		}
		else
		{
			result = f.readByte();
			currentPos = f.pos();
		}
		
		return result;
	}

public:		
	XMFileStreamer(XMFile& f) :
		f(f)
	{
		currentPos = f.pos();
	}

	virtual void read(void* buffer, pp_uint32 from, pp_uint32 len)
	{
		if (currentPos != from)
		{
			f.seek(from);
			f.read(buffer, 1, len);
			currentPos = f.pos();
		}
		else
		{
			f.read(buffer, 1, len);
			currentPos = f.pos();
		}
	}
};

struct ModuleIdentifyNotifier : public CLhaArchive::IDNotifier 
{
	virtual bool identify(void* buffer, pp_uint32 len) const
	{
		mp_ubyte buff[XModule::IdentificationBufferSize];
		memset(buff, 0, sizeof(buff));
		memcpy(buff, buffer, len > sizeof(buff) ? sizeof(buff) : len);
		
		return XModule::identifyModule(buff) != NULL;
	}
};

DecompressorLHA::DecompressorLHA(const PPSystemString& filename) :
	DecompressorBase(filename)
{
}

bool DecompressorLHA::identify(XMFile& f)
{
	f.seek(0);	

	XMFileStreamer streamer(f);

	CLhaArchive archive(streamer, f.size(), NULL);

	bool res = archive.IsArchive() != 0;

	return res;
}	
	
const PPSimpleVector<Descriptor>& DecompressorLHA::getDescriptors(Hints hint) const
{
	descriptors.clear();
	descriptors.add(new Descriptor("lha", "LHA Archive")); 	
	return descriptors;
}		
	
bool DecompressorLHA::decompress(const PPSystemString& outFilename, Hints hint)
{
	XMFile f(fileName);
	
	if (!f.isOpen())
		return false;

	XMFileStreamer streamer(f);

	ModuleIdentifyNotifier idnotifier;

	CLhaArchive archive(streamer, f.size(), &idnotifier);

	if (!archive.IsArchive())
		return false;

	pp_uint32 result = archive.ExtractFile();
	
	if (result && archive.GetOutputFile())
	{
		const char* id = XModule::identifyModule(archive.GetOutputFile());
		if (id)
		{
			XMFile outFile(outFilename, true);
			if (!outFile.isOpenForWriting())
				return false;
				
			outFile.write(archive.GetOutputFile(), 1, archive.GetOutputFileLength());						
			return true;
		}
	}
	
	return false;
}

DecompressorBase* DecompressorLHA::clone()
{
	return new DecompressorLHA(fileName);
}	

static Decompressor::RegisterDecompressor<DecompressorLHA> registerDecompressor;
