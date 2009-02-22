/*
 *  ppui/osinterface/PPOpenPanel.h
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

#ifndef __PPOPENPANEL_H__
#define __PPOPENPANEL_H__

#include "PPModalDialog.h"
#include "SimpleVector.h"

class PPOpenPanel : public PPModalDialog
{
protected:
	PPSimpleVector<Descriptor> items;

	PPSystemString fileName;

	char* caption;

public:
	PPOpenPanel(PPScreen* screen, const char* caption);
	virtual ~PPOpenPanel();
	
	// must contain pairs of extensions / description
	// terminated by TWO NULL pointers
	virtual void addExtensions(const char* const extensions[])
	{
		for (pp_uint32 i = 0; extensions[i] != NULL; i+=2)
			addExtension(extensions[i], extensions[i+1]);
	}

	virtual void addExtension(const PPString& ext, const PPString& desc);

	virtual const PPSystemString& getFileName() { return fileName; }

	virtual ReturnCodes runModal();

};

#endif
