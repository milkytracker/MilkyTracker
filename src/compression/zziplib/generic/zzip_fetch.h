#ifndef _ZZIP_FORMATS_H
#define _ZZIP_FORMATS_H

#include <zzip_types.h>
#include <zzip_format.h> 
#include <zzip_stdint.h>

/* linux knows "byteswap.h" giving us an optimized variant */
#ifdef ZZIP_HAVE_BYTESWAP_H
#include <byteswap.h>
#endif

/* get 16/32 bits from little-endian zip-file to host byteorder */
extern uint32_t __zzip_get32(zzip_byte_t * s) __zzip_attribute__((const));
extern uint16_t __zzip_get16(zzip_byte_t * s) __zzip_attribute__((const));
extern void     __zzip_set32(zzip_byte_t * s, uint32_t v);
extern void     __zzip_set16(zzip_byte_t * s, uint16_t v);

extern uint64_t __zzip_get64(zzip_byte_t * s) __zzip_attribute__((const));
extern void     __zzip_set64(zzip_byte_t * s, uint64_t v);

# define ZZIP_GET64(__p)     (__zzip_get64((__p)))
# define ZZIP_GET32(__p)     (__zzip_get32((__p)))
# define ZZIP_GET16(__p)     (__zzip_get16((__p)))
# define ZZIP_SET64(__p,__x) (__zzip_set64((__p),(__x)))
# define ZZIP_SET32(__p,__x) (__zzip_set32((__p),(__x)))
# define ZZIP_SET16(__p,__x) (__zzip_set16((__p),(__x)))

/* ..................... bitcorrect physical access .................... */

/* zzip_file_header - the local file header */
#define zzip_file_header_get_magic(__p)      ZZIP_GET32((__p)->z_magic)
#define zzip_file_header_set_magic(__p,__x)  ZZIP_SET32((__p)->z_magic,(__x))
#define zzip_file_header_get_flags(__p)      ZZIP_GET16((__p)->z_flags)
#define zzip_file_header_set_flags(__p,__x)  ZZIP_SET16((__p)->z_flags,(__x))
#define zzip_file_header_get_compr(__p)      ZZIP_GET16((__p)->z_compr)
#define zzip_file_header_set_compr(__p,__x)  ZZIP_SET16((__p)->z_compr,(__x))
#define zzip_file_header_get_crc32(__p)      ZZIP_GET32((__p)->z_crc32)
#define zzip_file_header_set_crc32(__p,__x)  ZZIP_SET32((__p)->z_crc32,(__x))
#define zzip_file_header_get_csize(__p)      ZZIP_GET32((__p)->z_csize)
#define zzip_file_header_set_csize(__p,__x)  ZZIP_SET32((__p)->z_csize,(__x))
#define zzip_file_header_get_usize(__p)      ZZIP_GET32((__p)->z_usize)
#define zzip_file_header_set_usize(__p,__x)  ZZIP_SET32((__p)->z_usize,(__x))
#define zzip_file_header_get_namlen(__p)     ZZIP_GET16((__p)->z_namlen)
#define zzip_file_header_set_namlen(__p,__x) ZZIP_SET16((__p)->z_namlen,(__x))
#define zzip_file_header_get_extras(__p)     ZZIP_GET16((__p)->z_extras)
#define zzip_file_header_set_extras(__p,__x) ZZIP_SET16((__p)->z_extras,(__x))
#define zzip_file_header_sizeof_tails(__p) (zzip_file_header_get_namlen(__p)+\
					    zzip_file_header_get_extras(__p) )
#define zzip_file_header_check_magic(__p)  ZZIP_FILE_HEADER_CHECKMAGIC((__p))

/* zzip_file_trailer - data descriptor per file block */
#define zzip_file_trailer_get_magic(__p)     ZZIP_GET32((__p)->z_magic)
#define zzip_file_trailer_set_magic(__p,__x) ZZIP_SET32((__p)->z_magic,(__x))
#define zzip_file_header_get_crc32(__p)      ZZIP_GET32((__p)->z_crc32)
#define zzip_file_trailer_set_crc32(__p,__x) ZZIP_SET32((__p)->z_crc32,(__x))
#define zzip_file_trailer_get_csize(__p)     ZZIP_GET32((__p)->z_csize)
#define zzip_file_trailer_set_csize(__p,__x) ZZIP_SET32((__p)->z_csize,(__x))
#define zzip_file_trailer_get_usize(__p)     ZZIP_GET32((__p)->z_usize)
#define zzip_file_trailer_set_usize(__p,__x) ZZIP_SET32((__p)->z_usize,(__x))
#define zzip_file_trailer_sizeof_tails(__p) 0
#define zzip_file_trailer_check_magic(__p)   ZZIP_FILE_TRAILER_CHECKMAGIC((__p))
/* zzip_disk_entry (currently named zzip_root_dirent) */
#define zzip_disk_entry_get_magic(__p)      ZZIP_GET32((__p)->z_magic)
#define zzip_disk_entry_set_magic(__p,__x)  ZZIP_SET32((__p)->z_magic,(__x))
#define zzip_disk_entry_get_flags(__p)      ZZIP_GET16((__p)->z_flags)
#define zzip_disk_entry_set_flags(__p,__x)  ZZIP_SET16((__p)->z_flags,(__x))
#define zzip_disk_entry_get_compr(__p)      ZZIP_GET16((__p)->z_compr)
#define zzip_disk_entry_set_compr(__p,__x)  ZZIP_SET16((__p)->z_compr,(__x))
#define zzip_disk_entry_get_crc32(__p)      ZZIP_GET32((__p)->z_crc32)
#define zzip_disk_entry_set_crc32(__p,__x)  ZZIP_SET32((__p)->z_crc32,(__x))
#define zzip_disk_entry_get_csize(__p)      ZZIP_GET32((__p)->z_csize)
#define zzip_disk_entry_set_csize(__p,__x)  ZZIP_SET32((__p)->z_csize,(__x))
#define zzip_disk_entry_get_usize(__p)      ZZIP_GET32((__p)->z_usize)
#define zzip_disk_entry_set_usize(__p,__x)  ZZIP_SET32((__p)->z_usize,(__x))
#define zzip_disk_entry_get_namlen(__p)     ZZIP_GET16((__p)->z_namlen)
#define zzip_disk_entry_set_namlen(__p,__x) ZZIP_SET16((__p)->z_namlen,(__x))
#define zzip_disk_entry_get_extras(__p)     ZZIP_GET16((__p)->z_extras)
#define zzip_disk_entry_set_extras(__p,__x) ZZIP_SET16((__p)->z_extras,(__x))
#define zzip_disk_entry_get_comment(__p)     ZZIP_GET16((__p)->z_comment)
#define zzip_disk_entry_set_comment(__p,__x) ZZIP_SET16((__p)->z_comment,(__x))
#define zzip_disk_entry_get_diskstart(__p)     ZZIP_GET16((__p)->z_diskstart)
#define zzip_disk_entry_set_diskstart(__p,__x) ZZIP_SET16((__p)->z_diskstart,(__x))
#define zzip_disk_entry_get_filetype(__p)     ZZIP_GET16((__p)->z_filetype)
#define zzip_disk_entry_set_filetype(__p,__x) ZZIP_SET16((__p)->z_filetype,(__x))
#define zzip_disk_entry_get_filemode(__p)     ZZIP_GET32((__p)->z_filemode)
#define zzip_disk_entry_set_filemode(__p,__x) ZZIP_SET32((__p)->z_filemode,(__x))
#define zzip_disk_entry_get_offset(__p)     ZZIP_GET32((__p)->z_offset)
#define zzip_disk_entry_set_offset(__p,__x) ZZIP_SET32((__p)->z_offset,(__x))
#define zzip_disk_entry_sizeof_tails(__p) (zzip_disk_entry_get_namlen(__p) +\
					   zzip_disk_entry_get_extras(__p) +\
					   zzip_disk_entry_get_comment(__p) )
#define zzip_disk_entry_check_magic(__p)  ZZIP_DISK_ENTRY_CHECKMAGIC((__p))

/* zzip_disk_trailer - the zip archive entry point */
#define zzip_disk_trailer_get_magic(__p)      ZZIP_GET32((__p)->z_magic)
#define zzip_disk_trailer_set_magic(__p,__x)  ZZIP_SET32((__p)->z_magic,(__x))
#define zzip_disk_trailer_get_disk(__p)     ZZIP_GET16((__p)->z_disk)
#define zzip_disk_trailer_set_disk(__p,__x) ZZIP_SET16((__p)->z_disk,(__x))
#define zzip_disk_trailer_get_finaldisk(__p)     ZZIP_GET16((__p)->z_finaldisk)
#define zzip_disk_trailer_set_finaldisk(__p,__x) ZZIP_SET16((__p)->z_finaldisk,(__x))
#define zzip_disk_trailer_get_entries(__p)     ZZIP_GET16((__p)->z_entries)
#define zzip_disk_trailer_set_entries(__p,__x) ZZIP_SET16((__p)->z_entries,(__x))
#define zzip_disk_trailer_get_finalentries(__p)     ZZIP_GET16((__p)->z_finalentries)
#define zzip_disk_trailer_set_finalentries(__p,__x) ZZIP_SET16((__p)->z_finalentries,(__x))
#define zzip_disk_trailer_get_rootsize(__p)     ZZIP_GET32((__p)->z_rootsize)
#define zzip_disk_trailer_set_rootsize(__p,__x) ZZIP_SET32((__p)->z_rootsize,(__x))
#define zzip_disk_trailer_get_rootseek(__p)     ZZIP_GET32((__p)->z_rootseek)
#define zzip_disk_trailer_set_rootseek(__p,__x) ZZIP_SET32((__p)->z_rootseek,(__x))
#define zzip_disk_trailer_get_comment(__p)     ZZIP_GET16((__p)->z_comment)
#define zzip_disk_trailer_set_comment(__p,__x) ZZIP_SET16((__p)->z_comment,(__x))
#define zzip_disk_trailer_sizeof_tails(__p) ( zzip_disk_entry_get_comment(__p))
#define zzip_disk_trailer_check_magic(__p)  ZZIP_DISK_TRAILER_CHECKMAGIC((__p))

/* extra field should be type + size + data + type + size + data ... */
#define zzip_extra_block_get_datatype(__p)     ZZIP_GET16((zzip_byte_t*)(__p))
#define zzip_extra_block_set_datatype(__p,__x) ZZIP_SET16((zzip_byte_t*)(__p),__x)
#define zzip_extra_block_get_datasize(__p)     ZZIP_GET16((zzip_byte_t*)(__p)+2)
#define zzip_extra_block_set_datasize(__p,__x) ZZIP_SET16((zzip_byte_t*)(__p)+2,__x)

/* zzip64_disk_trailer - the zip64 archive entry point */
#define zzip_disk64_trailer_get_magic(__p)      ZZIP_GET32((__p)->z_magic)
#define zzip_disk64_trailer_set_magic(__p,__x)  ZZIP_SET32((__p)->z_magic,(__x))
#define zzip_disk64_trailer_get_size(__p)     ZZIP_GET64((__p)->z_size)
#define zzip_disk64_trailer_set_size(__p,__x) ZZIP_SET64((__p)->z_size,(__x))
#define zzip_disk64_trailer_get_disk(__p)     ZZIP_GET32((__p)->z_disk)
#define zzip_disk64_trailer_set_disk(__p,__x) ZZIP_SET32((__p)->z_disk,(__x))
#define zzip_disk64_trailer_get_finaldisk(__p)     ZZIP_GET32((__p)->z_finaldisk)
#define zzip_disk64_trailer_set_finaldisk(__p,__x) ZZIP_SET32((__p)->z_finaldisk,(__x))
#define zzip_disk64_trailer_get_entries(__p)     ZZIP_GET64((__p)->z_entries)
#define zzip_disk64_trailer_set_entries(__p,__x) ZZIP_SET64((__p)->z_entries,(__x))
#define zzip_disk64_trailer_get_finalentries(__p)     ZZIP_GET64((__p)->z_finalentries)
#define zzip_disk64_trailer_set_finalentries(__p,__x) ZZIP_SET64((__p)->z_finalentries,(__x))
#define zzip_disk64_trailer_get_rootsize(__p)     ZZIP_GET64((__p)->z_rootsize)
#define zzip_disk64_trailer_set_rootsize(__p,__x) ZZIP_SET64((__p)->z_rootsize,(__x))
#define zzip_disk64_trailer_get_rootseek(__p)     ZZIP_GET64((__p)->z_rootseek)
#define zzip_disk64_trailer_set_rootseek(__p,__x) ZZIP_SET64((__p)->z_rootseek,(__x))
#define zzip_disk64_trailer_check_magic(__p)  ZZIP_DISK64_TRAILER_CHECKMAGIC((__p))

/* .............. some logical typed access wrappers ....................... */

/* zzip_file_header - the local file header */
#define zzip_file_header_csize(__p)   ((zzip_size_t) \
        zzip_file_header_get_csize(__p))
#define zzip_file_header_usize(__p)   ((zzip_size_t) \
        zzip_file_header_get_usize(__p))
#define zzip_file_header_namlen(__p)   ((zzip_size_t) \
        zzip_file_header_get_namlen(__p))
#define zzip_file_header_extras(__p)   ((zzip_size_t) \
        zzip_file_header_get_extras(__p))
#define zzip_file_header_sizeof_tail(__p) ((zzip_size_t) \
        zzip_file_header_sizeof_tails(__p))
#define zzip_file_header_sizeto_end(__p)   ((zzip_size_t) \
        (zzip_file_header_sizeof_tail(__p) + zzip_file_header_headerlength))
#define zzip_file_header_skipto_end(__p)   ((void*) (__p) + \
        (zzip_file_header_sizeof_tail(__p) + zzip_file_header_headerlength))

#define zzip_file_header_to_filename(__p)   ((char*) \
        ((char*)(__p) + zzip_file_header_headerlength))
#define zzip_file_header_to_extras(__p)   ((char*) \
        (zzip_file_header_to_filename(__p) + zzip_file_header_namlen(__p)))
#define zzip_file_header_to_data(__p)   ((zzip_byte_t*) \
        (zzip_file_header_to_extras(__p) + zzip_file_header_extras(__p)))
#define zzip_file_header_to_trailer(__p)   ((struct zzip_file_trailer*) \
        (zzip_file_header_to_data(__p) + zzip_file_header_csize(__p)))

/* zzip_file_trailer - data descriptor per file block */
#define zzip_file_trailer_csize(__p)   ((zzip_size_t) \
        zzip_file_trailer_get_csize(__p))
#define zzip_file_trailer_usize(__p)   ((zzip_size_t) \
        zzip_file_trailer_get_usize(__p))
#define zzip_file_trailer_sizeof_tail(__p) ((zzip_size_t) \
        zzip_file_trailer_sizeof_tails(__p))
#define zzip_file_trailer_sizeto_end(__p)   ((zzip_size_t) \
        (zzip_file_trailer_sizeof_tail(__p) + zzip_file_trailer_headerlength))
#define zzip_file_trailer_skipto_end(__p)   ((void*) (__p) + \
        (zzip_file_trailer_sizeof_tail(__p) + zzip_file_trailer_headerlength))

/* zzip_disk_entry (currently named zzip_root_dirent) */
#define zzip_disk_entry_csize(__p)   ((zzip_size_t) \
        zzip_disk_entry_get_csize(__p))
#define zzip_disk_entry_usize(__p)   ((zzip_size_t) \
        zzip_disk_entry_get_usize(__p))
#define zzip_disk_entry_namlen(__p)   ((zzip_size_t) \
        zzip_disk_entry_get_namlen(__p))
#define zzip_disk_entry_extras(__p)   ((zzip_size_t) \
        zzip_disk_entry_get_extras(__p))
#define zzip_disk_entry_comment(__p)   ((zzip_size_t) \
        zzip_disk_entry_get_comment(__p))
#define zzip_disk_entry_diskstart(__p) ((int) \
        zzip_disk_entry_get_diskstart(__p))
#define zzip_disk_entry_filetype(__p) ((int) \
        zzip_disk_entry_get_filetype(__p))
#define zzip_disk_entry_filemode(__p) ((int) \
        zzip_disk_entry_get_filemode(__p))
#define zzip_disk_entry_fileoffset(__p) ((zzip_off_t) \
        zzip_disk_entry_get_offset(__p))
#define zzip_disk_entry_sizeof_tail(__p) ((zzip_size_t) \
        zzip_disk_entry_sizeof_tails(__p))
#define zzip_disk_entry_sizeto_end(__p)   ((zzip_size_t) \
        (zzip_disk_entry_sizeof_tail(__p) + zzip_disk_entry_headerlength))
#define zzip_disk_entry_skipto_end(__p)   ((zzip_byte_t*) (__p) + \
        (zzip_disk_entry_sizeof_tail(__p) + zzip_disk_entry_headerlength))

#define zzip_disk_entry_to_filename(__p)   ((char*) \
        ((char*)(__p) + zzip_disk_entry_headerlength))
#define zzip_disk_entry_to_extras(__p)   ((char*) \
        (zzip_disk_entry_to_filename(__p) + zzip_disk_entry_namlen(__p)))
#define zzip_disk_entry_to_comment(__p)   ((char*) \
        (zzip_disk_entry_to_extras(__p) + zzip_disk_entry_extras(__p)))
#define zzip_disk_entry_to_next_entry(__p)   ((struct zzip_disk_entry*) \
        (zzip_disk_entry_to_comment(__p) + zzip_disk_entry_comment(__p)))

/* zzip_disk_trailer - the zip archive entry point */
#define zzip_disk_trailer_localdisk(__p) ((int) \
        zzip_disk_trailer_get_disk(__p))
#define zzip_disk_trailer_finaldisk(__p) ((int) \
        zzip_disk_trailer_get_finaldisk(__p))
#define zzip_disk_trailer_localentries(__p) ((int) \
        zzip_disk_trailer_get_entries(__p))
#define zzip_disk_trailer_finalentries(__p) ((int) \
        zzip_disk_trailer_get_finalentries(__p))
#define zzip_disk_trailer_rootsize(__p) ((zzip_off_t) \
        zzip_disk_trailer_get_rootsize(__p))
#define zzip_disk_trailer_rootseek(__p) ((zzip_off_t) \
        zzip_disk_trailer_get_rootseek(__p))
#define zzip_disk_trailer_comment(__p)   ((zzip_size_t) \
        zzip_disk_trailer_get_comment(__p))
#define zzip_disk_trailer_sizeof_tail(__p) ((zzip_size_t) \
        zzip_disk_trailer_sizeof_tails(__p))
#define zzip_disk_trailer_sizeto_end(__p)   ((zzip_size_t) \
        (zzip_disk_trailer_sizeof_tail(__p) + zzip_disk_trailer_headerlength))
#define zzip_disk_trailer_skipto_end(__p)   ((void*) (__p) \
        (zzip_disk_trailer_sizeof_tail(__p) + zzip_disk_trailer_headerlength))

#define zzip_disk_trailer_to_comment(__p)   ((char*) \
        ((char*)(__p) + zzip_disk_trailer_headerlength))
#define zzip_disk_trailer_to_endoffile(__p)   ((void*) \
        (zzip_disk_trailer_to_comment(__p) + zzip_disk_trailer_comment(__p)))

/* zzip_disk64_trailer - the zip archive entry point */
#define zzip_disk64_trailer_localdisk(__p) ((int) \
        zzip_disk64_trailer_get_disk(__p))
#define zzip_disk64_trailer_finaldisk(__p) ((int) \
        zzip_disk64_trailer_get_finaldisk(__p))
#define zzip_disk64_trailer_localentries(__p) ((int) \
        zzip_disk64_trailer_get_entries(__p))
#define zzip_disk64_trailer_finalentries(__p) ((int) \
        zzip_disk64_trailer_get_finalentries(__p))
#define zzip_disk64_trailer_rootsize(__p) ((zzip_off64_t) \
        zzip_disk64_trailer_get_rootsize(__p))
#define zzip_disk64_trailer_rootseek(__p) ((zzip_off64_t) \
        zzip_disk64_trailer_get_rootseek(__p))
#define zzip_disk64_trailer_sizeof_tail(__p)   ((zzip_size_t) \
        zzip_disk64_trailer_get_size(__p) - zzip_disk64_trailer_headerlength)
#define zzip_disk64_trailer_sizeto_end(__p)   ((zzip_size_t) \
        zzip_disk64_trailer_get_size(__p))
#define zzip_disk64_trailer_skipto_end(__p)   ((void*) \
        ((char*)(__p) + zzip_disk64_sizeto_end(__p)))

/* extra field should be type + size + data + type + size + data ... */
#define zzip_extra_block_sizeof_tail(__p)  ((zzip_size_t) \
        (zzip_extra_block_get_datasize(__p)))
#define zzip_extra_block_sizeto_end(__p)    ((zzip_size_t) \
        (zzip_extra_block_sizeof_tail(__p) + zzip_extra_block_headerlength))
#define zzip_extra_block_skipto_end(__p)    ((void*) (__p) \
        (zzip_extra_block_sizeof_tail(__p) + zzip_extra_block_headerlength))

/* ................... and put these to the next level ................ */

#define zzip_file_header_data_encrypted(__p) \
        ZZIP_IS_ENCRYPTED( zzip_file_header_get_flags(__p) )
#define zzip_file_header_data_comprlevel(__p) \
        ZZIP_IS_COMPRLEVEL( zzip_file_header_get_flags(__p) )
#define zzip_file_header_data_streamed(__p) \
        ZZIP_IS_STREAMED( zzip_file_header_get_flags(__p) )
#define zzip_file_header_data_stored(__p) \
        ( ZZIP_IS_STORED ==   zzip_file_header_get_compr(__p) )
#define zzip_file_header_data_deflated(__p) \
        ( ZZIP_IS_DEFLATED == zzip_file_header_get_compr(__p) )

#define zzip_disk_entry_data_encrypted(__p) \
        ZZIP_IS_ENCRYPTED( zzip_disk_entry_get_flags(__p) )
#define zzip_disk_entry_data_comprlevel(__p) \
        ZZIP_IS_COMPRLEVEL( zzip_disk_entry_get_flags(__p) )
#define zzip_disk_entry_data_streamed(__p) \
        ZZIP_IS_STREAMED( zzip_disk_entry_get_flags(__p) )
#define zzip_disk_entry_data_stored(__p) \
        ( ZZIP_IS_STORED ==  zzip_disk_entry_get_compr(__p) )
#define zzip_disk_entry_data_deflated(__p) \
        ( ZZIP_IS_DEFLATED ==  zzip_disk_entry_get_compr(__p) )
#define zzip_disk_entry_data_ascii(__p) \
        ( zzip_disk_entry_get_filetype(__p) & 1)

#define zzip_file_header_data_not_deflated(__p) \
        (zzip_file_header_data_stored(__p))
#define zzip_file_header_data_std_deflated(__p) \
        (zzip_file_header_data_deflated(__p) && \
	 zzip_file_header_data_comprlevel(__p) == ZZIP_DEFLATED_STD_COMPR)
#define zzip_file_header_data_max_deflated(__p) \
        (zzip_file_header_data_deflated(__p) && \
	 zzip_file_header_data_comprlevel(__p) == ZZIP_DEFLATED_MAX_COMPR)
#define zzip_file_header_data_low_deflated(__p) \
        (zzip_file_header_data_deflated(__p) && \
	 zzip_file_header_data_comprlevel(__p) == ZZIP_DEFLATED_LOW_COMPR)
#define zzip_file_header_data_min_deflated(__p) \
        (zzip_file_header_data_deflated(__p) && \
	 zzip_file_header_data_comprlevel(__p) == ZZIP_DEFLATED_MIN_COMPR)

#define zzip_disk_entry_data_not_deflated(__p) \
        (zzip_disk_entry_data_stored(__p))
#define zzip_disk_entry_data_std_deflated(__p) \
        (zzip_disk_entry_data_deflated(__p) && \
	 zzip_disk_entry_data_comprlevel(__p) == ZZIP_DEFLATED_STD_COMPR)
#define zzip_disk_entry_data_max_deflated(__p) \
        (zzip_disk_entry_data_deflated(__p) && \
	 zzip_disk_entry_data_comprlevel(__p) == ZZIP_DEFLATED_MAX_COMPR)
#define zzip_disk_entry_data_low_deflated(__p) \
        (zzip_disk_entry_data_deflated(__p) && \
	 zzip_disk_entry_data_comprlevel(__p) == ZZIP_DEFLATED_LOW_COMPR)
#define zzip_disk_entry_data_min_deflated(__p) \
        (zzip_disk_entry_data_deflated(__p) && \
	 zzip_disk_entry_data_comprlevel(__p) == ZZIP_DEFLATED_MIN_COMPR)

#endif
