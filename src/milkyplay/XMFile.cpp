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

//////////////////////////////////////////////////////////////////////////
// System dependent file loading routines								//
// to make future porting easier										//
//////////////////////////////////////////////////////////////////////////
#include "XMFile.h"

XMFileBase::XMFileBase() :
	baseOffset(0)
{
}

XMFileBase::~XMFileBase()
{
}

void XMFileBase::seekWithBaseOffset(mp_dword pos)
{
	seek(pos+baseOffset);
}

mp_uint32 XMFileBase::sizeWithBaseOffset()
{
	return size()-baseOffset;
}

mp_uint32 XMFileBase::posWithBaseOffset()
{
	return pos()-baseOffset;
}

//////////////////////////////////////////////////////////////////////////
// Reading/writing of little endian stuff								//
//////////////////////////////////////////////////////////////////////////
mp_ubyte XMFileBase::readByte()
{
	mp_ubyte c;
	mp_sint32 bytesRead = read(&c,1,1);
	//ASSERT(bytesRead == 1);
	
	if (!(bytesRead == 1))
		c = 0;
	
	return (mp_ubyte)c;
}

mp_uword XMFileBase::readWord()
{
	mp_ubyte c[2];
	mp_sint32 bytesRead = read(&c,1,2);
	//ASSERT(bytesRead == 2);

	if (!(bytesRead == 2))
		c[0] = c[1] = 0;

	return (mp_uword)((mp_uword)c[0]+((mp_uword)c[1]<<8));
}

mp_dword XMFileBase::readDword()
{
	mp_ubyte c[4];
	mp_sint32 bytesRead = read(&c,1,4);
	//ASSERT(bytesRead == 4);

	if (!(bytesRead == 4))
		c[0] = c[1] = c[2] = c[3] = 0;

	return (mp_dword)((mp_uint32)c[0]+
					  ((mp_uint32)c[1]<<8)+
					  ((mp_uint32)c[2]<<16)+
					  ((mp_uint32)c[3]<<24));
}

void XMFileBase::readWords(mp_uword* buffer,mp_sint32 count)
{
	for (mp_sint32 i = 0; i < count; i++)
		*buffer++ = readWord();
}

void XMFileBase::readDwords(mp_dword* buffer,mp_sint32 count)
{
	for (mp_sint32 i = 0; i < count; i++)
		*buffer++ = readDword();
}

void XMFileBase::writeByte(mp_ubyte b)
{
	mp_sint32 bytesWritten = write(&b, 1, 1);
	ASSERT(bytesWritten == 1);
}

void XMFileBase::writeWord(mp_uword w)
{
	mp_ubyte c[2];
	c[0] = (mp_ubyte)w;
	c[1] = (mp_ubyte)(w>>8);
	mp_sint32 bytesWritten = write(&c, 1, 2);
	ASSERT(bytesWritten == 2);	
}

void XMFileBase::writeDword(mp_dword dw)
{
	mp_ubyte c[4];
	c[0] = (mp_ubyte)dw;
	c[1] = (mp_ubyte)(dw>>8);
	c[2] = (mp_ubyte)(dw>>16);
	c[3] = (mp_ubyte)(dw>>24);
	mp_sint32 bytesWritten = write(&c, 1, 4);
	ASSERT(bytesWritten == 4);	
}

void XMFileBase::writeWords(const mp_uword* buffer,mp_sint32 count)
{
	for (mp_sint32 i = 0; i < count; i++)
	{
		writeWord(*buffer);
		buffer++;
	}
}

void XMFileBase::writeDwords(const mp_dword* buffer,mp_sint32 count)
{
	for (mp_sint32 i = 0; i < count; i++)
	{
		writeDword(*buffer);
		buffer++;
	}
}

void XMFileBase::writeString(const char* string)
{
	write(string, 1, static_cast<mp_uint32> (strlen(string)));
}

#define BUFFERSIZE 16384

//////////////////////////////////////////////////////////////////////////
// WIN32 implentation													//
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32

#include <tchar.h>

XMFile::XMFile(const SYSCHAR*	fileName, bool writeAccess /* = false*/) :
	XMFileBase(),
	fileName(fileName),
	cacheBuffer(NULL)
{
	this->writeAccess = writeAccess;

	bytesRead = 0;
	
	handle = CreateFile(fileName,
					    writeAccess ? GENERIC_WRITE : GENERIC_READ,
						writeAccess ? FILE_SHARE_WRITE : FILE_SHARE_READ,
						NULL,
						writeAccess ? CREATE_ALWAYS : OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL, 
						NULL);

	//ASSERT(handle != INVALID_HANDLE_VALUE);

	if (writeAccess)
	{
		cacheBuffer = new mp_ubyte[BUFFERSIZE+16];
		currentCacheBufferPtr = cacheBuffer;
	}
}

bool XMFile::isOpen()
{
	return handle != INVALID_HANDLE_VALUE;
}

XMFile::~XMFile()
{
	if (writeAccess && handle != INVALID_HANDLE_VALUE)
		flush();

	if (cacheBuffer)
		delete[] cacheBuffer;

	CloseHandle(handle);
}

mp_sint32 XMFile::read(void* ptr,mp_sint32 size,mp_sint32 count)
{
	unsigned long NumberOfBytesRead;
	bool bResult = (bool)ReadFile(handle,ptr,size*count,&NumberOfBytesRead,NULL);
	bytesRead+=(mp_uint32)NumberOfBytesRead;
	if (!bResult) return -1;
	return (mp_sint32)NumberOfBytesRead;
}

void XMFile::flush()
{
	unsigned long NumberOfBytesWritten;
	bool bResult = (bool)WriteFile(handle,cacheBuffer,
								   currentCacheBufferPtr-cacheBuffer,
								   &NumberOfBytesWritten,
								   NULL);
	bytesRead+=(mp_uint32)NumberOfBytesWritten;
	currentCacheBufferPtr = cacheBuffer;
}

mp_sint32 XMFile::write(const void* ptr,mp_sint32 size,mp_sint32 count)
{
#ifdef DEBUG
	unsigned long NumberOfBytesWritten;
	bool bResult = (bool)WriteFile(handle,ptr,size*count,&NumberOfBytesWritten,NULL);
	bytesRead+=(mp_uint32)NumberOfBytesWritten;
	if (!bResult) return -1;
	return (mp_sint32)NumberOfBytesWritten;
	FlushFileBuffers(handle);
#else
	// Buffer to be written is bigger than our internal cache
	// => Write through
	if (size*count > BUFFERSIZE)
	{
		// Flush first
		flush();
		unsigned long NumberOfBytesWritten;
		bool bResult = (bool)WriteFile(handle,ptr,size*count,&NumberOfBytesWritten,NULL);
		bytesRead+=(mp_uint32)NumberOfBytesWritten;
		if (!bResult) return -1;
		return (mp_sint32)NumberOfBytesWritten;
	}

	// Buffer to be written still fits into our cache buffer
	if (size*count + currentCacheBufferPtr <= cacheBuffer+BUFFERSIZE)
	{
		// Copy into cache
		memcpy(currentCacheBufferPtr, ptr, size*count);
		// Advance current cache ptr
		currentCacheBufferPtr += size*count;
	}
	else
	{
		// Buffer doesn't fit, flush buffer first
		flush();
		// Copy into cache
		memcpy(currentCacheBufferPtr, ptr, size*count);
		// Advance 
		currentCacheBufferPtr += size*count;
	}
	
	if (currentCacheBufferPtr == cacheBuffer + BUFFERSIZE)
		flush();

	bytesRead+=size*count;
	return size*count;
#endif
}

void XMFile::seek(mp_uint32 pos, SeekOffsetTypes seekOffsetType/* = SeekOffsetTypeStart*/)
{
	if (writeAccess)
	{
		flush();
	}

	DWORD moveMethod = FILE_BEGIN;

	if (seekOffsetType == XMFile::SeekOffsetTypeCurrent)
		moveMethod = FILE_CURRENT;
	else if (seekOffsetType == XMFile::SeekOffsetTypeEnd)
		moveMethod = FILE_END;
	
	SetFilePointer(handle, pos, NULL, moveMethod);
}

mp_uint32 XMFile::pos()
{
	return SetFilePointer(handle, 0, NULL, FILE_CURRENT);
}

mp_uint32 XMFile::size()
{
	mp_uint32 size = 0;
	mp_uint32 curPos = pos();
	SetFilePointer(handle, 0, NULL, FILE_END);
	size = pos();
	seek(curPos);
	return size;
}

bool XMFile::remove(const SYSCHAR* file)
{
	return DeleteFile(file);
}

bool XMFile::exists(const SYSCHAR* file)
{
	HANDLE handle = CreateFile(file,
					    GENERIC_READ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL, 
						NULL);

	bool res = handle != INVALID_HANDLE_VALUE;
	if (res)
		CloseHandle(handle);
	return res;
}

const char* XMFile::getFileNameASCII()
{
	const SYSCHAR* ptr = fileName+_tcslen(fileName);
	
	while (*ptr != '\\' && ptr > fileName)
		ptr--;
		
	if (*ptr == '\\') ptr++;
	
	fileNameASCII = new char[_tcslen(ptr)+1];
	
	for (mp_uint32 i = 0; i <= _tcslen(ptr); i++)
		fileNameASCII[i] = (char)ptr[i];

	//strcpy(fileNameASCII, ptr);
	
	return fileNameASCII;
}

//////////////////////////////////////////////////////////////////////////
// C compatible implentation											//
//////////////////////////////////////////////////////////////////////////
#else

#include <unistd.h>

XMFile::XMFile(const SYSCHAR*	fileName, bool writeAccess /* = false*/) :
	XMFileBase(),
	fileName(fileName),
	fileNameASCII(NULL),
	cacheBuffer(NULL)
{
	this->writeAccess = writeAccess;

	bytesRead = 0;	
	handle = fopen(fileName,writeAccess?"wb":"rb");

	//ASSERT(handle != NULL);
	
	if (writeAccess)
	{
		cacheBuffer = new mp_ubyte[BUFFERSIZE];
		currentCacheBufferPtr = cacheBuffer;
	}
}

XMFile::~XMFile()
{
	if (writeAccess && handle != NULL)
		flush();
	
	if (handle != NULL)
		fclose(handle);
	
	if (fileNameASCII)
		delete[] fileNameASCII;
		
	if (cacheBuffer)
		delete[] cacheBuffer;
}

bool XMFile::isOpen()
{
	return handle != NULL;
}

mp_sint32 XMFile::read(void* ptr, mp_sint32 size, mp_sint32 count)
{
	unsigned long NumberOfBytesRead = fread(ptr,size,count,handle)*size;
	bytesRead += NumberOfBytesRead;
	return (mp_sint32)NumberOfBytesRead;
}

void XMFile::flush()
{
	fwrite(cacheBuffer, 1, currentCacheBufferPtr-cacheBuffer, handle);
	currentCacheBufferPtr = cacheBuffer;
}

mp_sint32 XMFile::write(const void* ptr, mp_sint32 size, mp_sint32 count)
{
	// Buffer to be written is bigger than our internal cache
	// => Write through
	if (size*count > BUFFERSIZE)
	{
		// Flush first
		flush();
		unsigned long NumberOfBytesWritten = fwrite(ptr,size,count,handle)*size;
		bytesRead += NumberOfBytesWritten;
		return (mp_sint32)NumberOfBytesWritten;
	}

	// Buffer to be written still fits into our cache buffer
	if (size*count + currentCacheBufferPtr <= cacheBuffer+BUFFERSIZE)
	{
		// Copy into cache
		memcpy(currentCacheBufferPtr, ptr, size*count);
		// Advance current cache ptr
		currentCacheBufferPtr += size*count;
	}
	else
	{
		// Buffer doesn't fit, flush buffer first
		flush();
		// Copy into cache
		memcpy(currentCacheBufferPtr, ptr, size*count);
		// Advance 
		currentCacheBufferPtr += size*count;
	}
	
	if (currentCacheBufferPtr == cacheBuffer + BUFFERSIZE)
		flush();

	bytesRead+=size*count;
	return size*count;
}

void XMFile::seek(mp_uint32 pos, SeekOffsetTypes seekOffsetType/* = SeekOffsetTypeStart*/)
{
	if (writeAccess)
	{
		flush();
		fflush(handle);
	}
		
	int moveMethod = SEEK_SET;

	if (seekOffsetType == XMFile::SeekOffsetTypeCurrent)
		moveMethod = SEEK_CUR;
	else if (seekOffsetType == XMFile::SeekOffsetTypeEnd)
		moveMethod = SEEK_END;

	fseek(handle,pos,moveMethod);
}

mp_uint32 XMFile::pos()
{
	return static_cast<mp_uint32>(ftell(handle));
}

mp_uint32 XMFile::size()
{
	mp_uint32 size = 0;
	mp_uint32 curPos = pos();
	fseek(handle,0,SEEK_END);
	size = pos();
	seek(curPos);
	return size;
}

bool XMFile::exists(const SYSCHAR* file)
{
	FHANDLE handle = fopen(file,"rb");
	bool res = handle != NULL;
	if (res)
		fclose(handle);
	return res;
}

bool XMFile::remove(const SYSCHAR* file)
{
	return unlink(file) == 0;
}

const char* XMFile::getFileNameASCII()
{
	const SYSCHAR* ptr = fileName+strlen(fileName);
	
	while (*ptr != '/' && ptr > fileName)
		ptr--;
		
	if (*ptr == '/') ptr++;
	
	fileNameASCII = new char[strlen(ptr)+1];
	
	strcpy(fileNameASCII, ptr);
	
	return fileNameASCII;
}

#endif
