/*
 *  milkyplay/XMFile.h
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
 *  XMFile.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on Tue Oct 19 2004.
 *  
 */
#ifndef __XMFILE_H__
#define __XMFILE_H__

#include "MilkyPlayCommon.h"

class XMFileBase
{
private:
	mp_dword				baseOffset;
	
public:
							XMFileBase();
	virtual					~XMFileBase();

	virtual mp_sint32		read(void* ptr,mp_sint32 size,mp_sint32 count) = 0;
	virtual mp_sint32		write(const void* ptr,mp_sint32 size,mp_sint32 count) = 0;

	enum SeekOffsetTypes
	{
		SeekOffsetTypeStart,
		SeekOffsetTypeCurrent,
		SeekOffsetTypeEnd
	};
	virtual void			seek(mp_uint32 pos, SeekOffsetTypes seekOffsetType = SeekOffsetTypeStart) = 0;
	virtual mp_uint32		pos() = 0;
	virtual mp_uint32		size() = 0;

	virtual bool			isEOF() { return pos() >= size(); }

	virtual	const SYSCHAR*  getFileName() = 0;
	
	virtual	const char*		getFileNameASCII() = 0;
	
	virtual	bool			isOpen() = 0;
	virtual	bool			isOpenForWriting()  = 0;

	mp_ubyte				readByte();
	mp_uword				readWord();
	mp_dword				readDword();
	void					readWords(mp_uword* buffer,mp_sint32 count);
	void					readDwords(mp_dword* buffer,mp_sint32 count);

	void					writeByte(mp_ubyte b);
	void					writeWord(mp_uword w);
	void					writeDword(mp_dword dw);
	void					writeWords(const mp_uword* buffer,mp_sint32 count);
	void					writeDwords(const mp_dword* buffer,mp_sint32 count);
	
	void					writeString(const char* string);
	
	void					setBaseOffset(mp_dword baseOffset) { this->baseOffset = baseOffset; }
	mp_dword				getBaseOffset() const { return baseOffset; }
	void					seekWithBaseOffset(mp_dword pos);
	mp_uint32				sizeWithBaseOffset();
	mp_uint32				posWithBaseOffset();
};

class XMFile : public XMFileBase
{
private:
	const SYSCHAR*  fileName;

	char*			fileNameASCII;

	FHANDLE			handle;
	mp_uint32		bytesRead;
	
	bool			writeAccess;
	
	mp_ubyte*		cacheBuffer;
	mp_ubyte*		currentCacheBufferPtr;
	
	void			flush();
	
public:
							XMFile(const SYSCHAR* fileName, bool writeAccess = false);
	virtual					~XMFile();
	
	virtual mp_sint32		read(void* ptr,mp_sint32 size,mp_sint32 count);
	virtual mp_sint32		write(const void* ptr,mp_sint32 size,mp_sint32 count);
	
	virtual void			seek(mp_uint32 pos, SeekOffsetTypes seekOffsetType = SeekOffsetTypeStart);
	virtual mp_uint32		pos();
	virtual mp_uint32		size();
	
	virtual const SYSCHAR*  getFileName() { return fileName; }
	
	virtual const char*		getFileNameASCII();
	
	virtual bool			isOpen();
	virtual bool			isOpenForWriting() { return isOpen() && writeAccess; }
	
	static bool				exists(const SYSCHAR* file);
	static bool				remove(const SYSCHAR* file);
};

#endif
