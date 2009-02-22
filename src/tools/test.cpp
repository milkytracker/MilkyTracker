/*
 *  tools/test.cpp
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

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std; 

const int breakl = 16;
const int seekpos = 0;

const char* hextab = "0123456789abcdef";

void btoh(unsigned char i, char* input)
{
	input[0] = hextab[i>>4];
	input[1] = hextab[i&0xf];
	input[2] = '\0';
}

int main(int argc, const char* argv[])
{
	if (argc != 3)
	{
		cerr << "Forgot your parameters?";
		exit(-1);
	}

	FILE* infile = fopen(argv[1],"rb");
	if (!infile)
	{
		cerr  << "Could not open " << argv[1] << endl;
		exit(0);
	}
	fseek(infile, seekpos, 0);
	
	FILE* outfile = fopen(argv[2],"wb");
	if (!outfile)
	{
		cerr << "Could not create " << argv[2] << endl;
		exit(0);
	}
	
	int bytesread = 0;
	int colcnt = 0;
	do
	{
		unsigned char buffer[3*10];
		bytesread = fread(buffer, 1, sizeof(buffer), infile);
		
		/*for (int j = 0; j < bytesread/3; j++)
		{
			unsigned char v1 = buffer[j*3+2];
			unsigned char v2 = buffer[j*3+1];
			unsigned char v3 = buffer[j*3+0];
			buffer[j*3+0] = v1;
			buffer[j*3+1] = v2;
			buffer[j*3+2] = v3;
		}*/

		for (int i = 0; i < bytesread; i++)
		{
			char outbuffer[100];
			outbuffer[0] = '0';
			outbuffer[1] = 'x';

			btoh(buffer[i], outbuffer+2);

			outbuffer[4] = ',';
			outbuffer[5] = '\0';
			
			if (colcnt == breakl)
			{
				strcat(outbuffer, "\n");
				colcnt = 0;
			}
			
			fwrite(outbuffer, 1, strlen(outbuffer), outfile);
			colcnt++;
		}
		
	} while (bytesread != 0);
	
	fclose(outfile);
	fclose(infile);
	
	return 0;
}
