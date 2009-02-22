/*
 *  tools/archivewriter.cpp
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
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include "BasicTypes.h"
#include "PPPathFactory.h"
#include "XMFile.h"
#include "SimpleVector.h"

using namespace std;

class ArchiveWriter
{
public:
	enum 
	{
		RecordSize = (256+4+4)
	};

	void addFile(const PPSystemString& fileName)
	{
		files.add(new PPSystemString(fileName));
	}

	void flush(const PPSystemString& fileName)
	{
		XMFile f(fileName, true);
		
		f.write("pls\xff", 1, 4);
		f.writeDword(files.size());
		f.writeDword(RecordSize);
		
		pp_uint32 offset = 12;
		// write out toc first
		for (pp_int32 i = 0; i < files.size(); i++)
		{
			XMFile inf(*files.get(i));
			pp_uint32 size = inf.size();
			PPSystemString fileName = files.get(i)->stripPath();
			cout << fileName << " Size: " << size << endl;
			
			char* nameASCIIZ = fileName.toASCIIZ();	
			
			char result[256];
			memset(result, 0, sizeof(result));
			strcpy(result, nameASCIIZ);
			
			delete[] nameASCIIZ;
			
			f.write(result, 1, sizeof(result));
			f.writeDword(offset + RecordSize*files.size());
			f.writeDword(size);
			offset+=size;
		}
		
		// write out files
		for (pp_int32 i = 0; i < files.size(); i++)
		{
			XMFile inf(*files.get(i));
			pp_uint32 size = inf.size();
			char* chunk = new char[size];
			inf.read(chunk, 1, size);
			f.write(chunk, 1, size);
			delete[] chunk;
		}
	}

private:
	PPSimpleVector<PPSystemString> files;
};

int main(int argc, const char* argv[])
{
	ArchiveWriter writer;

	// Input
	PPPath* path = PPPathFactory::createPathFromString(argv[1]);
	
	const PPPathEntry* entry = path->getFirstEntry();
	
	pp_uint32 numFonts = 0;
	
	while (entry)
	{
		if (!entry->isHidden() && entry->isFile())
		{
			PPSystemString currentFile = path->getCurrent();
			currentFile.append(entry->getName());
			//cout << currentFile << endl;
			
			writer.addFile(currentFile);
		}
		
	
		entry = path->getNextEntry();
	}

	writer.flush("/data");

	return 0;
}
