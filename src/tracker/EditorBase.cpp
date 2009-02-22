/*
 *  tracker/EditorBase.cpp
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
 *  EditorBase.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 26.11.07.
 *
 */

#include "EditorBase.h"
#include "SimpleVector.h"

void EditorBase::notifyListener(EditorNotifications notification)
{
	for (pp_int32 i = 0; i < notificationListeners->size(); i++)
		notificationListeners->get(i)->editorNotification(this, notification);	
}

EditorBase::EditorBase() :
	lazyUpdateNotifications(false),
	module(NULL)
{
	notificationListeners = new PPSimpleVector<EditorNotificationListener>(16, false);
}

EditorBase::~EditorBase()
{
	notifyListener(EditorDestruction);
	delete notificationListeners;
}

void EditorBase::addNotificationListener(EditorNotificationListener* listener)
{	
	// remove it first, no duplicate event listener entries allowed
	removeNotificationListener(listener);
	notificationListeners->add(listener);
}

bool EditorBase::removeNotificationListener(EditorNotificationListener* listener)
{
	for (pp_int32 i = 0; i < notificationListeners->size(); i++)
	{
		// found? listeners can not be contained in the list multiple times
		// see addEditorNotificationListener()
		if (notificationListeners->get(i) == listener)
		{
			notificationListeners->remove(i);
			return true;
		}
	}
	
	return false;
}

void EditorBase::enterCriticalSection()
{
	notifyListener(NotificationPrepareCritical);
}

void EditorBase::leaveCriticalSection()
{
	notifyListener(NotificationUnprepareCritical);
}

