/***********************************************************
	shuf.c -- extract static Huffman coding
***********************************************************/

#define N1 286  /* alphabet size */
#define N2 (2 * N1 - 1)  /* # of nodes in Huffman tree */
#define EXTRABITS 8
	/* >= log2(F-THRESHOLD+258-N1) */
#define BUFBITS  16  /* >= log2(MAXBUF) */
#define LENFIELD  4  /* bit size of length field for tree output */
#define NP0 (8 * 1024 / 64)
#define NP2 (NP0 * 2 - 1)

void CLhaArchive::decode_start_st0()
{
	n_max = 286;
	maxmatch = MAXMATCH;
	init_getbits();
	np = 1 << (MAX_DICBIT - 6);
}

const int fixed[2][16] = 
{
	{3, 0x01, 0x04, 0x0c, 0x18, 0x30, 0},				/* old compatible */
	{2, 0x01, 0x01, 0x03, 0x06, 0x0D, 0x1F, 0x4E, 0}	/* 8K buf */
};

void CLhaArchive::ready_made(int method)
{
	int i, j;
	unsigned int code, weight;
	const int *tbl;

	tbl = fixed[method];
	j = *tbl++;
	weight = 1 << (16 - j);
	code = 0; 
	for (i = 0; i < (int)np; i++) 
	{
		while (*tbl == i) {
			j++;
			tbl++;
			weight >>= 1;
		}
		gpHufData->pt_len[i] = j;
		gpHufData->pt_code[i] = code;
		code += weight;
	}
}


void CLhaArchive::read_tree_c()  /* read tree from file */
{
	int i, c;

	i = 0;
	while (i < N1) {
		if (getbits(1)) gpHufData->c_len[i] = getbits(LENFIELD) + 1;
		else 		  gpHufData->c_len[i] = 0;
		if (++i == 3 && gpHufData->c_len[0] == 1 && gpHufData->c_len[1] == 1 && gpHufData->c_len[2] == 1) {
			c = getbits(CBIT);
			for (i = 0; i < N1; i++) gpHufData->c_len[i] = 0;
			for (i = 0; i < 4096; i++) gpHufData->c_table[i] = c;
			return;
		}
	}
	make_table(N1, gpHufData->c_len, 12, gpHufData->c_table);
}

void CLhaArchive::read_tree_p()  /* read tree from file */
{
	int i, c;

	i = 0;
	while (i < NP0) {
		gpHufData->pt_len[i] = (unsigned char)getbits(LENFIELD);
		if (++i == 3 && gpHufData->pt_len[0] == 1 && gpHufData->pt_len[1] == 1 && gpHufData->pt_len[2] == 1)
		{
			c = getbits(MAX_DICBIT - 6);
			for (i = 0; i < NP0; i++) gpHufData->c_len[i] = 0;
			for (i = 0; i < 256; i++) gpHufData->c_table[i] = c;
			return;
		}
	}
}

void CLhaArchive::decode_start_fix()
{
	n_max = 314;
	maxmatch = 60;
	init_getbits();
	np = 1 << (12 - 6);
	start_c_dyn();
	ready_made(0);
	make_table(np, gpHufData->pt_len, 8, gpHufData->pt_table);
}

unsigned short CLhaArchive::decode_c_st0()
{
	int i, j;

	if (blocksize == 0) {  /* read block head */
		blocksize = getbits(BUFBITS);  /* read block blocksize */
		read_tree_c();
		if (getbits(1)) {
			read_tree_p();
		} else 
		{
			ready_made(1);
		}
		make_table(NP0, gpHufData->pt_len, 8, gpHufData->pt_table);
	}
	blocksize--;
	j = gpHufData->c_table[bitbuf >> 4];
	if (j < N1) fillbuf(gpHufData->c_len[j]);
	else {
		fillbuf(12); i = bitbuf;
		do {
			j = ((short)i < 0) ? right[j] : left[j];
			i <<= 1;
		} while (j >= N1);
		fillbuf(gpHufData->c_len[j] - 12);
	}
	if (j == N1 - 1)
		j += getbits(EXTRABITS);
	return j;
}

unsigned short CLhaArchive::decode_p_st0()
{
	int j = gpHufData->pt_table[bitbuf >> 8];
	if (j < (int)np) 
	{
		fillbuf(gpHufData->pt_len[j]);
	} else 
	{
		int i;
		fillbuf(8); i = bitbuf;
		do {
			j = ((short)i < 0) ? right[j] : left[j];
			i <<= 1;
		} while (j >= (int)np);
		fillbuf(gpHufData->pt_len[j] - 8);
	}
	return (j << 6) + getbits(6);
}
