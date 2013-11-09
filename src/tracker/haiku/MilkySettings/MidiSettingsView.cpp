/*
 *  Copyright 2012 Julian Harnath
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

#include "MidiSettingsView.h"

#include "SettingsMessages.h"

#include <Autolock.h>
#include <Box.h>
#include <CheckBox.h>
#include <List.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Messenger.h>
#include <MidiProducer.h>
#include <MidiRoster.h>
#include <PopUpMenu.h>
#include <Slider.h>

#include <stdio.h>


struct MidiProducer {
	BMenuItem* menuItem;
	int32 id;
};

enum {
	kStandardVelocityAmplify = 100
};


MidiSettingsView::MidiSettingsView(BRect frame)
	:
	BView(frame, "MIDI input", B_FOLLOW_NONE, B_WILL_DRAW),
	fMidiProducers(new BList()),
	fFirstAttach(true),
	fSelectedMidiProducer(NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BRect rect = frame;
	rect.bottom = rect.top + 20;

	const char* midiInputMenuLabel = "MIDI input:";
	fMidiInputMenu = new BPopUpMenu("");
	fMidiInputMenuField = new BMenuField(rect, "MidiInputMenuField",
		midiInputMenuLabel, fMidiInputMenu);
	fMidiInputMenuField->SetDivider(StringWidth(midiInputMenuLabel) + 10);
	AddChild(fMidiInputMenuField);

	const char* recordVelocityLabel = "Record velocity";
	fRecordVelocityCheckBox = new BCheckBox(
		BRect(0, 0, StringWidth(recordVelocityLabel) + 30, 20),
		"RecordVelocity", recordVelocityLabel,
		new BMessage(kMsg_MidiRecordVelocityToggled));
	fRecordVelocityCheckBox->SetValue(0);

	rect.right = frame.right - 10;
	rect.OffsetBy(0, 40);
	rect.bottom = rect.top + 100;
	BBox* velocityBox = new BBox(rect, "VelocityBox");
	velocityBox->SetLabel(fRecordVelocityCheckBox);

	rect = velocityBox->Bounds();
	rect.OffsetBy(0, 15);
	rect.InsetBy(10, 10);
	fVelocityAmplificationSlider = new BSlider(rect,
		"VelocityAmplification", "Velocity response",
		new BMessage(kMsg_MidiVelocityAmplificationChanged), -50, 50);
	fVelocityAmplificationSlider->SetLimitLabels("Soft", "Hard");
	fVelocityAmplificationSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fVelocityAmplificationSlider->SetHashMarkCount(11);
	fVelocityAmplificationSlider->SetValue(100);
	rgb_color fillColor = {100, 100, 255};
	rgb_color barColor  = {255, 100, 100};
	fVelocityAmplificationSlider->UseFillColor(true, &fillColor);
	fVelocityAmplificationSlider->SetBarColor(barColor);
	velocityBox->AddChild(fVelocityAmplificationSlider);
	AddChild(velocityBox);

	fVelocityAmplificationSlider->SetEnabled(false);
	fRecordVelocityCheckBox->SetEnabled(false);

	MidiProducer* producer = new MidiProducer;
	producer->id = -1;

	producer->menuItem = new BMenuItem("None",
		new BMessage(kMsg_MidiInputSelected));
	fMidiInputMenu->AddItem(producer->menuItem);

	fMidiProducers->AddItem(producer);
	fSelectedMidiProducer = producer;

	int32 id = 0;
	BMidiProducer* midiProducer = BMidiRoster::NextProducer(&id);
	while (midiProducer != NULL) {
		MidiProducer* producer = new MidiProducer;
		producer->id = midiProducer->ID();
		producer->menuItem = new BMenuItem(midiProducer->Name(),
			new BMessage(kMsg_MidiInputSelected));
		fMidiInputMenu->AddItem(producer->menuItem);
		fMidiProducers->AddItem(producer);

		midiProducer->Release();
		midiProducer = BMidiRoster::NextProducer(&id);
	}
}


MidiSettingsView::~MidiSettingsView()
{
	BMidiRoster::StopWatching();

	MidiProducer* producer = NULL;
	for (int32 index = 0 ; ; index++) {
		producer = (MidiProducer*)fMidiProducers->ItemAt(index);
		if (producer == NULL)
			break;
		delete producer;
	}
	delete fMidiProducers;
}


void
MidiSettingsView::AttachedToWindow()
{
	// This hook is called every time the user switches to the MIDI tab,
	// but we want to do this init work only once
	if (fFirstAttach) {
		BAutolock _(fMidiProducersLock);

		MidiProducer* producer = NULL;
		for (int32 index = 0 ; ; index++) {
			producer = (MidiProducer*)fMidiProducers->ItemAt(index);
			if (producer == NULL)
				break;
			producer->menuItem->SetTarget(this);
		}

		fRecordVelocityCheckBox->SetTarget(this);
		fVelocityAmplificationSlider->SetTarget(this);

		// Not a bug, BMidiRoster creates a copy of the messenger
		BMessenger messenger(this);
		BMidiRoster::StartWatching(&messenger);

		fFirstAttach = false;
	}

	fSelectedMidiProducer->menuItem->SetMarked(true);
}


void
MidiSettingsView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_MIDI_EVENT:
			_HandleMidiEvent(message);
			break;

		case kMsg_MidiInputSelected: {
			BMenuItem* markedItem = fMidiInputMenu->FindMarked();
			fSelectedMidiProducer = NULL;

			BAutolock _(fMidiProducersLock);
			MidiProducer* producer = _FindMidiProducer(markedItem);
			fSelectedMidiProducer = producer;

			if (fSelectedMidiProducer == NULL)
				debugger("Menu item selected, but could not be found in list");

			_SetVelocityBoxEnabled(fSelectedMidiProducer->id != -1);

			message->AddInt32("id", fSelectedMidiProducer->id);
			be_app->PostMessage(message);
		}	break;

		case kMsg_MidiRecordVelocityToggled: {
			bool on = fRecordVelocityCheckBox->Value() > 0 ? true : false;

			fVelocityAmplificationSlider->SetEnabled(on);

			message->AddBool("on", on);
			be_app->PostMessage(message);
		}	break;

		case  kMsg_MidiVelocityAmplificationChanged:
			message->AddInt32("value",
				kStandardVelocityAmplify -
				fVelocityAmplificationSlider->Value());
			be_app->PostMessage(message);
			break;

		default:
			BView::MessageReceived(message);
			break;
	}
}


void
MidiSettingsView::AddSettings(BMessage* settings)
{
	BMenuItem* markedItem = fMidiInputMenu->FindMarked();

	if (fSelectedMidiProducer->id != -1)
		settings->AddString("midi_input",
			fSelectedMidiProducer->menuItem->Label());

	settings->AddBool("record_velocity", fRecordVelocityCheckBox->Value() > 0);
	settings->AddInt16("velocity_amplify",
		kStandardVelocityAmplify - fVelocityAmplificationSlider->Value());
}


void
MidiSettingsView::SetSettings(BMessage* settings)
{
	status_t status;

	bool recordVelocity;
	status = settings->FindBool("record_velocity", &recordVelocity);
	if (status == B_OK)
		fRecordVelocityCheckBox->SetValue(recordVelocity ? 1 : 0);

	int16 velocityAmplify;
	status = settings->FindInt16("velocity_amplify", &velocityAmplify);
	if (status == B_OK) {
		fVelocityAmplificationSlider->SetValue(
			kStandardVelocityAmplify - velocityAmplify);
	}

	BString midiInputName;
	status = settings->FindString("midi_input", &midiInputName);
	if (status == B_OK) {
		BAutolock _(fMidiProducersLock);
		MidiProducer* producer = _FindMidiProducer(midiInputName);
		if (producer != NULL && producer->id != -1) {
			fSelectedMidiProducer = producer;
			producer->menuItem->SetMarked(true);
			_SetVelocityBoxEnabled(true);
		}
	}
}


void
MidiSettingsView::_HandleMidiEvent(BMessage* message)
{
	int32 op = message->FindInt32("be:op");

	if (   op != B_MIDI_REGISTERED && op != B_MIDI_UNREGISTERED
		&& op != B_MIDI_CHANGED_NAME)
	    return;

	// We only care about MIDI producers
	BString type;
	message->FindString("be:type", &type);
	if (type != "producer")
		return;

	int32 id = message->FindInt32("be:id");

	BAutolock _(fMidiProducersLock);
	MidiProducer* producer = _FindMidiProducer(id);

	// For register and name change, find the BMidiProducer
	BMidiProducer* midiProducer = NULL;
	if (op == B_MIDI_REGISTERED || op == B_MIDI_CHANGED_NAME) {
		midiProducer = BMidiRoster::FindProducer(id);
		if (midiProducer == NULL)
			return;
	}

	switch (op) {
		case B_MIDI_REGISTERED:
			if (producer != NULL) {
				midiProducer->Release();
				return;	// producer already in lsit
			}

			producer = new MidiProducer;
			producer->id = id;
			producer->menuItem = new BMenuItem(midiProducer->Name(),
				new BMessage(kMsg_MidiInputSelected));
			producer->menuItem->SetTarget(this);

			fMidiProducers->AddItem(producer);
			fMidiInputMenu->AddItem(producer->menuItem);

			midiProducer->Release();
			break;

		case B_MIDI_UNREGISTERED:
			if (producer == fSelectedMidiProducer) {
				MidiProducer* producerNone =
					(MidiProducer*)fMidiProducers->FirstItem();
				producerNone->menuItem->SetMarked(true);
				fSelectedMidiProducer = producerNone;
			}

			fMidiInputMenu->RemoveItem(producer->menuItem);
			delete producer->menuItem;
			fMidiProducers->RemoveItem(producer);
			delete producer;
			break;

		case B_MIDI_CHANGED_NAME:
			producer->menuItem->SetLabel(midiProducer->Name());

			midiProducer->Release();
			break;
	}
}


void
MidiSettingsView::_SetVelocityBoxEnabled(bool enable)
{
	fRecordVelocityCheckBox->SetEnabled(enable);
	if (enable)
		fVelocityAmplificationSlider->SetEnabled(
			fRecordVelocityCheckBox->Value() > 0 ? true : false);
	else
		fVelocityAmplificationSlider->SetEnabled(false);
}


MidiProducer*
MidiSettingsView::_FindMidiProducer(int32 id)
{
	MidiProducer* producer = NULL;

	for (int32 index = 0 ; ; index++) {
		producer = (MidiProducer*)fMidiProducers->ItemAt(index);
		if (producer == NULL)
			break;
		if (producer->id == id)
			break;
	}

	return producer;
}


MidiProducer*
MidiSettingsView::_FindMidiProducer(BMenuItem* menuItem)
{
	MidiProducer* producer = NULL;

	for (int32 index = 0 ; ; index++) {
		producer = (MidiProducer*)fMidiProducers->ItemAt(index);
		if (producer == NULL)
			break;
		if (producer->menuItem == menuItem)
			break;
	}

	return producer;
}


MidiProducer*
MidiSettingsView::_FindMidiProducer(BString name)
{
	MidiProducer* producer = NULL;

	for (int32 index = 0 ; ; index++) {
		producer = (MidiProducer*)fMidiProducers->ItemAt(index);
		if (producer == NULL)
			break;
		if (name == producer->menuItem->Label())
			break;
	}

	return producer;
}
