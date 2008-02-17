/*
 *  milkyplay/AudioDriverManager.cpp
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

#include "AudioDriverManager.h"
#include "AudioDriverBase.h"
#include "MilkyPlayCommon.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define ALLOC_DRIVERLIST(NUMDRIVERS) \
	enumerationIndex = -1; \
	numDrivers = (NUMDRIVERS); \
	driverList = new AudioDriverBase*[numDrivers]; \
	memset(driverList, 0, numDrivers*sizeof(AudioDriverBase*));

#define CLEANUP_DRIVERLIST \
	for (mp_sint32 i = 0; i < numDrivers; i++) \
		delete driverList[i]; \
	delete[] driverList;

#if defined(DRIVER_WIN32)
//////////////////////////////////////////////////////////////////
//					 Windows implementation
//////////////////////////////////////////////////////////////////
#include "AudioDriver_MMSYSTEM.h"

#if !defined(_WIN32_WCE) && !defined(__SKIPRTAUDIO__)
#include "AudioDriver_RTAUDIO.h"
#endif
#if !defined(_WIN32_WCE) && defined(__WASAPI__)
#include "AudioDriver_PORTAUDIO.h"
#endif

AudioDriverManager::AudioDriverManager() :
	defaultDriverIndex(0)
{
	mp_sint32 driverListSize = 2;
#if !defined(_WIN32_WCE) && !defined(__SKIPRTAUDIO__)
	driverListSize+=3;
#endif
#if !defined(_WIN32_WCE) && defined(__WASAPI__)
	driverListSize++;
#endif

	mp_sint32 i = 0;
	ALLOC_DRIVERLIST(driverListSize);
	driverList[i++] = new AudioDriver_MMSYSTEM();
	driverList[i++] = new AudioDriver_MMSYSTEM(true);
#if !defined(_WIN32_WCE) && !defined(__SKIPRTAUDIO__)
	driverList[i++] = new AudioDriver_RTAUDIO();
	driverList[i++] = new AudioDriver_RTAUDIO(RtAudio::WINDOWS_ASIO);
	driverList[i++] = new AudioDriver_RTAUDIO(RtAudio::WINDOWS_DS);

	// On windows vista we set the DS driver to the default
#ifndef _WIN32_WCE
	OSVERSIONINFOEX osVersion;
	ZeroMemory(&osVersion, sizeof(osVersion));
	osVersion.dwOSVersionInfoSize = sizeof(osVersion);
	if (GetVersionEx((LPOSVERSIONINFO)&osVersion))
	{
		if (osVersion.dwMajorVersion > 5)
			defaultDriverIndex = i-1;
	}
#endif	
	
#endif
#if !defined(_WIN32_WCE) && defined(__WASAPI__)
	driverList[i++] = new AudioDriver_PORTAUDIO();
#endif
}

#elif defined(DRIVER_OSX)
//////////////////////////////////////////////////////////////////
//					 Mac OS X implementation
//////////////////////////////////////////////////////////////////
#include "AudioDriver_COREAUDIO.h"

#ifdef __OSX_PANTHER__
#undef __MACOSX_CORE__
#endif

#ifdef __MACOSX_CORE__
#include "AudioDriver_RTAUDIO.h"
#endif

AudioDriverManager::AudioDriverManager() :
	defaultDriverIndex(0)
{
#ifdef __MACOSX_CORE__
	ALLOC_DRIVERLIST(3);

	driverList[0] = new AudioDriver_COREAUDIO();
	driverList[1] = new AudioDriver_RTAUDIO();
	driverList[2] = new AudioDriver_RTAUDIO(RtAudio::MACOSX_CORE);
#else
	ALLOC_DRIVERLIST(1);
	driverList[0] = new AudioDriver_COREAUDIO();
#endif
}

#elif defined(DRIVER_UNIX)
//////////////////////////////////////////////////////////////////
//					  UNIX implementation
//////////////////////////////////////////////////////////////////
#include "AudioDriver_SDL.h"
// #include "AudioDriver_RTAUDIO.h"
#ifdef HAVE_LIBASOUND
#include "drivers/alsa/AudioDriver_ALSA.h"
#endif
#ifdef HAVE_JACK_JACK_H
#include "AudioDriver_JACK.h"
#endif

AudioDriverManager::AudioDriverManager() :
	defaultDriverIndex(0)
{
	int count = 1;
#ifdef HAVE_LIBASOUND
	count++;
#endif
#ifdef HAVE_JACK_JACK_H
	count++;
#endif
	ALLOC_DRIVERLIST(count);
	count = 0;
	driverList[count++] = new AudioDriver_SDL();
#ifdef HAVE_LIBASOUND
	driverList[count++] = new AudioDriver_ALSA();
#endif
#if HAVE_JACK_JACK_H
	driverList[count++] = new AudioDriver_JACK();
#endif
}

#elif defined(DRIVER_SDL)
//////////////////////////////////////////////////////////////////
//					 	SDL implementation
//////////////////////////////////////////////////////////////////
#include "AudioDriver_SDL.h"

AudioDriverManager::AudioDriverManager() :
	defaultDriverIndex(0)
{
	ALLOC_DRIVERLIST(1);
	driverList[0] = new AudioDriver_SDL();
}

#elif defined(DRIVER_PSP)
//////////////////////////////////////////////////////////////////
//					 	PSP implementation
//////////////////////////////////////////////////////////////////
#include "AudioDriver_PSP.h"

AudioDriverManager::AudioDriverManager() :
	defaultDriverIndex(0)
{
	ALLOC_DRIVERLIST(1);
	driverList[0] = new AudioDriver_PSP();
}

#endif

AudioDriverBase* AudioDriverManager::getPreferredAudioDriver()
{
	return driverList[defaultDriverIndex];
}

AudioDriverManager::~AudioDriverManager()
{
	CLEANUP_DRIVERLIST;
}

AudioDriverBase* AudioDriverManager::getAudioDriverByName(const char* name)
{
	if (name == NULL)
		return NULL;

	for (mp_sint32 i = 0; i < numDrivers; i++)
	{
		if (strcmp(driverList[i]->getDriverID(), name) == 0)
			return driverList[i];
	}

	return NULL;
}

const char* AudioDriverManager::getFirstDriverName() const
{
	enumerationIndex = 0;
	if (enumerationIndex < numDrivers)
		return driverList[enumerationIndex]->getDriverID();
	else
		return NULL;
}

const char* AudioDriverManager::getNextDriverName() const
{
	enumerationIndex++;
	if (enumerationIndex >= numDrivers)
	{
		enumerationIndex = -1;
		return NULL;
	}

	return driverList[enumerationIndex]->getDriverID();
}

mp_sint32 AudioDriverManager::getPreferredAudioDriverBufferSize() const
{
	return driverList[defaultDriverIndex]->getPreferredBufferSize();
}
