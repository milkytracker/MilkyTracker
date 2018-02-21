/*
 **  LZX Extract in (supposedly) portable C.
 **
 **  Based on unlzx 1.0 by David Tritscher.
 **  Rewritten by Oliver Gantert <lucyg@t-online.de>
 **
 **  Compiled with vbcc/Amiga and lcc/Win32
 */

#ifndef unlzx_unlzx_h

#undef AMIGA

#define UNLZX_VERSION "2.16"
#define UNLZX_VERDATE "14.11.2000"
/*
 #define UNLZX_DEBUG
 #define UNLZX_TIME
 */

#if defined(AMIGA)
#include <exec/types.h>
LONG mkdir(STRPTR path, UWORD perm);
#endif

#include "BasicTypes.h"

class XMFile;

class Unlzx
{
public:
	struct FileIdentificator
	{
		virtual bool identify(const PPSystemString& filename) const = 0;
	};


private:		
	struct filename_node
	{
		struct filename_node * next;
		unsigned long length;
		unsigned long crc;
		unsigned char filename[256];
	};
	
	struct UnLZX
	{
		unsigned char match_pattern[256];
		signed long use_outdir;
		unsigned char output_dir[768];
		unsigned char work_buffer[1024];
		
		signed long mode;
		
		unsigned char info_header[10];
		unsigned char archive_header[32];
		unsigned char header_filename[256];
		unsigned char header_comment[256];
		
		unsigned long pack_size;
		unsigned long unpack_size;
		
		unsigned long crc;
		unsigned long year;
		unsigned long month;
		unsigned long day;
		unsigned long hour;
		unsigned long minute;
		unsigned long second;
		unsigned char attributes;
		unsigned char pack_mode;
		
		struct filename_node *filename_list;
		
		unsigned char read_buffer[16384];
		unsigned char decrunch_buffer[66560];
		
		unsigned char *source;
		unsigned char *destination;
		unsigned char *source_end;
		unsigned char *destination_end;
		
		unsigned long decrunch_method;
		unsigned long decrunch_length;
		unsigned long last_offset;
		unsigned long global_control;
		signed long global_shift;
		
		unsigned char offset_len[8];
		unsigned short offset_table[128];
		unsigned char huffman20_len[20];
		unsigned short huffman20_table[96];
		unsigned char literal_len[768];
		unsigned short literal_table[5120];
		
		unsigned long sum;
		
		const PPSystemString* temporaryFile;
	};
	
	PPSystemString archiveFilename;
	const FileIdentificator* identificator;
	
	UnLZX* unlzx;

	void mkdir(const char* name, int len);
	struct UnLZX *unlzx_init(void);
	void unlzx_free(struct UnLZX *unlzx);
	int pmatch(const char *mask, const char *name);
	void just_wait(void);
	unsigned long argopt(unsigned char * ao_strg, unsigned long ao_argc, unsigned char **ao_argv);
	void crc_calc(unsigned char *memory, unsigned long length, struct UnLZX *unlzx);
	signed long make_decode_table(signed long number_symbols, signed long table_size, unsigned char *length, unsigned short *table);
	signed long read_literal_table(struct UnLZX *unlzx);
	void decrunch(struct UnLZX *unlzx);
	XMFile* open_output(const PPSystemString& filename);
	signed long extract_normal(XMFile* in_file, struct UnLZX *unlzx, bool& found);
	signed long extract_store(XMFile* in_file, struct UnLZX *unlzx, bool& found);
	signed long extract_unknown(XMFile* in_file, struct UnLZX *unlzx, bool& found);
	signed long extract_archive(XMFile* in_file, struct UnLZX *unlzx, bool& found);
	signed long view_archive(XMFile* in_file, struct UnLZX *unlzx);
	signed long process_archive(const PPSystemString& filename, struct UnLZX *unlzx, bool& found);

public:
	Unlzx(const PPSystemString& archiveFilename, const FileIdentificator* identificator = NULL);
	~Unlzx();
	
	bool extractFile(bool extract, const PPSystemString* outFilename);
};

#define PMATCH_MAXSTRLEN  512    /*  max string length  */

#define make_percent(p,m)   ((p*100)/m)

#endif /* unlzx_unlzx_h */

