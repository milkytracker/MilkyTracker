/***********************************************************
	maketbl.c -- makes decoding table
***********************************************************/

void CLhaArchive::make_table(unsigned int nchar, unsigned char *bitlen, unsigned int tablebits, unsigned short *table)
{
	unsigned short count[17];  /* count of bitlen */
	unsigned short weight[17]; /* 0x10000ul >> bitlen */
	unsigned short start[17];  /* first code of bitlen */
	unsigned short total;
	unsigned int i;
	int j, k, l, m, n, avail;
	unsigned short *p;

	avail = nchar;

/* initialize */
	for (i = 1; i <= 16; i++) {
		count[i] = 0;
		weight[i] = 1 << (16 - i);
	}

/* count */
	for (i = 0; i < nchar; i++) count[bitlen[i]]++;

/* calculate first code */
	total = 0;
	for (i = 1; i <= 16; i++) {
		start[i] = total;
		total += weight[i] * count[i];
	}
	if ((total & 0xffff) != 0)
	{
		return;
	}

/* shift data for make table. */
	m = 16 - tablebits;
	for (i = 1; i <= tablebits; i++) {
		start[i] >>= m;
		weight[i] >>= m;
	}

/* initialize */
	j = start[tablebits + 1] >> m;
	k = 1 << tablebits;
	if (j != 0)
		for (i = j; i < (unsigned int)k; i++) table[i] = 0;

/* create table and tree */
	for (j = 0; j < (int)nchar; j++) {
		k = bitlen[j];
		if (k == 0) continue;
		l = start[k] + weight[k];
		if (k <= (int)tablebits) {
		/* code in table */
			for (i = start[k]; i < (unsigned int)l; i++) table[i] = j;
		} else {
		/* code not in table */
			p = &table[(i = start[k]) >> m];
			i <<= tablebits;
			n = k - tablebits;
		/* make tree (n length) */
			while (--n >= 0) {
				if (*p == 0) {
					right[avail] = left[avail] = 0;
					*p = avail++;
				}
				if (i & 0x8000) p = &right[*p];
				else            p = &left[*p];
				i <<= 1;
			}
			*p = j;
		}
		start[k] = l;
	}
}
