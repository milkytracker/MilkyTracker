#ifndef ZZIP_INTERNAL_DIRENT_H
#define ZZIP_INTERNAL_DIRENT_H
#include <zzip_conf.h>

/*
 * DO NOT USE THIS CODE.
 *
 * It is an internal header file for zziplib that carries some inline
 * functions (or just static members) and a few defines, simply to be
 * able to reuse these across - and have everything in a specific place.
 *
 * Copyright (c) 2002,2003 Guido Draheim
 *          All rights reserved,
 *          use under the restrictions of the 
 *          Lesser GNU General Public License
 *          or alternatively the restrictions 
 *          of the Mozilla Public License 1.1
 */

#ifdef ZZIP_HAVE_DIRENT_H
#define USE_DIRENT 1

#define _zzip_opendir   opendir
#define _zzip_readdir   readdir
#define _zzip_closedir  closedir
#define _zzip_rewinddir rewinddir
#define _zzip_telldir   telldir
#define _zzip_seekdir   seekdir
#define _zzip_DIR       DIR

#include <dirent.h>

#elif defined ZZIP_HAVE_WINBASE_H
#define USE_DIRENT 2

#define _zzip_opendir   win32_opendir
#define _zzip_readdir   win32_readdir
#define _zzip_closedir  win32_closedir
#define _zzip_rewinddir win32_rewinddir
#define _zzip_telldir   win32_telldir
#define _zzip_seekdir   win32_seekdir
#define _zzip_DIR       DIR

/*
 * DIRENT.H (formerly DIRLIB.H)
 *
 * by M. J. Weinstein   Released to public domain 1-Jan-89
 *
 * Because I have heard that this feature (opendir, readdir, closedir)
 * it so useful for programmers coming from UNIX or attempting to port
 * UNIX code, and because it is reasonably light weight, I have included
 * it in the Mingw32 package. I have also added an implementation of
 * rewinddir, seekdir and telldir.
 *   - Colin Peters <colin@bird.fu.is.saga-u.ac.jp>
 *
 *  This code is distributed in the hope that is will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAMED. This includeds but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <io.h>

struct dirent
{
	long		d_ino;		/* Always zero. */
	unsigned short	d_reclen;	/* Always zero. */
	unsigned short	d_namlen;	/* Length of name in d_name. */
	char*		d_name;		/* File name. */
	/* NOTE: The name in the dirent structure points to the name in the
	 *       finddata_t structure in the DIR. */
};

/*
 * This is an internal data structure. Good programmers will not use it
 * except as an argument to one of the functions below.
 */
typedef struct
{
	/* disk transfer area for this dir */
	struct _finddata_t	dd_dta;

	/* dirent struct to return from dir (NOTE: this makes this thread
	 * safe as long as only one thread uses a particular DIR struct at
	 * a time) */
	struct dirent		dd_dir;

	/* _findnext handle */
	long			dd_handle;

	/*
         * Status of search:
	 *   0 = not started yet (next entry to read is first entry)
	 *  -1 = off the end
	 *   positive = 0 based index of next entry
	 */
	short			dd_stat;

	/* given path for dir with search pattern (struct is extended) */
	char			dd_name[1];
} DIR;

/*
 * dirent.c
 *
 * Derived from DIRLIB.C by Matt J. Weinstein 
 * This note appears in the DIRLIB.H
 * DIRLIB.H by M. J. Weinstein   Released to public domain 1-Jan-89
 *
 * Updated by Jeremy Bettis <jeremy@hksys.com>
 * Significantly revised and rewinddir, seekdir and telldir added by Colin
 * Peters <colin@fu.is.saga-u.ac.jp>
 */

#include <direct.h>
#include <errno.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define win32_SUFFIX	"*"
#define	win32_SLASH	"\\"

#ifndef S_ISDIR
#define S_ISDIR(m)	((m & S_IFMT) == S_IFDIR)	/* is a directory */
#endif  S_ISDIR


/*
  opendir

  Returns a pointer to a DIR structure appropriately filled in to begin
  searching a directory.
*/
static DIR*
win32_opendir (const char *szPath)
{
    DIR        *nd;
    struct _stat statDir;

    errno = 0;
    
    if (!szPath) {
	errno = EFAULT;
	return (DIR *) 0;
    }

    if (szPath[0] == '\0') {
	errno = ENOTDIR;
	return (DIR *) 0;
    }
    
    /* Attempt to determine if the given path really is a directory. */
    if (_stat (szPath, &statDir)) {
	/* Error, stat should have set an error value. */
	return (DIR *) 0;
    }
    
    if (!S_ISDIR (statDir.st_mode)) {
	/* Error, stat reports not a directory. */
	errno = ENOTDIR;
	return (DIR *) 0;
    }

    /* Allocate enough space to store DIR structure and the complete *
       directory path given. */
    nd = (DIR *) calloc (1, sizeof (DIR) + strlen (szPath) 
			 + strlen (win32_SLASH) + strlen (win32_SUFFIX));
    
    if (!nd) {
	/* Error, out of memory. */
	errno = ENOMEM;
	return (DIR *) 0;
    }

    /* Create the search expression. */
    strcpy (nd->dd_name, szPath);
    
    /* Add on a slash if the path does not end with one. */
    if (nd->dd_name[0] != '\0' &&
	nd->dd_name[strlen (nd->dd_name) - 1] != '/' &&
	nd->dd_name[strlen (nd->dd_name) - 1] != '\\') {
	strcat (nd->dd_name, win32_SLASH);
    }
    
    /* Add on the search pattern */
    strcat (nd->dd_name, win32_SUFFIX);
    
    /* Initialize handle to -1 so that a premature closedir doesn't try * to
       call _findclose on it. */
    nd->dd_handle = -1;
    
    /* Initialize the status. */
    nd->dd_stat = 0;
    
    /* Initialize the dirent structure. ino and reclen are invalid under *
       Win32, and name simply points at the appropriate part of the *
       findfirst_t structure. */
    nd->dd_dir.d_ino = 0;
    nd->dd_dir.d_reclen = 0;
    nd->dd_dir.d_namlen = 0;
    nd->dd_dir.d_name = nd->dd_dta.name;
    
    return nd;
}

/*
  readdir

  Return a pointer to a dirent structure filled with the information on the
  next entry in the directory.
*/
static struct dirent *
win32_readdir (DIR * dirp)
{
    errno = 0;
    
    /* Check for valid DIR struct. */
    if (!dirp) {
	errno = EFAULT;
	return (struct dirent *) 0;
    }
    
    if (dirp->dd_dir.d_name != dirp->dd_dta.name) {
	/* The structure does not seem to be set up correctly. */
	errno = EINVAL;
	return (struct dirent *) 0;
    }

    if (dirp->dd_stat < 0) {
	/* We have already returned all files in the directory * (or the
	   structure has an invalid dd_stat). */
	return (struct dirent *) 0;
    } else if (dirp->dd_stat == 0) {
	/* We haven't started the search yet. */
	/* Start the search */
	dirp->dd_handle = _findfirst (dirp->dd_name, &(dirp->dd_dta));
	
	if (dirp->dd_handle == -1) {
	    /* Whoops! Seems there are no files in that * directory. */
	    dirp->dd_stat = -1;
	} else {
	    dirp->dd_stat = 1;
	}
    } else {
	/* Get the next search entry. */
	if (_findnext (dirp->dd_handle, &(dirp->dd_dta))) {
	    /* We are off the end or otherwise error. */
	    _findclose (dirp->dd_handle);
	    dirp->dd_handle = -1;
	    dirp->dd_stat = -1;
	} else {
	    /* Update the status to indicate the correct * number. */
	    dirp->dd_stat++;
	}
    }
    
    if (dirp->dd_stat > 0) {
	/* Successfully got an entry. Everything about the file is * already
	   appropriately filled in except the length of the * file name. */
	dirp->dd_dir.d_namlen = (unsigned short) strlen (dirp->dd_dir.d_name);
	return &dirp->dd_dir;
    }
    
    return (struct dirent *) 0;
}

/*
  closedir

  Frees up resources allocated by opendir.
*/
static int
win32_closedir (DIR * dirp)
{
    int         rc;

    errno = 0;
    rc = 0;

    if (!dirp) {
	errno = EFAULT;
	return -1;
    }
    
    if (dirp->dd_handle != -1) {
	rc = _findclose (dirp->dd_handle);
    }
    
    /* Delete the dir structure. */
    free (dirp);
    
    return rc;
}

/*
  rewinddir

  Return to the beginning of the directory "stream". We simply call findclose
  and then reset things like an opendir.
*/
static void
win32_rewinddir (DIR * dirp)
{
    errno = 0;

    if (!dirp) {
	errno = EFAULT;
	return;
    }
    
    if (dirp->dd_handle != -1) {
	_findclose (dirp->dd_handle);
    }
    
    dirp->dd_handle = -1;
    dirp->dd_stat = 0;
}

/*
  telldir

  Returns the "position" in the "directory stream" which can be used with
  seekdir to go back to an old entry. We simply return the value in stat.
*/
static long
win32_telldir (DIR * dirp)
{
    errno = 0;

    if (!dirp) {
	errno = EFAULT;
	return -1;
    }
    return dirp->dd_stat;
}

/*
  seekdir

  Seek to an entry previously returned by telldir. We rewind the directory
  and call readdir repeatedly until either dd_stat is the position number
  or -1 (off the end). This is not perfect, in that the directory may
  have changed while we weren't looking. But that is probably the case with
  any such system.
*/
static void
win32_seekdir (DIR * dirp, long lPos)
{
    errno = 0;
    
    if (!dirp) {
	errno = EFAULT;
	return;
    }
    
    if (lPos < -1) {
	/* Seeking to an invalid position. */
	errno = EINVAL;
	return;
    } else if (lPos == -1) {
	/* Seek past end. */
	if (dirp->dd_handle != -1) {
	    _findclose (dirp->dd_handle);
	}
	dirp->dd_handle = -1;
	dirp->dd_stat = -1;
    } else {
	/* Rewind and read forward to the appropriate index. */
	win32_rewinddir (dirp);
	
	while ((dirp->dd_stat < lPos) && win32_readdir (dirp));
    }
}

#else
#define USE_DIRENT 0

#define _zzip_opendir(_N_) 0
#define _zzip_readdir(_D_) 0
#define _zzip_closedir(_D_) /* omit return code */
#define _zzip_rewinddir(_D_)
#define _zzip_telldir(_D_) 0
#define _zzip_seekdir(_D_,_V_) /* omit return code */
#define _zzip_DIR void*

/* end of DIRENT implementation */
#endif

/* once */
#endif
