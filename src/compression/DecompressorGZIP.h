/*
 *  compression/DecompressorGZIP.h
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
 *  DecompressorGZIP.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 22.06.08.
 *
 */

#ifndef __DECOMPRESSOR_GZIP_H__
#define __DECOMPRESSOR_GZIP_H__

#include "Decompressor.h"

/*****************************************************************************
 * gzip decompression
 *****************************************************************************/
class DecompressorGZIP : public DecompressorBase
{
public:
	DecompressorGZIP(const PPSystemString& fileName);

	virtual bool identify(XMFile& f);
	
	// this type of archive can contain any file type
	virtual bool doesServeHint(Hints hint) { return true; }
	
	virtual const PPSimpleVector<Descriptor>& getDescriptors(Hints hint) const;
	
	virtual bool decompress(const PPSystemString& outFileName, Hints hint);
	
	virtual DecompressorBase* clone();
};


#endif

