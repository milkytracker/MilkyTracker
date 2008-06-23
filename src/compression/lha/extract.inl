/***********************************************************
	extract.c -- extract file from archive
***********************************************************/

int CLhaArchive::decode_lzhuf(pp_uint32 &infp, pp_uint8* outfp, long original_size, long packed_size, int method)
{
	LzInterface.method = method;
	LzInterface.dicbit = 13; // method + 8;
	LzInterface.infile = infp;
	LzInterface.outfile = outfp;
	LzInterface.original = original_size;
	LzInterface.packed = packed_size;
#ifdef LHADEBUG
	Log("Decode_lzhuf: method=%d\n", method);
#endif
	switch (method)
	{
	case 0:
	case 8:
		{
			int len1 = m_dwOutputLen - (int)(outfp - m_lpOutputFile);
			int len2 = m_dwStreamLen - infp;
			int len = (len1 < len2) ? len1 : len2;
			for (int i=0; i<len; i++)
			{
				outfp[i] = m_lpStream[i+infp];
			}
			LzInterface.infile += len;
			outfp += len;
			crc = 0;
		}
		break;
	case 6:	// -lzs-
		LzInterface.dicbit = 11;
		decode(&LzInterface);
		break;
	case 1: // -lh1-
	case 4: // -lh4-
	case 7: // -lz5-
		LzInterface.dicbit = 12;
	default:
		decode(&LzInterface);
	}
	infp = LzInterface.infile;
	return crc;
}
