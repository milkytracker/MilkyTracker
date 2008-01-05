/*
 *  EditorBase.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 26.11.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
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

