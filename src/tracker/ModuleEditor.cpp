/*
 *  tracker/ModuleEditor.cpp
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

#include <new>
#include "ModuleEditor.h"
#include "PatternEditor.h"
#include "SampleEditor.h"
#include "EnvelopeEditor.h"
#include "ModuleServices.h"
#include "PlayerCriticalSection.h"
#include "TrackerConfig.h"
#include "PPSystem.h"

static const char validCharacters[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_!.";

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// convert string
static mp_sint32 convertStr(SYSCHAR* buffer, char* src)
{
	mp_sint32 j = 0;
	for (mp_sint32 i = 0; i < MP_MAXTEXT; i++)
	{
		if (src[i] == '\0')
			break;
		
		bool found = false;
		for (mp_sint32 k = 0; k < (signed)(sizeof(validCharacters)/sizeof(char))-1; k++)
			if (src[i] == validCharacters[k])
			{
				found = true;
				break;
			}
		
		if (found)
			buffer[j++] = src[i];
	}
	
	buffer[j] = '\0';
	return j;
}

class ChangesListener : public EditorBase::EditorNotificationListener
{
private:
	virtual void editorNotification(EditorBase* sender, EditorBase::EditorNotifications notification)
	{
		switch (notification)
		{
			case EditorBase::NotificationChanges:
			{
				if (sender == moduleEditor.sampleEditor)
					moduleEditor.finishSamples();
				moduleEditor.setChanged();
				break;
			}
			
			case EditorBase::NotificationPrepareCritical:
				moduleEditor.enterCriticalSection();
				break;

			case EditorBase::NotificationUnprepareCritical:
				moduleEditor.leaveCriticalSection();
				break;
			default:
				break;
		}
	}
	
	ModuleEditor& moduleEditor;
	
public:
	ChangesListener(ModuleEditor& moduleEditor) :
		moduleEditor(moduleEditor)
	{
	}
};

ModuleEditor::ModuleEditor() :
	module(NULL),
	patternEditor(NULL),
	sampleEditor(NULL),
	envelopeEditor(NULL),
	playerCriticalSection(NULL),
	changed(false),
	eSaveType(ModSaveTypeXM),
	lastRequestedPatternIndex(0),
	currentOrderIndex(0),
	currentPatternIndex(0),
	currentInstrumentIndex(0),
	currentSampleIndex(0),
	enumerationIndex(-1)
{
	instruments = new TEditorInstrument[MAX_INSTRUMENTS];

	moduleFileName = TrackerConfig::untitledSong;

	// no extension here
	adjustExtension(false);

	module = new XModule();
	
	createNewSong();

	changesListener = new ChangesListener(*this);

	// create pattern editor
	patternEditor = new PatternEditor();
	patternEditor->addNotificationListener(changesListener);
	reloadCurrentPattern();
	
	// create sample editor
	sampleEditor = new SampleEditor();
	sampleEditor->addNotificationListener(changesListener);
	reloadSample(0, 0);
	
	// create envelope editor
	envelopeEditor = new EnvelopeEditor();
	envelopeEditor->addNotificationListener(changesListener);
	envelopeEditor->attachEnvelope(NULL, module);
	
	moduleServices = new ModuleServices(*module);

	currentCursorPosition.row = currentCursorPosition.channel = currentCursorPosition.inner = 0;	
}

ModuleEditor::~ModuleEditor()
{
	delete moduleServices;
	delete sampleEditor;
	delete patternEditor;
	delete envelopeEditor;
	// must be deleted AFTER the editors
	delete changesListener;
	delete module;

	delete[] instruments;
}

PPSystemString ModuleEditor::getModuleFileNameFull(ModSaveTypes extension/* = ModSaveTypeDefault*/) 
{ 
	PPSystemString s = moduleFileName;
	
	PPSystemString s2;
	if (extension != ModSaveTypeDefault)
	{
		ModSaveTypes eOldType = eSaveType;
		eSaveType = extension;
		adjustExtension();
		eSaveType = eOldType;
	}
	s2 = moduleFileName;
	moduleFileName = s;
	return s2;
}

PPSystemString ModuleEditor::getModuleFileName(ModSaveTypes extension/* = ModSaveTypeDefault*/) 
{
	PPSystemString s = getModuleFileNameFull(extension);
	
	return s.stripPath(); 
}

void ModuleEditor::reloadCurrentPattern()
{
	TXMPattern* pattern = patternEditor->getPattern();

	if (pattern && 
		pattern->patternData && 
		pattern->channum == TrackerConfig::numPlayerChannels &&
		module && 
		pattern == &module->phead[getCurrentPatternIndex()])
		return;
	
	patternEditor->attachPattern(getPattern(getCurrentPatternIndex()), module);
}

void ModuleEditor::reloadSample(mp_sint32 insIndex, mp_sint32 smpIndex)
{
	sampleEditor->attachSample(getSampleInfo(insIndex, smpIndex), module);
}

void ModuleEditor::reloadEnvelope(mp_sint32 insIndex, mp_sint32 smpIndex, mp_sint32 type)
{
	envelopeEditor->attachEnvelope(getEnvelope(insIndex, smpIndex, type), module);
}

void ModuleEditor::enterCriticalSection()
{
	if (playerCriticalSection)
		playerCriticalSection->enter();
}

void ModuleEditor::leaveCriticalSection()
{
	if (playerCriticalSection)
		playerCriticalSection->leave();
}

void ModuleEditor::adjustExtension(bool hasExtension/* = true*/)
{
	if (hasExtension)
		moduleFileName = moduleFileName.stripExtension();

	switch (eSaveType)
	{
		case ModSaveTypeXM:
			moduleFileName.append(".xm");
			break;

		case ModSaveTypeMOD:
			moduleFileName.append(".mod");
			break;
			
		default:
			ASSERT(false);
	}
}

void ModuleEditor::convertInstrument(mp_sint32 i)
{
	mp_sint32 j,k;
	
	TXMInstrument* ins = &module->instr[i];	
	instruments[i].instrument = ins;	
	instruments[i].numUsedSamples = 16;

	for (j = 0; j < 16; j++)
		instruments[i].usedSamples[j] = i*16+j;

	// build FT2 compatible note->sample LUT
	for (j = 0; j < MAX_NOTE; j++)
	{
		mp_sword s = ins->snum[j];
		
		// empty entry
		if (s == -1)
		{
			instruments[i].nbu[j] = /*255*/0;
			ins->snum[j] = i*16;
			continue;
		}
		
		bool found = false;
		for (k = 0; k < 16; k++)
			if (i*16+k == s)
			{
				instruments[i].nbu[j] = k;
				found = true;
				break;
			}

		if (!found)
		{
			instruments[i].nbu[j] = /*255*/0;
			ins->snum[j] = i*16;
			//exit(1);
		}
	}		

	if (ins->samp)
	{
		// take default envelope from first sample in instrument
		mp_sint32 venvIndex = module->smp[i*16].venvnum - 1;		
		if (venvIndex == -1)
		{			
			for (mp_sint32 e = 0; e < module->header.volenvnum; e++)
			{
				bool used = false;
				for (mp_sint32 s = 0; s < module->header.smpnum; s++)
				{
					if (module->smp[s].venvnum - 1 == e)
					{
						used = true; 
						break;
					}
				}

				if (!used)
				{
					venvIndex = e;
					break;
				}
			}
		}

		if (venvIndex == -1)
		{
			TEnvelope venv;
			memset(&venv, 0, sizeof(venv));
			venv.type = 0;
			venv.num = 2;
			venv.loops = 0;
			venv.loope = 1;
			venv.sustain = 0;
			venv.env[0][0] = 0;
			venv.env[0][1] = 256;
			venv.env[1][0] = 32;
			venv.env[1][1] = 256;
			module->addVolumeEnvelope(venv);
			venvIndex = module->numVEnvs - 1;
			module->header.volenvnum = module->numVEnvs;
		}

		instruments[i].volumeEnvelope = venvIndex;

		// same for panning envelope index
		mp_sint32 penvIndex = module->smp[i*16].penvnum - 1;
		if (penvIndex == -1)
		{
			for (mp_sint32 e = 0; e < module->header.panenvnum; e++)
			{
				bool used = false;
				for (mp_sint32 s = 0; s < module->header.smpnum; s++)
				{
					if (module->smp[s].penvnum - 1 == e)
					{
						used = true; 
						break;
					}
				}

				if (!used)
				{
					penvIndex = e;
					break;
				}
			}
		}

		if (penvIndex == -1)
		{
			TEnvelope penv;
			memset(&penv, 0, sizeof(penv));
			penv.type=0;
			penv.num=2;
			penv.loops=0;
			penv.loope=1;
			penv.sustain = 0;
			penv.env[0][0]=0;
			penv.env[0][1]=128;
			penv.env[1][0]=32;
			penv.env[1][1]=128;
			module->addPanningEnvelope(penv);
			penvIndex = module->numPEnvs - 1;
			module->header.panenvnum = module->numPEnvs;
		}

		instruments[i].panningEnvelope = penvIndex;

		mp_sint32 s = instruments[i].usedSamples[0];
		
		// take all fadeout/autovibrato settings from first sample in instrument
		instruments[i].volfade = module->smp[s].volfade >> 1;
		instruments[i].vibtype = module->smp[s].vibtype;
		instruments[i].vibrate = module->smp[s].vibrate;
		instruments[i].vibdepth = module->smp[s].vibdepth >> 1;
		instruments[i].vibsweep = module->smp[s].vibsweep;
	}
	else
	{
	
		for (j = 0; j < MAX_NOTE; j++)
			ins->snum[j] = i*16;
	
		mp_sint32 e;
		mp_sint32 venvIndex = - 1;
		
		for (e = 0; e < module->header.volenvnum; e++)
		{
			bool used = false;
			for (mp_sint32 s = 0; s < module->header.smpnum; s++)
			{
				if (module->smp[s].venvnum - 1 == e)
				{
					used = true; 
					break;
				}
			}
			
			if (!used)
			{
				venvIndex = e;
				break;
			}
		}

		if (venvIndex == -1)
		{
			TEnvelope venv;
			memset(&venv, 0, sizeof(venv));
			venv.type=0;
			venv.num=2;
			venv.loops=0;
			venv.loope=1;
			venv.sustain = 0;
			venv.env[0][0]=0;
			venv.env[0][1]=256;
			venv.env[1][0]=32;
			venv.env[1][1]=256;
			module->addVolumeEnvelope(venv);
			venvIndex = module->numVEnvs - 1;
			module->header.volenvnum = module->numVEnvs;
		}

		instruments[i].volumeEnvelope = venvIndex;

		mp_sint32 penvIndex = - 1;
		
		for (e = 0; e < module->header.panenvnum; e++)
		{
			bool used = false;
			for (mp_sint32 s = 0; s < module->header.smpnum; s++)
			{
				if (module->smp[s].penvnum - 1 == e)
				{
					used = true; 
					break;
				}
			}
			
			if (!used)
			{
				penvIndex = e;
				break;
			}
		}

		if (penvIndex == -1)
		{
			TEnvelope penv;
			memset(&penv, 0, sizeof(penv));
			penv.type=0;
			penv.num=2;
			penv.loops=0;
			penv.loope=1;
			penv.sustain = 0;
			penv.env[0][0]=0;
			penv.env[0][1]=128;
			penv.env[1][0]=32;
			penv.env[1][1]=128;
			module->addPanningEnvelope(penv);
			penvIndex = module->numPEnvs - 1;
			module->header.panenvnum = module->numPEnvs;
		}

		instruments[i].panningEnvelope = penvIndex;

		//-----------------------------------------------------------
		// this will only work if there are 16 sample per instrument
		//-----------------------------------------------------------
		for (j = 0; j < 16; j++)
		{
			module->smp[i*16+j].venvnum = venvIndex + 1;
			module->smp[i*16+j].penvnum = penvIndex + 1;
			// default fade out to cut
			module->smp[i*16+j].volfade = 0xFFFF;
		}

		instruments[i].volfade = 0xFFFF >> 1;
		instruments[i].vibtype = 0;
		instruments[i].vibrate = 0;
		instruments[i].vibdepth = 0;
		instruments[i].vibsweep = 0;
	}


	for (j = 0; j < 16; j++)
	{
		if (!module->smp[i*16+j].sample)
		{
			module->smp[i*16+j].vol = 0xFF;
			module->smp[i*16+j].pan = 0x80;
		}
	}
}

void ModuleEditor::buildInstrumentTable()
{	
	mp_sint32 i,j;

	for (i = 0; i < MAX_INSTRUMENTS; i++)
	{
		instruments[i].instrument = NULL;
		instruments[i].numUsedSamples = 0;
		for (j = 0; j < 16; j++)
			instruments[i].usedSamples[j] = -1;
		instruments[i].volumeEnvelope = instruments[i].panningEnvelope = -1;
		
		memset(instruments[i].nbu, 0, MAX_NOTE);
	}

	for (i = 0; i < module->header.insnum; i++)
	{
		convertInstrument(i);
	}

	validateInstruments();
}

void ModuleEditor::validateInstruments()
{
	for (mp_sint32 i = 0; i < module->header.insnum; i++)
	{	
		mp_sint32 lastUsedInstrument = -1;
		for (mp_sint32 j = 15; j >= 0; j--)
		{
			mp_sint32 s = instruments[i].usedSamples[j];
			if (module->smp[s].sample)
			{
				lastUsedInstrument = j;
			}
			
			module->smp[s].flags = 3;
		}
		
		instruments[i].instrument->samp = 16;
	}
}

bool ModuleEditor::allocatePattern(TXMPattern* pattern)
{
	// create empty pattern
	pattern->channum = (mp_ubyte)/*numChannels*/TrackerConfig::numPlayerChannels;
	pattern->rows = 64;
	// create XM style pattern, two effects
	pattern->effnum = 2;
	
	mp_sint32 slotSize = pattern->effnum * 2 + 2;

	mp_sint32 patternSize = slotSize * pattern->channum * pattern->rows;

	pattern->patternData = new mp_ubyte[patternSize];

	if (pattern->patternData == NULL)
		return false;

	memset(pattern->patternData, 0, patternSize);
	return true;
}

void ModuleEditor::createEmptySong(bool clearPatterns/* = true*/, bool clearInstruments/* = true*/, mp_sint32 numChannels/* = 8*/)
{
	if (module)
	{
		enterCriticalSection();
	
		setCurrentOrderIndex(0);
		setCurrentPatternIndex(0);
	
		module->createEmptySong(clearPatterns, clearInstruments, numChannels);

		if (clearPatterns && clearInstruments)
		{
			changed = false;

			eSaveType = ModSaveTypeXM;
			
			moduleFileName = TrackerConfig::untitledSong;
			
			// no extension
			adjustExtension(false);
		}
		else
		{
			changed = true;
		}
	
		buildInstrumentTable();
		
		lastRequestedPatternIndex = 0;
		
		leaveCriticalSection();	
	}
}

bool ModuleEditor::createNewSong(mp_uword numChannels/*= 8*/)
{
	module->createEmptySong(true, true, numChannels);

	changed = false;

	eSaveType = ModSaveTypeXM;

	moduleFileName = TrackerConfig::untitledSong;

	// no extension
	adjustExtension(false);

	buildInstrumentTable();

	lastRequestedPatternIndex = 0;
	
	return true;
}

bool ModuleEditor::isEmpty() const
{
	mp_sint32 patNum = module->header.patnum;
	if (patNum != 1)
		return false;
	
	TXMPattern* pattern = &module->phead[0];
	
	if (pattern->patternData != NULL)
	{
		mp_sint32 slotSize = pattern->effnum * 2 + 2;
		
		mp_sint32 patternSize = slotSize * pattern->channum * pattern->rows;
		
		bool empty = true;
		for (mp_sint32 j = 0; j < patternSize; j++)
			if (pattern->patternData[j])
			{
				empty = false;
				break;
			}
	
		if (!empty)
			return false;
	} 

	// step two, find last used instrument
	mp_sint32 insNum = module->getNumUsedInstruments();
	if (insNum)
		return false;
		
	char temp[MAX_TITLETEXT+1];
	memset(temp, 0, sizeof(temp));
	getTitle(temp, MAX_TITLETEXT);	

	if (strlen(temp))
		return true;

	return true;
}

bool ModuleEditor::openSong(const SYSCHAR* fileName, const SYSCHAR* preferredFileName/* = NULL*/)
{
	if (!XMFile::exists(fileName))
		return false;

	mp_sint32 nRes = module->loadModule(fileName);
	
	// unknown format
	if (nRes == MP_UNKNOWN_FORMAT)
	{
		return false;
	}

	bool res = (nRes == MP_OK);

	XModule::ModuleTypes type = XModule::ModuleType_NONE;

	if (module->getType() != XModule::ModuleType_XM)
	{	
		type = module->getType();
	
		mp_ubyte oneShotFlags[MAX_INSTRUMENTS];
		// save flags for one shot PT style looping, it will be stripped out
		// when exporting to XM
		if (type == XModule::ModuleType_MOD)
		{
			memset(oneShotFlags, 0, sizeof(oneShotFlags));
			for (mp_sint32 i = 0; i < module->header.insnum; i++)
			{
				mp_sint32 snum = module->instr[i].snum[0];
				if (snum >= 0)
				{
					oneShotFlags[i] = module->smp[snum].type & 32;
				}
			}
		} 
	
		PPSystemString tempFile(getTempFilename());
	
		try
		{
			res = module->saveExtendedModule(tempFile) == MP_OK;
			if(!res)
				return res;

			res = module->loadModule(tempFile) == MP_OK;
		} catch (const std::bad_alloc &) {
			return false;
		}
	
		// restore one shot looping flag
		if (type == XModule::ModuleType_MOD)
		{
			for (mp_sint32 i = 0; i < module->header.insnum; i++)
			{
				mp_sint32 snum = module->instr[i].snum[0];
				if (snum >= 0)
				{
					if (oneShotFlags[i])
						module->smp[snum].type |= 32;
				}
			}
		} 
	
		XMFile::remove(tempFile);
	}

	if (module->header.channum > TrackerConfig::numPlayerChannels)
		res = false;

	lastRequestedPatternIndex = 0;
	
	if (res)
	{
		changed = false;
		
		buildInstrumentTable();
		
		// expand patterns to 32 channels width
		for (mp_sint32 i = 0; i < module->header.patnum; i++)
			getPattern(i);
		
		PPSystemString strFileName = preferredFileName ? preferredFileName : fileName;

		moduleFileName = strFileName.stripExtension();
		
		if (type == XModule::ModuleType_MOD)
			eSaveType = ModSaveTypeMOD;
		else	
			eSaveType = ModSaveTypeXM;
		
		// no extension
		adjustExtension(false);
	}
	else
	{
		createNewSong();
	}

	cleanUnusedPatterns();
	
	return res;
}

bool ModuleEditor::saveSong(const SYSCHAR* fileName, ModSaveTypes saveType/* = eXM*/)
{
	// too risky
	//cleanUnusedPatterns();

	bool res = false;
	
	switch (saveType)
	{
		case ModSaveTypeDefault:
		case ModSaveTypeXM:
			res = module->saveExtendedModule(fileName) == 0;
			break;
			
		case ModSaveTypeMOD:
			res = module->saveProtrackerModule(fileName) == 0;
			break;
	}

	eSaveType = saveType;

	moduleFileName = fileName;
	
	// has extension
	adjustExtension();

	changed = false;

	return res;
}

mp_sint32 ModuleEditor::saveBackup(const SYSCHAR* fileName)
{
	return module->saveExtendedModule(fileName);
}


void ModuleEditor::increaseSongLength()
{
	if (module->header.ordnum < 255)
	{
		module->header.ordnum++;
		changed = true;
	}
}

void ModuleEditor::decreaseSongLength()
{
	if (module->header.ordnum > 1)
	{
		module->header.ordnum--;
		changed = true;
	}
}

void ModuleEditor::increaseRepeatPos()
{
	mp_uword old = module->header.restart;

	if (module->header.restart < 255)
		module->header.restart++;
		
	if (module->header.restart >= module->header.ordnum)
		module->header.restart = module->header.ordnum - 1;

	if (old != module->header.restart)
		changed = true;
}

void ModuleEditor::decreaseRepeatPos()
{
	if (module->header.restart > 0)
	{
		module->header.restart--;
		changed = true;
	}
}

bool ModuleEditor::insertNewOrderPosition(mp_sint32 index)
{

	if (module->header.ordnum >= 255)
		return false;

	mp_ubyte temp[256];

	mp_sint32 i;

	for (i = 0; i <= index; i++)
		temp[i] = module->header.ord[i];

	temp[index+1] = module->header.ord[index];

	for (i = index+2; i <= module->header.ordnum; i++)
		temp[i] = module->header.ord[i-1];

	module->header.ordnum++;

	memcpy(module->header.ord, temp, module->header.ordnum);

	changed = true;

	return true;
}

void ModuleEditor::deleteOrderPosition(mp_sint32 index)
{
	if (index < module->header.ordnum && module->header.ordnum > 1)
	{
		for (mp_sint32 i = index; i < module->header.ordnum - 1; i++)
			module->header.ord[i] = module->header.ord[i+1];

		module->header.ordnum--;
	
		changed = true;
	}
}

bool ModuleEditor::seqCurrentOrderPosition(mp_sint32 index, bool clone/* = false*/)
{
	mp_sint32 i;

	if (module->header.ordnum >= 255)
		return false;
		
	pp_int32 srcPatternIndex = module->header.ord[index];

	mp_sint32 highestPattern = module->header.ord[0];
	for (i = 1; i < module->header.ordnum; i++)
		if (module->header.ord[i] > highestPattern)
			highestPattern = module->header.ord[i];

	mp_ubyte temp[256];

	for (i = 0; i <= index; i++)
		temp[i] = module->header.ord[i];

	temp[index+1] = /*module->header.ord[index] + 1*/highestPattern+1;

	pp_int32 dstPatternIndex = highestPattern+1;

	for (i = index+2; i <= module->header.ordnum; i++)
		temp[i] = module->header.ord[i-1];

	module->header.ordnum++;

	memcpy(module->header.ord, temp, module->header.ordnum);

	if (clone)
	{
		// now clone pattern
		module->phead[dstPatternIndex] = module->phead[srcPatternIndex];
	}

	changed = true;

	return true;
}

mp_sint32 ModuleEditor::getOrderPosition(mp_sint32 index) const
{
	return module->header.ord[index];
}

void ModuleEditor::increaseOrderPosition(mp_sint32 index)
{
	if (module->header.ord[index] < 255)
	{
		module->header.ord[index]++;

		changed = true;
	}
}

void ModuleEditor::decreaseOrderPosition(mp_sint32 index)
{
	if (module->header.ord[index] > 0)
	{
		module->header.ord[index]--;

		changed = true;
	}
}

bool ModuleEditor::isEditingOrderPosition(mp_sint32 index) const
{
	if (index < 0 || index >= module->header.ordnum)
		return false;
		
	return patternEditor->getPattern() == &module->phead[module->header.ord[index]];
}

void ModuleEditor::cleanUnusedPatterns()
{
	if (!module)
		return;
		
	for (mp_sint32 i = module->header.patnum - 1; i > lastRequestedPatternIndex; i--)
	{
		TXMPattern* pattern = &module->phead[i];

		if (pattern->patternData == NULL)
			continue;
			
		mp_sint32 slotSize = pattern->effnum * 2 + 2;
		
		mp_sint32 patternSize = slotSize * pattern->channum * pattern->rows;
		
		bool empty = true;
		for (mp_sint32 j = 0; j < patternSize; j++)
			if (pattern->patternData[j])
			{
				empty = false;
				break;
			}
		
		if (empty)
		{
			bool found = false;
			for (mp_sint32 j = 0; j < module->header.ordnum; j++)
				if (module->header.ord[j] == i)
				{
					found = true;
					break;
				}
			
			if (found)
				break;
			
			delete[] pattern->patternData;
			memset(pattern, 0, sizeof(TXMPattern));
			module->header.patnum = i;
		}
		else
		{
			break;
		}
	} 
}

///////////////////////////////////////////////////////////////////////////
// Whenever a pattern is requested we will return a pattern that is
// 32 channels wide, this makes adding/subtracting channels easier
// without losing data in that channel
// When another pattern pattern is selected we will discard unused channels
///////////////////////////////////////////////////////////////////////////
TXMPattern* ModuleEditor::getPattern(mp_sint32 index, bool cleanUnusedPatterns/* = true*/)
{
	lastRequestedPatternIndex = index;

	// handle with care, this might throw away patterns while the player
	// is using them
	if (cleanUnusedPatterns)
		this->cleanUnusedPatterns();
	
	// get requested pattern, allocate one if it's empty
	TXMPattern* pattern = &module->phead[index];

	if (pattern->patternData == NULL)
	{
		bool res = allocatePattern(pattern);

		if (!res)
			return NULL;
	}

	// if the number of channels in this pattern is
	// smaller then 32 we resize to 32 channels
	if (pattern->channum < TrackerConfig::numPlayerChannels)
	{
		mp_sint32 slotSize = pattern->effnum * 2 + 2;
		
		mp_sint32 patternSize = slotSize * TrackerConfig::numPlayerChannels * pattern->rows;

		mp_ubyte* newPatternData = new mp_ubyte[patternSize];
		
		memset(newPatternData, 0, patternSize);
		
		for (mp_sint32 i = 0; i < pattern->rows; i++)
		{
			mp_sint32 srcOffset = i * slotSize * pattern->channum;
			mp_sint32 dstOffset = i * slotSize * TrackerConfig::numPlayerChannels;
			for (mp_sint32 j = 0; j < slotSize * pattern->channum; j++)
				newPatternData[dstOffset++] = pattern->patternData[srcOffset++];
		}
		
		delete[] pattern->patternData;
		
		pattern->patternData = newPatternData;
	
		pattern->channum = (mp_ubyte)TrackerConfig::numPlayerChannels;
	}

	// update number of patterns in module header if necessary
	if (module->header.patnum < index + 1)
		module->header.patnum = index + 1;

	return pattern;

}

#if 0
mp_sint32 ModuleEditor::allocateSample(mp_sint32 index)
{
	if (index < 0) 
		return -1;

	if (index >= module->header.insnum)
		return -1;

	// sorry, XM can only handle 16 samples per instrument
	if (instruments[index].numUsedSamples >= 16)
		return -1;


	// look if we can find an unused sample
	mp_sint32 s = -1;
	
	for (mp_sint32 i = 0; i < module->header.smpnum; i++)
	{
		
		bool used = false;
		//if (module->smp[i].sample == NULL && module->smp[i].samplen == 0)
		//{
		
			for (mp_sint32 j = 0; j < module->header.insnum; j++)
			{
				for (mp_sint32 k = 0; k < instruments[j].numUsedSamples; k++)
				{
					if (instruments[j].usedSamples[k] == i)
					{
						used = true;
						break;
					}
				}

				if (used)
					break;

			}
		
		//}
		//else
		//	used = true;

		if (!used)
		{
			s = i;
			break;
		}

	}

	if (s == -1)
	{
		if (module->header.smpnum >= 254)
			return -2;

		s = module->header.smpnum++;
	}

	if (module->smp[s].sample)
	{
		module->freeSampleMem((mp_ubyte*)module->smp[s].sample);
		module->smp[s].sample = NULL;
	}

	if (!instruments[index].numUsedSamples)
	{
		for (mp_sint32 i = 0; i < MAX_NOTE; i++)
		{
			module->instr[index].snum[i] = s;
			instruments[index].nbu[i] = 0;
		}
	}

	strcpy((char*)module->smp[s].name,"<new sample>");
	module->smp[s].venvnum = instruments[index].volumeEnvelope + 1;
	module->smp[s].penvnum = instruments[index].panningEnvelope + 1;

	instruments[index].usedSamples[instruments[index].numUsedSamples++] = s;

	return 0;

}
#endif

void ModuleEditor::finishSamples()
{
	module->postProcessSamples();
}

mp_sint32 ModuleEditor::allocateInstrument()
{
	if (module->header.insnum >= 255)
		return -1;

	enterCriticalSection();

	mp_sint32 i = module->header.insnum++;

	module->header.smpnum+=16;

	convertInstrument(i);

	validateInstruments();

	leaveCriticalSection();

	changed = true;
	
	return 0;
}

// free sample
void ModuleEditor::freeSample(mp_sint32 index)
{
	return;
	
	if (index < 0) 
		return;

	if (index >= module->header.insnum)
		return;

	if (instruments[index].numUsedSamples)
	{
		instruments[index].usedSamples[--instruments[index].numUsedSamples] = -1;
	
		mp_sint32 i;

		mp_ubyte* nbu = instruments[index].nbu;

		for (i = 0; i < MAX_NOTE; i++)
			if (nbu[i] == instruments[index].numUsedSamples)
				nbu[i] = 255;

 		for (i = 0; i < MAX_NOTE; i++)
		{
			if (nbu[i] != 255)
				module->instr[index].snum[i] = instruments[index].usedSamples[nbu[i]];
			else
				module->instr[index].snum[i] = 255;
		}

	}

}

void ModuleEditor::clearSample(mp_sint32 smpIndex)
{
	if (smpIndex >= 0 && smpIndex < MP_MAXSAMPLES)
	{
		TXMSample* dst = &module->smp[smpIndex];

		module->freeSampleMem((mp_ubyte*)dst->sample);
		dst->sample = NULL;
		dst->samplen = 0;
		dst->loopstart = 0;
		dst->looplen = 0;
		
		changed = true;
	}
}

void ModuleEditor::clearSample(mp_sint32 insIndex, mp_sint32 smpIndex)
{
	if (insIndex < module->header.insnum && smpIndex < 16)
	{
		mp_sint32 s = instruments[insIndex].usedSamples[smpIndex];
		
		clearSample(s);
	}
}

bool ModuleEditor::loadSample(const SYSCHAR* fileName, 
							  mp_sint32 insIndex, 
							  mp_sint32 smpIndex, 
							  mp_sint32 channelIndex,
							  const SYSCHAR* preferredFileName/* = NULL*/)
{
	PPSystemString sysPreferredName(preferredFileName ? preferredFileName : fileName);
	sysPreferredName = sysPreferredName.stripPath();
	char* preferredNameASCIIZ = sysPreferredName.toASCIIZ();
	PPString preferredName(preferredNameASCIIZ);
	delete[] preferredNameASCIIZ;
	
	SampleLoaderGeneric sampleLoader(fileName, *module);
	sampleLoader.setPreferredDefaultName(preferredName);

	if (insIndex < module->header.insnum && smpIndex < 16)
	{
		enterCriticalSection();
	
		bool res = sampleLoader.loadSample(instruments[insIndex].usedSamples[smpIndex], channelIndex) == 0;
		
		if (res)
		{
			TXMSample* dst = &module->smp[instruments[insIndex].usedSamples[smpIndex]];
			
			ASSERT(dst);
			
			// default values first	
			dst->flags = 3;
			dst->venvnum = instruments[insIndex].volumeEnvelope+1;
			dst->penvnum = instruments[insIndex].panningEnvelope+1;
			dst->fenvnum = dst->vibenvnum = 0;
			
			dst->vibtype = instruments[insIndex].vibtype;
			dst->vibsweep = instruments[insIndex].vibsweep;
			dst->vibdepth = instruments[insIndex].vibdepth << 1;
			dst->vibrate = instruments[insIndex].vibrate;
			dst->volfade = instruments[insIndex].volfade << 1;
			
			finishSamples();
			
			validateInstruments();

			changed = true;
		}
		
		leaveCriticalSection();

		return res;
	}

	return false;
}

mp_sint32 ModuleEditor::getNumSampleChannels(const SYSCHAR* fileName)
{
	SampleLoaderGeneric sampleLoader(fileName, *module);
	return sampleLoader.getNumChannels();
}

// get name of channel in sample as returned by sample loader
const char* ModuleEditor::getNameOfSampleChannel(const SYSCHAR* fileName, mp_sint32 index)
{
	SampleLoaderGeneric sampleLoader(fileName, *module);
	return sampleLoader.getChannelName(index);
}

bool ModuleEditor::saveSample(const SYSCHAR* fileName, mp_sint32 insIndex, mp_sint32 smpIndex, SampleFormatTypes format)
{
	SampleLoaderGeneric sampleLoader(fileName, *module);

	if (insIndex < module->header.insnum && smpIndex < 16)
	{
		bool res = false;
	
		switch (format)
		{
			case SampleFormatTypeWAV:
				res = sampleLoader.saveSample(fileName, instruments[insIndex].usedSamples[smpIndex], SampleLoaderGeneric::OutputFiletypeWAV) == 0;
				break;
			case SampleFormatTypeIFF:
				res = sampleLoader.saveSample(fileName, instruments[insIndex].usedSamples[smpIndex], SampleLoaderGeneric::OutputFiletypeIFF) == 0;
				break;
		}
				
		return res;
	}

	return false;
}

const PPSystemString& ModuleEditor::getSampleFileName(mp_sint32 insIndex, mp_sint32 smpIndex)
{
	SYSCHAR buffer[MP_MAXTEXT+1];
	memset(buffer, 0, sizeof(buffer));
	
	TXMSample* smp = getSampleInfo(insIndex,smpIndex);

	mp_sint32 len = convertStr(buffer, smp->name);

	if (!len)
		sampleFileName = getInstrumentFileName(insIndex);
	else
	{
		sampleFileName = buffer;
		sampleFileName = sampleFileName.stripExtension();
	}
	
	return sampleFileName;
}

// free last instrument
void ModuleEditor::freeInstrument()
{
	if (module->header.insnum <= 1)
		return;

	enterCriticalSection();

	for (mp_sint32 j = 0; j < 16; j++)
		instruments[module->header.insnum].usedSamples[j] = -1;

	instruments[module->header.insnum].volumeEnvelope = -1;
	instruments[module->header.insnum].panningEnvelope = -1;

	memset(instruments[module->header.insnum].nbu, 0, MAX_NOTE);

	module->header.insnum--;
	
	module->header.smpnum-=16;

	leaveCriticalSection();

	changed = true;
}

bool ModuleEditor::insertXIInstrument(mp_sint32 index, const XIInstrument* ins)
{
	ASSERT(index < module->header.insnum);

	mp_sint32 j;
	
	ASSERT(instruments[index].numUsedSamples == 16);
	
	memcpy(instruments[index].nbu, ins->nbu, MAX_NOTE);
	
	for (j = 0; j < MAX_NOTE; j++)
		instruments[index].instrument->snum[j] = index*16 + ins->nbu[j];
	
	memcpy(instruments[index].instrument->name, ins->name, MAX_INSTEXT);
	
	instruments[index].instrument->samp = ins->numsamples;
	
	memcpy(&module->venvs[instruments[index].volumeEnvelope], &ins->venv, sizeof(ins->venv));
	memcpy(&module->penvs[instruments[index].panningEnvelope], &ins->penv, sizeof(ins->penv));
	
	instruments[index].volfade = ins->volfade>>1;
	instruments[index].vibtype = ins->vibtype;
	instruments[index].vibrate = ins->vibrate;
	instruments[index].vibdepth = ins->vibdepth>>1;
	instruments[index].vibsweep = ins->vibsweep;
	
	// Wipe samples first
	for (j = 0; j < 16; j++)
	{
		mp_sint32 s = instruments[index].usedSamples[j];
		
		TXMSample* smp = &module->smp[s];

		if (smp->sample)
		{
			module->freeSampleMem((mp_ubyte*)smp->sample);
			smp->sample = NULL;
		}

		smp->vol = 255;
		smp->pan = 0x80;
		smp->finetune = 0;
		smp->relnote = 0;
		smp->looplen = smp->loopstart = smp->samplen = 0;
		smp->flags = smp->type = 0;
		memset(smp->name, 0, sizeof(smp->name));
	}
	
	
	for (j = 0; j < ins->numsamples; j++)
	{
		mp_sint32 s = instruments[index].usedSamples[j];
		
		// 16 bit samples
		if (ins->samples[j].samplen && ins->samples[j].sample)
		{
			if (ins->samples[j].type & 16)
			{
				module->smp[s].sample = (mp_sbyte*)module->allocSampleMem(ins->samples[j].samplen*2);
				if (module->smp[s].sample == NULL)
					return false;
				
				TXMSample::copyPaddedMem(module->smp[s].sample, ins->samples[j].sample, ins->samples[j].samplen*2);
			}
			else
			{
				module->smp[s].sample = (mp_sbyte*)module->allocSampleMem(ins->samples[j].samplen);
				if (module->smp[s].sample == NULL)
					return false;
				
				TXMSample::copyPaddedMem(module->smp[s].sample, ins->samples[j].sample, ins->samples[j].samplen);
			}
		}
		
		const TXMSample* src = &ins->samples[j];
		TXMSample* dst = &module->smp[s];
		
		// default values first	
		dst->flags = 3;
		dst->venvnum = instruments[index].volumeEnvelope+1;
		dst->penvnum = instruments[index].panningEnvelope+1;
		dst->fenvnum = dst->vibenvnum = 0;
		
		// copy from original instrument
		dst->samplen = src->samplen;
		dst->loopstart = src->loopstart;
		dst->looplen = src->looplen;
		dst->vol = src->vol;
		dst->finetune = src->finetune;
		dst->type = src->type;
		dst->pan = src->pan;
		dst->relnote = src->relnote;
		
		dst->vibtype = src->vibtype;
		dst->vibsweep = src->vibsweep;
		dst->vibdepth = src->vibdepth;
		dst->vibrate = src->vibrate;
		dst->volfade = src->volfade;
		
		memcpy(dst->name, src->name, sizeof(dst->name));
	}
	
	changed = true;
	
	return true;
}

XIInstrument* ModuleEditor::extractXIInstrument(mp_sint32 index)
{
	ASSERT(index < module->header.insnum);

	XIInstrument* ins = new XIInstrument();

	if (ins == NULL)
		return NULL;
		
	mp_sint32 j;
	
	ASSERT(instruments[index].numUsedSamples == 16);
	
	memcpy(ins->nbu, instruments[index].nbu, MAX_NOTE);
	
	memcpy(ins->name, instruments[index].instrument->name, MAX_INSTEXT);
	
	ins->numsamples = 16;
	
	memcpy(&ins->venv, &module->venvs[instruments[index].volumeEnvelope], sizeof(ins->venv));
	memcpy(&ins->penv, &module->penvs[instruments[index].panningEnvelope], sizeof(ins->penv));
	
	ins->volfade = instruments[index].volfade<<1;
	ins->vibtype = instruments[index].vibtype;
	ins->vibrate = instruments[index].vibrate;
	ins->vibdepth = instruments[index].vibdepth<<1;
	ins->vibsweep = instruments[index].vibsweep;
	
	for (j = 0; j < ins->numsamples; j++)
	{
		mp_sint32 s = instruments[index].usedSamples[j];
		
		TXMSample* dst = &ins->samples[j];
		TXMSample* src = &module->smp[s];
		
		// default values first	
		dst->flags = 3;
		dst->venvnum = instruments[index].volumeEnvelope+1;
		dst->penvnum = instruments[index].panningEnvelope+1;
		dst->fenvnum = dst->vibenvnum = 0;
		
		// copy from original instrument
		dst->samplen = src->samplen;
		dst->loopstart = src->loopstart;
		dst->looplen = src->looplen;
		dst->vol = src->vol;
		dst->finetune = src->finetune;
		dst->type = src->type;
		dst->pan = src->pan;
		dst->relnote = src->relnote;
		dst->sample = src->sample;
		
		dst->vibtype = src->vibtype;
		dst->vibsweep = src->vibsweep;
		dst->vibdepth = src->vibdepth;
		dst->vibrate = src->vibrate;
		dst->volfade = src->volfade;
		
		memcpy(dst->name, src->name, sizeof(dst->name));
	}	

	return ins;
}

bool ModuleEditor::loadInstrument(const SYSCHAR* fileName, mp_sint32 index)
{
	ASSERT(index < module->header.insnum);

	XIInstrument* ins = new XIInstrument();
	
	bool res = ins->load(fileName) == 0; 
	
	if (res)
	{
		res = insertXIInstrument(index, ins);
	
		finishSamples();
		
		validateInstruments();
	}
	
	delete ins;

	return res;
}

bool ModuleEditor::saveInstrument(const SYSCHAR* fileName, mp_sint32 index)
{
	ASSERT(index < module->header.insnum);

	XIInstrument* ins = extractXIInstrument(index);
	
	bool res = true;
	
	if (ins)
	{		
		ins->save(fileName);
	}
	
	return res;
}

bool ModuleEditor::zapInstrument(mp_sint32 index)
{
	ASSERT(index < module->header.insnum);

	enterCriticalSection();

	XIInstrument ins;
	
	bool res = insertXIInstrument(index, &ins);
	
	finishSamples();
		
	validateInstruments();

	leaveCriticalSection();

	changed = true;

	return res;
}

const PPSystemString& ModuleEditor::getInstrumentFileName(mp_sint32 index)
{
	SYSCHAR buffer[MP_MAXTEXT+1];
	memset(buffer, 0, sizeof(buffer));
	
	TEditorInstrument* ins = getInstrumentInfo(index);
	
	mp_sint32 len = convertStr(buffer, ins->instrument->name);
	
	if (!len)
		instrumentFileName = "Untitled";
	else
	{
		instrumentFileName = buffer;
		instrumentFileName = instrumentFileName.stripExtension();
	}
	
	return instrumentFileName;
}

bool ModuleEditor::copyInstrument(ModuleEditor& dstModule, mp_sint32 dstIndex, 
								  ModuleEditor& srcModule, mp_sint32 srcIndex)
{
	ASSERT(srcIndex < srcModule.module->header.insnum);
	ASSERT(dstIndex < dstModule.module->header.insnum);

	XIInstrument* srcIns = srcModule.extractXIInstrument(srcIndex);

	if (!srcIns)
		return false;
	
	XIInstrument* dstIns = new XIInstrument(*srcIns);

	if (!dstIns)
	{
		delete srcIns;		
		return false;
	}

	dstModule.enterCriticalSection();
		
	bool res = dstModule.insertXIInstrument(dstIndex, dstIns);
	
	if (res)
	{
		dstModule.finishSamples();
		dstModule.validateInstruments();
	}	
	
	delete dstIns;
	delete srcIns;

	dstModule.leaveCriticalSection();
	
	return res;
}

bool ModuleEditor::swapInstruments(ModuleEditor& dstModule, mp_sint32 dstIndex, 
								   ModuleEditor& srcModule, mp_sint32 srcIndex)
{
	ASSERT(srcIndex < srcModule.module->header.insnum);
	ASSERT(dstIndex < dstModule.module->header.insnum);

	XIInstrument* srcIns = srcModule.extractXIInstrument(srcIndex);

	if (!srcIns)
		return false;
	
	XIInstrument* dstIns = dstModule.extractXIInstrument(dstIndex);
	
	if (!dstIns)
	{
		delete srcIns;
		return false;
	}
	
	XIInstrument* swapSrc = new XIInstrument(*srcIns);
	
	if (!swapSrc)
	{
		delete dstIns;
		delete srcIns;
		return false;
	}

	XIInstrument* swapDst = new XIInstrument(*dstIns);
	
	if (!swapDst)
	{
		delete swapSrc;
		delete dstIns;
		delete srcIns;
		return false;
	}

	delete dstIns;
	delete srcIns;
	
	if (&dstModule == &srcModule)
	{
		dstModule.enterCriticalSection();
	}
	else
	{
		srcModule.enterCriticalSection();
		dstModule.enterCriticalSection();
	}
	
	bool res = dstModule.insertXIInstrument(dstIndex, swapSrc);
	res = srcModule.insertXIInstrument(srcIndex, swapDst) && res;
	
	if (res)
	{
		srcModule.finishSamples();
		srcModule.validateInstruments();

		dstModule.finishSamples();
		dstModule.validateInstruments();
	}	

	delete swapDst;
	delete swapSrc;
	
	if (&dstModule == &srcModule)
	{
		dstModule.leaveCriticalSection();
	}
	else
	{
		dstModule.leaveCriticalSection();
		srcModule.leaveCriticalSection();
	}
	
	return res;
}

bool ModuleEditor::copySample(ModuleEditor& dstModule, mp_sint32 dstInsIndex, mp_sint32 dstIndex, 
							  ModuleEditor& srcModule, mp_sint32 srcInsIndex, mp_sint32 srcIndex)
{
	ASSERT(srcInsIndex < srcModule.module->header.insnum);
	ASSERT(dstInsIndex < dstModule.module->header.insnum);

	ASSERT(srcIndex < 16);
	ASSERT(dstIndex < 16);

	bool res = true;
	
	TXMSample* dstSmp = dstModule.getSampleInfo(dstInsIndex, dstIndex);	
	TXMSample* srcSmp = srcModule.getSampleInfo(srcInsIndex, srcIndex);

	dstModule.enterCriticalSection();
	
	if (dstSmp->sample && dstSmp->samplen)
		dstModule.module->freeSampleMem((mp_ubyte*)dstSmp->sample);
	
	// assign attributes
	*dstSmp = *srcSmp;
	
	mp_sint32 sampleSize = (srcSmp->samplen * ((srcSmp->type & 16) ? 16:8)) >> 3;
	if (sampleSize && srcSmp->sample)
	{
		dstSmp->sample = (mp_sbyte*)dstModule.module->allocSampleMem(sampleSize);
		if (dstSmp->sample)
		{
			TXMSample::copyPaddedMem(dstSmp->sample, srcSmp->sample, sampleSize);			
			dstModule.finishSamples();
			dstModule.validateInstruments();
		}
		else res = false;
	}
	
	dstModule.leaveCriticalSection();
	
	return res;
}

bool ModuleEditor::swapSamples(ModuleEditor& dstModule, mp_sint32 dstInsIndex, mp_sint32 dstIndex, 
							   ModuleEditor& srcModule, mp_sint32 srcInsIndex, mp_sint32 srcIndex)
{
	ASSERT(srcInsIndex < srcModule.module->header.insnum);
	ASSERT(dstInsIndex < dstModule.module->header.insnum);

	ASSERT(srcIndex < 16);
	ASSERT(dstIndex < 16);

	bool res = true;
	
	if (&dstModule == &srcModule)
	{
		dstModule.enterCriticalSection();
	}
	else
	{
		srcModule.enterCriticalSection();
		dstModule.enterCriticalSection();
	}
	
	TXMSample* dstSmp = dstModule.getSampleInfo(dstInsIndex, dstIndex);	
	TXMSample* srcSmp = srcModule.getSampleInfo(srcInsIndex, srcIndex);
	
	TXMSample tmpSmp;
	
	srcModule.module->removeSamplePtr((mp_ubyte*)srcSmp->sample);
	dstModule.module->removeSamplePtr((mp_ubyte*)dstSmp->sample);
	
	tmpSmp = *dstSmp;
	*dstSmp = *srcSmp;
	*srcSmp = tmpSmp;
	
	srcModule.module->insertSamplePtr((mp_ubyte*)srcSmp->sample);
	dstModule.module->insertSamplePtr((mp_ubyte*)dstSmp->sample);	
	
	if (&dstModule == &srcModule)
	{
		dstModule.leaveCriticalSection();
	}
	else
	{
		dstModule.leaveCriticalSection();
		srcModule.leaveCriticalSection();
	}
	
	return res;
}

void ModuleEditor::setNumChannels(mp_uint32 channels)
{
	if (module->header.channum != channels)
		changed = true;
	module->header.channum = channels;
}

void ModuleEditor::setTitle(const char* name, mp_uint32 length)
{
	insertText(module->header.name, name, length);
	changed = true;
}

void ModuleEditor::getTitle(char* name, mp_uint32 length) const
{
	if (length > MAX_TITLETEXT)
		length = MAX_TITLETEXT;
	XModule::convertStr(name, (char*)module->header.name, length, false);	
}

void ModuleEditor::setNumOrders(mp_sint32 numOrders)
{
	if (numOrders > 255)
		numOrders = 255;
	if (numOrders < 1)
		numOrders = 1;
	
	if (module->header.ordnum != numOrders)
		changed = true;
	
	module->header.ordnum = numOrders;
}

void ModuleEditor::setFrequency(Frequencies frequency)
{
	// changes are made using the settings panel
	// do not flag changes here
	//mp_sint32 old = module->header.freqtab;
	module->header.freqtab &= ~1;
	module->header.freqtab |= frequency;
	//if (old != module->header.freqtab)
	//	changed = true;
}

void ModuleEditor::setSampleName(mp_sint32 insIndex, mp_sint32 smpIndex, const char* name, mp_uint32 length)
{
	insertText((char*)getSampleInfo(insIndex, smpIndex)->name, name, length);
	changed = true;
}

void ModuleEditor::getSampleName(mp_sint32 insIndex, mp_sint32 smpIndex, char* name, mp_uint32 length) const
{
	if (length > MAX_SMPTEXT)
		length = MAX_SMPTEXT;
	XModule::convertStr(name, (char*)getSampleInfoInternal(insIndex, smpIndex)->name, length, false);
}

void ModuleEditor::setCurrentSampleName(const char* name, mp_uint32 length)
{
	if (sampleEditor->getSample() == NULL)
		return;
		
	insertText((char*)sampleEditor->getSample()->name, name, length);
	changed = true;
}

TXMSample* ModuleEditor::getFirstSampleInfo()
{
	enumerationIndex = 0;
	if (enumerationIndex < module->header.smpnum)
		return &module->smp[enumerationIndex];
	else
		return NULL;
}

TXMSample* ModuleEditor::getNextSampleInfo()
{
	enumerationIndex++;
	if (enumerationIndex >= module->header.smpnum)
	{
		enumerationIndex = -1;
		return NULL;
	}
	
	return &module->smp[enumerationIndex];
}

void ModuleEditor::setInstrumentName(mp_sint32 insIndex, const char* name, mp_uint32 length)
{
	insertText(module->instr[insIndex].name, name, length);
	changed = true;
}

void ModuleEditor::getInstrumentName(mp_sint32 insIndex, char* name, mp_uint32 length) const
{
	if (length > MAX_INSTEXT)
		length = MAX_INSTEXT;
	
	XModule::convertStr(name, module->instr[insIndex].name, length, false);
}

TEnvelope* ModuleEditor::getEnvelope(mp_sint32 insIndex, mp_sint32 smpIndex, mp_sint32 type)
{
	if (insIndex < 0 || insIndex >= module->header.insnum)
		return NULL;

	if (smpIndex < 0 || smpIndex >= instruments[insIndex].numUsedSamples)
		return NULL;
	
	TXMSample* smp = getSampleInfo(insIndex,smpIndex);
	
	// no envelopes available, assign some
	if (smp->venvnum == 0)
		smp->venvnum = instruments[insIndex].volumeEnvelope+1;
	// no envelope available, assign one 
	if (smp->penvnum == 0)
		smp->penvnum = instruments[insIndex].panningEnvelope+1;
	
	if (type == 0 && smp->venvnum)
	{
		return &module->venvs[smp->venvnum-1];
	}
	else if (type == 1 && smp->penvnum)
	{
		return &module->penvs[smp->penvnum-1];
	}

	return NULL;
}

const mp_ubyte* ModuleEditor::getSampleTable(mp_sint32 insIndex)
{
	if (insIndex < 0 || insIndex >= module->header.insnum)
		return NULL;

	return instruments[insIndex].nbu;
}

void ModuleEditor::updateSampleTable(mp_sint32 index, const mp_ubyte* nbu)
{
	if (nbu == NULL)
		return;

	if (index < 0 || index >= module->header.insnum)
		return;

	// cope with FT2 noterange (= 96)
	memcpy(instruments[index].nbu, nbu, MAX_NOTE);

	// update module data
	for (mp_sint32 i = 0; i < MAX_NOTE; i++)
	{
		if (nbu[i] != 255)
			module->instr[index].snum[i] = instruments[index].usedSamples[nbu[i]];
		else
			module->instr[index].snum[i] = 255;

	}

	changed = true;
}

void ModuleEditor::updateInstrumentData(mp_sint32 index)
{
	
	if (index < 0)
		return;

	if (index >= module->header.insnum)
		return;

	for (mp_sint32 i = 0; i < instruments[index].numUsedSamples; i++)
	{
		mp_sint32 s = instruments[index].usedSamples[i];

		TXMSample* smp = &module->smp[s];

		smp->volfade = instruments[index].volfade << 1;
		if (smp->volfade == 65534) smp->volfade++;
		smp->vibtype = instruments[index].vibtype;
		smp->vibrate = instruments[index].vibrate;
		smp->vibdepth = instruments[index].vibdepth << 1;
		smp->vibsweep = instruments[index].vibsweep;
	}

	changed = true;

}

pp_int32 ModuleEditor::insRemapSong(pp_int32 oldIns, pp_int32 newIns)
{
	mp_sint32 resCnt = 0;

	PatternEditorTools patternEditorTools;

	for (mp_sint32 k = 0; k < module->header.patnum; k++)
	{
		patternEditorTools.attachPattern(&module->phead[k]);
		resCnt+=patternEditorTools.insRemap(oldIns, newIns);				
	}

	if (resCnt)
		changed = true;
		
	return resCnt;
}

pp_int32 ModuleEditor::noteTransposeSong(const PatternEditorTools::TransposeParameters& transposeParameters, bool evaluate/* = false*/)
{
	mp_sint32 resCnt = 0;
	pp_int32 fuckupCnt = 0;

	PatternEditorTools patternEditorTools;

	for (mp_sint32 k = 0; k < module->header.patnum; k++)
	{
		patternEditorTools.attachPattern(&module->phead[k]);
		
		if (evaluate)
			fuckupCnt+=patternEditorTools.noteTranspose(transposeParameters, evaluate);				
		else
			resCnt+=patternEditorTools.noteTranspose(transposeParameters, evaluate);				
	}

	if (!evaluate)
	{
		if (resCnt)
			changed = true;
		
		return resCnt;
	}
	else
		return fuckupCnt;
}

pp_int32 ModuleEditor::panConvertSong(PanConversionTypes type)
{
	mp_sint32 resCnt = 0;

	for (mp_sint32 k = 0; k < module->header.patnum; k++)
	{
		TXMPattern* pattern = &module->phead[k];

		if (pattern->patternData == NULL)
			continue;
			
		mp_sint32 slotSize = pattern->effnum * 2 + 2;
		mp_sint32 rowSizeSrc = slotSize*pattern->channum;
		
		for (pp_int32 i = 0; i < pattern->rows; i++)
			for (pp_int32 j = 0; j < pattern->channum; j++)
			{
				mp_ubyte* src = pattern->patternData + i*rowSizeSrc+j*slotSize;
				
				switch (type)
				{
					case PanConversionTypeConvert_E8x:
						if (src[4] == 0x38)
						{
							src[4] = 0x08;
							src[5] = (mp_ubyte)XModule::pan15to255(src[5]);
							resCnt++;
						}
						break;
					case PanConversionTypeConvert_80x:
						if (src[4] == 0x08)
						{
							src[5] = (mp_ubyte)XModule::pan15to255(src[5]);
							resCnt++;
						}
						break;
					case PanConversionTypeRemove_E8x:
						if (src[4] == 0x38)
						{
							src[4] = src[5] = 0x0;
							resCnt++;
						}
						break;
					case PanConversionTypeRemove_8xx:
						if (src[4] == 0x08)
						{
							src[4] = src[5] = 0x0;
							resCnt++;
						}
						break;
				}
			}
				
	}
	
	if (resCnt)
		changed = true;

	return resCnt;
}

pp_int32 ModuleEditor::removeUnusedPatterns(bool evaluate)
{
	mp_sint32 result = module->removeUnusedPatterns(evaluate);

	if (!evaluate && result)
	{
		changed = true;
		if (currentPatternIndex > module->header.patnum - 1)
			currentPatternIndex = module->header.patnum - 1;
	}
	return result;
}

pp_int32 ModuleEditor::removeUnusedInstruments(bool evaluate, bool remap)
{
	mp_sint32 i,j,k;

	mp_ubyte* bitMap = new mp_ubyte[MAX_INSTRUMENTS];

	memset(bitMap, 0, sizeof(mp_ubyte)*MAX_INSTRUMENTS);

	for (k = 0; k < module->header.patnum; k++)
	{
		TXMPattern* pattern = &module->phead[k];

		if (pattern->patternData == NULL)
			continue;
			
		mp_sint32 slotSize = pattern->effnum * 2 + 2;
		mp_sint32 rowSizeSrc = slotSize*pattern->channum;
		
		for (i = 0; i < pattern->rows; i++)
			for (j = 0; j < pattern->channum; j++)
			{
				mp_ubyte* src = pattern->patternData + i*rowSizeSrc+j*slotSize;

				if (src[1])
				{
					bitMap[src[1]-1] = TRUE;
				}
			}
				
	}
	
	mp_sint32 result = 0;
	for (i = 0; i < module->header.insnum; i++)
	{
		if (!bitMap[i])
		{
			result++;
			if (!evaluate)
				zapInstrument(i);
		}
	}

	if (!evaluate)
	{
		mp_sint32* insRelocTable = new mp_sint32[MAX_INSTRUMENTS];

		for (i = 0, j = 0; i < module->header.insnum; i++)
		{
			if (bitMap[i])
			{
				insRelocTable[i] = j++;
			}
		}

		for (i = 0, k = 0; i < module->header.insnum; i++)
		{
			if (bitMap[i])
			{
				j = insRelocTable[i];

				if (j < i)
				{
					XIInstrument* ins = extractXIInstrument(i);

					insertXIInstrument(j, ins);			

					delete ins;

					insRemapSong(i+1, j+1);

					zapInstrument(i);

				}
				k++;
			}
		}
		delete[] insRelocTable;

		// zero number of instruments is not allowed
		if (k == 0)
		{
			result--;
			k++;
		}

		module->header.insnum = k;
		module->header.smpnum = k*16;
	}
	else
	{
		if (module->header.insnum - result <= 0)
			result--;
	}

	if (!evaluate && result)
		changed = true;

	delete[] bitMap;

	return result;
}

pp_int32 ModuleEditor::removeUnusedSamples(bool evaluate)
{
	mp_sint32 i,j,k;

	mp_ubyte* bitMap = new mp_ubyte[MP_MAXSAMPLES];

	memset(bitMap, 0, sizeof(mp_ubyte)*MP_MAXSAMPLES);

	mp_ubyte* lastIns = new mp_ubyte[module->header.channum];
	memset(lastIns, 0, module->header.channum);

	for (mp_sint32 l = 0; l < module->header.ordnum; l++)
	{
		k = module->header.ord[l];

		TXMPattern* pattern = &module->phead[k];

		if (pattern->patternData == NULL)
			continue;
			
		mp_sint32 slotSize = pattern->effnum * 2 + 2;
		mp_sint32 rowSizeSrc = slotSize*pattern->channum;
		
		for (i = 0; i < pattern->rows; i++)
			for (j = 0; j < pattern->channum; j++)
			{
				mp_ubyte* src = pattern->patternData + i*rowSizeSrc+j*slotSize;

				if (src[1])
				{
					lastIns[j] = src[1];
#if 0
					// just assume, that if an instrument is used
					// in the pattern, that the first sample of this instrument
					// is kept even if there isn't any note played with that instrument
					mp_sint32 insIndex = lastIns[j] - 1;					
					mp_sint32 smpIndex = module->instr[insIndex].snum[0];
					if (smpIndex >= 0 && smpIndex < MP_MAXSAMPLES)
						bitMap[smpIndex] = TRUE;
#endif
				}

				if (src[0] && src[0] < 120)
				{
					if (lastIns[j])
					{
						mp_sint32 insIndex = lastIns[j] - 1;
					
						mp_sint32 smpIndex = module->instr[insIndex].snum[src[0]-1];
					
						if (smpIndex >= 0 && smpIndex < MP_MAXSAMPLES)
							bitMap[smpIndex] = TRUE;
					}
				}
			}
				
	}

	delete[] lastIns;
	
	mp_sint32 result = 0;
	for (i = 0; i < module->header.smpnum; i++)
	{
		if (!bitMap[i] && module->smp[i].sample)
		{
			result++;
			if (!evaluate)
			{
				clearSample(i);
				// wipe out sample slot
				memset(module->smp + i, 0, sizeof(TXMSample));
			}
		}
	}
	
	// relocate samples 
	if (!evaluate)
	{
		for (i = 0; i < module->header.insnum; i++)
		{
			mp_sint32 smpRelocTable[16];
			for (j = 0; j < 16; j++)
				smpRelocTable[j] = -1;
			
			mp_sint32 s = 0;
			TXMSample* src = module->smp + 16*i;
			TXMSample* dst = src;
			for (j = 0; j < 16; j++, src++)
			{
				k = i*16+j;
				if (bitMap[k])
				{
					if (src != dst)
					{
						*dst = *src;
						// wipe out source sample
						memset(src, 0, sizeof(TXMSample));
					}
					smpRelocTable[j] = s++;
					dst++;
				}
			}
			
			// adjust the FT2 style sample->note mapping table
			TEditorInstrument* ins = instruments + i;
			
			for (j = 0; j < MAX_NOTE; j++)
				if (ins->nbu[j] < 16 && smpRelocTable[ins->nbu[j]] != -1)
					ins->nbu[j] = smpRelocTable[ins->nbu[j]];
				else
					ins->nbu[j] = 0;
		
			// convert back to milkytracker module style mapping
			for (j = 0; j < MAX_NOTE; j++)
				module->instr[i].snum[j] = i * 16 + ins->nbu[j];
			
		}
	}
	
	if (!evaluate && result)
		changed = true;

	delete[] bitMap;

	return result;
}

pp_int32 ModuleEditor::relocateCommands(const PatternEditorTools::RelocateParameters& relocateParameters, bool evaluate)
{
	mp_sint32 result = 0;

	PatternEditorTools patternEditorTools;

	for (mp_sint32 k = 0; k < module->header.patnum; k++)
	{
		patternEditorTools.attachPattern(&module->phead[k]);
		result+=patternEditorTools.relocateCommands(relocateParameters, evaluate);				
	}
	
	if (!evaluate && result)
		changed = true;

	return result;
}

pp_int32 ModuleEditor::zeroOperands(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	mp_sint32 result = 0;

	PatternEditorTools patternEditorTools;

	for (mp_sint32 k = 0; k < module->header.patnum; k++)
	{
		patternEditorTools.attachPattern(&module->phead[k]);
		result+=patternEditorTools.zeroOperands(optimizeParameters, evaluate);				
	}

	if (!evaluate && result)
		changed = true;

	return result;
}

pp_int32 ModuleEditor::fillOperands(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	mp_sint32 result = 0;

	PatternEditorTools patternEditorTools;

	for (mp_sint32 k = 0; k < module->header.patnum; k++)
	{
		patternEditorTools.attachPattern(&module->phead[k]);
		result+=patternEditorTools.fillOperands(optimizeParameters, evaluate);				
	}

	if (!evaluate && result)
		changed = true;

	return result;
}

void ModuleEditor::optimizeSamples(bool convertTo8Bit, bool minimize, 
								   mp_sint32& numConvertedSamples, mp_sint32& numMinimizedSamples,
								   bool evaluate)
{
	TXMSample* oldSmp = sampleEditor->getSample();

	TXMSample* smp = getFirstSampleInfo();

	sampleEditor->activateUndoStack(false);

	numConvertedSamples = numMinimizedSamples = 0;

	while (smp)
	{
		sampleEditor->attachSample(smp, module);

		// check for 16 bit sample
		if ((smp->type & 16) && smp->sample && smp->samplen && convertTo8Bit)
		{
			if (!evaluate)
				sampleEditor->convertSampleResolution(true);
			numConvertedSamples++;
		}
		if (smp->sample && smp->samplen && smp->isMinimizable() && minimize)
		{
			if (!evaluate)
				sampleEditor->minimizeSample();
			numMinimizedSamples++;
		}

		smp = getNextSampleInfo();
	}

	sampleEditor->activateUndoStack(true);

	sampleEditor->attachSample(oldSmp, module);

	if (!evaluate && (numMinimizedSamples || numConvertedSamples))
		changed = true;
}

void ModuleEditor::insertText(char* dst, const char* src, mp_sint32 max)
{
	char name[MP_MAXTEXT+1];
	
	ASSERT((signed)sizeof(name) >= max && strlen(src) <= sizeof(name));
				
	memset(name, 0, sizeof(name));
	memcpy(name, src, (signed)strlen(src) <= max ? strlen(src) : max);
	memcpy(dst, name, max);
}

PPSystemString ModuleEditor::getTempFilename()
{
	return PPSystemString(System::getTempFileName());
}

