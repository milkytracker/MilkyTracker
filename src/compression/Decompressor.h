/*
 *  tracker/Decompressor.h
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
	enum Hints
	{
		HintAll,
		HintModules,
		HintInstruments,
		HintSamples,
		HintPatterns,
		HintTracks
	};
	
	DecompressorBase(const PPSystemString& fileName) :
		fileName(fileName)
	{
	}
	
	virtual ~DecompressorBase()
	{
	}
	
	virtual bool identify(XMFile& f) = 0;

	virtual bool identify();
	
	virtual bool doesServeHint(Hints hint) = 0;
	
	virtual const PPSimpleVector<Descriptor>& getDescriptors(Hints hint) const = 0;
	
	virtual bool decompress(const PPSystemString& outFileName, Hints hint) = 0;
	
	static void removeFile(const PPSystemString& fileName);
	
	virtual void setFilename(const PPSystemString& filename);

	virtual DecompressorBase* clone() = 0;
	
protected:
	PPSystemString fileName;

	mutable PPSimpleVector<Descriptor> descriptors;
};

/*****************************************************************************
 * Generic decompressor
 *****************************************************************************/
class Decompressor : public DecompressorBase
{
public:
	Decompressor(const PPSystemString& fileName);

	virtual bool identify(XMFile& f);
	
	virtual bool doesServeHint(Hints hint);
	
	virtual const PPSimpleVector<Descriptor>& getDescriptors(Hints hint) const;

	virtual bool decompress(const PPSystemString& outFileName, Hints hint);
	
	virtual DecompressorBase* clone();

	virtual void setFilename(const PPSystemString& filename);
	
private:
	void adjustFilenames(const PPSystemString& filename);	

	PPSimpleVector<DecompressorBase> decompressors;
	
public:
	static PPSimpleVector<DecompressorBase>& decompressorList();

	template<class type>
	struct RegisterDecompressor
	{
		RegisterDecompressor()
		{
			Decompressor::decompressorList().add(new type(""));
		}
	};
};

#endif

