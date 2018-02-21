/*
 *  compression/DecompressorLHA.cpp
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
 *  DecompressorLHA.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 22.06.08.
 *
 */

#include <lhasa.h>

#include "DecompressorLHA.h"
#include "XMFile.h"
#include "XModule.h"

#define LHA_BUFFER_SIZE 0x10000

namespace
{
	static int lha_read_callback(void *handle, void *buf, size_t buf_len)
	{
		return static_cast<XMFile*>(handle)->read(buf, 1, buf_len);
	}

	static const LHAInputStreamType lha_callbacks =
	{
		lha_read_callback,
		NULL,
		NULL
	};

	// Simple wrapper for lha_reader_*
	class LHAReaderWrapper
	{
	public:
		explicit LHAReaderWrapper(XMFile& file)
		{
			// Open input stream
			input_stream = lha_input_stream_new(&lha_callbacks, &file);
			if (input_stream == NULL)
				return;

			// Open reader
			reader = lha_reader_new(input_stream);
		}

		bool isOpen() const
		{
			return reader != NULL;
		}

		LHAFileHeader* nextFile()
		{
			return lha_reader_next_file(reader);
		}

		size_t read(void* buf, size_t length)
		{
			return lha_reader_read(reader, buf, length);
		}

		~LHAReaderWrapper()
		{
			if (reader != NULL)
				lha_reader_free(reader);
			if (input_stream != NULL)
				lha_input_stream_free(input_stream);
		}

	private:
		LHAInputStream* input_stream = NULL;
		LHAReader* reader = NULL;

		LHAReaderWrapper(const LHAReaderWrapper&) {}
		LHAReaderWrapper& operator=(const LHAReaderWrapper&) {}
	};
}

DecompressorLHA::DecompressorLHA(const PPSystemString& filename) :
	DecompressorBase(filename)
{
}

bool DecompressorLHA::identify(XMFile& f)
{
	f.seek(0);

	// Attempt to create the reader and read the header of the first file
	LHAReaderWrapper reader(f);
	if (!reader.isOpen())
		return false;

	return reader.nextFile() != NULL;
}	
	
const PPSimpleVector<Descriptor>& DecompressorLHA::getDescriptors(Hints hint) const
{
	descriptors.clear();
	descriptors.add(new Descriptor("lha", "LHA Archive")); 	
	return descriptors;
}		
	
bool DecompressorLHA::decompress(const PPSystemString& outFilename, Hints hint)
{
	XMFile f(fileName);
	
	if (!f.isOpen())
		return false;

	// Create reader object
	LHAReaderWrapper reader(f);
	if (!reader.isOpen())
		return false;

	// Loop through each file until we find a sutible module
	while (1)
	{
		LHAFileHeader* header = reader.nextFile();
		if (header == NULL)
			break;

		// Skip directories and symlinks
		if (strcmp(header->compress_method, LHA_COMPRESS_TYPE_DIR) == 0)
			continue;

		// Identify the current file
		mp_ubyte buf[LHA_BUFFER_SIZE];
		memset(buf, 0, sizeof(buf));
		size_t bytes_read = reader.read(buf, sizeof(buf));

		if (bytes_read > 0 && XModule::identifyModule(buf) != NULL)
		{
			// Write to output file
			XMFile outFile(outFilename, true);
			if (!outFile.isOpenForWriting())
				return false;

			// Decompress into outFile
			do
			{
				outFile.write(buf, 1, bytes_read);
			}
			while ((bytes_read = reader.read(buf, sizeof(buf))) > 0);

			return (bytes_read == 0);
		}
	}

	// No sutible modules found
	return false;
}

DecompressorBase* DecompressorLHA::clone()
{
	return new DecompressorLHA(fileName);
}	

static Decompressor::RegisterDecompressor<DecompressorLHA> registerDecompressor;
