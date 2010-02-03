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

    Android port:
     - Sergiy Pylypenko <pelya> (generic code)
     - Guillaume Legris (MilkyTracker adaptations and improvements)

*/
#include "SDL_config.h"

/* Dummy SDL video driver implementation; this is just enough to make an
 *  SDL-based application THINK it's got a working video driver, for
 *  applications that call SDL_Init(SDL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting SDL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that SDL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 *
 * Initial work by Ryan C. Gordon (icculus@icculus.org). A good portion
 *  of this was cut-and-pasted from Stephane Peter's work in the AAlib
 *  SDL video driver.  Renamed to "ANDROID" by Sam Lantinga.
 */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_androidvideo.h"

#include <jni.h>
#include <android/log.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>


#define ANDROIDVID_DRIVER_NAME "android"

/* Initialization/Query functions */
static int ANDROID_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **ANDROID_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *ANDROID_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int ANDROID_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void ANDROID_VideoQuit(_THIS);

/* Hardware surface functions */
static int ANDROID_AllocHWSurface(_THIS, SDL_Surface *surface);
static int ANDROID_LockHWSurface(_THIS, SDL_Surface *surface);
static void ANDROID_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void ANDROID_FreeHWSurface(_THIS, SDL_Surface *surface);
static int ANDROID_FlipHWSurface(_THIS, SDL_Surface *surface);

/* etc. */
static void ANDROID_UpdateRects(_THIS, int numrects, SDL_Rect *rects);


#define SDL_NUMMODES 3

/* Private display data */

struct SDL_PrivateVideoData {
	SDL_Rect *SDL_modelist[SDL_NUMMODES + 1];
};

#define SDL_modelist		(this->hidden->SDL_modelist)

static int sWindowWidth = 320;
static int sWindowHeight = 240;
static SDL_mutex * WaitForNativeRender = NULL;
static SDL_cond * WaitForNativeRender1 = NULL;
static enum {
	Render_State_Started, Render_State_Processing, Render_State_Finished
} WaitForNativeRenderState = Render_State_Finished;
// Pointer to in-memory video surface
static int memX = 0;
static int memY = 0;
static void * memBuffer1 = NULL;
static void * memBuffer2 = NULL;
static void * memBuffer = NULL;
static SDL_Thread * mainThread = NULL;
static enum {
	GL_State_Init, GL_State_Ready, GL_State_Uninit, GL_State_Uninit2
} openglInitialized = GL_State_Uninit2;
static GLuint texture = 0;

static bool autoResizingActivated = true;
static bool mustResize = false;

static SDLKey keymap[KEYCODE_LAST + 1];

static int processAndroidTrackballKeyDelays(int key, int action);

/* ANDROID driver bootstrap functions */

static int ANDROID_Available(void) {
	/*
	 const char *envr = SDL_getenv("SDL_VIDEODRIVER");
	 if ((envr) && (SDL_strcmp(envr, ANDROIDVID_DRIVER_NAME) == 0)) {
	 return(1);
	 }

	 return(0);
	 */
	return 1;
}

static void ANDROID_DeleteDevice(SDL_VideoDevice *device) {
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_VideoDevice *ANDROID_CreateDevice(int devindex) {
	SDL_VideoDevice *device;

	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *) SDL_malloc(sizeof(SDL_VideoDevice));
	if (device) {
		SDL_memset(device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *) SDL_malloc((sizeof *device->hidden));
	}
	if ((device == NULL) || (device->hidden == NULL)) {
		SDL_OutOfMemory();
		if (device) {
			SDL_free(device);
		}
		return (0);
	}
	SDL_memset(device->hidden, 0, (sizeof *device->hidden));

	/* Set the function pointers */
	device->VideoInit = ANDROID_VideoInit;
	device->ListModes = ANDROID_ListModes;
	device->SetVideoMode = ANDROID_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = ANDROID_SetColors;
	device->UpdateRects = ANDROID_UpdateRects;
	device->VideoQuit = ANDROID_VideoQuit;
	device->AllocHWSurface = ANDROID_AllocHWSurface;
	device->CheckHWBlit = NULL;
	device->FillHWRect = NULL;
	device->SetHWColorKey = NULL;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = ANDROID_LockHWSurface;
	device->UnlockHWSurface = ANDROID_UnlockHWSurface;
	device->FlipHWSurface = ANDROID_FlipHWSurface;
	device->FreeHWSurface = ANDROID_FreeHWSurface;
	device->SetCaption = NULL;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
	device->InitOSKeymap = ANDROID_InitOSKeymap;
	device->PumpEvents = ANDROID_PumpEvents;

	device->free = ANDROID_DeleteDevice;

	return device;
}

VideoBootStrap ANDROID_bootstrap = { ANDROIDVID_DRIVER_NAME, "SDL android video driver", ANDROID_Available, ANDROID_CreateDevice };

int ANDROID_VideoInit( _THIS, SDL_PixelFormat *vformat) {
	int i;
	/* Determine the screen depth (use default 16-bit depth) */
	/* we change this during the SDL_SetVideoMode implementation... */
	vformat->BitsPerPixel = 16;
	vformat->BytesPerPixel = 2;

	for (i = 0; i < SDL_NUMMODES; ++i) {
		SDL_modelist[i] = SDL_malloc(sizeof(SDL_Rect));
		SDL_modelist[i]->x = SDL_modelist[i]->y = 0;
	}
	/* Modes sorted largest to smallest */
	SDL_modelist[0]->w = sWindowWidth;
	SDL_modelist[0]->h = sWindowHeight;
	SDL_modelist[1]->w = 320;
	SDL_modelist[1]->h = 240; // Always available on any screen and any orientation
	SDL_modelist[2]->w = 320;
	SDL_modelist[2]->h = 200; // Always available on any screen and any orientation
	SDL_modelist[3] = NULL;

	WaitForNativeRender = SDL_CreateMutex();
	WaitForNativeRender1 = SDL_CreateCond();
	/* We're done! */
	return (0);
}

SDL_Rect **ANDROID_ListModes( _THIS, SDL_PixelFormat *format, Uint32 flags) {
	if (format->BitsPerPixel != 16)
		return NULL;
	return SDL_modelist;
}

SDL_Surface *ANDROID_SetVideoMode( _THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags) {
	if (memBuffer1)
		SDL_free(memBuffer1);
	if (memBuffer2)
		SDL_free(memBuffer2);

	memX = width;
	memY = height;

	memBuffer1 = SDL_malloc(memX * memY * (bpp / 8));
	if (!memBuffer1) {
		SDL_SetError("Couldn't allocate buffer for requested mode");
		return (NULL);
	}
	SDL_memset(memBuffer1, 0, memX * memY * (bpp / 8));

	if (flags & SDL_DOUBLEBUF) {
		memBuffer2 = SDL_malloc(memX * memY * (bpp / 8));
		if (!memBuffer2) {
			SDL_SetError("Couldn't allocate buffer for requested mode");
			return (NULL);
		}
		SDL_memset(memBuffer2, 0, memX * memY * (bpp / 8));
	}

	memBuffer = memBuffer1;
	openglInitialized = GL_State_Init;

	/* Allocate the new pixel format for the screen */
	if (!SDL_ReallocFormat(current, bpp, 0, 0, 0, 0)) {
		SDL_free(memBuffer);
		memBuffer = NULL;
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return (NULL);
	}

	/* Set up the new mode framebuffer */
	current->flags = SDL_FULLSCREEN; //(flags & SDL_FULLSCREEN) | (flags & SDL_DOUBLEBUF);
	current->w = width;
	current->h = height;
	current->pitch = memX * (bpp / 8);
	current->pixels = memBuffer;

	/* Wait 'till we can draw */
	ANDROID_FlipHWSurface(this, current);
	/* We're done */
	return (current);
}

/* Note:  If we are terminated, this could be called in the middle of
 another SDL video routine -- notably UpdateRects.
 */
void ANDROID_VideoQuit( _THIS) {
	openglInitialized = GL_State_Uninit;
	while (openglInitialized != GL_State_Uninit2)
		SDL_Delay(50);

	memX = 0;
	memY = 0;
	memBuffer = NULL;
	SDL_free(memBuffer1);
	memBuffer1 = NULL;
	if (memBuffer2)
		SDL_free(memBuffer2);
	memBuffer2 = NULL;
	SDL_DestroyMutex(WaitForNativeRender);
	WaitForNativeRender = NULL;
	SDL_DestroyCond(WaitForNativeRender1);
	WaitForNativeRender1 = NULL;

	int i;

	if (this->screen->pixels != NULL) {
		SDL_free(this->screen->pixels);
		this->screen->pixels = NULL;
	}
	/* Free video mode lists */
	for (i = 0; i < SDL_NUMMODES; ++i) {
		if (SDL_modelist[i] != NULL) {
			SDL_free(SDL_modelist[i]);
			SDL_modelist[i] = NULL;
		}
	}
}

void ANDROID_PumpEvents( _THIS) {
}

/* We don't actually allow hardware surfaces other than the main one */
// TODO: use OpenGL textures here
static int ANDROID_AllocHWSurface( _THIS, SDL_Surface *surface) {
	return (-1);
}
static void ANDROID_FreeHWSurface( _THIS, SDL_Surface *surface) {
	return;
}

/* We need to wait for vertical retrace on page flipped displays */
static int ANDROID_LockHWSurface( _THIS, SDL_Surface *surface) {
	return (0);
}

static void ANDROID_UnlockHWSurface( _THIS, SDL_Surface *surface) {
	return;
}

static void ANDROID_UpdateRects( _THIS, int numrects, SDL_Rect *rects) {
	ANDROID_FlipHWSurface(this, SDL_VideoSurface);
}

static int ANDROID_FlipHWSurface( _THIS, SDL_Surface *surface) {
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Frame is ready to render");
//	SDL_mutexP(WaitForNativeRender);
//	while (WaitForNativeRenderState != Render_State_Finished) {
//		if (SDL_CondWaitTimeout(WaitForNativeRender1, WaitForNativeRender, 1000) != 0) {
//			__android_log_print(ANDROID_LOG_INFO, "libSDL", "Frame failed to render");
//			SDL_mutexV(WaitForNativeRender);
//			return (0);
//		}
//	}
//
//	WaitForNativeRenderState = Render_State_Started;
//
//	SDL_CondSignal(WaitForNativeRender1);
//
//	if (surface->flags & SDL_DOUBLEBUF) {
//
//		if (WaitForNativeRenderState != Render_State_Started)
//			SDL_CondWaitTimeout(WaitForNativeRender1, WaitForNativeRender, 1000);
//
//		if (WaitForNativeRenderState != Render_State_Started) {
//			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Frame rendering done");
//			if (memBuffer == memBuffer1)
//				memBuffer = memBuffer2;
//			else
//				memBuffer = memBuffer1;
//		}
//
//		surface->pixels = memBuffer;
//	} else {
//		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Frame rendering done (no double buffer)");
//		while (WaitForNativeRenderState != Render_State_Finished) {
//			if (SDL_CondWaitTimeout(WaitForNativeRender1, WaitForNativeRender, 1000) != 0) {
//				SDL_mutexV(WaitForNativeRender);
//				return (0);
//			};
//		}
//	}
//
//	SDL_mutexV(WaitForNativeRender);
//
//	processAndroidTrackballKeyDelays(-1, 0);

	return (0);
}
;

int ANDROID_SetColors( _THIS, int firstcolor, int ncolors, SDL_Color *colors) {
	return (1);
}

/* JNI-C++ wrapper stuff */

extern int main(int argc, char ** argv);
static int SDLCALL MainThreadWrapper(void * dummy)
{
	int argc = 1;
	char * argv[] = {"sdl"};
	chdir(SDL_CURDIR_PATH);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_CURDIR_PATH=%s", SDL_CURDIR_PATH);
	return main( argc, argv );
};

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

extern void JAVA_EXPORT_NAME(DemoRenderer_nativeInit)(JNIEnv* env, jobject thiz) {
	mainThread = SDL_CreateThread(MainThreadWrapper, NULL);
}

extern void JAVA_EXPORT_NAME(DemoRenderer_nativeResize)(JNIEnv* env, jobject thiz, jint w, jint h) {
	sWindowWidth = w;
	sWindowHeight = h;
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "resize w=%d h=%d", w, h);
}

/* Call to finalize the graphics state */
extern void JAVA_EXPORT_NAME(DemoRenderer_nativeDone)(JNIEnv* env, jobject thiz) {
	if (mainThread) {
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "quitting...");
		SDL_PrivateQuit();
		SDL_WaitThread(mainThread, NULL);
		mainThread = NULL;
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "quit OK");
	}
}

enum MOUSE_ACTION {
	MOUSE_DOWN = 0, MOUSE_UP = 1, MOUSE_MOVE = 2
};

extern void JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMouse)(JNIEnv* env, jobject thiz, jint x, jint y, jint action) {
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "mouse event %i at (%03i, %03i)", action, x, y);
	if (action == MOUSE_DOWN || action == MOUSE_UP)
		SDL_PrivateMouseButton((action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, 1, x, y);
	if (action == MOUSE_MOVE)
		SDL_PrivateMouseMotion(0, 0, x, y);
}

static SDL_keysym *TranslateKey(int scancode, SDL_keysym *keysym) {
	/* Sanity check */
	if (scancode >= SDL_arraysize(keymap))
		scancode = KEYCODE_UNKNOWN;

	/* Set the keysym information */
	keysym->scancode = scancode;
	keysym->sym = keymap[scancode];
	keysym->mod = KMOD_NONE;

	/* If UNICODE is on, get the UNICODE value for the key */
	keysym->unicode = 0;
	if (SDL_TranslateUNICODE) {
		/* Populate the unicode field with the ASCII value */
		keysym->unicode = scancode;
	}
	return (keysym);
}

static int AndroidTrackballKeyDelays[4] = {0,0,0,0};

// Key = -1 if we want to send KeyUp events from main loop
static int processAndroidTrackballKeyDelays(int key, int action) {
#if ! defined(SDL_TRACKBALL_KEYUP_DELAY) || (SDL_TRACKBALL_KEYUP_DELAY == 0)
	return 0;
#else
	// Send Directional Pad Up events with a delay, so app wil lthink we're holding the key a bit
	static const int KeysMapping[4] = {KEYCODE_DPAD_UP, KEYCODE_DPAD_DOWN, KEYCODE_DPAD_LEFT, KEYCODE_DPAD_RIGHT};
	int idx, idx2;
	SDL_keysym keysym;

	if( key < 0 )
	{
		for( idx = 0; idx < 4; idx ++ )
		{
			if( AndroidTrackballKeyDelays[idx] > 0 )
			{
				AndroidTrackballKeyDelays[idx] --;
				if( AndroidTrackballKeyDelays[idx] == 0 )
				SDL_PrivateKeyboard( SDL_RELEASED, TranslateKey(KeysMapping[idx], &keysym) );
			}
		}
	}
	else
	{
		idx = -1;
		// Too lazy to do switch or function
		if( key == KEYCODE_DPAD_UP )
		idx = 0;
		else if( key == KEYCODE_DPAD_DOWN )
		idx = 1;
		else if( key == KEYCODE_DPAD_LEFT )
		idx = 2;
		else if( key == KEYCODE_DPAD_RIGHT )
		idx = 3;
		if( idx >= 0 )
		{
			if( action && AndroidTrackballKeyDelays[idx] == 0 )
			{
				// User pressed key for the first time
				idx2 = (idx + 2) % 4; // Opposite key for current key - if it's still pressing, release it
				if( AndroidTrackballKeyDelays[idx2] > 0 )
				{
					AndroidTrackballKeyDelays[idx2] = 0;
					SDL_PrivateKeyboard( SDL_RELEASED, TranslateKey(KeysMapping[idx2], &keysym) );
				}
				SDL_PrivateKeyboard( SDL_PRESSED, TranslateKey(key, &keysym) );
			}
			else if( !action && AndroidTrackballKeyDelays[idx] == 0 )
			{
				// User released key - make a delay, do not send release event
				AndroidTrackballKeyDelays[idx] = SDL_TRACKBALL_KEYUP_DELAY;
			}
			else if( action && AndroidTrackballKeyDelays[idx] > 0 )
			{
				// User pressed key another time - add some more time for key to be pressed
				AndroidTrackballKeyDelays[idx] += SDL_TRACKBALL_KEYUP_DELAY;
				if( AndroidTrackballKeyDelays[idx] < SDL_TRACKBALL_KEYUP_DELAY * 4 )
				AndroidTrackballKeyDelays[idx] = SDL_TRACKBALL_KEYUP_DELAY * 4;
			}
			return 1;
		}
	}
	return 0;

#endif
}

void JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeKey)(JNIEnv* env, jobject thiz, jint key, jint action) {
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "key event %i %s", key, action ? "down" : "up");
	SDL_keysym keysym;
	if (!processAndroidTrackballKeyDelays(key, action))
		SDL_PrivateKeyboard(action ? SDL_PRESSED : SDL_RELEASED, TranslateKey(key, &keysym));
}

/* Call to render the next GL frame */
extern void JAVA_EXPORT_NAME(DemoRenderer_nativeRender)(JNIEnv* env, jobject thiz) {
	// Set up an array of values to use as the sprite vertices.
	static GLfloat vertices[] = { 0, 0, 1, 0, 0, 1, 1, 1, };

	// Set up an array of values for the texture coordinates.
	static GLfloat texcoords[] = { 0, 0, 1, 0, 0, 1, 1, 1, };

	static GLint texcoordsCrop[] = { 0, 0, 0, 0, };

	static float clearColor = 0.0f;
	static int clearColorDir = 1;
	int textX, textY;
	void * memBufferTemp;

	if (memBuffer && openglInitialized != GL_State_Uninit2) {
		if (openglInitialized == GL_State_Init) {

			__android_log_print(ANDROID_LOG_INFO, "libSDL", " sWindowWidth=%i, sWindowHeight=%i, memX=%i, memY=%i", sWindowWidth,
					sWindowHeight, memX, memY);

			openglInitialized = GL_State_Ready;

			// Texture sizes should be 2^n
			textX = memX;
			textY = memY;

			if (textX <= 256)
				textX = 256;
			else if (textX <= 512)
				textX = 512;
			else
				textX = 1024;

			if (textY <= 256)
				textY = 256;
			else if (textY <= 512)
				textY = 512;
			else
				textY = 1024;

			//glViewport(0, 0, textX, textY);
			glViewport(0, 0, sWindowWidth, sWindowHeight);

			glClearColor(0, 0, 0, 0);
			// Set projection
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
#if defined(GL_VERSION_ES_CM_1_0)
#define glOrtho glOrthof
#endif
			glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
			//glOrthof(0.0f, sWindowWidth, sWindowHeight, 0.0f, -1.0f, 1.0f);

			// Now Initialize modelview matrix
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DITHER);
			glDisable(GL_MULTISAMPLE);

			glEnable(GL_TEXTURE_2D);

			glGenTextures(1, &texture);

			glBindTexture(GL_TEXTURE_2D, texture);

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			void * textBuffer = SDL_malloc(textX * textY * 2);
			SDL_memset(textBuffer, 0, textX * textY * 2);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textX, textY, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, textBuffer);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			// Make the texture fit the device screen if required
			if (autoResizingActivated) {
				GLfloat ratioX = 1.0f;
				GLfloat ratioY = 1.0f;
				//if (memX > sWindowWidth) {
					ratioX = ((GLfloat) memX) / ((GLfloat) textX);
				//}
				//if (memY > sWindowHeight) {
					ratioY = ((GLfloat) memY) / ((GLfloat) textY);
				//}
				texcoords[0] *= ratioX;
				texcoords[1] *= ratioY;
				texcoords[2] *= ratioX;
				texcoords[3] *= ratioY;
				texcoords[4] *= ratioX;
				texcoords[5] *= ratioY;
				texcoords[6] *= ratioX;
				texcoords[7] *= ratioY;
				//__android_log_print(ANDROID_LOG_INFO, "libSDL", "textX=%i textY=%i, ratioX=%f ratioY=%f", textX, textY, ratioX , ratioY);
			}

			glVertexPointer(2, GL_FLOAT, 0, vertices);
			glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			// GLES extension (should be faster)
			//			texcoordsCrop[0] = 0;
			//			texcoordsCrop[1] = memY;
			//			texcoordsCrop[2] = memX;
			//			texcoordsCrop[3] = -memY;
			//			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, texcoordsCrop);

			glFinish();

			SDL_free(textBuffer);
		} else if (openglInitialized == GL_State_Uninit) {
			openglInitialized = GL_State_Uninit2;

			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);

			glDeleteTextures(1, &texture);

			return;
		}

//		if (WaitForNativeRender) {
//			SDL_mutexP(WaitForNativeRender);
//
//			WaitForNativeRenderState = Render_State_Finished;
//			SDL_CondSignal(WaitForNativeRender1);
//
//			while (WaitForNativeRenderState != Render_State_Started) {
//				if (SDL_CondWaitTimeout(WaitForNativeRender1, WaitForNativeRender, 1000) != 0) {
//					__android_log_print(ANDROID_LOG_INFO, "libSDL", "Frame failed to render");
//					SDL_mutexV(WaitForNativeRender);
//					return;
//				}
//			}
//
//			memBufferTemp = memBuffer;
//
//			WaitForNativeRenderState = Render_State_Processing;
//
//			SDL_CondSignal(WaitForNativeRender1);
//
//			SDL_mutexV(WaitForNativeRender);
//		} else {
			memBufferTemp = memBuffer;
//		}

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, memX, memY, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, memBufferTemp);

		//glClearColor(0, 1.0f, 0, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//		glDrawTexiOES(0, sWindowHeight-memY, 1, memX, memY); // GLES extension (should be faster)
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "glDrawTexiOES(%i, 1, %i, %i)", sWindowHeight-memY ,memX, memY);
		glFinish(); //glFlush();

	} else {
		/*
		 // Flash the screen
		 if( clearColor >= 1.0f )
		 clearColorDir = -1;
		 else if( clearColor <= 0.0f )
		 clearColorDir = 1;

		 clearColor += (float)clearColorDir * 0.01f;
		 glClearColor(clearColor,clearColor,clearColor,0);
		 glClear(GL_COLOR_BUFFER_BIT);
		 SDL_Delay(50);
		 */
	}
}

void ANDROID_InitOSKeymap( _THIS) {
	int i;

	/* Initialize the DirectFB key translation table */
	for (i = 0; i < SDL_arraysize(keymap); ++i)
		keymap[i] = SDLK_UNKNOWN;

	keymap[KEYCODE_UNKNOWN] = SDLK_UNKNOWN;

	keymap[KEYCODE_BACK] = SDLK_ESCAPE;

	keymap[KEYCODE_MENU] = SDLK_LALT;
	keymap[KEYCODE_CALL] = SDLK_LCTRL;
	keymap[KEYCODE_ENDCALL] = SDLK_LSHIFT;
	keymap[KEYCODE_CAMERA] = SDLK_RSHIFT;
	keymap[KEYCODE_POWER] = SDLK_RALT;

	keymap[KEYCODE_BACK] = SDLK_ESCAPE; // Note: generates SDL_QUIT
	keymap[KEYCODE_0] = SDLK_0;
	keymap[KEYCODE_1] = SDLK_1;
	keymap[KEYCODE_2] = SDLK_2;
	keymap[KEYCODE_3] = SDLK_3;
	keymap[KEYCODE_4] = SDLK_4;
	keymap[KEYCODE_5] = SDLK_5;
	keymap[KEYCODE_6] = SDLK_6;
	keymap[KEYCODE_7] = SDLK_7;
	keymap[KEYCODE_8] = SDLK_8;
	keymap[KEYCODE_9] = SDLK_9;
	keymap[KEYCODE_STAR] = SDLK_ASTERISK;
	keymap[KEYCODE_POUND] = SDLK_DOLLAR;

	keymap[KEYCODE_DPAD_UP] = SDLK_UP;
	keymap[KEYCODE_DPAD_DOWN] = SDLK_DOWN;
	keymap[KEYCODE_DPAD_LEFT] = SDLK_LEFT;
	keymap[KEYCODE_DPAD_RIGHT] = SDLK_RIGHT;
	keymap[KEYCODE_DPAD_CENTER] = SDLK_RETURN;

	keymap[KEYCODE_SOFT_LEFT] = SDLK_KP4;
	keymap[KEYCODE_SOFT_RIGHT] = SDLK_KP6;
	keymap[KEYCODE_ENTER] = SDLK_KP_ENTER;

	keymap[KEYCODE_VOLUME_UP] = SDLK_PAGEUP;
	keymap[KEYCODE_VOLUME_DOWN] = SDLK_PAGEDOWN;
	keymap[KEYCODE_SEARCH] = SDLK_END;
	keymap[KEYCODE_HOME] = SDLK_HOME;

	keymap[KEYCODE_CLEAR] = SDLK_CLEAR;
	keymap[KEYCODE_A] = SDLK_a;
	keymap[KEYCODE_B] = SDLK_b;
	keymap[KEYCODE_C] = SDLK_c;
	keymap[KEYCODE_D] = SDLK_d;
	keymap[KEYCODE_E] = SDLK_e;
	keymap[KEYCODE_F] = SDLK_f;
	keymap[KEYCODE_G] = SDLK_g;
	keymap[KEYCODE_H] = SDLK_h;
	keymap[KEYCODE_I] = SDLK_i;
	keymap[KEYCODE_J] = SDLK_j;
	keymap[KEYCODE_K] = SDLK_k;
	keymap[KEYCODE_L] = SDLK_l;
	keymap[KEYCODE_M] = SDLK_m;
	keymap[KEYCODE_N] = SDLK_n;
	keymap[KEYCODE_O] = SDLK_o;
	keymap[KEYCODE_P] = SDLK_p;
	keymap[KEYCODE_Q] = SDLK_q;
	keymap[KEYCODE_R] = SDLK_r;
	keymap[KEYCODE_S] = SDLK_s;
	keymap[KEYCODE_T] = SDLK_t;
	keymap[KEYCODE_U] = SDLK_u;
	keymap[KEYCODE_V] = SDLK_v;
	keymap[KEYCODE_W] = SDLK_w;
	keymap[KEYCODE_X] = SDLK_x;
	keymap[KEYCODE_Y] = SDLK_y;
	keymap[KEYCODE_Z] = SDLK_z;
	keymap[KEYCODE_COMMA] = SDLK_COMMA;
	keymap[KEYCODE_PERIOD] = SDLK_PERIOD;
	keymap[KEYCODE_TAB] = SDLK_TAB;
	keymap[KEYCODE_SPACE] = SDLK_SPACE;
	keymap[KEYCODE_DEL] = SDLK_DELETE;
	keymap[KEYCODE_GRAVE] = SDLK_BACKQUOTE;
	keymap[KEYCODE_MINUS] = SDLK_MINUS;
	keymap[KEYCODE_EQUALS] = SDLK_EQUALS;
	keymap[KEYCODE_LEFT_BRACKET] = SDLK_LEFTBRACKET;
	keymap[KEYCODE_RIGHT_BRACKET] = SDLK_RIGHTBRACKET;
	keymap[KEYCODE_BACKSLASH] = SDLK_BACKSLASH;
	keymap[KEYCODE_SEMICOLON] = SDLK_SEMICOLON;
	keymap[KEYCODE_APOSTROPHE] = SDLK_QUOTE;
	keymap[KEYCODE_SLASH] = SDLK_SLASH;
	keymap[KEYCODE_AT] = SDLK_AT;

	keymap[KEYCODE_PLUS] = SDLK_PLUS;

	/*

	 keymap[KEYCODE_SYM] = SDLK_SYM;
	 keymap[KEYCODE_NUM] = SDLK_NUM;

	 keymap[KEYCODE_SOFT_LEFT] = SDLK_SOFT_LEFT;
	 keymap[KEYCODE_SOFT_RIGHT] = SDLK_SOFT_RIGHT;

	 keymap[KEYCODE_ALT_LEFT] = SDLK_ALT_LEFT;
	 keymap[KEYCODE_ALT_RIGHT] = SDLK_ALT_RIGHT;
	 keymap[KEYCODE_SHIFT_LEFT] = SDLK_SHIFT_LEFT;
	 keymap[KEYCODE_SHIFT_RIGHT] = SDLK_SHIFT_RIGHT;

	 keymap[KEYCODE_EXPLORER] = SDLK_EXPLORER;
	 keymap[KEYCODE_ENVELOPE] = SDLK_ENVELOPE;
	 keymap[KEYCODE_HEADSETHOOK] = SDLK_HEADSETHOOK;
	 keymap[KEYCODE_FOCUS] = SDLK_FOCUS;
	 keymap[KEYCODE_NOTIFICATION] = SDLK_NOTIFICATION;
	 keymap[KEYCODE_MEDIA_PLAY_PAUSE=] = SDLK_MEDIA_PLAY_PAUSE=;
	 keymap[KEYCODE_MEDIA_STOP] = SDLK_MEDIA_STOP;
	 keymap[KEYCODE_MEDIA_NEXT] = SDLK_MEDIA_NEXT;
	 keymap[KEYCODE_MEDIA_PREVIOUS] = SDLK_MEDIA_PREVIOUS;
	 keymap[KEYCODE_MEDIA_REWIND] = SDLK_MEDIA_REWIND;
	 keymap[KEYCODE_MEDIA_FAST_FORWARD] = SDLK_MEDIA_FAST_FORWARD;
	 keymap[KEYCODE_MUTE] = SDLK_MUTE;
	 */

}

