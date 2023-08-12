/*
 *  tracker/DialogSliders.h
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
 *  DialogSliders.h
 *  MilkyTracker
 *
 *  Created by coderofsalvation/Leon van Kammen 
 *
 */

#ifndef __DIALOGSLIDERS_H__
#define __DIALOGSLIDERS_H__

#include "DialogBase.h"
#include "Slider.h"
#include "SampleEditor.h"

#define MAX_SLIDERS 20
#define TEXTVALUES_OFFSET MAX_SLIDERS
#define TEXT_OFFSET 2*TEXTVALUES_OFFSET

class DialogSliders : public PPDialogBase
{
public:

private:
	pp_uint32 numSliders;
	PPScreen* screen;
	DialogResponder *responder;
	pp_int32 id;
	SampleEditor *sampleEditor;
  SampleEditor *sampleEditor_;
  void (SampleEditor::*func)(const FilterParameters*);
	class PPListBox* listBoxes[MAX_SLIDERS];
  
	bool needUpdate;
	bool preview;

	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);	
	
	void resetSliders();
	void update();

public:
  DialogSliders(PPScreen *parentScreen, DialogResponder *toolHandlerResponder, pp_int32 id, const PPString& title, pp_int32 sliders, SampleEditor *sampleEditor, void (SampleEditor::*fn)(const FilterParameters*) );


	void setSlider(pp_uint32 index, float param);
	float getSlider(pp_uint32 index) const;
  pp_int32 getNumSliders(){ return this->numSliders; }

	void initSlider(int i, float min, float max, float value, PPString caption);

	void setSampleEditor(SampleEditor *s){ this->sampleEditor = s; }
	SampleEditor * getSampleEditor(){ return this->sampleEditor; }

};

#endif
