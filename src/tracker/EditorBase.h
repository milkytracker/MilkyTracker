/*
 *  EditorBase.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 26.11.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
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
