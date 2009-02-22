/*
 *  fx/Texture.h
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

#ifndef TEXTURE__H
#define TEXTURE__H

class Texture
{
public:
	static void	createSplineTexture(unsigned char* tex,
									int numBlocks = 10000,
									int blockSize = 16);

	static void createFlareTexture(unsigned char* tex,
								   int r,int g,int b,
								   float pw = 4.0f, 
								   unsigned int size = 256);

	static void createPlasmaTexture(unsigned char* tex,
									unsigned int size = 256,
									int smooth = 3,
									int r = 255, int g = 255, int b = 255);

	static void convert24to16(unsigned short* dstImage, unsigned char* srcImage, int size = 256*256, unsigned int shifter = 0);

	static void blur24(unsigned char* tex, unsigned int width, unsigned int height, unsigned int passes = 1);
};

#endif
