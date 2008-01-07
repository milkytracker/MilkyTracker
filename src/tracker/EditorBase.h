/*
 *  tracker/EditorBase.h
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
 *  EditorBase.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 26.11.07.
 *
 */

#ifndef __EDITORBASE_H__
#define __EDITORBASE_H__

#include "BasicTypes.h"

class XModule;

template<class Type>
class PPSimpleVector;

class EditorBase
{
public:
	enum EditorNotifications
	{
		EditorDestruction,
	
		NotificationReload,

		NotificationChangesValidate,
		NotificationChanges,
		NotificationUpdateNoChanges,

		NotificationFeedUndoData,
		NotificationFetchUndoData,
		NotificationUndoRedo,

		NotificationPrepareLengthy,
		NotificationUnprepareLengthy,

		NotificationPrepareCritical,
		NotificationUnprepareCritical
	};

	class EditorNotificationListener
	{
	public:
		virtual void editorNotification(EditorBase* sender, EditorNotifications notification) = 0;
	};
	
	
private:
	PPSimpleVector<EditorNotificationListener>* notificationListeners;
	bool lazyUpdateNotifications;
	
protected:
	XModule* module;
	
	EditorBase();
	
	void attachModule(XModule* module) { this->module = module; }

	void notifyListener(EditorNotifications notification);

	void enterCriticalSection();

	void leaveCriticalSection();

public:
	virtual ~EditorBase();
	
	void addNotificationListener(EditorNotificationListener* listener);
	bool removeNotificationListener(EditorNotificationListener* listener);

	// query status
	void setLazyUpdateNotifications(bool lazyUpdateNotifications) { this->lazyUpdateNotifications = lazyUpdateNotifications; }
	bool getLazyUpdateNotifications() const { return lazyUpdateNotifications; }
	
	XModule* getModule() { return module; }	
};

#endif
