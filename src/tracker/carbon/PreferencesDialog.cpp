/*
 *  tracker/carbon/PreferencesDialog.cpp
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
 *  PreferencesDialog.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 07.04.06.
 *
 */

#include "PreferencesDialog.h"
#include "Carbon_Definitions.h"
#include "TrackerSettingsDatabase.h"
#include "RtMidi.h"

#define KEY_USEMIDI			"USEMIDI"
#define KEY_RECORDVELOCITY	"RECORDVELOCITY"
#define KEY_VELOCITYAMPLIFY	"VELOCITYAMPLIFY"
#define KEY_SAVEPREFS		"SAVEPREFS"
#define KEY_MIDIDEVICE		"MIDIDEVICE"
#define KEY_INSERTEMULATION "insertemulation"
#define KEY_SIXTEENBITCOLOR "sixteenbitcolor"

static CFStringRef applicationID = CFSTR("com.Titan.MilkyTracker");

pascal OSStatus PreferencesDialog::WindowEventHandler(EventHandlerCallRef myHandler, EventRef event, void* userData)
{
    OSStatus	result = eventNotHandledErr;    
    UInt32		eventClass, eventKind;
    
    eventClass	= GetEventClass(event);
    eventKind	= GetEventKind(event);

	PreferencesDialog* prefDlg = reinterpret_cast<PreferencesDialog*>(userData);
	
    switch (eventClass) 
	{
        case kEventClassControl: 
		{
			ControlRef	targetControl = NULL;
			ControlID	targetControlID;

			GetEventParameter(event, kEventParamDirectObject, typeControlRef, 
							  NULL, sizeof(targetControl), NULL, &targetControl);
			if (targetControl)
				GetControlID(targetControl, &targetControlID);

			switch (eventKind) 
			{
				case kEventControlHit: 
				{					
					switch (targetControlID.id)
					{
						// simulate insert key with ctrl+up
						case 131:
							prefDlg->setFakeInsertKey(GetControl32BitValue(targetControl)-1);
							result = noErr;
							break;

						// 15 bit color toggle
						case 132:
							prefDlg->toggleUse15BitColorDepth();
							result = noErr;
							break;
					
						// Enable MIDI device
						case 129:
							prefDlg->toggleUseMidiDevice();
							result = noErr;
							break;
							
						// Save MIDI preferences
						case 128:
							prefDlg->toggleSavePreferences();
							result = noErr;
							break;

						// Record key velocity
						case 127:
							prefDlg->toggleRecordVelocity();
							result = noErr;
							break;

						// velocity amplify slider
						case 125:
						{
							prefDlg->storeVelocityAmplify(GetControlValue(targetControl));
							result = noErr;
							break;
						}
					}
					
					break;
				}
			}
			
			break;
		}
		
		// Handle window close event (= discard)
		case kEventClassWindow:
		{
			switch (eventKind) 
			{
				case kEventWindowClose: 
				{
					prefDlg->restoreDataBase();
					prefDlg->hide();
					result = noErr;
					break;
				}
			}
			
			break;
		}

        case kEventClassCommand: 
		{
			switch (eventKind) 
			{
				case kEventCommandProcess: 
				{
					HICommand command;
					GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);
					
					switch (command.commandID)
					{
						case kDiscardPreferences:
							prefDlg->restoreDataBase();
							
						case kConfirmPreferences:
						{
							// These two events are better off when sent to the main window
							SendEventToWindow(event, prefDlg->mainWindow);
							break;
						}
						
						default:
							// Handle MIDI device selection from pop-up menu
							if (command.commandID >= kMIDIDeviceBaseCommand && 
								command.commandID <= kMIDIDeviceBaseCommand + prefDlg->getNumMidiDevices())
							{
								prefDlg->storeMidiDeviceName(command.commandID - kMIDIDeviceBaseCommand);
								result = noErr;
							}
					}
					
					break;
				}
			}
			break;
		}
	}

	return result;
}

PreferencesDialog::PreferencesDialog(WindowRef	windowRef, WindowRef mainWindowRef) :
	preferencesWindow(windowRef),
	mainWindow(mainWindowRef),
	m_dataBase(NULL),
	m_dataBaseCopy(NULL),
	midiin(NULL)
{
    EventTypeSpec eventSpec[] = {{kEventClassCommand,kEventCommandProcess}, 
								 {kEventClassControl, kEventControlHit}, 
								 {kEventClassWindow, kEventWindowClose}};

	InstallWindowEventHandler(windowRef, 
							  NewEventHandlerUPP(WindowEventHandler),
							  sizeof(eventSpec)/sizeof(EventTypeSpec), 
							  (EventTypeSpec*)&eventSpec, 
							  (void*)this, 
							  NULL);

	// Create RtMidi instance for querying device information
	try 
	{
		midiin = new RtMidiIn();
	}
	catch (RtMidiError &error) 
	{
		error.printMessage();
		midiin = NULL;
	}	

	initDataBase();
}

PreferencesDialog::~PreferencesDialog()
{
	shutdownDataBase();
	delete m_dataBase;
	delete m_dataBaseCopy;
	delete midiin;
}

void PreferencesDialog::show()
{
	backupDataBase();
	initDialog();
	ShowWindow(preferencesWindow);
	SelectWindow(preferencesWindow);
}

void PreferencesDialog::hide()
{
	HideWindow(preferencesWindow);
}

void PreferencesDialog::initDataBase()
{
	m_dataBase = new TrackerSettingsDatabase();	

	m_dataBase->store(KEY_USEMIDI, 0);
	m_dataBase->store(KEY_SAVEPREFS, 0);
	m_dataBase->store(KEY_MIDIDEVICE, "");
	m_dataBase->store(KEY_RECORDVELOCITY, 0);
	m_dataBase->store(KEY_VELOCITYAMPLIFY, 100);
	m_dataBase->store(KEY_INSERTEMULATION, 0);
	m_dataBase->store(KEY_SIXTEENBITCOLOR, 0);

	// try to retrieve the values from the PLIST
	Boolean success = FALSE;
	CFStringRef key = CFSTR(KEY_USEMIDI);	
	Boolean b = CFPreferencesGetAppBooleanValue(key, applicationID, &success);
	if (success)
		m_dataBase->store(KEY_USEMIDI, b);		
	
	// Return boolean
	success = FALSE;
	key = CFSTR(KEY_SAVEPREFS);	
	b = CFPreferencesGetAppBooleanValue(key, applicationID, &success);
	if (success)
		m_dataBase->store(KEY_SAVEPREFS, b);		

	// Return string, little bit more complicated
	key = CFSTR(KEY_MIDIDEVICE);	
	CFPropertyListRef plistRef = CFPreferencesCopyAppValue(key, applicationID);
	if (plistRef)
	{
		if (CFGetTypeID(plistRef) == CFStringGetTypeID())
		{
			CFStringRef stringRef = static_cast<CFStringRef>(plistRef);
			char buffer[512];
			CFStringGetCString(stringRef, buffer, 512, kCFStringEncodingASCII);
			m_dataBase->store(KEY_MIDIDEVICE, buffer);	
		}
	}

	// More boolean values following
	success = FALSE;
	key = CFSTR(KEY_RECORDVELOCITY);	
	b = CFPreferencesGetAppBooleanValue(key, applicationID, &success);
	if (success)
		m_dataBase->store(KEY_RECORDVELOCITY, b);		

	// Integer value
	success = FALSE;
	key = CFSTR(KEY_VELOCITYAMPLIFY);	
	int i = CFPreferencesGetAppIntegerValue(key, applicationID, &success);
	if (success)
		m_dataBase->store(KEY_VELOCITYAMPLIFY, i);		

	// Integer value
	success = FALSE;
	key = CFSTR(KEY_INSERTEMULATION);	
	i = CFPreferencesGetAppIntegerValue(key, applicationID, &success);
	if (success)
		m_dataBase->store(KEY_INSERTEMULATION, i);		

	// More boolean values following
	success = FALSE;
	key = CFSTR(KEY_SIXTEENBITCOLOR);	
	b = CFPreferencesGetAppBooleanValue(key, applicationID, &success);
	if (success)
		m_dataBase->store(KEY_SIXTEENBITCOLOR, b);		
}

void PreferencesDialog::shutdownDataBase()
{
	CFStringRef yes = CFSTR("yes");
	CFStringRef no  = CFSTR("no");

	if (m_dataBase->restore(KEY_SAVEPREFS)->getIntValue())
	{
		CFStringRef key = CFSTR(KEY_USEMIDI);		
		CFPreferencesSetAppValue(key, m_dataBase->restore(KEY_USEMIDI)->getBoolValue() ? yes : no, applicationID);

		key = CFSTR(KEY_SAVEPREFS);		
		CFPreferencesSetAppValue(key, m_dataBase->restore(KEY_SAVEPREFS)->getBoolValue() ? yes : no, applicationID);

		key = CFSTR(KEY_MIDIDEVICE);		
		CFStringRef CFStrDevName = CFStringCreateWithCString(NULL, m_dataBase->restore(KEY_MIDIDEVICE)->getStringValue(), kCFStringEncodingASCII);
		CFPreferencesSetAppValue(key, CFStrDevName, applicationID);
		CFRelease(CFStrDevName);

		key = CFSTR(KEY_RECORDVELOCITY);		
		CFPreferencesSetAppValue(key, m_dataBase->restore(KEY_RECORDVELOCITY)->getBoolValue() ? yes : no, applicationID);

		SInt32 number = m_dataBase->restore(KEY_VELOCITYAMPLIFY)->getIntValue();
		CFNumberRef numberRef = CFNumberCreate(NULL, kCFNumberSInt32Type, &number);
		key = CFSTR(KEY_VELOCITYAMPLIFY);		
		CFPreferencesSetAppValue(key,  numberRef, applicationID);
		CFRelease(numberRef);
	}
	else
	{
		// remove keys from preferences (or something like that if it's possible)
		CFStringRef key = CFSTR(KEY_USEMIDI);		
		CFPreferencesSetAppValue(key, NULL, applicationID);

		key = CFSTR(KEY_SAVEPREFS);		
		CFPreferencesSetAppValue(key, NULL, applicationID);

		key = CFSTR(KEY_MIDIDEVICE);		
		CFPreferencesSetAppValue(key, NULL, applicationID);

		key = CFSTR(KEY_RECORDVELOCITY);		
		CFPreferencesSetAppValue(key, NULL, applicationID);

		key = CFSTR(KEY_VELOCITYAMPLIFY);		
		CFPreferencesSetAppValue(key, NULL, applicationID);
	}
	
	SInt32 number = m_dataBase->restore(KEY_INSERTEMULATION)->getIntValue();
	CFNumberRef numberRef = CFNumberCreate(NULL, kCFNumberSInt32Type, &number);
	CFStringRef key = CFSTR(KEY_INSERTEMULATION);		
	CFPreferencesSetAppValue(key,  numberRef, applicationID);
	CFRelease(numberRef);
	
	key = CFSTR(KEY_SIXTEENBITCOLOR);		
	CFPreferencesSetAppValue(key, m_dataBase->restore(KEY_SIXTEENBITCOLOR)->getBoolValue() ? yes : no, applicationID);
	
	CFPreferencesAppSynchronize(applicationID);
}

void PreferencesDialog::backupDataBase()
{
	if (!m_dataBase)
		return;

	if (m_dataBaseCopy)
		delete m_dataBaseCopy;

	m_dataBaseCopy = new TrackerSettingsDatabase(*m_dataBase);
}

void PreferencesDialog::restoreDataBase()
{
	if (!m_dataBaseCopy)
		return;

	if (m_dataBase)
		delete m_dataBase;

	m_dataBase = new TrackerSettingsDatabase(*m_dataBaseCopy);
}

void PreferencesDialog::updateControls()
{
	if (NULL==m_dataBase)
		return;

	ControlHandle checkBoxControl, control;
	ControlID checkBoxControlID[] = {{kAppSignature, 129}, {kAppSignature, 128}, {kAppSignature, 127}, 
									 {kAppSignature, 126}, {kAppSignature, 125}, {kAppSignature, 124},
									 {kAppSignature, 131}, {kAppSignature, 132}};

	GetControlByID(preferencesWindow, &checkBoxControlID[0], &checkBoxControl);
	bool b = false;

	if ((midiin && !midiin->getPortCount()) || !midiin)
		DisableControl(checkBoxControl);	
	else
	{
		EnableControl(checkBoxControl);	
		b = m_dataBase->restore(KEY_USEMIDI)->getBoolValue();
		SetControl32BitValue(checkBoxControl, b ? TRUE : FALSE);
	}

	GetControlByID(preferencesWindow, &checkBoxControlID[1], &checkBoxControl);
	b = m_dataBase->restore(KEY_SAVEPREFS)->getBoolValue();
	SetControl32BitValue(checkBoxControl, b ? TRUE : FALSE);

	GetControlByID(preferencesWindow, &checkBoxControlID[2], &checkBoxControl);
	b = false;
	
	if ((midiin && !midiin->getPortCount()) || !midiin)
	{
		// Disable checkbox
		DisableControl(checkBoxControl);	
		
		// disable min. value text for slider
		GetControlByID(preferencesWindow, &checkBoxControlID[3], &control);
		DisableControl(control);	
		
		// disable slider
		GetControlByID(preferencesWindow, &checkBoxControlID[4], &control);
		DisableControl(control);	
		
		// disable max. value text for slider
		GetControlByID(preferencesWindow, &checkBoxControlID[5], &control);
		DisableControl(control);	
	}
	else
	{
		EnableControl(checkBoxControl);	
		b = m_dataBase->restore(KEY_RECORDVELOCITY)->getBoolValue();
		SetControl32BitValue(checkBoxControl, b ? TRUE : FALSE);

		// enable min. value text for slider
		GetControlByID(preferencesWindow, &checkBoxControlID[3], &control);
		EnableControl(control);	
		
		// enable slider
		GetControlByID(preferencesWindow, &checkBoxControlID[4], &control);
		EnableControl(control);	
		
		// enable max. value text for slider
		GetControlByID(preferencesWindow, &checkBoxControlID[5], &control);
		EnableControl(control);	
	}

	GetControlByID(preferencesWindow, &checkBoxControlID[6], &checkBoxControl);
	UInt32 value = m_dataBase->restore(KEY_INSERTEMULATION)->getIntValue();
	SetControl32BitValue(checkBoxControl, value+1);

	GetControlByID(preferencesWindow, &checkBoxControlID[7], &checkBoxControl);
	b = m_dataBase->restore(KEY_SIXTEENBITCOLOR)->getBoolValue();
	SetControl32BitValue(checkBoxControl, b ? TRUE : FALSE);
}

void PreferencesDialog::initDialog()
{
	ControlHandle popupButtonControl;
	ControlID popupButtonControlID = {kAppSignature, 130};
	MenuRef menu;
	
	GetControlByID(preferencesWindow, &popupButtonControlID, &popupButtonControl);
	GetControlData(popupButtonControl, kControlEntireControl, kControlPopupButtonMenuRefTag, sizeof(menu), &menu, NULL);

	unsigned int nPorts = midiin ? midiin->getPortCount() : 0;

	if (nPorts)
	{
		UInt32 numItems = CountMenuItems(menu);
		DeleteMenuItems(menu, 1, numItems); 
		
		// Check inputs.
		std::string portName;
		unsigned int nSelectedDevice = 0;
		for (unsigned int i = 0; i < nPorts; i++ ) 
		{
			try 
			{
				portName = midiin->getPortName(i);
			}
			catch (RtMidiError &error) 
			{
				error.printMessage();
			}
			
			if (strcmp(portName.c_str(), m_dataBase->restore(KEY_MIDIDEVICE)->getStringValue()) == 0)
				nSelectedDevice = i;
			
			CFStringRef CFStrPortName = CFStringCreateWithCString(NULL, portName.c_str(), kCFStringEncodingASCII);
			AppendMenuItemTextWithCFString(menu, CFStrPortName, 0, kMIDIDeviceBaseCommand + i, 0);
			CFRelease(CFStrPortName);
			
		}

		/*AppendMenuItemTextWithCFString(menu, CFSTR("Temp1"), 0, kMIDIDeviceBaseCommand + 1, 0);
		AppendMenuItemTextWithCFString(menu, CFSTR("Temp2"), 0, kMIDIDeviceBaseCommand + 2, 0);
		nPorts+=2;*/
		
		SetControlMaximum(popupButtonControl, nPorts);
		
		SetControlValue(popupButtonControl, nSelectedDevice + 1);
		
		storeMidiDeviceName(nSelectedDevice);

		EnableControl(popupButtonControl);			
	}
	else
	{
		DisableControl(popupButtonControl);	
	}
	
	updateControls();

	updateSliderVelocityAmplify();	
}

UInt32 PreferencesDialog::getComboSelection()
{
	ControlHandle popupButtonControl;
	ControlID popupButtonControlID = {kAppSignature, 130};
	
	GetControlByID(preferencesWindow, &popupButtonControlID, &popupButtonControl);
	return (unsigned int)GetControlValue(popupButtonControl) - 1;
}

void PreferencesDialog::toggleRecordVelocity()
{
	if (m_dataBase)
	{
		int i = m_dataBase->restore(KEY_RECORDVELOCITY)->getIntValue();
		m_dataBase->restore(KEY_RECORDVELOCITY)->store(!i);
		updateControls();
	}
}

void PreferencesDialog::toggleUseMidiDevice()
{
	if (m_dataBase)
	{
		int i = m_dataBase->restore(KEY_USEMIDI)->getIntValue();
		m_dataBase->restore(KEY_USEMIDI)->store(!i);
		updateControls();
	}
}

void PreferencesDialog::toggleSavePreferences()
{
	if (m_dataBase)
	{
		int i = m_dataBase->restore(KEY_SAVEPREFS)->getIntValue();
		m_dataBase->restore(KEY_SAVEPREFS)->store(!i);
		updateControls();
	}
}

void PreferencesDialog::updateSliderVelocityAmplify()
{
	ControlHandle control, controlSlider;
	ControlID controlID1 = {kAppSignature, 127};
	ControlID controlID2 = {kAppSignature, 125};
	GetControlByID(preferencesWindow, &controlID1, &control);
	GetControlByID(preferencesWindow, &controlID2, &controlSlider);

	char buffer[1024];
	sprintf(buffer, "Record velocity (amplify: %i%%)", m_dataBase->restore(KEY_VELOCITYAMPLIFY)->getIntValue());
	
	CFStringRef CFStrVelocityAmplify = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingASCII);
	SetControlTitleWithCFString(control, CFStrVelocityAmplify);
	CFRelease(CFStrVelocityAmplify);
	
	SetControlValue(controlSlider, m_dataBase->restore(KEY_VELOCITYAMPLIFY)->getIntValue());
}

void PreferencesDialog::storeMidiDeviceName(UInt32 deviceID)
{
	if (midiin && m_dataBase && deviceID < getNumMidiDevices())
	{
		try 
		{
			std::string portName = midiin->getPortName(deviceID);
			m_dataBase->restore(KEY_MIDIDEVICE)->store(portName.c_str());		
		}
		catch (RtMidiError &error) 
		{
			error.printMessage();
		}
	}
}

void PreferencesDialog::storeVelocityAmplify(UInt32 amplify)
{
	if (m_dataBase)
	{
		m_dataBase->restore(KEY_VELOCITYAMPLIFY)->store(amplify);
		updateSliderVelocityAmplify();
	}
}

void PreferencesDialog::setFakeInsertKey(UInt32 style)
{
	if (m_dataBase)
	{
		m_dataBase->restore(KEY_INSERTEMULATION)->store(style);
		updateControls();
	}
}

void PreferencesDialog::toggleUse15BitColorDepth()
{
	if (m_dataBase)
	{
		int i = m_dataBase->restore(KEY_SIXTEENBITCOLOR)->getIntValue();
		m_dataBase->restore(KEY_SIXTEENBITCOLOR)->store(!i);
		updateControls();
	}
}

UInt32 PreferencesDialog::getMidiDevIDFromString(const char* string)
{
	UInt32 nSelectedDevice = (unsigned)-1;

	if (!midiin)
		return nSelectedDevice;

	std::string portName;
	for (unsigned int i = 0; i < getNumMidiDevices(); i++ ) 
	{
		try 
		{
			portName = midiin->getPortName(i);
		}
		catch (RtMidiError &error) 
		{
			error.printMessage();
		}
		
		if (strcmp(portName.c_str(), string) == 0)
			nSelectedDevice = i;		
	}
	
	return nSelectedDevice;
}

UInt32 PreferencesDialog::getNumMidiDevices()
{
	return midiin ? midiin->getPortCount() : 0;
}

UInt32 PreferencesDialog::getSelectedMidiDeviceID()
{
	if (m_dataBase)
	{
		return getMidiDevIDFromString(m_dataBase->restore(KEY_MIDIDEVICE)->getStringValue());
	}

	return (unsigned)-1;
}

bool PreferencesDialog::getUseMidiDeviceFlag()
{
	if (m_dataBase)
	{
		return m_dataBase->restore(KEY_USEMIDI)->getBoolValue();
	}

	return false;
}

bool PreferencesDialog::getRecordVelocityFlag()
{
	if (m_dataBase)
	{
		return m_dataBase->restore(KEY_RECORDVELOCITY)->getBoolValue();
	}

	return false;
}

UInt32 PreferencesDialog::getVelocityAmplify()
{
	if (m_dataBase)
	{
		return m_dataBase->restore(KEY_VELOCITYAMPLIFY)->getIntValue();
	}

	return 100;
}

UInt32 PreferencesDialog::getFakeInsertKey()
{
	if (m_dataBase)
	{
		return m_dataBase->restore(KEY_INSERTEMULATION)->getIntValue();
	}

	return 0;
}

bool PreferencesDialog::getUse15BitColorDepth()
{
	if (m_dataBase)
	{
		return m_dataBase->restore(KEY_SIXTEENBITCOLOR)->getBoolValue();
	}

	return false;
}
