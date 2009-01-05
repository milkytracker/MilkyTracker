/*
 *  MyIO.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.12.07.
 *
 */

#include "MyIO.h"
#include "XMFile.h"

__zzipfd Myopen(const zzip_char_t* name, int flags, ...)
{
	XMFile* f = new XMFile((SYSCHAR*)name, (flags & O_WRONLY) ? true : false);
	if (!f->isOpen())
	{
		delete f;
		return (__zzipfd)-1;
	}
	//TODO: find a better solution
	ASSERT(sizeof(f) == sizeof(__zzipfd));
	return reinterpret_cast<__zzipfd>(f);
}

int Myclose(__zzipfd fd)
{
	if (fd == (__zzipfd)-1)
		return -1;

	delete reinterpret_cast<XMFile*>(fd);
	return 0;
}

zzip_ssize_t Myread(__zzipfd fd, void *buffer, zzip_size_t count)
{
	if (fd == (__zzipfd)-1)
		return -1;

	return reinterpret_cast<XMFile*>(fd)->read(buffer, 1, count);
}

zzip_off_t Mylseek(__zzipfd fd, zzip_off_t offset, int origin)
{
	if (fd == (__zzipfd)-1)
		return -1;

	XMFile::SeekOffsetTypes moveMethod = XMFile::SeekOffsetTypeStart;

	if (origin == SEEK_CUR)
		moveMethod = XMFile::SeekOffsetTypeCurrent;
	else if (origin == SEEK_END)
		moveMethod = XMFile::SeekOffsetTypeEnd;

	reinterpret_cast<XMFile*>(fd)->seek(offset, moveMethod);
	return reinterpret_cast<XMFile*>(fd)->pos();
}

zzip_off_t Myfsize(__zzipfd fd)
{
	if (fd == (__zzipfd)-1)
		return -1;
	
	return reinterpret_cast<XMFile*>(fd)->size();
}
