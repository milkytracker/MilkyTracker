/////////////////////////////////////////////////////////////////
//
//	Static text control class
//
/////////////////////////////////////////////////////////////////
#ifndef STATICTEXT__H
#define STATICTEXT__H

#include "BasicTypes.h"
#include "Control.h"

// Forwards
class PPGraphicsAbstract;
class PPFont;
class PPButton;

class PPStaticText : public PPControl
{
private:
	const PPColor* color;

	bool drawShadow;
	bool underlined;
	bool autoShrink;

	PPColor shadowColor;

	PPString text;

	PPFont* font;
	
	PPSize extent;

public:
	PPStaticText(pp_int32 id, 
			   PPScreen* parentScreen, 
			   EventListenerInterface* eventListener, 
			   PPPoint location, 
			   const PPString& text, 
			   bool drawShadow = false,
			   bool drawUnderlined = false,
			   bool autoShrink = false);
			   
	~PPStaticText();

	void setColor(const PPColor& color) { this->color = &color; }

	void setText(const PPString& text);
	void setText(const char* text);

	PPString& getText() { return text; }

	void setFont(PPFont* font) { this->font = font; calcExtent(); }

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 callEventListener(PPEvent* event);

	virtual bool gainsFocus() { return false; }

	virtual bool isActive() { return true; }
	
	void setUnderlined(bool b) { underlined = b; }

	void setValue(pp_int32 value, bool hex, pp_uint32 numDigits = 0, bool negative = false);

	void setIntValue(pp_int32 value, pp_uint32 numDecDigits = 0, bool negative = false);

	void setHexValue(pp_int32 value, pp_uint32 numHexDigits = 0);

	void setExtent(PPSize extent) { this->extent = extent; }

private:
	void calcExtent();
};

#endif
