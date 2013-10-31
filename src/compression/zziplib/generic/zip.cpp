/*
 * Author: 
 *      Guido Draheim <guidod@gmx.de>
 *      Tomi Ollila <too@iki.fi>
 *
 * Copyright (c) 1999,2000,2001,2002,2003 Guido Draheim
 *          All rights reserved,
 *          use under the restrictions of the 
 *          Lesser GNU General Public License
 *          or alternatively the restrictions 
 *          of the Mozilla Public License 1.1
 */

#include <zzip_lib.h>                                  /* archive handling */
#include <zzip_file.h>
#include <zzip_format.h>
#include <zzip_fetch.h>

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#ifdef ZZIP_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <zzip___mmap.h>
#include <zzip___debug.h>

#define __sizeof(X) ((zzip_ssize_t)(sizeof(X)))

/* per default, we use a little hack to correct bad z_rootseek parts */
#define ZZIP_CORRECT_ROOTSEEK 1

#if (__GNUC__ == 3 && __GNUC_MINOR__ >= 3) || (__GNUC__ >= 4)
# ifdef DEBUG
# warning suppress a warning where the compiler should have optimized instead.
# endif
#define _255 254
#else
#define _255 255
#endif

#define ZZIP_DISK64_TRAILER

#ifdef ZZIP_DISK64_TRAILER
struct _disk_trailer {
    void*         zz_tail;
    void*         zz_for_correct_rootseek; // ZZIP_CORRECT_ROOTSEEK
    zzip_off64_t  zz_entries; 
    zzip_off64_t  zz_finalentries;
    zzip_off64_t  zz_rootseek;
    zzip_off64_t  zz_rootsize;
};
#define _disk_trailer_entries(__p) ((__p)->zz_entries)
#define _disk_trailer_localentries(__p) ((__p)->zz_entries)
#define _disk_trailer_finalentries(__p) ((__p)->zz_entries)
#define _disk_trailer_rootseek(__p) ((__p)->zz_rootseek)
#define _disk_trailer_rootsize(__p) ((__p)->zz_rootsize)
#define _disk_trailer_set_rootseek(__p,__v) ((__p)->rootseek = (__v))

#else
#define _disk_trailer zzip_disk_trailer
#define _disk_trailer_entries zzip_disk_trailer_entries
#define _disk_trailer_localentries zzip_disk_trailer_localentries
#define _disk_trailer_finalentries zzip_disk_trailer_finalentries
#define _disk_trailer_rootseek zzip_disk_trailer_rootseek
#define _disk_trailer_rootsize zzip_disk_trailer_rootsize
#define _disk_trailer_set_rootseek zzip_disk_trailer_set_rootseek
#define __zzip_fetch_disk_trailer __zzip_find_disk_trailer
#endif

/* ---------------------------  internals  -------------------------------- */
/* internal functions of zziplib, avoid at all cost, changes w/o warning.
 * we do export them for debugging purpose and special external tools
 * which know what they do and which can adapt from version to version
 */

int __zzip_fetch_disk_trailer( __zzipfd fd, zzip_off_t filesize, 
			      struct _disk_trailer * _zzip_restrict trailer,
			      zzip_plugin_io_t io);
int __zzip_parse_root_directory( __zzipfd fd, 
				 struct _disk_trailer * trailer, 
				 struct zzip_dir_hdr ** hdr_return,
				 zzip_plugin_io_t io);

_zzip_inline char* __zzip_aligned4(char* p);

/* ------------------------  harden routines ------------------------------ */

#ifdef ZZIP_HARDEN
/*
 * check for inconsistent values in trailer and prefer lower seek value
 * - we fix values assuming the root directory was written at the end
 * and it is just before the zip trailer. Therefore, ...
 */
_zzip_inline static void __fixup_rootseek(
    zzip_off_t offset_of_trailer,
    struct _disk_trailer* trailer)
{
    if (                    _disk_trailer_rootseek (trailer) >
	offset_of_trailer - _disk_trailer_rootsize (trailer) &&
	offset_of_trailer > _disk_trailer_rootsize (trailer))
    {
	register zzip_off_t offset;
	offset = offset_of_trailer - _disk_trailer_rootsize (trailer);
	_disk_trailer_set_rootseek (trailer, offset);
	HINT2("new rootseek=%li", (long) _disk_trailer_rootseek (trailer));
    }
}
#define __correct_rootseek(A,B,C)

#elif defined ZZIP_CORRECT_ROOTSEEK
/* store the seekvalue of the trailer into the "z_magic" field and with 
 * a 64bit off_t we overwrite z_disk/z_finaldisk as well. If you change
 * anything in zziplib or dump the trailer structure then watch out that
 * these are still unused, so that this code may still (ab)use those. */
#define __fixup_rootseek(_offset_of_trailer, _trailer)          \
                      *(zzip_off_t*)_trailer = _offset_of_trailer;
#define __correct_rootseek( _u_rootseek, _u_rootsize, _trailer) \
    if (_u_rootseek > *(zzip_off_t*)_trailer - _u_rootsize)     \
	_u_rootseek = *(zzip_off_t*)_trailer - _u_rootsize;
#else
#define __fixup_rootseek(A,B) 
#define __correct_rootseek(A,B,C)
#endif


#ifdef DEBUG
_zzip_inline static void __debug_dir_hdr (struct zzip_dir_hdr* hdr)
{
    if (sizeof(struct zzip_dir_hdr) > sizeof(struct zzip_disk_entry))
    { WARN1("internal sizeof-mismatch may break wreakage"); }
    /*  the internal directory structure is never bigger than the
     *  external zip central directory space had been beforehand
     *  (as long as the following assertion holds...) 
     */

    if (((zzip_off_t)hdr)&3)
    { NOTE1("this machine's malloc(3) returns sth. not u32-aligned"); }
    /* we assume that if this machine's malloc has returned a non-aligned 
     * memory block, then it is actually safe to access misaligned data, and 
     * since it does only affect the first hdr it should not even bring about
     * too much of that cpu's speed penalty
     */
}
#else
#define __debug_dir_hdr(X)
#endif

/* -------------------------- low-level interface -------------------------- */

#if defined BUFSIZ 
#if BUFSIZ == 1024 || BUFSIZ == 512 || BUFSIZ == 256
#define ZZIP_BUFSIZ BUFSIZ
#endif
#endif

#ifndef ZZIP_BUFSIZ
#define ZZIP_BUFSIZ 512
/* #define ZZIP_BUFSIZ 64 */ /* for testing */
#endif

/**
 * This function is used by => zzip_file_open. It tries to find
 * the zip's central directory info that is usually a few
 * bytes off the end of the file.
 */
int 
__zzip_fetch_disk_trailer(__zzipfd fd, zzip_off_t filesize, 
			  struct _disk_trailer * _zzip_restrict trailer,
			  zzip_plugin_io_t io)
{
#ifdef DEBUG
#define return(val) { e=val; HINT2("%s", zzip_strerror(e)); goto cleanup; }
#else
#define return(val) { e=val; goto cleanup; }
#endif
    register int e;
    
#ifndef _LOWSTK
    char buffer[2*ZZIP_BUFSIZ];
    char* buf = buffer;
#else
    char* buf = malloc(2*ZZIP_BUFSIZ);
#endif
    zzip_off_t offset = 0;
    zzip_ssize_t maplen = 0; /* mmap(),read(),getpagesize() use size_t !! */
    char* fd_map = 0;

    if (!trailer)
        { return(EINVAL); }
  
    if (filesize < __sizeof(struct zzip_disk_trailer))
        { return(ZZIP_DIR_TOO_SHORT); }
          
    if (!buf)
        { return(ZZIP_OUTOFMEM); }

    offset = filesize; /* a.k.a. old offset */
    while(1) /* outer loop */
    {
        register unsigned char* mapped;

	 if (offset <= 0) { return(ZZIP_DIR_EDH_MISSING); }

	 /* trailer cannot be farther away than 64K from fileend */
         if (filesize-offset > 64*1024) 
             { return(ZZIP_DIR_EDH_MISSING); }

	/* the new offset shall overlap with the area after the old offset! */
        if (USE_MMAP && io->fd.sys)
        {
	    zzip_off_t mapoff = offset;
	    { 
		zzip_ssize_t pagesize = _zzip_getpagesize (io->fd.sys);
		if (pagesize < ZZIP_BUFSIZ) goto non_mmap; /* an error? */
		if (mapoff == filesize && filesize > pagesize) 
		    mapoff -= pagesize;
		if (mapoff < pagesize) {
		    maplen = (zzip_ssize_t)mapoff + pagesize; mapoff = 0;
		} else {               
		    mapoff -= pagesize; maplen = 2*pagesize; 
		    if ((zzip_ssize_t)mapoff & (pagesize-1)) { /*only 1. run */
			pagesize -= (zzip_ssize_t)mapoff & (pagesize-1);
			mapoff += pagesize;
			maplen -= pagesize;
		    }   
		}
		if (mapoff + maplen > filesize) maplen = filesize - mapoff;
	    }

            fd_map = _zzip_mmap(io->fd.sys, fd, mapoff, (zzip_size_t)maplen);
            if (fd_map == MAP_FAILED) goto non_mmap;
	    mapped = (unsigned char*) fd_map; offset = mapoff; /* success */
	    HINT3("mapped *%p len=%li", fd_map, (long) maplen);
        } else {
        non_mmap:
	    fd_map = 0; /* have no mmap */
	    {
		zzip_off_t pagesize = ZZIP_BUFSIZ;
		if (offset == filesize && filesize > pagesize)
		    offset -= pagesize;
		if (offset < pagesize) {
		    maplen = (zzip_ssize_t)offset + pagesize; offset = 0;
		} else {
		    offset -= pagesize; maplen = 2*pagesize;
		    if ((zzip_ssize_t)offset & (pagesize-1)) { /*on 1st run*/
			pagesize -= (zzip_ssize_t)offset & (pagesize-1);
			offset += pagesize;
			maplen -= pagesize; 
		    }    
		}
		if (offset + maplen > filesize) maplen = filesize - offset;
	    }
	    
            if (io->fd.seeks(fd, offset, SEEK_SET) < 0)
                { return(ZZIP_DIR_SEEK); }
            if (io->fd.read(fd, buf, (zzip_size_t)maplen) < maplen)
                { return(ZZIP_DIR_READ); }
            mapped = (unsigned char*) buf; /* success */
	    HINT5("offs=$%lx len=%li filesize=%li pagesize=%i", 
		  (long)offset, (long)maplen, (long)filesize, ZZIP_BUFSIZ);
        }

	{/* now, check for the trailer-magic, hopefully near the end of file */
	    register unsigned char* end = mapped + maplen;
	    register unsigned char* tail;
	    for (tail = end-1; (tail >= mapped); tail--)
	    {
		if ((*tail == 'P') && /* quick pre-check for trailer magic */
		    end-tail >= __sizeof(struct zzip_disk_trailer)-2 &&
		    zzip_disk_trailer_check_magic(tail))
		{
#                  ifndef ZZIP_DISK64_TRAILER
		    /* if the file-comment is not present, it happens
		       that the z_comment field often isn't either */
		    if (end-tail >= __sizeof(*trailer))
		    {
			memcpy (trailer, tail, sizeof(*trailer)); 
		    }else{
			memcpy (trailer, tail, sizeof(*trailer)-2);
			trailer->z_comment[0] = 0; 
			trailer->z_comment[1] = 0;
		    }
#                  else
		    struct zzip_disk_trailer* orig = (struct zzip_disk_trailer*) tail;
		    trailer->zz_tail = tail;
		    trailer->zz_entries = zzip_disk_trailer_localentries (orig);
		    trailer->zz_finalentries = zzip_disk_trailer_finalentries (orig);
		    trailer->zz_rootseek = zzip_disk_trailer_rootseek (orig);
		    trailer->zz_rootsize = zzip_disk_trailer_rootsize (orig);
#                  endif

		    __fixup_rootseek (offset + tail-mapped, trailer);
		    { return(0); }
		} else if ((*tail == 'P') && 
		    end-tail >= __sizeof(struct zzip_disk64_trailer)-2 &&
		    zzip_disk64_trailer_check_magic(tail))
		{
#                  ifndef ZZIP_DISK64_TRAILER
		    return(ZZIP_DIR_LARGEFILE);
#                  else
		    struct zzip_disk64_trailer* orig = (struct zzip_disk64_trailer*) tail;
		    trailer->zz_tail = tail;
		    trailer->zz_entries = zzip_disk64_trailer_localentries (orig);
		    trailer->zz_finalentries = zzip_disk64_trailer_finalentries (orig);
		    trailer->zz_rootseek = zzip_disk64_trailer_rootseek (orig);
		    trailer->zz_rootsize = zzip_disk64_trailer_rootsize (orig);
		    { return(0); }
#                  endif
		}
	    }
        }
        
         if (USE_MMAP && fd_map) 
	 { 
	     HINT3("unmap *%p len=%li",  fd_map, (long) maplen);
	     _zzip_munmap(io->fd.sys, fd_map, (zzip_size_t)maplen); 
	     fd_map = 0; 
	 }
    } /*outer loop*/
               
 cleanup:
    if (USE_MMAP && fd_map)
    { 
	HINT3("unmap *%p len=%li",  fd_map, (long) maplen);
	_zzip_munmap(io->fd.sys, fd_map, (zzip_size_t)maplen); 
    }
#   ifdef _LOWSTK
    free(buf);
#   endif
#   undef return
    return e; 
}

/*
 * making pointer alignments to values that can be handled as structures
 * is tricky. We assume here that an align(4) is sufficient even for
 * 64 bit machines. Note that binary operations are not usually allowed
 * to pointer types but we do need only the lower bits in this implementation,
 * so we can just cast the value to a long value.
 */
_zzip_inline char* __zzip_aligned4(char* p)
{
#define aligned4   __zzip_aligned4
    p += ((long)p)&1;            /* warnings about truncation of a "pointer" */
    p += ((long)p)&2;            /* to a "long int" may be safely ignored :) */
    return p;
}

/**
 * This function is used by => zzip_file_open, it is usually called after
 * => __zzip_find_disk_trailer. It will parse the zip's central directory
 * information and create a zziplib private directory table in
 * memory.
 */
int 
__zzip_parse_root_directory(__zzipfd fd, 
    struct _disk_trailer * trailer, 
    struct zzip_dir_hdr ** hdr_return,
    zzip_plugin_io_t io)
{
    struct zzip_disk_entry dirent;
    struct zzip_dir_hdr * hdr;
    struct zzip_dir_hdr * hdr0;
    uint16_t * p_reclen = 0;
    unsigned long entries;           /* 32bit is enough */
    zzip_off64_t zz_offset;          /* offset from start of root directory */
    char* fd_map = 0; 
    zzip_off64_t zz_fd_gap = 0;
    zzip_off64_t zz_entries  = _disk_trailer_localentries (trailer);   
    zzip_off64_t zz_rootsize = _disk_trailer_rootsize (trailer);  
    zzip_off64_t zz_rootseek = _disk_trailer_rootseek (trailer);
    __correct_rootseek (zz_rootseek, zz_rootsize, trailer);

    hdr0 = (struct zzip_dir_hdr*) malloc(zz_rootsize);
    if (!hdr0) 
        return ZZIP_DIRSIZE;
    hdr = hdr0;                  __debug_dir_hdr (hdr);

    if (USE_MMAP && io->fd.sys)
    {
        zz_fd_gap = zz_rootseek & (_zzip_getpagesize(io->fd.sys)-1) ;
        HINT4(" fd_gap=%ld, mapseek=0x%lx, maplen=%ld", (long)(zz_fd_gap),
	      (long)(zz_rootseek-zz_fd_gap), (long)(zz_rootsize+zz_fd_gap));
        fd_map = _zzip_mmap(io->fd.sys, fd,
			    zz_rootseek-zz_fd_gap, zz_rootsize+zz_fd_gap);
        /* if mmap failed we will fallback to seek/read mode */
        if (fd_map == MAP_FAILED) { 
            NOTE2("map failed: %s",strerror(errno)); 
            fd_map=0; 
	}else{
	    HINT3("mapped *%p len=%li", fd_map, (long)(zz_rootsize+zz_fd_gap));
	}
    }

    for (entries=zz_entries, zz_offset=0; entries; entries--)
    {
        register struct zzip_disk_entry * d;
        uint16_t u_extras, u_comment, u_namlen;

        if (fd_map) 
	{ d = ( struct zzip_disk_entry *)((void*)(fd_map+zz_fd_gap+zz_offset)); } /* fd_map+fd_gap==u_rootseek */
        else
        {
            if (io->fd.seeks(fd, zz_rootseek+zz_offset, SEEK_SET) < 0)
                return ZZIP_DIR_SEEK;
            if (io->fd.read(fd, &dirent, sizeof(dirent)) < __sizeof(dirent))
                return ZZIP_DIR_READ;
            d = &dirent;
        }

	if ((zzip_off64_t)(zz_offset+sizeof(*d)) > zz_rootsize ||
	    (zzip_off64_t)(zz_offset+sizeof(*d)) < 0)
	{ FAIL4("%li's entry stretches beyond root directory (O:%li R:%li)", 
		(long)entries, (long)(zz_offset), (long)zz_rootsize); break;}

#       if 0 && defined DEBUG
        zzip_debug_xbuf ((unsigned char*) d, sizeof(*d) + 8);
#       endif        
        
        u_extras  = zzip_disk_entry_get_extras (d);
        u_comment = zzip_disk_entry_get_comment (d); 
        u_namlen  = zzip_disk_entry_get_namlen (d); 
        HINT5("offset=0x%lx, size %ld, dirent *%p, hdr %p\n",
	      (long)(zz_offset+zz_rootseek), (long)zz_rootsize, d, hdr);

        /* writes over the read buffer, Since the structure where data is
           copied is smaller than the data in buffer this can be done.
           It is important that the order of setting the fields is considered
           when filling the structure, so that some data is not trashed in
           first structure read.
           at the end the whole copied list of structures  is copied into
           newly allocated buffer */
        hdr->d_crc32 = zzip_disk_entry_get_crc32 (d);
        hdr->d_csize = zzip_disk_entry_get_csize (d); 
        hdr->d_usize = zzip_disk_entry_get_usize (d);
        hdr->d_off   = zzip_disk_entry_get_offset (d);
        hdr->d_compr = zzip_disk_entry_get_compr (d);
	if (hdr->d_compr > _255) hdr->d_compr = 255;

	if ((zzip_off64_t)(zz_offset+sizeof(*d) + u_namlen) > zz_rootsize ||
	    (zzip_off64_t)(zz_offset+sizeof(*d) + u_namlen) < 0)
	{ FAIL4("%li's name stretches beyond root directory (O:%li N:%li)", 
		(long)entries, (long)(zz_offset), (long)(u_namlen)); break; }

	if (fd_map) 
	{  memcpy(hdr->d_name, fd_map+zz_fd_gap + zz_offset+sizeof(*d), u_namlen); }
	else { io->fd.read(fd, hdr->d_name, u_namlen); }
        hdr->d_name[u_namlen] = '\0'; 
        hdr->d_namlen = u_namlen;
    
        /* update offset by the total length of this entry -> next entry */
        zz_offset += sizeof(*d) + u_namlen + u_extras + u_comment;
    
        if (zz_offset > zz_rootsize)
	{ FAIL3("%li's entry stretches beyond root directory (O:%li)", 
		(long)entries, (long)(zz_offset)); entries--; break; }

        HINT5("file %ld { compr=%d crc32=$%x offset=%d", 
	      (long)entries,  hdr->d_compr, hdr->d_crc32, hdr->d_off);
        HINT5("csize=%d usize=%d namlen=%d extras=%d", 
	      hdr->d_csize, hdr->d_usize, u_namlen, u_extras);
        HINT5("comment=%d name='%s' %s <sizeof %d> } ", 
	      u_comment, hdr->d_name, "",(int) sizeof(*d));
  
        p_reclen = &hdr->d_reclen;
    
        {   register char* p = (char*) hdr; 
            register char* q = aligned4 (p + sizeof(*hdr) + u_namlen + 1);
            *p_reclen = (uint16_t)(q - p);
            hdr = (struct zzip_dir_hdr*) q;
        }
    }/*for*/
    
    if (USE_MMAP && fd_map) 
    {
	HINT3("unmap *%p len=%li",   fd_map, (long)(zz_rootsize+zz_fd_gap));
        _zzip_munmap(io->fd.sys, fd_map, zz_rootsize+zz_fd_gap);
    }
    
    if (p_reclen)
    {
	*p_reclen = 0; /* mark end of list */
    
	if (hdr_return) 
	    *hdr_return = hdr0;
    } /* else zero (sane) entries */
    return (entries ?  ZZIP_CORRUPTED : 0);
}

/* ------------------------- high-level interface ------------------------- */

#ifndef O_BINARY
#define O_BINARY 0
#endif

static zzip_strings_t* zzip_get_default_ext(void)
{
    static zzip_strings_t ext [] =
    {
       ".zip", ".ZIP", /* common extension */
#  ifdef ZZIP_USE_ZIPLIKES
       ".pk3", ".PK3", /* ID Software's Quake3 zipfiles */
       ".jar", ".JAR", /* Java zipfiles */ 
#  endif
       0
    };

    return ext;
}

/**
 * allocate a new ZZIP_DIR handle and do basic 
 * initializations before usage by => zzip_dir_fdopen
 * => zzip_dir_open => zzip_file_open or through
 * => zzip_open
 * (ext==null flags uses { ".zip" , ".ZIP" } )
 * (io ==null flags use of posix io defaults)
 */
ZZIP_DIR*
zzip_dir_alloc_ext_io (zzip_strings_t* ext, const zzip_plugin_io_t io)
{
    ZZIP_DIR* dir;
    if ((dir = (ZZIP_DIR *)calloc(1, sizeof(*dir))) == NULL)
        return 0; 

    /* dir->fileext is currently unused - so what, still initialize it */
    dir->fileext = ext ? ext : zzip_get_default_ext();
    dir->io = io ? io : zzip_get_default_io ();
    return dir;
}

/** => zzip_dir_alloc_ext_io
 * this function is obsolete - it was generally used for implementation
 * and exported to let other code build on it. It is now advised to
 * use => zzip_dir_alloc_ext_io now on explicitly, just set that second
 * argument to zero to achieve the same functionality as the old style.
 */
ZZIP_DIR*
zzip_dir_alloc (zzip_strings_t* fileext)
{
    return zzip_dir_alloc_ext_io (fileext, 0);
}

/**
 * will free the zzip_dir handle unless there are still 
 * zzip_files attached (that may use its cache buffer).
 * This is the inverse of => zzip_dir_alloc , and both
 * are helper functions used implicitly in other zzipcalls
 * e.g. => zzip_dir_close = zzip_close 
 *
 * returns zero on sucess
 * returns the refcount when files are attached.
 */
int 
zzip_dir_free(ZZIP_DIR * dir)
{
    if (dir->refcount)
        return (dir->refcount); /* still open files attached */

    if ((signed)dir->fd >= 0)      dir->io->fd.close(dir->fd);
    if (dir->hdr0)         free(dir->hdr0);
    if (dir->cache.fp)     free(dir->cache.fp);
    if (dir->cache.buf32k) free(dir->cache.buf32k);
    if (dir->realname)     free(dir->realname);
    free(dir);
    return 0;
}

/** => zzip_dir_free
 * It will also => free(2) the => ZZIP_DIR-handle given. 
 * the counterpart for => zzip_dir_open
 * see also => zzip_dir_free
 */
int 
zzip_dir_close(ZZIP_DIR * dir)
{
    dir->refcount &=~ 0x10000000; /* explicit dir close */
    return zzip_dir_free(dir);
}

/** 
 * used by the => zzip_dir_open and zzip_opendir(2) call. Opens the
 * zip-archive as specified with the fd which points to an
 * already openend file. This function then search and parse
 * the zip's central directory.
 *  
 * NOTE: refcount is zero, so an _open/_close pair will also delete 
 *       this _dirhandle 
 */
ZZIP_DIR * 
zzip_dir_fdopen(__zzipfd fd, zzip_error_t * errcode_p)
{
    return zzip_dir_fdopen_ext_io(fd, errcode_p, 0, 0);
}

static zzip_error_t __zzip_dir_parse (ZZIP_DIR* dir); /* forward */

/** => zzip_dir_fdopen
 * this function uses explicit ext and io instead of the internal 
 * defaults, setting these to zero is equivalent to => zzip_dir_fdopen
 */
ZZIP_DIR * 
zzip_dir_fdopen_ext_io(__zzipfd fd, zzip_error_t * errcode_p,
                       zzip_strings_t* ext, const zzip_plugin_io_t io)
{
    zzip_error_t rv;
    ZZIP_DIR * dir;

    if ((dir = zzip_dir_alloc_ext_io (ext, io)) == NULL)
        { rv = ZZIP_OUTOFMEM; goto error; }

    dir->fd = fd;
    if ((rv = __zzip_dir_parse (dir)))
	goto error;

    dir->hdr = dir->hdr0;
    dir->refcount |= 0x10000000; 
  
    if (errcode_p) *errcode_p = rv;
    return dir;
error:
    if (dir) zzip_dir_free(dir);
    if (errcode_p) *errcode_p = rv;
    return NULL;
}

static zzip_error_t
__zzip_dir_parse (ZZIP_DIR* dir)
{
    zzip_error_t rv;
    zzip_off_t filesize;
    struct _disk_trailer trailer;
    /* if (! dir || dir->fd < 0) 
     *     { rv = EINVAL; goto error; } 
     */

    HINT2("------------------ fd=%i", (int) dir->fd);
    if ((filesize = dir->io->fd.filesize(dir->fd)) < 0)
        { rv = ZZIP_DIR_STAT; goto error; }

    HINT2("------------------ filesize=%ld", (long) filesize);
    if ((rv = (zzip_error_t)__zzip_fetch_disk_trailer(dir->fd, filesize, &trailer, 
                                       dir->io)) != 0)
        { goto error; }
                
    HINT5("directory = { entries= %ld/%ld, size= %ld, seek= %ld } ", 
	  (long) _disk_trailer_localentries (&trailer),
	  (long) _disk_trailer_finalentries (&trailer),
	  (long) _disk_trailer_rootsize (&trailer),
	  (long) _disk_trailer_rootseek (&trailer));
    
    if ( (rv = (zzip_error_t)__zzip_parse_root_directory(dir->fd, &trailer, &dir->hdr0, 
                                           dir->io)) != 0)
        { goto error; }
 error:
    return rv;
}

/**
 * will attach a .zip extension and tries to open it
 * the with => open(2). This is a helper function for
 * => zzip_dir_open, => zzip_opendir and => zzip_open.
 */
int
__zzip_try_open(zzip_char_t* filename, int filemode, 
                zzip_strings_t* ext, zzip_plugin_io_t io)
{
    char file[PATH_MAX];
    __zzipfd fd;
    zzip_size_t len = strlen (filename);
    
    if (len+4 >= PATH_MAX) return -1;
    memcpy(file, filename, len+1);

    if (!io) io = zzip_get_default_io();
    if (!ext) ext = zzip_get_default_ext();

    for ( ; *ext ; ++ext)
    {
        strcpy (file+len, *ext);
        fd = io->fd.open(file, filemode);
        if (fd != (unsigned)-1) return fd;
    }
    return -1;
}    

/**
 * Opens the zip-archive (if available).
 * the two ext_io arguments will default to use posix io and 
 * a set of default fileext that can atleast add .zip ext itself.
 */
ZZIP_DIR* 
zzip_dir_open(zzip_char_t* filename, zzip_error_t* e)
{
    return zzip_dir_open_ext_io (filename, e, 0, 0);
}

/** => zzip_dir_open
 * this function uses explicit ext and io instead of the internal 
 * defaults. Setting these to zero is equivalent to => zzip_dir_open
 */
ZZIP_DIR* 
zzip_dir_open_ext_io(zzip_char_t* filename, zzip_error_t* e,
                     zzip_strings_t* ext, zzip_plugin_io_t io)
{
    __zzipfd fd;

    if (!io) io = zzip_get_default_io();
    if (!ext) ext = zzip_get_default_ext();

    fd = io->fd.open(filename, O_RDONLY|O_BINARY);
    if (fd != (unsigned)-1) 
      { return zzip_dir_fdopen_ext_io(fd, e, ext, io); }
    else
    {
        fd = __zzip_try_open(filename, O_RDONLY|O_BINARY, ext, io);
        if (fd != (unsigned)-1) 
          { return zzip_dir_fdopen_ext_io(fd, e, ext, io); }
        else
        {
            if (e) { *e = ZZIP_DIR_OPEN; } 
            return 0; 
        }
    }
}

/** => zzip_dir_open
 * fills the dirent-argument with the values and 
 * increments the read-pointer of the dir-argument.
 * 
 * returns 0 if there no entry (anymore).
 */
int
zzip_dir_read(ZZIP_DIR * dir, ZZIP_DIRENT * d )
{
    if (! dir || ! dir->hdr || ! d) return 0;

    d->d_compr = dir->hdr->d_compr;
    d->d_csize = dir->hdr->d_csize;
    d->st_size = dir->hdr->d_usize;
    d->d_name  = dir->hdr->d_name;

    if (! dir->hdr->d_reclen) 
    { dir->hdr = 0; }
    else  
    { dir->hdr = (struct zzip_dir_hdr *)((char *)dir->hdr + dir->hdr->d_reclen); }
  
    return 1;
}

/* 
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
