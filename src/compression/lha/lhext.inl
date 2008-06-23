/*----------------------------------------------------------------------*/
/*			LHarc Extract Command										*/
/*		This is part of LHarc UNIX Archiver Driver						*/
/*																		*/
/*		Copyright(C) MCMLXXXIX  Yooichi.Tagawa							*/
/*																		*/
/*  V0.00  Original				1988.05.23  Y.Tagawa					*/
/*  V1.00  Fixed				1989.09.22  Y.Tagawa					*/
/*  V0.03  LHa for UNIX				1991.12.17  M.Oki					*/
/*----------------------------------------------------------------------*/


pp_uint8* CLhaArchive::open_with_make_path(const char *, int size)
//-------------------------------------------------------------
{
	if (m_lpOutputFile)
	{
		if (size < (int)m_dwOutputLen) return NULL;
		delete[] m_lpOutputFile;
		m_dwOutputLen = 0;
		m_lpOutputFile = NULL;
	}
	m_dwOutputLen = size;
	m_lpOutputFile = new pp_uint8[size];
	return m_lpOutputFile;
}


const char *methods[10] =
{
	"-lh0-", "-lh1-", "-lh2-", "-lh3-", "-lh4-",
	"-lh5-", "-lzs-", "-lz5-", "-lz4-", NULL
};


void CLhaArchive::extract_one(pp_uint32 &afp, LzHeader *hdr)
//------------------------------------------------------
{
	pp_uint8* fp;	// output file
	int method;

	if ((hdr->unix_mode & UNIX_FILE_TYPEMASK) == UNIX_FILE_REGULAR)
	{
		for (method = 0; ; method++)
		{
			if (methods[method] == NULL)
			{
			#ifdef LHADEBUG
				CHAR s[6];
				memcpy(s, hdr->method, 5);
				s[5] = 0;
				Log("Unknown method skipped (%s)...\n", s);
			#endif
				return;
			}
			if (!strncmp(hdr->method, methods[method], 5)) break;
		}

		if ((fp = open_with_make_path(hdr->name, hdr->original_size)) != NULL)
		{
			int crc = decode_lzhuf(afp, fp, hdr->original_size, hdr->packed_size, method);

			if (hdr->has_crc && crc != hdr->crc)
			{
			#ifdef LHADEBUG
				Log("CRC error\n");
			#endif
			}
		}
	}
#ifdef LHADEBUG
	else
	{
		Log("Nothing to do for %s\n", hdr->name);
	}
#endif
}

