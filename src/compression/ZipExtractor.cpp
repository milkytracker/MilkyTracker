/*
 *  compression/ZipExtractor.cpp
 *
 *  Copyright 2009 Peter Barth
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 *  ZipExtractor.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 31.01.07.
 *
 */

#ifdef VERBOSE
#include <stdio.h>
#endif
#include "ZipExtractor.h"
#include "XModule.h"
#include "XMFile.h"
#include "MyIO.h"
#include "zzip_lib.h"

ZipExtractor::ZipExtractor(const PPSystemString& archivePath) :
	archivePath(archivePath)
{
}

bool ZipExtractor::parseZip(pp_int32& err, bool extract, const PPSystemString* outFile)
{
    int i;
	__zzipfd fd;
    
	ZZIP_DIR * dir;
    zzip_error_t rv;
  
#ifdef VERBOSE
    printf("Opening zip file `%s'... ", (const char*)archivePath);
#endif
    { 	
		const SYSCHAR* fileName = archivePath;
		fd = Myopen((const zzip_char_t*)fileName, O_RDONLY);
        
		if (fd == (__zzipfd)-1) 
		{ 
			err = 1;
			return false;
		}
        
		if (! (dir = zzip_dir_fdopen(fd, &rv)))
        {
			err = 2;
			return false;
        }
    } 
#ifdef VERBOSE
	printf("OK.\n");  
    printf("{check...\n");
#endif
    { 
		struct zzip_dir_hdr * hdr = dir->hdr0;
    
        if (hdr == NULL) 
        { 
#ifdef VERBOSE
			printf ("could not find first header in dir_hdr"); 
#endif
			err = 3;
			zzip_dir_close(dir);
			return false;
		}
        else
        {   
            while (1)
            {
#ifdef VERBOSE
                printf("\ncompression method: %d", hdr->d_compr);
                if (hdr->d_compr == 0) printf(" (stored)");
                else if (hdr->d_compr == 8) printf(" (deflated)");
                else printf(" (unknown)");
                printf("\ncrc32: %x\n", hdr->d_crc32);
                printf("compressed size: %d\n", hdr->d_csize);
                printf("uncompressed size: %d\n", hdr->d_usize);
                printf("offset of file in archive: %d\n", hdr->d_off);
                printf("filename: %s\n\n", hdr->d_name);
#endif    
				{   
					ZZIP_FILE *fp;
					mp_ubyte* buf = new mp_ubyte[16384];

					if (buf == NULL)
					{
						err = 6;
						zzip_dir_close(dir);
						return false;
					}

					memset(buf, 0, 16384);

					const char *name = hdr->d_name;
					
#ifdef VERBOSE
					printf("Opening file `%s' in zip archive... ", name);    
#endif
					fp = zzip_file_open(dir, (char *)name, ZZIP_CASEINSENSITIVE);
					
					if (! fp)
					{ 
#ifdef VERBOSE
						printf("error %d: %s\n", zzip_error(dir), zzip_strerror_of(dir)); 
#endif
						err = 7;
						delete[] buf;
						zzip_dir_close(dir);
						return false;
					}
					else
					{
#ifdef VERBOSE
						printf("OK.\n");
						printf("Contents of the file:\n");
#endif						
						i = zzip_file_read(fp, (char*)buf, 16384);
						
						const char* id = XModule::identifyModule(buf);

						if (id)
						{														
							if (extract)
							{
								XMFile f(*outFile, true);
								f.write(buf, 1, i);								
								while (0 < (i = zzip_file_read(fp, (char*)buf, 16384)))
								{
									f.write(buf, 1, i);
								}
								if (i < 0)
								{
#ifdef VERBOSE
									printf("error %d\n", zzip_error(dir));
#endif
									err = 4;
									delete[] buf;
									
									zzip_file_close(fp);							
									zzip_dir_close(dir);									
									return false;
								}								
							}

							delete[] buf;
							
							err = 0;
							
							zzip_file_close(fp);							
							zzip_dir_close(dir);							
							return true;

						}

						zzip_file_close(fp);

					}
					
					delete[] buf;
				}  

                if (hdr->d_reclen == 0) break;
            
				char* temp = (char*)hdr;
				temp+=hdr->d_reclen;
				hdr = (zzip_dir_hdr*)temp;
				
				//(char *)hdr += hdr->d_reclen;
            }
        }
    } 
#ifdef VERBOSE
	printf ("\n}\n");
#endif
	zzip_dir_close(dir);	
	err = 5;
	return false;
}
