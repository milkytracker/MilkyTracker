/*
 *  milkyplay/drivers/generic/rtaudio/asio/asiolist.h
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

#ifndef __asiolist__
#define __asiolist__

#define DRVERR			-5000
#define DRVERR_INVALID_PARAM		DRVERR-1
#define DRVERR_DEVICE_ALREADY_OPEN	DRVERR-2
#define DRVERR_DEVICE_NOT_FOUND		DRVERR-3

#define MAXPATHLEN			512
#define MAXDRVNAMELEN		128

struct asiodrvstruct
{
	int						drvID;
	CLSID					clsid;
	char					dllpath[MAXPATHLEN];
	char					drvname[MAXDRVNAMELEN];
	LPVOID					asiodrv;
	struct asiodrvstruct	*next;
};

typedef struct asiodrvstruct ASIODRVSTRUCT;
typedef ASIODRVSTRUCT	*LPASIODRVSTRUCT;

class AsioDriverList {
public:
	AsioDriverList();
	~AsioDriverList();
	
	LONG asioOpenDriver (int,VOID **);
	LONG asioCloseDriver (int);

	// nice to have
	LONG asioGetNumDev (VOID);
	LONG asioGetDriverName (int,char *,int);		
	LONG asioGetDriverPath (int,char *,int);
	LONG asioGetDriverCLSID (int,CLSID *);

	// or use directly access
	LPASIODRVSTRUCT	lpdrvlist;
	int				numdrv;
};

typedef class AsioDriverList *LPASIODRIVERLIST;

#endif
