/***********************************************************
	larc.c -- extract *.lzs
***********************************************************/

#define MAGIC0 18
#define MAGIC5 19

unsigned short CLhaArchive::decode_c_lzs(void)
{
	if (getbits(1)) {
		return getbits(8);
	} else 
	{
		matchpos = getbits(11);
		return getbits(4) + 0x100;
	}
}

unsigned short CLhaArchive::decode_p_lzs(void)
{
	return (loc - matchpos - MAGIC0) & 0x7ff;
}

void CLhaArchive::decode_start_lzs(void)
{
	init_getbits();
}

unsigned short CLhaArchive::decode_c_lz5(void)
{
	int c;

	if (flagcnt == 0) {
		flagcnt = 8;
		flag = m_lpStream[LzInterface.infile++];
	}
	flagcnt--;
	c = m_lpStream[LzInterface.infile++];
	if ((flag & 1) == 0) {
		matchpos = c;
		c = m_lpStream[LzInterface.infile++];
		matchpos += (c & 0xf0) << 4;
		c &= 0x0f;
		c += 0x100;
	}
	flag >>= 1;
	return c;
}

unsigned short CLhaArchive::decode_p_lz5(void)
{
	return (loc - matchpos - MAGIC5) & 0xfff;
}

void CLhaArchive::decode_start_lz5(unsigned char *text)
{
	int i;

	flagcnt = 0;
	for (i = 0; i < 256; i++) memset(&text[i * 13 + 18], i, 13);
	for (i = 0; i < 256; i++) text[256 * 13 + 18 + i] = i;
	for (i = 0; i < 256; i++) text[256 * 13 + 256 + 18 + i] = 255 - i;
	memset(&text[256 * 13 + 512 + 18], 0, 128);
	memset(&text[256 * 13 + 512 + 128 + 18], ' ', 128 - 18);
}
