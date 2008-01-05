/* ppcrack 0.1 - decrypts PowerPacker encrypted data files with brute force
 * by Stuart Caie <kyzer@4u.net>, this software is in the Public Domain
 *
 * The whole keyspace is scanned, unless you supply the -key argument, where
 * that key (in hexadecimal) to key FFFFFFFF is scanned.
 *
 * Anything which decrypts then decrunches to valid data is saved to disk
 * as <original filename>.<decryption key>
 *
 * As a bonus, if any file is a PowerPacker data file, but not encrypted,
 * it will be decrunched anyway, and saved as <original filename>.decrunched
 *
 * - changed to work with UADE (mld) 
 *   Thanks to Kyzer for help and support.
 */

/* Code from Heikki Orsila's amigadepack 0.02 to replace previous
 * PowerPack depacker with license issues.
 *
 * You'll probably want to use ppcrack stand-alone to crack encrypted
 * powerpack files once instead of using brute force at each replay.
 *
 * $Id: ppdepack.c,v 1.2 2007/10/08 16:38:29 cmatsuoka Exp $
 *
 * Modified for xmp by Claudio Matsuoka, 08/2007
 * - merged mld's checks from the old depack sources. Original credits:
 *   - corrupt file and data detection
 *     (thanks to Don Adan and Dirk Stoecker for help and infos)
 *   - implemeted "efficiency" checks
 *   - further detection based on code by Georg Hoermann
 */
 
/* changed into class to work with milkytracker
 */

#ifndef __PP20_H__
#define __PP20_H__

#include "BasicTypes.h"

class PP20
{
public:
    
    PP20();
	
    bool isCompressed(const void* source, const pp_uint32 size);
    
    // If successful, allocates a new buffer containing the
    // uncompresse data and returns the uncompressed length.
    // Else, returns 0.
    pp_uint32 decompress(const void* source, 
						  pp_uint32 size,
						  pp_uint8** destRef);
    
private:
	bool checkEfficiency(const void* source);

	pp_int32 ppDecrunch(pp_uint8 *src, pp_uint8 *dest, pp_uint8 *offset_lens,
				   pp_uint32 src_len, pp_uint32 dest_len, pp_uint8 skip_bits);

	pp_int32 ppValidate(pp_uint8 *src, pp_uint8 *offset_lens,
				   pp_uint32 src_len, pp_uint32 dest_len, pp_uint8 skip_bits);
	
	pp_int32 ppcrack(pp_uint8** destRef, pp_uint8 *data, pp_uint32 len);

	pp_int32 ppdepack(pp_uint8 *src, pp_uint32 s, pp_uint8** destRef);

    static const char* PP_ID;
	pp_uint8 efficiency[4];
    bool globalError;           // exception-free version of code
	
	pp_uint32 key_start;
};

#endif  /* PP_DECOMPRESSOR_H */

