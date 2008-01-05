/////////////////////////////////////////////////////////////////
//
//	Contextmenu control class
//
/////////////////////////////////////////////////////////////////
#ifndef CONTEXTMENU__H
#define CONTEXTMENU__H

#include "BasicTypes.h"
#include "Control.h"
#include "Menu.h"

// Forwards
class PPGraphicsAbstract;
class PPFont;
class PPContextMenu;

class PPContextMenu : public PPControl
{
private:
	const PPColor* color;
	const PPColor* selectionColor;
	bool autoAddHide;
	bool notifyParentOnHide;

	PPFont* font;
	
	PPMenu* menu;

	pp_int32 menuSelection;
	bool hasDragged;
	bool pressInvoke;
	bool hadCursor;

	void showSubMenu(PPContextMenu* subMenu);

public:
	PPContextMenu(pp_int32 id, 
				PPScreen* parentScreen, 
				EventListenerInterface* eventListener, 
				PPPoint location, 
				const PPColor& selColor, 
				bool doAutoAddHide = false,
				PPFont* font = NULL);

	~PPContextMenu();

	void setColor(const PPColor& color) { this->color = &color; }

	void setFont(PPFont* font) 
	{ 
		this->font = font; 
		menu->setFont(font);
		PPRect r = menu->getBoundingRect();		
		setSize(PPSize(r.width(), r.height()));
	}
	PPFont* getFont() { return font; }

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 callEventListener(PPEvent* event);

	virtual bool gainsFocus() { return false; }

	virtual bool isActive() { return true; }
	
	virtual void setSize(PPSize size);
	virtual void setLocation(PPPoint location);
	
	// menu controlling
	void addEntry(const PPString& s, pp_int32 theId, PPContextMenu* contextMenu = NULL);

	bool setState(pp_int32 theId, pp_uint32 newState) { return menu->setState(theId, newState); }
	
	bool hitMenu(const PPPoint& p);

	void processMenuHit(const PPPoint& p);
	
	void setSubMenu(bool bSubMenu) { menu->setSubMenu(bSubMenu); }
	bool isSubMenu() { return menu->isSubMenu(); }

	void setParentMenu(PPContextMenu* parent) { menu->setParentMenu(parent); }
	PPContextMenu* getParentMenu() { return menu->getParentMenu(); }
	
	void setNotifyParentOnHide(bool notifyParentOnHide) { this->notifyParentOnHide = notifyParentOnHide; }
	bool getNotifyParentOnHide() const { return notifyParentOnHide; }
};

#endif
