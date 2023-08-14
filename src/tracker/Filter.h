/*
 *  tracker/Filter.h 
 *
 *  Copyright 2023 Leon van Kammen (coderofsalvation)
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
 * This is a simple (static) hp/lp resonant filter which is cheap for the CPU 
 * and its resonance-param has enough growl for music production.
 */

#ifndef __FILTER_H
#define __FILTER_H

#ifndef M_PI
#define M_PI   3.14159265358979323846264338327950288
#endif

struct filter_t{
  // params
  float q;
  float cutoff;
  float srate;
  // output
  float out_hp;
  float out_lp;
  // cache vars
  float s0;
  float s1;
};

class Filter
{

  public:
    static void process(float in, filter_t *p);
    static void init( filter_t *p, int samplerate);
};

#endif
