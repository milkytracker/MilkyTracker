/*
 * NOTE: this is part of libzzipfseeko (i.e. it is not libzzip).
 *
 * These routines are fully independent from the traditional zzip
 * implementation. They assume a readonly seekable stdio handle
 * representing a complete zip file. The functions show how to 
 * parse the structure, find files and return a decoded bytestream.
 *
 * These routines are a bit simple and really here for documenting
 * the way to access a zip file. The complexity of zip access comes
 * from staggered reading of bytes and reposition of a filepointer in
 * a big archive with lots of files and long compressed datastreams.
 * Plus varaints of drop-in stdio replacements, obfuscation routines,
 * auto fileextensions, drop-in dirent replacements, and so on...
 *
 * btw, we can _not_ use fgetpos/fsetpos since an fpos_t has no asserted
 * relation to a linear seek value as specified in zip info headers. In
 * general it is not a problem if your system has no fseeko/ftello pair
 * since we can fallback to fseek/ftell which limits the zip disk size
 * to 2MiBs but the zip-storable seek values are 32bit limited anyway.
 *
 * Author: 
 *      Guido Draheim <guidod@gmx.de>
 *
 * Copyright (c) 2003,2004 Guido Draheim
 *          All rights reserved,
 *          use under the restrictions of the 
 *          Lesser GNU General Public License
 *          or alternatively the restrictions 
 *          of the Mozilla Public License 1.1
 */

#define _LARGEFILE_SOURCE 1
#define _ZZIP_ENTRY_STRUCT 1

#include <zzip_types.h>

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>

#if   defined ZZIP_HAVE_STRING_H
#include <string.h>
#elif defined ZZIP_HAVE_STRINGS_H
#include <strings.h>
#endif

#include <zlib.h>
#include <zzip_format.h>
#include <zzip_fseeko.h>
#include <zzip_fetch.h>
#include <zzip___mmap.h>
#include <zzip___fnmatch.h>

#if __STDC_VERSION__+0 > 199900L
#define ___
#define ____
#else
#define ___ {
#define ____ }
#endif

#ifndef ZZIP_HAVE_FSEEKO
#define fseeko fseek
#define ftello ftello
#endif

/* note that the struct zzip_entry inherits the zzip_disk_entry values
 * and usually carries a copy of its values (in disk format!). To make the
 * following code more readable, we use a shorthand notation for the
 * upcast needed in C (not needed in C++) as "disk_(entry)".
 */
#ifdef __zzip_entry_extends_zzip_disk_entry
#define disk_(_entry_) _entry_
#else
#define disk_(_entry_) (& (_entry_)->head)
#endif

/* we try to round all seeks to the pagesize - since we do not use
 * the sys/mmap interface we have to guess a good value here: */
#define PAGESIZE 8192

/* ====================================================================== */
/*                      helper functions                                  */

/** => zzip_entry_data_offset
 * This functions read the correspoding struct zzip_file_header from 
 * the zip disk of the given "entry". The returned off_t points to the
 * end of the file_header where the current fseek pointer has stopped.
 * This is used to immediatly parse out any filename/extras block following
 * the file_header. The return value is null on error.
 */
static zzip_off_t
zzip_entry_fread_file_header (ZZIP_ENTRY* entry, 
			     struct zzip_file_header* file_header)
{
    if (! entry || ! file_header) return 0;
    ___ zzip_off_t offset = zzip_disk_entry_fileoffset (disk_(entry));
    if (0 > offset || offset >= entry->disksize) return 0;

    fseeko (entry->diskfile, offset, SEEK_SET);
    return (fread (file_header, sizeof(*file_header), 1, entry->diskfile)
	    ? offset+sizeof(*file_header) : 0 ); ____;
}

/** helper functions for (fseeko) zip access api
 *
 * This functions returns the seekval offset of the data portion of the
 * file referenced by the given zzip_entry. It requires an intermediate
 * check of the file_header structure (i.e. it reads it from disk). After 
 * this call, the contained diskfile readposition is already set to the 
 * data_offset returned here. On error -1 is returned.
 */
zzip_off_t
zzip_entry_data_offset(ZZIP_ENTRY* entry)
{
    struct zzip_file_header file_header;
    if (! entry) return -1;
    ___ zzip_off_t offset = 
	zzip_entry_fread_file_header (entry, & file_header);
    if (! offset) return -1;
    offset += zzip_file_header_sizeof_tails (& file_header);
    fseeko (entry->diskfile, offset, SEEK_SET);
    return offset; ____;
}

/** => zzip_entry_data_offset
 * This function is a big helper despite its little name: in a zip file the
 * encoded filenames are usually NOT zero-terminated but for common usage
 * with libc we need it that way. Secondly, the filename SHOULD be present
 * in the zip central directory but if not then we fallback to the filename
 * given in the file_header of each compressed data portion.
 */
zzip__new__ char*
zzip_entry_strdup_name(ZZIP_ENTRY* entry)
{
    if (! entry) return 0;

    ___ zzip_size_t len;
    if ((len = zzip_disk_entry_namlen (disk_(entry)))) {
	char* name = (char*)malloc (len+1);
	if (! name) return 0;
	memcpy (name, entry->tail, len);
	name[len] = '\0';
	return name;
    }
    ___ struct zzip_file_header header;
    if (zzip_entry_fread_file_header (entry, &header) 
	&& ( len = zzip_file_header_namlen(&header) )) {
	char* name = (char*)malloc (len+1);
	if (! name) return 0;
	fread (name, 1, len, entry->diskfile);
	name[len] = '\0';
	return name;
    }
    return 0;
    ____;____;
}

static int
prescan_entry(ZZIP_ENTRY* entry) 
{
    assert (entry);
    ___ zzip_off_t tailsize = zzip_disk_entry_sizeof_tails (disk_(entry));
    if (tailsize+1 > entry->tailalloc) {
	char* newtail = (char*)realloc (entry->tail, tailsize+1);
	if (! newtail) return ENOMEM;
	entry->tail = newtail;
	entry->tailalloc = tailsize+1;
    }
    fread (entry->tail, 1, tailsize, entry->diskfile);
    /* name + comment + extras */
    return 0; ____;
}

static void
prescan_clear(ZZIP_ENTRY* entry)
{
    assert (entry);
    if (entry->tail) free (entry->tail);
    entry->tail = 0; entry->tailalloc = 0;
}

/* ====================================================================== */

/** => zzip_entry_findfile
 *
 * This function is the first call of all the zip access functions here.
 * It contains the code to find the first entry of the zip central directory. 
 * Here we require the stdio handle to represent a real zip file where the
 * disk_trailer is _last_ in the file area, so that its position would be at 
 * a fixed offset from the end of the file area if not for the comment field 
 * allowed to be of variable length (which needs us to do a little search
 * for the disk_tailer). However, in this simple implementation we disregard
 * any disk_trailer info telling about multidisk archives, so we just return
 * a pointer to the first entry in the zip central directory of that file.
 * 
 * For an actual means, we are going to search backwards from the end 
 * of the mmaped block looking for the PK-magic signature of a 
 * disk_trailer. If we see one then we check the rootseek value to
 * find the first disk_entry of the root central directory. If we find
 * the correct PK-magic signature of a disk_entry over there then we 
 * assume we are done and we are going to return a pointer to that label.
 *
 * The return value is a pointer to the first zzip_disk_entry being checked
 * to be within the bounds of the file area specified by the arguments. If
 * no disk_trailer was found then null is returned, and likewise we only 
 * accept a disk_trailer with a seekvalue that points to a disk_entry and 
 * both parts have valid PK-magic parts. Beyond some sanity check we try to
 * catch a common brokeness with zip archives that still allows us to find
 * the start of the zip central directory.
 */
zzip__new__ ZZIP_ENTRY*
zzip_entry_findfirst(FILE* disk)
{
    ___ zzip_off_t mapoffs;
    ___ zzip_off_t mapsize;

    if (! disk) return 0;
    fseeko (disk, 0, SEEK_END);
    ___ zzip_off_t disksize = ftell/*o*/ (disk);
    if (disksize < (zzip_off_t) sizeof(struct zzip_disk_trailer)) return 0;
    /* we read out chunks of 8 KiB in the hope to match disk granularity */
    ___ zzip_off_t pagesize = PAGESIZE; /* getpagesize() */
    ___ ZZIP_ENTRY* entry = (ZZIP_ENTRY*)malloc (sizeof(*entry));  if (! entry) return 0;
    ___ unsigned char* buffer = (unsigned char*)malloc (pagesize);    if (! buffer) goto nomem;

    assert (pagesize/2 > (zzip_off_t) sizeof (struct zzip_disk_trailer));
    /* at each step, we will fread a pagesize block which overlaps with the
     * previous read by means of pagesize/2 step at the end of the while(1) */
    mapoffs = disksize &~ (pagesize-1);
    mapsize = disksize - mapoffs;
    if (mapoffs && mapsize < pagesize/2) { 
	mapoffs -= pagesize/2; mapsize += pagesize/2; }
    while(1) {
	fseeko (disk, mapoffs, SEEK_SET);
	fread (buffer, 1, mapsize, disk);
	___ unsigned char* p = 
	    buffer + mapsize - sizeof(struct zzip_disk_trailer);
	for (; p >= buffer ; p--)
	{
	    zzip_off_t root;  /* (struct zzip_disk_entry*) */
	    if (zzip_disk_trailer_check_magic(p)) {
		root = zzip_disk_trailer_rootseek (
		    (struct zzip_disk_trailer*)p);
		if (root > disksize - (long)sizeof(struct zzip_disk_trailer)) {
		    /* first disk_entry is after the disk_trailer? can't be! */
		    zzip_off_t rootsize = zzip_disk_trailer_rootsize (
			(struct zzip_disk_trailer*)p);
		    if (rootsize > mapoffs) continue;
		    /* a common brokeness that can be fixed: we just assume the
		     * central directory was written directly before : */
		    root = mapoffs - rootsize;
		}
	    } else if (zzip_disk64_trailer_check_magic(p)) {
		if (sizeof(zzip_off_t) < 8) return 0;
		root = zzip_disk64_trailer_rootseek (
		    (struct zzip_disk64_trailer*)p);
	    } else continue;

	    assert (0 <= root && root < mapsize);
	    fseeko (disk, root, SEEK_SET);
	    fread (disk_(entry), 1, sizeof(*disk_(entry)), disk);
	    if (zzip_disk_entry_check_magic(entry)) {
		free (buffer);
		entry->headseek = root;
		entry->diskfile = disk;
		entry->disksize = disksize;
		if (prescan_entry(entry)) goto nomem;
		return entry;
	    }
	} ____;
	if (! mapoffs) break;             assert (mapsize >= pagesize/2);
	mapoffs -= pagesize/2;            /* mapsize += pagesize/2; */
	mapsize = pagesize;               /* if (mapsize > pagesize) ... */
	if (disksize - mapoffs > 64*1024) break;
    }
    free (buffer);
 nomem:
    free (entry);                         ____;____;____;____;____;____;
    return 0;
}

/** => zzip_entry_findfile
 *
 * This function takes an existing "entry" in the central root directory
 * (e.g. from zzip_entry_findfirst) and moves it to point to the next entry.
 * On error it returns 0, otherwise the old entry. If no further match is
 * found then null is returned and the entry already free()d. If you want
 * to stop searching for matches before that case then please call 
 * => zzip_entry_free on the cursor struct ZZIP_ENTRY.
 */
zzip__new__ ZZIP_ENTRY*
zzip_entry_findnext(ZZIP_ENTRY* _zzip_restrict entry)
{
	___ zzip_off_t seek;
    if (! entry) return entry;
    if (! zzip_disk_entry_check_magic (entry)) goto err;
    seek = 
	entry->headseek + zzip_disk_entry_sizeto_end (disk_(entry));
    if (seek + (zzip_off_t) sizeof(*disk_(entry)) > entry->disksize) goto err;

    fseeko (entry->diskfile, seek, SEEK_SET);
    fread (disk_(entry), 1, sizeof(*disk_(entry)), entry->diskfile);
    entry->headseek = seek;
    if (! zzip_disk_entry_check_magic (entry)) goto err;
    if (prescan_entry(entry)) goto err;
    return entry;
 err:
    zzip_entry_free (entry);
    return 0; ____;
}

/** => zzip_entry_findfile
 * this function releases the malloc()ed areas needed for zzip_entry, the
 * pointer is invalid afterwards. This function has #define synonyms of
 * zzip_entry_findlast(), zzip_entry_findlastfile(), zzip_entry_findlastmatch()
 */
int
zzip_entry_free(ZZIP_ENTRY* entry)
{
    if (! entry) return 0;
    prescan_clear (entry);
    free (entry);
    return 1;
}

/** search for files in the (fseeko) zip central directory
 *
 * This function is given a filename as an additional argument, to find the 
 * disk_entry matching a given filename. The compare-function is usually 
 * strcmp or strcasecmp or perhaps strcoll, if null then strcmp is used. 
 * - use null as argument for "old"-entry when searching the first 
 * matching entry, otherwise the last returned value if you look for other
 * entries with a special "compare" function (if null then a doubled search
 * is rather useless with this variant of _findfile). If no further entry is
 * found then null is returned and any "old"-entry gets already free()d.
 */
zzip__new__ ZZIP_ENTRY*
zzip_entry_findfile(FILE* disk, char* filename, 
		    ZZIP_ENTRY* _zzip_restrict entry, 
		    zzip_strcmp_fn_t compare)
{
    if (! filename || ! disk) return 0;
    entry = ( ! entry ) ? zzip_entry_findfirst (disk) 
	: zzip_entry_findnext (entry);
    if (! compare) compare = (zzip_strcmp_fn_t)(strcmp);

    for (; entry ; entry = zzip_entry_findnext (entry))
    {	/* filenames within zip files are often not null-terminated! */
	char* realname = zzip_entry_strdup_name (entry);
	if (! realname) continue;
	if (! compare (filename, realname)) {
	    free (realname);    return entry;
	} else {
	    free (realname);    continue;
	}
    }
    return 0;
}

//#ifdef ZZIP_HAVE_FNMATCH_H
//#define _zzip_fnmatch fnmatch
//# ifdef FNM_CASEFOLD
//# define _zzip_fnmatch_CASEFOLD FNM_CASEFOLD
//# else
//# define _zzip_fnmatch_CASEFOLD 0
//# endif
//#else
//# define _zzip_fnmatch_CASEFOLD 0
///* if your system does not have fnmatch, we fall back to strcmp: */
//static int _zzip_fnmatch(char* pattern, char* string, int flags)
//{ 
//    puts ("<zzip:strcmp>");
//    return strcmp (pattern, string); 
//}
//#endif

/** => zzip_entry_findfile
 *
 * This function uses a compare-function with an additional argument
 * and it is called just like fnmatch(3) from POSIX.2 AD:1993), i.e.
 * the argument filespec first and the ziplocal filename second with
 * the integer-flags put in as third to the indirect call. If the
 * platform has fnmatch available then null-compare will use that one
 * and otherwise we fall back to mere strcmp, so if you need fnmatch
 * searching then please provide an implementation somewhere else.
 * - use null as argument for "after"-entry when searching the first 
 * matching entry, or the last disk_entry return-value to find the
 * next entry matching the given filespec. If no further entry is
 * found then null is returned and any "old"-entry gets already free()d.
 */
zzip__new__ ZZIP_ENTRY*
zzip_entry_findmatch(FILE* disk, char* filespec, 
		     ZZIP_ENTRY* _zzip_restrict entry,
		     zzip_fnmatch_fn_t compare, int flags)
{
    if (! filespec || ! disk) return 0;
    entry = ( ! entry ) ? zzip_entry_findfirst (disk) 
	: zzip_entry_findnext (entry);
    if (! compare) compare = (zzip_fnmatch_fn_t) _zzip_fnmatch; 

    for (; entry ; entry = zzip_entry_findnext (entry))
    {	/* filenames within zip files are often not null-terminated! */
	char* realname = zzip_entry_strdup_name (entry);
	if (! realname) continue;
	if (! compare (filespec, realname, flags)) {
	    free (realname);    return entry;
	} else {
	    free (realname);    continue;
	}
    }
    return 0;
}

/* ====================================================================== */

/**
 * typedef struct zzip_disk_file ZZIP_ENTRY_FILE;
 */
struct zzip_entry_file /* : zzip_file_header */
{
    struct zzip_file_header header;    /* fopen detected header */
    ZZIP_ENTRY*   entry;               /* fopen entry */
    zzip_off_t    data;                /* for stored blocks */
    zzip_size_t   avail;               /* memorized for checks on EOF */
    zzip_size_t   compressed;          /* compressed flag and datasize */
    zzip_size_t   dataoff;             /* offset from data start */
    z_stream      zlib;                /* for inflated blocks */
    unsigned char buffer[PAGESIZE];    /* work buffer for inflate algorithm */
};

/** open a file within a zip disk for reading
 *
 * This function does take an "entry" argument and copies it (or just takes
 * it over as owner) to a new ZZIP_ENTRY_FILE handle structure. That
 * structure contains also a zlib buffer for decoding. This function does
 * seek to the file_header of the given "entry" and validates it for the
 * data buffer following it. We do also prefetch some data from the data
 * buffer thereby trying to match the disk pagesize for faster access later.
 * The => zzip_entry_fread will then read in chunks of pagesizes which is
 * the size of the internal readahead buffer. If an error occurs then null
 * is returned.
 */
zzip__new__ ZZIP_ENTRY_FILE*
zzip_entry_fopen (ZZIP_ENTRY* entry, int takeover)
{
    if (! entry) return 0;
    if (! takeover) {
	ZZIP_ENTRY* found = (ZZIP_ENTRY*)malloc (sizeof(*entry));
	if (! found) return 0;
	memcpy (found, entry, sizeof(*entry));   /* prescan_copy */
	found->tail = (char*)malloc (found->tailalloc);
	if (! found->tail) { free (found); return 0; }
	memcpy (found->tail, entry->tail, entry->tailalloc);
	entry = found;
    }
    ___ ZZIP_ENTRY_FILE* file = (ZZIP_ENTRY_FILE*)malloc(sizeof(*file));
    if (! file) goto fail1;
    file->entry = entry;
    if (! zzip_entry_fread_file_header (entry, &file->header))
	goto fail2;
    file->avail = zzip_file_header_usize (&file->header);
    file->data = zzip_entry_data_offset (entry);
    file->dataoff = 0;

    if (! file->avail || zzip_file_header_data_stored (&file->header))
    { file->compressed = 0; return file; }

    file->compressed = zzip_file_header_csize (&file->header);
    file->zlib.opaque = 0;
    file->zlib.zalloc = Z_NULL;
    file->zlib.zfree = Z_NULL;

    ___ zzip_off_t seek = file->data;
    seek += sizeof(file->buffer); seek -= seek & (sizeof(file->buffer)-1);
    assert (file->data < seek); /* pre-read to next PAGESIZE boundary... */
    fseeko (file->entry->diskfile, file->data + file->dataoff, SEEK_SET);
    file->zlib.next_in = file->buffer;
    file->zlib.avail_in = fread (file->buffer, 1, seek - file->data,
				 file->entry->diskfile);
    file->dataoff += file->zlib.avail_in; ____;

    if (! zzip_file_header_data_deflated (&file->header) 
	|| inflateInit2 (& file->zlib, -MAX_WBITS) != Z_OK) goto fail2;

    return file;
 fail2:
    free (file);
 fail1:
    zzip_entry_free (entry);
    return 0; ____;
}

/** => zzip_entry_fopen
 *
 * This function opens a file found by name, so it does a search into
 * the zip central directory with => zzip_entry_findfile and whatever
 * is found first is given to => zzip_entry_fopen
 */
zzip__new__ ZZIP_ENTRY_FILE*
zzip_entry_ffile (FILE* disk, char* filename)
{
    ZZIP_ENTRY* entry = zzip_entry_findfile (disk, filename, 0, 0);
    if (! entry) return 0;
    return zzip_entry_fopen (entry, 1);
}


/** => zzip_entry_fopen
 *
 * This function reads more bytes into the output buffer specified as
 * arguments. The return value is null on eof or error, the stdio-like
 * interface can not distinguish between these so you need to check
 * with => zzip_entry_feof for the difference.
 */
zzip_size_t
zzip_entry_fread (void* ptr, zzip_size_t sized, zzip_size_t nmemb,
		  ZZIP_ENTRY_FILE* file)
{
    if (! file) return 0;
    ___ zzip_size_t size = sized*nmemb;
    if (! file->compressed) {
	if (size > file->avail) size = file->avail;
	fread (ptr, 1, size, file->entry->diskfile);
	file->dataoff += size;
	file->avail -= size;
	return size;
    }
    
    file->zlib.avail_out = size;
    file->zlib.next_out = (Bytef*)ptr;
    ___ zzip_size_t total_old = file->zlib.total_out;
    while (1) {
	if (! file->zlib.avail_in) {   
	    size = file->compressed - file->dataoff;
	    if (size > sizeof(file->buffer)) size = sizeof(file->buffer);
	    /* fseek (file->data + file->dataoff, file->entry->diskfile); */
	    file->zlib.avail_in = fread (file->buffer, 1, size,
					 file->entry->diskfile);
	    file->zlib.next_in = file->buffer;
	    file->dataoff += file->zlib.avail_in;
	}
	if (! file->zlib.avail_in) return 0;
	
	___ int err = inflate (& file->zlib, Z_NO_FLUSH);
	if (err == Z_STREAM_END)
	    file->avail = 0;
	else if (err == Z_OK)
	    file->avail -= file->zlib.total_out - total_old;
	else
	    return 0;
	____;
	if (file->zlib.avail_out && ! file->zlib.avail_in) continue;
	return file->zlib.total_out - total_old;
    }____;____;
}

/** => zzip_entry_fopen
 * This function releases any zlib decoder info needed for decompression
 * and dumps the ZZIP_ENTRY_FILE struct then.
 */
int
zzip_entry_fclose (ZZIP_ENTRY_FILE* file)
{
    if (! file) return 0;
    if (file->compressed)
	inflateEnd (& file->zlib);
    zzip_entry_free (file->entry);
    free (file);
    return 0;
}

/** => zzip_entry_fopen
 *
 * This function allows to distinguish an error from an eof condition. 
 * Actually, if we found an error but we did already reach eof then we
 * just keep on saying that it was an eof, so the app can just continue.
 */ 
int
zzip_entry_feof (ZZIP_ENTRY_FILE* file)
{
    return ! file || ! file->avail;
}
