/*
 * Author: 
 *	Guido Draheim <guidod@gmx.de>
 *	Tomi Ollila <Tomi.Ollila@iki.fi>
 *
 * Copyright (c) 1999,2000,2001,2002 Guido Draheim
 * 	    All rights reserved,
 *	    use under the restrictions of the
 *	    Lesser GNU General Public License
 *          or alternatively the restrictions 
 *          of the Mozilla Public License 1.1
 *
 * Description:
 *      although this file is defining a function called zzip_stat it
 *      will not need a real stat(2) exported by the Operating System.
 *      It will just try to fill the fields of the ZZIP_STAT structure
 *      of 
 */

#include <zzip_lib.h>                                   /* exported...*/
#include <zzip_file.h>
#include <string.h>
#include <sys/stat.h>

#define ZZIP_USE_INTERNAL
#include <zzip_info.h>

/**
 * obtain information about a filename in an opened zip-archive without 
 * opening that file first. Mostly used to obtain the uncompressed 
 * size of a file inside a zip-archive. see => zzip_dir_open.
 */
int 
zzip_dir_stat(ZZIP_DIR * dir, zzip_char_t* name, ZZIP_STAT * zs, int flags)
{
    struct zzip_dir_hdr * hdr = dir->hdr0;
    int (*cmp)(zzip_char_t*, zzip_char_t*);

    cmp = (flags & ZZIP_CASEINSENSITIVE) ? strcasecmp : strcmp;

    if (flags & ZZIP_IGNOREPATH)
    {
        char* n = strrchr((char*)name, '/');
        if (n)  name = n + 1;
    }

    if (hdr)
    while (1)
    {
        register char* hdr_name = hdr->d_name;
        if (flags & ZZIP_IGNOREPATH)
        {
            register char* n = strrchr(hdr_name, '/');
            if (n)  hdr_name = n + 1;
        }

	if (! cmp(hdr_name, name))
            break;

	if (! hdr->d_reclen)
	{
            dir->errcode = ZZIP_ENOENT;
            return -1;
	}

	hdr = (struct zzip_dir_hdr *) ((char *)hdr + hdr->d_reclen);
    }

    zs->d_compr = hdr->d_compr;
    zs->d_csize = hdr->d_csize;
    zs->st_size = hdr->d_usize;
    zs->d_name  = hdr->d_name;

    return 0;
}

/** => zzip_dir_stat
 * This function will obtain information about a opened file _within_ a 
 * zip-archive. The file is supposed to be open (otherwise -1 is returned). 
 * The st_size stat-member contains the uncompressed size. The optional 
 * d_name is never set here. 
 */
int zzip_file_stat (ZZIP_FILE* file, ZZIP_STAT* zs)
{
    if (! file) return -1;
    zs->d_compr = file->method;
    zs->d_csize = file->csize;
    zs->st_size = file->usize;
    zs->d_name  = 0;
    return 0;
}

/** => zzip_dir_stat
 * This function will obtain information about a opened file which may be
 * either real/zipped. The file is supposed to be open (otherwise -1 is 
 * returned). The st_size stat-member contains the uncompressed size. 
 * The optional d_name is never set here. For a real file, we do set the
 * d_csize := st_size and d_compr := 0 for meaningful defaults.
 */
int zzip_fstat (ZZIP_FILE* file, ZZIP_STAT* zs)
{
    if (ZZIP_file_real(file))
    {
	struct stat st;
	if (fstat (file->fd, &st) < 0) return -1;
	zs->st_size = st.st_size;
	zs->d_csize = st.st_size;
	zs->d_compr = 0;
	return 0;
    }else{
	return zzip_file_stat (file, zs);
    }
}

/* 
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
