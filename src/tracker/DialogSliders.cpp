/*
 *  tracker/DialogSliders.cpp
 *
 *  Copyright 2022 coderofsalvation/Leon van Kammen 
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
 *  DialogSliders.cpp
 *  MilkyTracker
 *
 *  Created by Leon van Kammen 
 *
 */

#include "DialogSliders.h"
#include "Screen.h"
#include "StaticText.h"
#include "MessageBoxContainer.h"
#include "ScrollBar.h"
#include "Slider.h"
#include "Seperator.h"
#include "PPSystem.h"
#include "FilterParameters.h"
#include "SampleEditor.h"
#include "ListBox.h"

DialogSliders::DialogSliders(PPScreen *parentScreen, DialogResponder *toolHandlerResponder, pp_int32 id, const PPString& title, pp_int32 sliders, SampleEditor *sampleEditor, void (SampleEditor::*fn)(const FilterParameters*) ) : sampleEditor_(sampleEditor), func(fn)
{
	needUpdate    = false;
	preview       = false;
  numSliders    = sliders; 
  responder     = toolHandlerResponder;
  screen        = parentScreen;
  this->sampleEditor = sampleEditor;
  this->id      = id;
	float dheight   = (sliders+5) * (SCROLLBUTTONSIZE+6);
	initDialog(screen, responder, id, title.getStrBuffer(), screen->getWidth() > 320 ? 400 : 330, dheight, 26, "Ok", "Cancel");
}

void DialogSliders::initSlider(int i, float min, float max, float value, PPString caption)
{
	pp_int32 x      = getMessageBoxContainer()->getLocation().x;
	pp_int32 y      = getMessageBoxContainer()->getLocation().y;
	pp_int32 width  = getMessageBoxContainer()->getSize().width;
	pp_int32 height = getMessageBoxContainer()->getSize().height;
	pp_uint32 borderSpace = 12;
	pp_uint32 scalaSpace = 16*7+8;
	pp_int32 y2 = ((SCROLLBUTTONSIZE+6) * i+1 ) +  getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y + 20;
	pp_int32 x2 = x + borderSpace;
	pp_int32 size = width-192;
	
	// create slider
  PPSlider* slider = new PPSlider(MESSAGEBOX_CONTROL_USER1+i, screen, this, PPPoint(x2+scalaSpace, y2), size, true, false);
  slider->setBarSize(8192);
  slider->setMinValue((int)min);
  slider->setMaxValue((int)max);
  slider->setCurrentValue(value);
  getMessageBoxContainer()->addControl(slider);
  if( screen->getWidth() < 320 ) caption = caption.subString(0,10);
  PPFont* font = PPFont::getFont(PPFont::FONT_SYSTEM);
  PPStaticText* staticText = new PPStaticText(MESSAGEBOX_CONTROL_USER1+TEXT_OFFSET+i, screen, this, PPPoint(x2+(SCROLLBUTTONSIZE/2), y2), caption.getStrBuffer(), true);
  staticText->setFont(font);
  getMessageBoxContainer()->addControl(staticText);
  // value
  char v[255];
  sprintf(v,"%i",(int)value);
  //staticText = new PPStaticText(MESSAGEBOX_CONTROL_USER1+TEXTVALUES_OFFSET+i, screen, this, PPPoint(x+width-(4*12), y2), v, true);
  //staticText->setFont(font);
  //getMessageBoxContainer()->addControl(staticText);
	listBoxes[i] = new PPListBox(MESSAGEBOX_CONTROL_USER1+TEXTVALUES_OFFSET+i, screen, this, PPPoint(x+width-(5*12)+1, y2), PPSize(10*4,12), true, true, false);
	listBoxes[i]->showSelection(false);
	listBoxes[i]->setBorderColor(messageBoxContainerGeneric->getColor());
	listBoxes[i]->setMaxEditSize(4);
  listBoxes[i]->addItem( PPString(v) );
  listBoxes[i]->commitChanges();
	getMessageBoxContainer()->addControl(listBoxes[i]);	
}

void DialogSliders::update()
{
	parentScreen->paintControl(messageBoxContainerGeneric);
	needUpdate = false;
}

pp_int32 DialogSliders::handleEvent(PPObject* sender, PPEvent* event)
{
	char v[255];
	pp_uint32 id = reinterpret_cast<PPControl*>(sender)->getID();
	if( id >= MESSAGEBOX_CONTROL_USER1 && id <= MESSAGEBOX_CONTROL_USER1+numSliders ){
		pp_uint32 slider = id-MESSAGEBOX_CONTROL_USER1;
		float val = getSlider( slider );
		sprintf(v,"%i",(int)val);
    listBoxes[slider]->updateItem( 0, PPString(v) );
    listBoxes[slider]->commitChanges();
    update();
		needUpdate = true;
	}

	if( event->getID() == eCommand ){
		if( preview && sampleEditor != NULL ) sampleEditor->undo();
	}
	if( event->getID() == eLMouseUp && needUpdate ){
		if( sampleEditor != NULL ){
			FilterParameters par(numSliders);
      pp_int32 i;
      for( i = 0; i < numSliders; i++ ){
			  par.setParameter(i, FilterParameters::Parameter( getSlider(i) ) );
      }
			if( preview ) sampleEditor->undo();
      (sampleEditor_->*func)(&par);
			preview = true;
		}
		update();
	}
	return PPDialogBase::handleEvent(sender, event);
}

void DialogSliders::setSlider(pp_uint32 index, float param)
{
	if (index >= numSliders)
		return;
	PPSlider* slider = static_cast<PPSlider*>(getMessageBoxContainer()->getControlByID(MESSAGEBOX_CONTROL_USER1+index));
	pp_int32 value = (pp_int32)param;
	slider->setCurrentValue(value);
}

float DialogSliders::getSlider(pp_uint32 index) const
{
	if (index >= numSliders)
		return 0.0f;
	PPSlider* slider = static_cast<PPSlider*>(getMessageBoxContainer()->getControlByID(MESSAGEBOX_CONTROL_USER1+index));
	float v = slider->getCurrentValue();
	return v;
}
