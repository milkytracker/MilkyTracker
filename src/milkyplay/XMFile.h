/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
