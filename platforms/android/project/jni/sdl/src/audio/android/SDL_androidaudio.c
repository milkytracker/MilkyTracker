/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org

    This file written by Ryan C. Gordon (icculus@icculus.org)
*/
#include "SDL_config.h"

/* Output audio to nowhere... */

#include "SDL_rwops.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"
#include "SDL_androidaudio.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include <jni.h>
#include <android/log.h>

#define ANDROIDAUD_DRIVER_NAME         "android"

/* Audio driver functions */
static int ANDROIDAUD_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void ANDROIDAUD_WaitAudio(_THIS);
static void ANDROIDAUD_PlayAudio(_THIS);
static Uint8 *ANDROIDAUD_GetAudioBuf(_THIS);
static void ANDROIDAUD_CloseAudio(_THIS);

/* Audio driver bootstrap functions */
static int ANDROIDAUD_Available(void)
{
	return(1);
}

static void ANDROIDAUD_DeleteDevice(SDL_AudioDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_AudioDevice *ANDROIDAUD_CreateDevice(int devindex)
{
	SDL_AudioDevice *this;

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
	if ( this ) {
		SDL_memset(this, 0, (sizeof *this));
		this->hidden = (struct SDL_PrivateAudioData *)
				SDL_malloc((sizeof *this->hidden));
	}
	if ( (this == NULL) || (this->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( this ) {
			SDL_free(this);
		}
		return(0);
	}
	SDL_memset(this->hidden, 0, (sizeof *this->hidden));

	/* Set the function pointers */
	this->OpenAudio = ANDROIDAUD_OpenAudio;
	this->WaitAudio = ANDROIDAUD_WaitAudio;
	this->PlayAudio = ANDROIDAUD_PlayAudio;
	this->GetAudioBuf = ANDROIDAUD_GetAudioBuf;
	this->CloseAudio = ANDROIDAUD_CloseAudio;

	this->free = ANDROIDAUD_DeleteDevice;

	return this;
}

AudioBootStrap ANDROIDAUD_bootstrap = {
	ANDROIDAUD_DRIVER_NAME, "SDL Android audio driver",
	ANDROIDAUD_Available, ANDROIDAUD_CreateDevice
};


static SDL_mutex * audioMutex = NULL;
static SDL_cond * audioCond = NULL;
static SDL_cond * audioCond2 = NULL;
static unsigned char * audioBuffer = NULL;
static size_t audioBufferSize = 0;
static SDL_AudioSpec *audioFormat = NULL;
static int audioInitialized = 0;
static int audioPlayed = 0;
static jbyteArray audioBufferJNI = NULL;
static JNIEnv * jniEnv = NULL;

static Uint8 *ANDROIDAUD_GetAudioBuf(_THIS)
{
	return(this->hidden->mixbuf);
}

static void ANDROIDAUD_CloseAudio(_THIS)
{
	SDL_mutex * audioMutex1;

	/*	
	if ( this->hidden->mixbuf != NULL ) {
		SDL_FreeAudioMem(this->hidden->mixbuf);
		this->hidden->mixbuf = NULL;
	}
	*/
	if( audioMutex != NULL )
	{
		audioMutex1 = audioMutex;
		SDL_mutexP(audioMutex1);
		audioInitialized = 0;
		SDL_CondSignal(audioCond);
		SDL_CondSignal(audioCond2);
		audioMutex = NULL;
		SDL_DestroyCond(audioCond);
		SDL_DestroyCond(audioCond2);
		audioCond = NULL;
		audioCond2 = NULL;
		audioFormat = NULL;
		// TODO: this crashes JNI, so we're just memleaking it
		/*
		(*jniEnv)->ReleaseByteArrayElements(jniEnv, audioBufferJNI, (jbyte *)audioBuffer, 0);
		(*jniEnv)->DeleteGlobalRef(jniEnv, audioBufferJNI);
		*/
		jniEnv = NULL;
		audioBufferJNI = NULL;
		audioBuffer = NULL;
		audioBufferSize = 0;
		SDL_mutexV(audioMutex1);
		SDL_DestroyMutex(audioMutex1);
		
	}
}

static int ANDROIDAUD_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
	if( ! (spec->format == AUDIO_S8 || spec->format == AUDIO_S16) )
		return (-1); // TODO: enable format conversion? Don't know how to do that in SDL
	
	if( audioMutex == NULL )
	{
		audioInitialized = 0;
		audioFormat = spec;
		audioMutex = SDL_CreateMutex();
		audioCond = SDL_CreateCond();
		audioCond2 = SDL_CreateCond();
		audioPlayed == 0;
	}

	SDL_mutexP(audioMutex);
	
	while( !audioInitialized )
	{
		if( SDL_CondWaitTimeout( audioCond, audioMutex, 1000 ) != 0 )
		{
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio() failed! timeout when waiting callback");
			SDL_mutexV(audioMutex);
			ANDROIDAUD_CloseAudio(this);
			return(-1);
		}
	}

	this->hidden->mixbuf = audioBuffer;
	this->hidden->mixlen = audioBufferSize;

	audioFormat = NULL;
	
	SDL_mutexV(audioMutex);

	return(0);
}

/* This function waits until it is possible to write a full sound buffer */
static void ANDROIDAUD_WaitAudio(_THIS)
{
	/* We will block in PlayAudio(), do nothing here */
}

static void ANDROIDAUD_PlayAudio(_THIS)
{
	SDL_mutexP(audioMutex);

	//audioBuffer = this->hidden->mixbuf;
	//audioBufferSize = this->hidden->mixlen;

	audioPlayed = 1;
	
	SDL_CondSignal(audioCond2);
	SDL_CondWaitTimeout( audioCond, audioMutex, 1000 );

	this->hidden->mixbuf = audioBuffer;
	
	SDL_mutexV(audioMutex);
}

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

extern jintArray JAVA_EXPORT_NAME(AudioThread_nativeAudioInit) (JNIEnv * env, jobject jobj)
{
	jintArray ret = NULL;
	int initData[4] = { 0, 0, 0, 0 }; // { rate, channels, encoding, bufsize };
	
	if( audioMutex == NULL )
		return;

	SDL_mutexP(audioMutex);
	
	if( audioInitialized == 0 )
	{
		initData[0] = audioFormat->freq;
		initData[1] = audioFormat->channels;
		int bytesPerSample = (audioFormat->format & 0xFF) / 8;
		initData[2] = ( bytesPerSample == 2 ) ? 1 : 0;
		audioFormat->format = ( bytesPerSample == 2 ) ? AUDIO_S16 : AUDIO_S8;
		initData[3] = audioFormat->size;
		ret=(*env)->NewIntArray(env, 4);
		(*env)->SetIntArrayRegion(env, ret, 0, 4, (jint *)initData);
	}
	
	SDL_mutexV(audioMutex);

	return (ret);
};

extern jobject JAVA_EXPORT_NAME(AudioThread_nativeAudioInit2) (JNIEnv * env, jobject jobj, jbyteArray buf)
{
	jobject ret = NULL;

	if( audioMutex == NULL )
		return;

	SDL_mutexP(audioMutex);
	
	if( audioInitialized == 0 )
	{
		/* Allocate mixing buffer */
		audioBufferJNI = (jbyteArray*)(*env)->NewGlobalRef(env, buf);
		audioBufferSize = (*env)->GetArrayLength(env, audioBufferJNI);
		jboolean isCopy = JNI_TRUE;
		audioBuffer = (unsigned char *) (*env)->GetByteArrayElements(env, audioBufferJNI, &isCopy);
		if( isCopy == JNI_TRUE )
			__android_log_print(ANDROID_LOG_ERROR, "libSDL", "AudioThread_nativeAudioInit2() JNI returns a copy of byte array - no audio will be played");

		jniEnv = env;

		int bytesPerSample = (audioFormat->format & 0xFF) / 8;
		audioFormat->samples = audioBufferSize / bytesPerSample / audioFormat->channels;
		audioFormat->size = audioBufferSize;
		SDL_memset(audioBuffer, audioFormat->silence, audioFormat->size);
		char t[512];
		//sprintf(t, "AudioThread_nativeAudioInit2() got byte array from JNI: size %i samples %i direct memory %i", audioBufferSize, audioFormat->samples, (isCopy == JNI_FALSE) );
		__android_log_print(ANDROID_LOG_INFO, "libSDL", t);
		
		/*
		audioBuffer = (Uint8 *) SDL_AllocAudioMem(audioBufferSize);
		if ( audioBuffer == NULL ) {
			SDL_mutexV(audioMutex);
			return NULL;
		}
		
		ret = (*env)->NewDirectByteBuffer(env, audioBuffer, audioBufferSize);
		*/

		audioInitialized = 1;
		SDL_CondSignal(audioCond);
	}
	
	SDL_mutexV(audioMutex);

	return 0;
}

extern jint JAVA_EXPORT_NAME(AudioThread_nativeAudioBufferLock) ( JNIEnv * env, jobject jobj )
{
	int ret = 0;
	
	if( audioMutex == NULL )
		return(-1);
	
	SDL_mutexP(audioMutex);
	
	if( !audioInitialized )
	{
		SDL_mutexV(audioMutex);
		SDL_CondSignal(audioCond);
		return (-1);
	}
	
	if( audioPlayed == 0 )
		SDL_CondWaitTimeout(audioCond2, audioMutex, 1000);

	if( audioBuffer == NULL ) // Should not happen
		ret = 0;
	else
	{
		(*jniEnv)->ReleaseByteArrayElements(jniEnv, audioBufferJNI, (jbyte *)audioBuffer, 0);
		audioBuffer == NULL;
		ret = audioBufferSize;
	}

	return ret;
};

extern jint JAVA_EXPORT_NAME(AudioThread_nativeAudioBufferUnlock) ( JNIEnv * env, jobject jobj )
{
	if( audioMutex == NULL )
		return(-1);

	jboolean isCopy = JNI_TRUE;
	audioBuffer = (unsigned char *) (*env)->GetByteArrayElements(env, audioBufferJNI, &isCopy);
	if( isCopy == JNI_TRUE )
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "AudioThread_nativeAudioBufferUnlock() JNI returns a copy of byte array - that's slow");

	audioPlayed = 0;
	
	SDL_mutexV(audioMutex);
	SDL_CondSignal(audioCond);

	return 0;
}

