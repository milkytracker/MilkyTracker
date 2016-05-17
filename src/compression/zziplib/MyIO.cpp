/*
 *  MyIO.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.12.07.
 *
 */

#include "MyIO.h"
#include "XMFile.h"

#define FILE_TABLE_ENTRIES 256
static XMFile* file_table[FILE_TABLE_ENTRIES];

int Myopen(const zzip_char_t* name, int flags, ...)
{
	XMFile* f = new XMFile((SYSCHAR*)name, (flags & O_WRONLY) ? true : false);
	if (f->isOpen())
	{
		// Find free space in the file table
		//  zziplib has a bug where it forbid using fd 0, so skip it
		for (int i = 1; i < FILE_TABLE_ENTRIES; i++)
		{
			if (file_table[i] == NULL)
			{
				file_table[i] = f;
				return i;
			}
		}
	}

	delete f;
	return -1;
}

int Myclose(int fd)
{
	if (fd == -1)
		return -1;

	delete file_table[fd];
	file_table[fd] = NULL;
	return 0;
}

zzip_ssize_t Myread(int fd, void *buffer, zzip_size_t count)
{
	if (fd == -1)
		return -1;

	return file_table[fd]->read(buffer, 1, static_cast<mp_sint32> (count));
}

zzip_off_t Mylseek(int fd, zzip_off_t offset, int origin)
{
	if (fd == -1)
		return -1;

	XMFile::SeekOffsetTypes moveMethod = XMFile::SeekOffsetTypeStart;

	if (origin == SEEK_CUR)
		moveMethod = XMFile::SeekOffsetTypeCurrent;
	else if (origin == SEEK_END)
		moveMethod = XMFile::SeekOffsetTypeEnd;

	file_table[fd]->seek(offset, moveMethod);
	return file_table[fd]->pos();
}

zzip_off_t Myfsize(int fd)
{
	if (fd == -1)
		return -1;
	
	return file_table[fd]->size();
}
