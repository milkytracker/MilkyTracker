/*
 *  tracker/DecompressorLHA.h
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
 *  DecompressorLHA.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 22.06.08.
 *
 */

#ifndef __DECOMPRESSOR_LHA_H__
#define __DECOMPRESSOR_LHA_H__

#include "Decompressor.h"

/*****************************************************************************
 * LHA decompressor
 *****************************************************************************/
class DecompressorLHA : public DecompressorBase
{
public:
	DecompressorLHA(const PPSystemString& filename);

	virtual bool identify(XMFile& f);
	
	virtual bool decompress(const PPSystemString& outFilename);
	
	virtual DecompressorBase* clone();
};

#endif

