/*
 *  KeyTranslation.h
 *  PPUI SDL
 *
 *  Created by Peter Barth on 19.11.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef KEYTRANSLATION__H
#define KEYTRANSLATION__H

#include <SDL/SDL.h>
#include "BasicTypes.h"

// #define NOT_PC_KB	// Set this if you're using non-PC type keyboard

pp_uint16 toVK(const SDL_keysym& keysym);
pp_uint16 toSC(const SDL_keysym& keysym);
#ifndef isX11
extern bool isX11;
extern bool stdKb;
#endif

#endif
