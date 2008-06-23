/***********************************************************
	slide.c -- sliding dictionary with percolating update
***********************************************************/


/*
static decode_option decode_define[7] = 
{
	// lh1
	{decode_c_dyn, decode_p_st0, decode_start_fix},
	// lh2
	{decode_c_dyn, decode_p_dyn, decode_start_dyn},
	// lh3
	{decode_c_st0, decode_p_st0, decode_start_st0},
	// lh4
	{decode_c_st1, decode_p_st1, decode_start_st1},
	// lh5
	{decode_c_st1, decode_p_st1, decode_start_st1},
	// lzs
	{decode_c_lzs, decode_p_lzs, decode_start_lzs},
	// lz5
	{decode_c_lz5, decode_p_lz5, decode_start_lz5}
};
*/

void CLhaArchive::decode(LzInterfacing *pinterface)
{
	int dicsiz1, offset;
	unsigned char *text;

	dicbit = pinterface->dicbit;
	compsize = pinterface->packed;
	crc = 0;
	prev_char = -1;
	dicsiz = 1 << dicbit;
	text = new unsigned char[dicsiz];
	if (!text) return;
	memset(text, ' ', dicsiz);
#ifdef LHADEBUG
	Log("Initializing Decode...\n");
#endif
	switch(pinterface->method - 1)
	{
	case 1:		decode_start_dyn(); break;
	case 2:		decode_start_st0(); break;
	case 3:		decode_start_st1(); break;
	case 4:		decode_start_st1(); break;
	case 5:		decode_start_lzs(); break;
	case 6:		decode_start_lz5(text); break;
	default:	decode_start_fix(); break;
	}
#ifdef LHADEBUG
	Log("Starting Decode (original size=%d)...\n", pinterface->original);
#endif
	dicsiz1 = dicsiz - 1;
	offset = (pinterface->method == 6) ? 0x100 - 2 : 0x100 - 3;
	count = 0;
	loc = 0;
	while (count < pinterface->original)
	{
		int c;
		switch(pinterface->method - 1)
		{
		case 2:		c = decode_c_st0(); break;
		case 3:		c = decode_c_st1(); break;
		case 4:		c = decode_c_st1(); break;
		case 5:		c = decode_c_lzs(); break;
		case 6:		c = decode_c_lz5(); break;
		default:	c = decode_c_dyn(); break;
		}
		if (c <= 255)
		{
			text[loc++] = c;
			if (loc == dicsiz) 
			{
				fwrite_crc(text, dicsiz, pinterface->outfile);
				loc = 0;
			}
			count++;
		} else 
		{
			int d;
			switch(pinterface->method - 1)
			{
			case 1:		d = decode_p_dyn(); break;
			case 2:		d = decode_p_st0(); break;
			case 3:		d = decode_p_st1(); break;
			case 4:		d = decode_p_st1(); break;
			case 5:		d = decode_p_lzs(); break;
			case 6:		d = decode_p_lz5(); break;
			default:	d = decode_p_st0(); break;
			}
			int j = c - offset;
			int i = (loc - d - 1) & dicsiz1;
			count += j;
			for (int k = 0; k < j; k++) 
			{
				c = text[(i + k) & dicsiz1];
				text[loc++] = c;
				if (loc == dicsiz) 
				{
					fwrite_crc(text, dicsiz, pinterface->outfile);
					loc = 0;
				}
			}
		}
	}
	if (loc != 0) 
	{
		fwrite_crc(text, loc, pinterface->outfile);
	}
	delete text;
#ifdef LHADEBUG
	Log("Decoding Done!\n");
#endif
}

