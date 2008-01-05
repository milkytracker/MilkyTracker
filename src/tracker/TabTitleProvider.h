/*
 *  TabTitleProvider.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 11.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __TABTITLEPROVIDER_H__
#define __TABTITLEPROVIDER_H__

#include "BasicTypes.h"

class TabTitleProvider
{
private:
	class ModuleEditor& moduleEditor;

public:
	TabTitleProvider(ModuleEditor& moduleEditor) :
		moduleEditor(moduleEditor)
	{
	}
	
	PPString getTabTitle();
};

#endif
