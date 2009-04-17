/*
 *  compression/DecompressorQT.h
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
 *  DecompressorQT.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 22.06.08.
 *
 */

#ifndef __DECOMPRESSOR_QT_H__
#define __DECOMPRESSOR_QT_H__

#include "Decompressor.h"

/*****************************************************************************
 * QT decompressor
 * This class uses QTKit to extract the audio from quicktime media content
 * to an uncompressed 16 bit AIFF file which MilkyTracker can read directly.
 * Note: QTKit is only available on OS X 10.4 and later.
 *****************************************************************************/
class DecompressorQT : public DecompressorBase
{
public:
	DecompressorQT(const PPSystemString& filename);

	virtual bool identify(XMFile& f);
	
	// this type of archive can only contain samples
	virtual bool doesServeHint(Hints hint) { return (hint == HintAll || hint == HintSamples); }

	virtual const PPSimpleVector<Descriptor>& getDescriptors(Hints hint) const;

	virtual bool decompress(const PPSystemString& outFilename, Hints hint);
	
	virtual DecompressorBase* clone();
};

#endif

