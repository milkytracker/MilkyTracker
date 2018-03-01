/*
 **  LZX Extract in (supposedly) portable C.
 **
 **  Based on unlzx 1.0 by David Tritscher.
 **  Rewritten by Oliver Gantert <lucyg@t-online.de>
 **
 **  Compiled with vbcc/Amiga and lcc/Win32
 */

#include "unlzx.h"
#include "XMFile.h"

#include <ctype.h>

#ifdef AMIGA
static unsigned char *version_string = "$VER: UnLZX " UNLZX_VERSION " (" UNLZX_VERDATE ")";
#endif /* AMIGA */

const unsigned char *month_str[16] =
{
	(unsigned char*)"jan", (unsigned char*)"feb", (unsigned char*)"mar", (unsigned char*)"apr", (unsigned char*)"may", (unsigned char*)"jun", (unsigned char*)"jul", (unsigned char*)"aug",
	(unsigned char*)"sep", (unsigned char*)"oct", (unsigned char*)"nov", (unsigned char*)"dec", (unsigned char*)"?13", (unsigned char*)"?14", (unsigned char*)"?15", (unsigned char*)"?16"
};

const unsigned long crc_table[256] =
{
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
	0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
	0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
	0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
	0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
	0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
	0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
	0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
	0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
	0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
	0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
	0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
	0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
	0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
	0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
	0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
	0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
	0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
	0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
	0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
	0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
	0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

const unsigned char table_one[32] =
{
	0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x02,
	0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06,
	0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x0a, 0x0a,
	0x0b, 0x0b, 0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e
};

const unsigned long table_two[32]=
{
	0,    1,    2,    3,
	4,    6,    8,    12,
	16,   24,   32,   48,
	64,   96,   128,  192,
	256,  384,  512,  768,
	1024, 1536, 2048, 3072,
	4096, 6144, 8192, 12288,
	16384,24576,32768,49152
};

const unsigned long table_three[16]=
{
	0,    1,    3,    7,
	15,   31,   63,   127,
	255,  511,  1023, 2047,
	4095, 8191, 16383,32767
};

const unsigned char table_four[34]=
{
	0,    1,    2,    3,
	4,    5,    6,    7,
	8,    9,    10,   11,
	12,   13,   14,   15,
	16,   0,    1,    2,
	3,    4,    5,    6,
	7,    8,    9,    10,
	11,   12,   13,   14,
	15,   16
};

/* -------------------------------------------------------------------------- */
void Unlzx::mkdir(const char* name, int len)
{
}

struct Unlzx::UnLZX *Unlzx::unlzx_init(void)
{
	struct UnLZX *unlzx = NULL;
	
	if ((unlzx = (struct UnLZX *)malloc(sizeof(struct UnLZX))))
	{
		memset(unlzx, 0, sizeof(struct UnLZX));
		unlzx->match_pattern[0] = '*';
	}
	return(unlzx);
}

void Unlzx::unlzx_free(struct UnLZX *unlzx)
{
	if (unlzx)
	{
		free(unlzx);
	}
}

int Unlzx::pmatch(const char *mask, const char *name)
{
	int           calls = 0,
	wild  = 0,
	q     = 0;
	const char  * m     = mask,
	* n     = name,
	* ma    = mask,
	* na    = name;
	
	for(;;)
	{
		if (++calls > PMATCH_MAXSTRLEN) return(1);
		if (*m == '*')
		{
			while (*m == '*') ++m;
			wild = 1;
			ma = m;
			na = n;
		}
		if (!*m)
		{
			if (!*n) return(0);
			for (--m; (m > mask) && (*m == '?'); --m);
			if ((*m == '*') && (m > mask) && (m[-1] != '\\')) return(0);
			if (!wild) return(1);
			m = ma;
		}
		else if (!*n)
		{
			while(*m == '*') ++m;
			return(*m != 0);
		}
		if ((*m == '\\') && ((m[1] == '*') || (m[1] == '?')))
		{
			++m;
			q = 1;
		}
		else
		{
			q = 0;
		}
		if ((tolower(*m) != tolower(*n)) && ((*m != '?') || q))
		{
			if (!wild) return(1);
			m = ma;
			n = ++na;
		}
		else
		{
			if (*m) ++m;
			if (*n) ++n;
		}
	}
}

void Unlzx::just_wait(void)
{
	/*
	 Under certain conditions UnLZX needs to wait
	 some cycles for disk access to finish. This
	 only seems to happen on fast CPUs, but it
	 doesn't hurt anyway...
	 */
}

unsigned long Unlzx::argopt(unsigned char * ao_strg, unsigned long ao_argc, unsigned char **ao_argv)
{
	while(ao_argc > 1)
	{
		if (!strcmp((char*)ao_strg, (char*)ao_argv[ao_argc - 1]))
			return(ao_argc - 1);
		ao_argc--;
	}
	return(0);
}

void Unlzx::crc_calc(unsigned char *memory, unsigned long length, struct UnLZX *unlzx)
{
	unsigned long temp;
	
	if (length)
	{
		temp = ~unlzx->sum;
		do
		{
			temp = crc_table[(*memory++ ^ temp) & 255] ^ (temp >> 8);
		} while(--length);
		unlzx->sum = ~temp;
	}
}

signed long Unlzx::make_decode_table(signed long number_symbols, signed long table_size, unsigned char *length, unsigned short *table)
{
	unsigned char bit_num = 0;
	signed long symbol, abort = 0;
	unsigned long leaf, table_mask, bit_mask, pos, fill, next_symbol, reverse;
	
	pos = 0;
	table_mask = 1 << table_size;
	bit_mask = table_mask >> 1;
	bit_num++;
	while ((!abort) && (bit_num <= table_size))
	{
		for (symbol = 0; symbol < number_symbols; symbol++)
		{
			if (length[symbol] == bit_num)
			{
				reverse = pos;
				leaf = 0;
				fill = table_size;
				do
				{
					leaf = (leaf << 1)+(reverse & 1);
					reverse >>= 1;
				} while (--fill);
				if ((pos += bit_mask) > table_mask)
				{
					abort = 1;
					break;
				}
				fill = bit_mask;
				next_symbol = 1 << bit_num;
				do
				{
					table[leaf] = (unsigned short)symbol;
					leaf += next_symbol;
				} while (--fill);
			}
		}
		bit_mask >>= 1;
		bit_num++;
	}
	if ((!abort) && (pos != table_mask))
	{
		for (symbol = pos; symbol < (signed)table_mask; symbol++)
		{
			reverse = symbol;
			leaf = 0;
			fill = table_size;
			do
			{
				leaf = (leaf << 1)+(reverse & 1);
				reverse >>= 1;
			} while (--fill);
			table[leaf] = 0;
		}
		next_symbol = table_mask >> 1;
		pos <<= 16;
		table_mask <<= 16;
		bit_mask = 32768;
		while((!abort) && (bit_num <= 16))
		{
			for(symbol = 0; symbol < number_symbols; symbol++)
			{
				if (length[symbol] == bit_num)
				{
					reverse = pos >> 16;
					leaf = 0;
					fill = table_size;
					do
					{
						leaf = (leaf << 1)+(reverse & 1);
						reverse >>= 1;
					} while (--fill);
					for (fill = 0; fill < (unsigned)(bit_num - table_size); fill++)
					{
						if (!table[leaf])
						{
							table[(next_symbol << 1)] = 0;
							table[(next_symbol << 1) + 1] = 0;
							table[leaf] = (unsigned short)(next_symbol++);
						}
						leaf = (unsigned short)(table[leaf] << 1);
						leaf += (pos >> (15 - fill)) & 1;
					}
					table[leaf] = (unsigned short)symbol;
					if ((pos += bit_mask) > table_mask)
					{
						abort = 1;
						break;
					}
				}
			}
			bit_mask >>= 1;
			bit_num++;
		}
	}
	if (pos != table_mask) abort = 1;
	return(abort);
}

signed long Unlzx::read_literal_table(struct UnLZX *unlzx)
{
	signed long shift = unlzx->global_shift, abort = 0;
	unsigned long control = unlzx->global_control, temp, symbol, pos, count, fix, max_symbol;
	
	if (shift < 0)
	{
		shift += 16;
		control += *unlzx->source++ << (8 + shift);
		control += *unlzx->source++ << shift;
	}
	unlzx->decrunch_method = control & 7;
	control >>= 3;
	if ((shift -= 3) < 0)
	{
		shift += 16;
		control += *unlzx->source++ << (8 + shift);
		control += *unlzx->source++ << shift;
	}
	if ((!abort) && (unlzx->decrunch_method == 3))
	{
		for (temp = 0; temp < 8; temp++)
		{
			unlzx->offset_len[temp] = (unsigned char)(control & 7);
			control >>= 3;
			if ((shift -= 3) < 0)
			{
				shift += 16;
				control += *unlzx->source++ << (8 + shift);
				control += *unlzx->source++ << shift;
			}
		}
		abort = make_decode_table(8, 7, unlzx->offset_len, unlzx->offset_table);
	}
	if (!abort)
	{
		unlzx->decrunch_length = (control & 255) << 16;
		control >>= 8;
		if ((shift -= 8) < 0)
		{
			shift += 16;
			control += *unlzx->source++ << (8 + shift);
			control += *unlzx->source++ << shift;
		}
		unlzx->decrunch_length += (control & 255) << 8;
		control >>= 8;
		if ((shift -= 8) < 0)
		{
			shift += 16;
			control += *unlzx->source++ << (8 + shift);
			control += *unlzx->source++ << shift;
		}
		unlzx->decrunch_length += (control & 255);
		control >>= 8;
		if ((shift -= 8) < 0)
		{
			shift += 16;
			control += *unlzx->source++ << (8 + shift);
			control += *unlzx->source++ << shift;
		}
	}
	if ((!abort) && (unlzx->decrunch_method != 1))
	{
		pos = 0;
		fix = 1;
		max_symbol = 256;
		do
		{
			for (temp = 0; temp < 20; temp++)
			{
				unlzx->huffman20_len[temp] = (unsigned char)(control & 15);
				control >>= 4;
				if ((shift -= 4) < 0)
				{
					shift += 16;
					control += *unlzx->source++ << (8 + shift);
					control += *unlzx->source++ << shift;
				}
			}
			if ((abort = make_decode_table(20, 6, unlzx->huffman20_len, unlzx->huffman20_table))) break;
			do
			{
				if ((symbol = unlzx->huffman20_table[control & 63]) >= 20)
				{
					do
					{
						symbol = unlzx->huffman20_table[((control >> 6) & 1) + (symbol << 1)];
						if (!shift--)
						{
							shift += 16;
							control += *unlzx->source++ << 24;
							control += *unlzx->source++ << 16;
						}
						control >>= 1;
					} while(symbol >= 20);
					temp = 6;
				}
				else
				{
					temp = unlzx->huffman20_len[symbol];
				}
				control >>= temp;
				if ((shift -= temp) < 0)
				{
					shift += 16;
					control += *unlzx->source++ << (8 + shift);
					control += *unlzx->source++ << shift;
				}
				switch(symbol)
				{
					case 17:
					case 18:
						if(symbol == 17)
						{
							temp = 4;
							count = 3;
						}
						else
						{
							temp = 6 - fix;
							count = 19;
						}
						count += (control & table_three[temp]) + fix;
						control >>= temp;
						if ((shift -= temp) < 0)
						{
							shift += 16;
							control += *unlzx->source++ << (8 + shift);
							control += *unlzx->source++ << shift;
						}
						while ((pos < max_symbol) && (count--)) unlzx->literal_len[pos++] = 0;
						break;
						case 19:
						count = (control & 1) + 3 + fix;
						if (!shift--)
						{
							shift += 16;
							control += *unlzx->source++ << 24;
							control += *unlzx->source++ << 16;
						}
						control >>= 1;
						if ((symbol = unlzx->huffman20_table[control & 63]) >= 20)
						{
							do
							{
								symbol = unlzx->huffman20_table[((control >> 6) & 1) + (symbol << 1)];
								if (!shift--)
								{
									shift += 16;
									control += *unlzx->source++ << 24;
									control += *unlzx->source++ << 16;
								}
								control >>= 1;
							} while(symbol >= 20);
							temp = 6;
						}
						else
						{
							temp = unlzx->huffman20_len[symbol];
						}
						control >>= temp;
						if ((shift -= temp) < 0)
						{
							shift += 16;
							control += *unlzx->source++ << (8 + shift);
							control += *unlzx->source++ << shift;
						}
						symbol = table_four[unlzx->literal_len[pos] + 17 - symbol];
						while ((pos < max_symbol) && (count--)) unlzx->literal_len[pos++] = (unsigned char)symbol;
						break;
						default:
						symbol = table_four[unlzx->literal_len[pos] + 17 - symbol];
						unlzx->literal_len[pos++] = (unsigned char)symbol;
						break;
				}
			} while(pos < max_symbol);
			fix--;
			max_symbol += 512;
		} while(max_symbol == 768);
		if(!abort) abort = make_decode_table(768, 12, unlzx->literal_len, unlzx->literal_table);
	}
	unlzx->global_control = control;
	unlzx->global_shift = shift;
	return(abort);
}

void Unlzx::decrunch(struct UnLZX *unlzx)
{
	unsigned long control = unlzx->global_control, temp, symbol, count;
	signed long shift = unlzx->global_shift;
	unsigned char *string;
	
	do
	{
		if ((symbol = unlzx->literal_table[control & 4095]) >= 768)
		{
			control >>= 12;
			if ((shift -= 12) < 0)
			{
				shift += 16;
				control += *unlzx->source++ << (8 + shift);
				control += *unlzx->source++ << shift;
			}
			do
			{
				symbol = unlzx->literal_table[(control & 1) + (symbol << 1)];
				if (!shift--)
				{
					shift += 16;
					control += *unlzx->source++ << 24;
					control += *unlzx->source++ << 16;
				}
				control >>= 1;
			} while(symbol >= 768);
		}
		else
		{
			temp = unlzx->literal_len[symbol];
			control >>= temp;
			if ((shift -= temp) < 0)
			{
				shift += 16;
				control += *unlzx->source++ << (8 + shift);
				control += *unlzx->source++ << shift;
			}
		}
		if (symbol < 256)
		{
			*unlzx->destination++ = (unsigned char)symbol;
		}
		else
		{
			symbol -= 256;
			count = table_two[temp = symbol & 31];
			temp = table_one[temp];
			if ((temp >= 3) && (unlzx->decrunch_method == 3))
			{
				temp -= 3;
				count += ((control & table_three[temp]) << 3);
				control >>= temp;
				if ((shift -= temp) < 0)
				{
					shift += 16;
					control += *unlzx->source++ << (8 + shift);
					control += *unlzx->source++ << shift;
				}
				count += (temp = unlzx->offset_table[control & 127]);
				temp = unlzx->offset_len[temp];
			}
			else
			{
				count += control & table_three[temp];
				if (!count) count = unlzx->last_offset;
			}
			control >>= temp;
			if ((shift -= temp) < 0)
			{
				shift += 16;
				control += *unlzx->source++ << (8 + shift);
				control += *unlzx->source++ << shift;
			}
			unlzx->last_offset = count;
			count = table_two[temp = (symbol >> 5) & 15] + 3;
			temp = table_one[temp];
			count += (control & table_three[temp]);
			control >>= temp;
			if ((shift -= temp) < 0)
			{
				shift += 16;
				control += *unlzx->source++ << (8 + shift);
				control += *unlzx->source++ << shift;
			}
			string = (unlzx->decrunch_buffer + unlzx->last_offset < unlzx->destination) ? unlzx->destination - unlzx->last_offset : unlzx->destination + 65536 - unlzx->last_offset;
			do
			{
				*unlzx->destination++ = *string++;
			} while(--count);
		}
	} while((unlzx->destination < unlzx->destination_end) && (unlzx->source < unlzx->source_end));
	unlzx->global_control = control;
	unlzx->global_shift = shift;
}

XMFile* Unlzx::open_output(const PPSystemString& filename)
{
	XMFile *file = new XMFile(filename, true);
	
	if (!file->isOpenForWriting())
		return NULL;
	
	return(file);
}

signed long Unlzx::extract_normal(XMFile* in_file, struct UnLZX *unlzx, bool& found)
{
	found = false;
	struct filename_node *node;
	XMFile *out_file = NULL;
	unsigned char *pos, *temp;
	unsigned long count;
	signed long abort = 0;
	
	unlzx->global_control = 0;
	unlzx->global_shift = -16;
	unlzx->last_offset = 1;
	unlzx->unpack_size = 0;
	unlzx->decrunch_length = 0;
	for(count = 0; count < 8; count++) unlzx->offset_len[count] = 0;
	for(count = 0; count < 768; count ++) unlzx->literal_len[count] = 0;
	unlzx->source_end = (unlzx->source = unlzx->read_buffer + 16384) - 1024;
	pos = unlzx->destination_end = unlzx->destination = unlzx->decrunch_buffer + 65794;
	for (node = unlzx->filename_list; (!abort) && node; node = node->next)
	{
		unlzx->sum = 0;
		if (unlzx->use_outdir)
		{
			strcpy((char*)unlzx->work_buffer, (char*)unlzx->output_dir);
			strcat((char*)unlzx->work_buffer, (char*)node->filename);
		}
		else
		{
			strcpy((char*)unlzx->work_buffer, (char*)node->filename);
		}
		fflush(stdout);
		if (!pmatch((char*)unlzx->match_pattern, (char*)node->filename))
		{
#ifdef UNLZX_DEBUG
			printf("Extracting \"%s\"...", (char *)node->filename);
#endif			
			out_file = unlzx->temporaryFile ? open_output(*(unlzx->temporaryFile)) : open_output(PPSystemString((const char*)unlzx->work_buffer));
		}
		else
		{
			out_file = NULL;
		}
		unlzx->unpack_size = node->length;
		while(unlzx->unpack_size > 0)
		{
			if (pos == unlzx->destination)
			{
				if(unlzx->source >= unlzx->source_end)
				{
					temp = unlzx->read_buffer;
					if ((count = temp - unlzx->source + 16384))
					{
						do
						{
							*temp++ = *unlzx->source++;
						} while(--count);
					}
					unlzx->source = unlzx->read_buffer;
					count = unlzx->source - temp + 16384;
					if (unlzx->pack_size < count) count = unlzx->pack_size;
					if (in_file->read(temp, 1, count) != count)
					{
#ifdef UNLZX_DEBUG
						printf("\n");
						if (ferror(in_file))
						{
							perror("FRead(Data)");
						}
						else
						{
							fprintf(stderr, "EOF: Data\n");
						}
#endif
						abort = 1;
						break;
					}
					unlzx->pack_size -= count;
					temp += count;
					if (unlzx->source >= temp) break;
				}
				if (unlzx->decrunch_length <= 0)
				{
					if (read_literal_table(unlzx)) break;
				}
				if (unlzx->destination >= unlzx->decrunch_buffer + 65794)
				{
					if ((count = unlzx-> destination - unlzx-> decrunch_buffer - 65536))
					{
						temp = (unlzx->destination = unlzx->decrunch_buffer) + 65536;
						do
						{
							*unlzx->destination++ = *temp++;
						} while(--count);
					}
					pos = unlzx->destination;
				}
				unlzx->destination_end = unlzx->destination + unlzx->decrunch_length;
				if(unlzx->destination_end > unlzx->decrunch_buffer + 65794) unlzx->destination_end = unlzx->decrunch_buffer + 65794;
				temp = unlzx->destination;
				decrunch(unlzx);
				unlzx->decrunch_length -= (unlzx->destination - temp);
			}
			count = unlzx->destination - pos;
			if (count > unlzx->unpack_size) count = unlzx->unpack_size;
			crc_calc(pos, count, unlzx);
			if (out_file)
			{
				if (out_file->write(pos, 1, count) != count)
				{
#ifdef UNLZX_DEBUG
					perror("FWrite");
#endif
					delete out_file;
					out_file = 0;
				}
			}
			unlzx->unpack_size -= count;
			pos += count;
		}
		if (out_file)
		{
			PPSystemString fileName = out_file->getFileName();
			delete out_file;
			if (!abort)
			{
#ifdef UNLZX_DEBUG
				printf(" crc %s\n", (char *)((node->crc == unlzx->sum) ? "good" : "bad"));
#endif				
				if (identificator)
					found = identificator->identify(fileName);
			}
		}
	}
	return(abort);
}

signed long Unlzx::extract_store(XMFile* in_file, struct UnLZX *unlzx, bool& found)
{
	struct filename_node *node;
	XMFile *out_file = NULL;
	unsigned long count;
	signed long abort = 0;
	
	for (node = unlzx->filename_list; (!abort) && (node); node = node->next)
	{
		unlzx->sum = 0;
		if (unlzx->use_outdir)
		{
			strcpy((char*)unlzx->work_buffer, (char*)unlzx->output_dir);
			strcat((char*)unlzx->work_buffer, (char*)node->filename);
		}
		else
		{
			strcpy((char*)unlzx->work_buffer, (char*)node->filename);
		}
		fflush(stdout);
		if (!pmatch((char*)unlzx->match_pattern, (char*)node->filename))
		{
#ifdef UNLZX_DEBUG
			printf("Storing \"%s\"...", (char *)node->filename);
#endif
			out_file = unlzx->temporaryFile ? open_output(*(unlzx->temporaryFile)) : open_output(PPSystemString((const char*)unlzx->work_buffer));
		}
		else
		{
			out_file = 0;
		}
		unlzx->unpack_size = node->length;
		if (unlzx->unpack_size > unlzx->pack_size) unlzx->unpack_size = unlzx->pack_size;
		while (unlzx->unpack_size > 0)
		{
			count = (unlzx->unpack_size > 16384) ? 16384 : unlzx->unpack_size;
			if (in_file->read(unlzx->read_buffer, 1, count) != count)
			{
#ifdef UNLZX_DEBUG
				printf("\n");
				if (ferror(in_file))
				{
					perror("FRead(Data)");
				}
				else
				{
					fprintf(stderr, "EOF: Data\n");
				}
#endif
				abort = 1;
				break;
			}
			unlzx->pack_size -= count;
			crc_calc(unlzx->read_buffer, count, unlzx);
			if (out_file)
			{
				if (out_file->write(unlzx->read_buffer, 1, count) != count)
				{
#ifdef UNLZX_DEBUG
					perror("FWrite");
#endif
					delete out_file;
					out_file = 0;
				}
			}
			unlzx->unpack_size -= count;
		}
		if (out_file)
		{
			PPSystemString fileName = out_file->getFileName();
			delete out_file;
			if (!abort)
			{
#ifdef UNLZX_DEBUG
				printf(" crc %s\n", (char *)((node->crc == unlzx->sum) ? "good" : "bad"));
#endif				
				if (identificator)
					found = identificator->identify(fileName);
			}
		}
	}
	return(abort);
}

signed long Unlzx::extract_unknown(XMFile *in_file, struct UnLZX *unlzx, bool& found)
{
	struct filename_node *node;
	
	for (node = unlzx->filename_list; (node); node = node->next)
	{
#ifdef UNLZX_DEBUG
		printf("Unknown \"%s\"\n", (char *)node->filename);
#endif
	}
	return(0);
}

signed long Unlzx::extract_archive(XMFile *in_file, struct UnLZX *unlzx, bool& found)
{
	unsigned long temp;
	struct filename_node **filename_next;
	struct filename_node *node;
	struct filename_node *temp_node;
	signed long actual, abort, result = 1;
	
	unlzx->filename_list = 0;
	filename_next = &unlzx->filename_list;
	do
	{
		abort = 1;
		actual = in_file->read(unlzx->archive_header, 1, 31);
		if (!in_file->isEOF())
		{
			if (actual)
			{
				if (actual == 31)
				{
					unlzx->sum = 0;
					unlzx->crc = (unlzx->archive_header[29] << 24) + (unlzx->archive_header[28] << 16) + (unlzx->archive_header[27] << 8) + unlzx->archive_header[26];
					unlzx->archive_header[29] =
					unlzx->archive_header[28] =
					unlzx->archive_header[27] =
					unlzx->archive_header[26] = 0;
					crc_calc(unlzx->archive_header, 31, unlzx);
					temp = unlzx->archive_header[30];
					actual = in_file->read(unlzx->header_filename, 1, temp);
					if (!in_file->isEOF())
					{
						if (actual == temp)
						{
							unlzx->header_filename[temp] = 0;
							crc_calc(unlzx->header_filename, temp, unlzx);
							temp = unlzx->archive_header[14];
							actual = in_file->read(unlzx->header_comment, 1, temp);
							if (!in_file->isEOF())
							{
								if (actual == temp)
								{
									unlzx->header_comment[temp] = 0;
									crc_calc(unlzx->header_comment, temp, unlzx);
									if (unlzx->sum == unlzx->crc)
									{
										unlzx->unpack_size = (unlzx->archive_header[5] << 24) + (unlzx->archive_header[4] << 16) + (unlzx->archive_header[3] << 8) + unlzx->archive_header[2];
										unlzx->pack_size = (unlzx->archive_header[9] << 24) + (unlzx->archive_header[8] << 16) + (unlzx->archive_header[7] << 8) + unlzx->archive_header[6];
										unlzx->pack_mode = unlzx->archive_header[11];
										unlzx->crc = (unlzx->archive_header[25] << 24) + (unlzx->archive_header[24] << 16) + (unlzx->archive_header[23] << 8) + unlzx->archive_header[22];
										if ((node = (struct filename_node *) malloc(sizeof(struct filename_node))))
										{
											*filename_next = node;
											filename_next = &(node->next);
											node->next = 0;
											node->length = unlzx->unpack_size;
											node->crc = unlzx->crc;
											for (temp = 0; (node->filename[temp] = unlzx->header_filename[temp]); temp++);
											
											if (unlzx->pack_size)
											{
												switch(unlzx->pack_mode)
												{
													case 0:
														abort = extract_store(in_file, unlzx, found);
														break;
													case 2:
														abort = extract_normal(in_file, unlzx, found);
														break;
													default:
														abort = extract_unknown(in_file, unlzx, found);
														break;
												}
												
												if (found)
												{
													abort = 1;
													result = 0;
												}
												
												if (abort) break;
												temp_node = unlzx->filename_list;
												while ((node = temp_node))
												{
													temp_node = node->next;
													free(node);
												}
												unlzx->filename_list = 0;
												filename_next = &unlzx->filename_list;
												in_file->seek(unlzx->pack_size, XMFile::SeekOffsetTypeCurrent);
											}
											else
											{
												abort = 0;
											}
										}
#ifdef UNLZX_DEBUG
										else
										{
											fprintf(stderr, "MAlloc(Filename_node)\n");
										}
#endif
									}
#ifdef UNLZX_DEBUG
									else
									{
										fprintf(stderr, "CRC: Archive_Header\n");
									}
#endif
								}
#ifdef UNLZX_DEBUG
								else
								{
									fprintf(stderr, "EOF: Header_Comment\n");
								}
#endif
							}
#ifdef UNLZX_DEBUG
							else
							{
								perror("FRead(Header_Comment)");
							}
#endif
						}
#ifdef UNLZX_DEBUG
						else
						{
							fprintf(stderr, "EOF: Header_Filename\n");
						}
#endif
					}
#ifdef UNLZX_DEBUG
					else
					{
						perror("FRead(Header_Filename)");
					}
#endif
				}
#ifdef UNLZX_DEBUG
				else
				{
					fprintf(stderr, "EOF: Archive_Header\n");
				}
#endif
			}
			else
			{
				result = 0;
			}
		}
#ifdef UNLZX_DEBUG
		else
		{
			perror("FRead(Archive_Header)");
		}
#endif
	} while(!abort);
	temp_node = unlzx->filename_list;
	while((node = temp_node))
	{
		temp_node = node->next;
		free(node);
	}
	return(result);
}

signed long Unlzx::view_archive(XMFile* in_file, struct UnLZX *unlzx)
{
	unsigned long temp, total_pack = 0, total_unpack = 0, total_files = 0, merge_size = 0;
	signed long actual, abort, result = 1;
	int percent;
	
	printf("Unpacked   Packed Time     Date        Attrib   Name\n");
	printf("-------- -------- -------- ----------- -------- ----\n");
	do
	{
		abort = 1;
		actual = in_file->read(unlzx->archive_header, 1, 31);
		if (!in_file->isEOF())
		{
			if (actual)
			{
				if (actual == 31)
				{
					unlzx->sum = 0;
					unlzx->crc = (unlzx->archive_header[29] << 24) + (unlzx->archive_header[28] << 16) + (unlzx->archive_header[27] << 8) + unlzx->archive_header[26];
					unlzx->archive_header[29] =
					unlzx->archive_header[28] =
					unlzx->archive_header[27] =
					unlzx->archive_header[26] = 0;
					crc_calc(unlzx->archive_header, 31, unlzx);
					temp = unlzx->archive_header[30];
					actual = in_file->read(unlzx->header_filename, 1, temp);
					if (!in_file->isEOF())
					{
						if (actual == temp)
						{
							unlzx->header_filename[temp] = 0;
							crc_calc(unlzx->header_filename, temp, unlzx);
							temp = unlzx->archive_header[14];
							actual = in_file->read(unlzx->header_comment, 1, temp);
							if (!in_file->isEOF())
							{
								if (actual == temp)
								{
									unlzx->header_comment[temp] = 0;
									crc_calc(unlzx->header_comment, temp, unlzx);
									if (unlzx->sum == unlzx->crc)
									{
										unlzx->attributes = unlzx->archive_header[0];
										unlzx->unpack_size = (unlzx->archive_header[5] << 24) + (unlzx->archive_header[4] << 16) + (unlzx->archive_header[3] << 8) + unlzx->archive_header[2];
										unlzx->pack_size = (unlzx->archive_header[9] << 24) + (unlzx->archive_header[8] << 16) + (unlzx->archive_header[7] << 8) + unlzx->archive_header[6];
										temp = (unlzx->archive_header[18] << 24) + (unlzx->archive_header[19] << 16) + (unlzx->archive_header[20] << 8) + unlzx->archive_header[21];
										unlzx->year = ((temp >> 17) & 63) + 1970;
										unlzx->month = (temp >> 23) & 15;
										unlzx->day = (temp >> 27) & 31;
										unlzx->hour = (temp >> 12) & 31;
										unlzx->minute = (temp >> 6) & 63;
										unlzx->second = temp & 63;
										total_pack += unlzx->pack_size;
										total_unpack += unlzx->unpack_size;
										total_files++;
										merge_size += unlzx->unpack_size;
										printf("%8ld ", (long)unlzx->unpack_size);
										if (unlzx->archive_header[12] & 1)
										{
											printf("     n/a ");
										}
										else
										{
											printf("%8ld ", (long)unlzx->pack_size);
										}
										printf("%02ld:%02ld:%02ld ", (long)unlzx->hour, (long)unlzx->minute, (long)unlzx->second);
										printf("%2ld-%s-%4ld ", (long)unlzx->day, (char *)month_str[unlzx->month], (long)unlzx->year);
										printf("%c%c%c%c%c%c%c%c ",
											   ((unlzx->attributes & 32) ? 'h' : '-'),
											   ((unlzx->attributes & 64) ? 's' : '-'),
											   ((unlzx->attributes & 128) ? 'p' : '-'),
											   ((unlzx->attributes & 16) ? 'a' : '-'),
											   ((unlzx->attributes & 1) ? 'r' : '-'),
											   ((unlzx->attributes & 2) ? 'w' : '-'),
											   ((unlzx->attributes & 8) ? 'e' : '-'),
											   ((unlzx->attributes & 4) ? 'd' : '-'));
										printf("\"%s\"\n", (char *)unlzx->header_filename);
										if(unlzx->header_comment[0]) printf(": \"%s\"\n", (char *)unlzx->header_comment);
										if ((unlzx->archive_header[12] & 1) && unlzx->pack_size)
										{
											percent = make_percent(unlzx->pack_size, merge_size);
											printf("%8ld %8ld Merged (%3d%%)\n", (long)merge_size, (long)unlzx->pack_size, (int)percent);
										}
										if (unlzx->pack_size)
										{
											merge_size = 0;
											in_file->seek(unlzx->pack_size, XMFile::SeekOffsetTypeCurrent);
											abort = 0;
											just_wait();
										}
										else
										{
											abort = 0;
										}
									}
#ifdef UNLZX_DEBUG
									else
									{
										fprintf(stderr, "CRC: Archive_Header\n");
									}
#endif
								}
#ifdef UNLZX_DEBUG
								else
								{
									fprintf(stderr, "EOF: Header_Comment\n");
								}
#endif
							}
#ifdef UNLZX_DEBUG
							else
							{
								perror("FRead(Header_Comment)");
							}
#endif
						}
#ifdef UNLZX_DEBUG
						else
						{
							fprintf(stderr, "EOF: Header_Filename\n");
						}
#endif
					}
#ifdef UNLZX_DEBUG
					else
					{
						perror("FRead(Header_Filename)");
					}
#endif
				}
#ifdef UNLZX_DEBUG
				else
				{
					fprintf(stderr, "EOF: Archive_Header\n");
				}
#endif
			}
			else
			{
				printf("-------- -------- -------- ----------- -------- ----\n");
				percent = make_percent(total_pack, total_unpack);
				printf("%8ld %8ld  (%3d%%)  ", (long)total_unpack, (long)total_pack, (int)percent);
				printf("%ld file%s\n", (long)total_files, (char *)((total_files == 1) ? "" : "s"));
				result = 0;
			}
		}
#ifdef UNLZX_DEBUG
		else
		{
			perror("FRead(Archive_Header)");
		}
#endif
	} while(!abort);
	return(result);
}

signed long Unlzx::process_archive(const PPSystemString& filename, struct UnLZX *unlzx, bool& found)
{
	signed long result = 1, actual;
	XMFile* in_file = new XMFile(filename);
	
	{
		actual = in_file->read(unlzx->info_header, 1, 10);
		if (!in_file->isEOF())
		{
			if (actual == 10)
			{
				if ((unlzx->info_header[0] == 76) && (unlzx->info_header[1] == 90) && (unlzx->info_header[2] == 88))
				{
					switch (unlzx->mode)
					{
						case 1:
							result = extract_archive(in_file, unlzx, found);
							break;
						case 2:
							result = view_archive(in_file, unlzx);
							break;
					}
				}
#ifdef UNLZX_DEBUG
				else
				{
					fprintf(stderr, "Info_Header: Bad ID\n");
				}
#endif
			}
#ifdef UNLZX_DEBUG
			else
			{
				fprintf(stderr, "EOF: Info_Header\n");
			}
#endif
		}
#ifdef UNLZX_DEBUG
		else
		{
			perror("FRead(Info_Header)");
		}
#endif
		delete in_file;
	}
#ifdef UNLZX_DEBUG
	else
	{
		perror("FOpen(Archive)");
	}
#endif
	return(result);
}

#if 0
int main(int argc, char **argv)
{
#ifdef UNLZX_TIME
	int init_time, time_taken;
#endif
	int result = 0;
	unsigned long option;
	unsigned char tmp;
	struct UnLZX *unlzx;
	
	if (unlzx = unlzx_init())
	{
		if (option = argopt((unsigned char*)"-p", (unsigned long)argc, (unsigned char**)argv))
		{
			if (++option < argc)
			{
				strcpy((char*)unlzx->match_pattern, (char*)argv[option]);
			}
		}
		if (option = argopt((unsigned char*)"-o", (unsigned long)argc, (unsigned char **)argv))
		{
			if (++option < argc)
			{
				strcpy((char*)unlzx->output_dir, (char*)argv[option]);
				unlzx->use_outdir = 1;
				
				tmp = strlen((char*)unlzx->output_dir) - 1;
#ifdef AMIGA
				if ((unlzx->output_dir[tmp] != '/') && (unlzx->output_dir[tmp] != ':'))
				{
					unlzx->output_dir[tmp+1] = '/';
#else
					if (unlzx->output_dir[tmp] != '\\')
					{
						unlzx->output_dir[tmp+1] = '\\';
#endif /* AMIGA */
						unlzx->output_dir[tmp+2] = '\0';
					}
				}
			}
			
			if (option = argopt((unsigned char*)"-v", (unsigned long)argc, (unsigned char **)argv))
			{
				unlzx->mode = 2;
			}
			else if (option = argopt((unsigned char*)"-x", (unsigned long)argc, (unsigned char **)argv))
			{
				unlzx->mode = 1;
			}
			else
			{
				result = 1;
			}
			if ((!result) && (++option < argc))
			{
#ifdef UNLZX_TIME
				init_time = (int)time(NULL);
#endif
				result = process_archive((unsigned char*)argv[option], unlzx);
#ifdef UNLZX_TIME
				time_taken = (int)time(NULL) - init_time;
				printf("Time taken: %ds\n", time_taken);
#endif
			}
			else
			{
				printf( "Usage: %s <options>\n", (char *)argv[0] );
				printf( "\t-v <archive> : list archive\n"
					   "\t-x <archive> : extract archive\n"
					   "\t-p <pattern> : only matching files\n"
					   "\t-o <outpath> : destination path\n" );
				result = 2;
			}
			unlzx_free(unlzx);
		}
		return(result);
	}
	
#endif
	
Unlzx::Unlzx(const PPSystemString& archiveFilename, const FileIdentificator* identificator)
	: archiveFilename(archiveFilename)
	, identificator(identificator)
	, unlzx(NULL)
{
}

Unlzx::~Unlzx()
{
	if (unlzx)
		unlzx_free(unlzx);
}

bool Unlzx::extractFile(bool extract, const PPSystemString* outFilename)
{
	int result = 0;
	
	if ((unlzx = unlzx_init()))
	{
		if (extract)
		{
			unlzx->mode = 1;
			unlzx->temporaryFile = outFilename;
			bool found = false;
			// TODO: make this all type safe
			result = process_archive(archiveFilename, unlzx, found);
			return found && (result == 0);
		}
	}
	
	return false;
}
