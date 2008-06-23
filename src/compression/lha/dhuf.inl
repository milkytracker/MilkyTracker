/**************************************************************
	title   dhuf.c
***************************************************************
    Dynamic Huffman routine
                                                H.Yoshizaki
**************************************************************/

#define N_CHAR      (256 + 60 - THRESHOLD + 1)
#define TREESIZE_C  (N_CHAR * 2)
#define TREESIZE_P  (128 * 2)
#define TREESIZE    (TREESIZE_C + TREESIZE_P)
#define ROOT_C      0
#define ROOT_P      TREESIZE_C


void CLhaArchive::InitDecodeTables()
{
	short *pDecodeTables = (short *)m_pDecoderData;
	child = pDecodeTables;
	parent = pDecodeTables + TREESIZE;
	block = pDecodeTables + TREESIZE*2;
	edge = pDecodeTables + TREESIZE*3;
	stock = pDecodeTables + TREESIZE*4;
	node = pDecodeTables + TREESIZE*5;
	freq = (unsigned short *)(pDecodeTables + TREESIZE*6);
}


void CLhaArchive::start_c_dyn()
{
	int i, j, f;

	n1 = ((int)n_max >= 256 + maxmatch - THRESHOLD + 1) ? 512 : n_max - 1;
	for (i = 0; i < TREESIZE_C; i++) {
		stock[i] = i;
		block[i] = 0;
	}
	for (i = 0, j = n_max * 2 - 2; i < (int)n_max; i++, j--) 
	{
		freq[j] = 1;
		child[j] = ~i;
		node[i] = j;
		block[j] = 1;
	}
	avail = 2;
	edge[1] = n_max - 1;
	i = n_max * 2 - 2;
	while (j >= 0) {
		f = freq[j] = freq[i] + freq[i - 1];
		child[j] = i;
		parent[i] = parent[i - 1] = j;
		if (f == freq[j + 1]) {
			edge[block[j] = block[j + 1]] = j;
		} else {
			edge[block[j] = stock[avail++]] = j;
		}
		i -= 2;
		j--;
	}
}

void CLhaArchive::start_p_dyn()
{
	freq[ROOT_P] = 1;
	child[ROOT_P] = ~(N_CHAR);
	node[N_CHAR] = ROOT_P;
	edge[block[ROOT_P] = stock[avail++]] = ROOT_P;
	most_p = ROOT_P;
	total_p = 0;
	nn = 1 << dicbit;
	nextcount = 64;
}

void CLhaArchive::decode_start_dyn()
{
	n_max = 286;
	maxmatch = MAXMATCH;
	init_getbits();
	start_c_dyn();
	start_p_dyn();
}

void CLhaArchive::reconst(int start, int end)
{
	int i, j, k, l, b;
	unsigned int f, g;

	for (i = j = start; i < end; i++) {
		if ((k = child[i]) < 0) {
			freq[j] = (freq[i] + 1) / 2;
			child[j] = k;
			j++;
		}
		if (edge[b = block[i]] == i) {
			stock[--avail] = b;
		}
	}
	j--;
	i = end - 1;
	l = end - 2;
	while (i >= start) {
		while (i >= l) {
			freq[i] = freq[j]; child[i] = child[j];
			i--, j--;
		}
		f = freq[l] + freq[l + 1];
		for (k = start; f < freq[k]; k++);
		while(j >= k) {
			freq[i] = freq[j]; child[i] = child[j];
			i--, j--;
		}
		freq[i] = f; child[i] = l + 1;
		i--;
		l -= 2;
	}
	f = 0;
	for (i = start; i < end; i++) {
		if ((j = child[i]) < 0) node[~j] = i;
		else parent[j] = parent[j - 1] = i;
		if ((g = freq[i]) == f) {
			block[i] = b;
		} else {
			edge[b = block[i] = stock[avail++]] = i;
			f = g;
		}
	}
}

int CLhaArchive::swap_inc(int p)
{
	int b, q, r, s;

	b = block[p];
	if ((q = edge[b]) != p) {	/* swap for leader */
		r = child[p]; s = child[q];
		child[p] = s; child[q] = r;
		if (r >= 0) parent[r] = parent[r - 1] = q;
		else		node[~r] = q;
		if (s >= 0)	parent[s] = parent[s - 1] = p;
		else		node[~s] = p;
		p = q;
		goto Adjust;
	} else if (b == block[p + 1]) {
Adjust:
		edge[b]++;
		if (++freq[p] == freq[p - 1]) {
			block[p] = block[p - 1];
		} else {
			edge[block[p] = stock[avail++]] = p;	/* create block */
		}
	} else if (++freq[p] == freq[p - 1]) {
		stock[--avail] = b;		/* delete block */
		block[p] = block[p - 1];
	}
	return parent[p];
}

void CLhaArchive::update_c(int p)
{
	int q;

	if (freq[ROOT_C] == 0x8000) {
		reconst(0, n_max * 2 - 1);
	}
	freq[ROOT_C]++;
	q = node[p];
	do {
		q = swap_inc(q);
	} while (q != ROOT_C);
}

void CLhaArchive::update_p(int p)
{
	int q;

	if (total_p == 0x8000) {
		reconst(ROOT_P, most_p + 1);
		total_p = freq[ROOT_P];
		freq[ROOT_P] = 0xffff;
	}
	q = node[p + N_CHAR];
	while (q != ROOT_P) {
		q = swap_inc(q);
	}
	total_p++;
}

void CLhaArchive::make_new_node(int p)
{
	int q, r;

	r = most_p + 1; q = r + 1;
	node[~(child[r] = child[most_p])] = r;
	child[q] = ~(p + N_CHAR);
	child[most_p] = q;
	freq[r] = freq[most_p];
	freq[q] = 0;
	block[r] = block[most_p];
	if (most_p == ROOT_P) {
		freq[ROOT_P] = 0xffff;
		edge[block[ROOT_P]]++;
	}
	parent[r] = parent[q] = most_p;
	edge[block[q] = stock[avail++]] = node[p + N_CHAR] = most_p = q;
	update_p(p);
}


unsigned short CLhaArchive::decode_c_dyn()
{
	int c;
	short buf, cnt;

	c = child[ROOT_C];
	buf = bitbuf;
	cnt = 0;
	do {
		c = child[c - (buf < 0)];
		buf <<= 1;
		if (++cnt == 16) {
			fillbuf(16);
			buf = bitbuf; cnt = 0;
		}
	} while (c > 0);
	fillbuf((unsigned char)cnt);
	c = ~c;
	update_c(c);
	if (c == n1) c += getbits(8);
	return c;
}


unsigned short CLhaArchive::decode_p_dyn()
{
	int c;
	short buf, cnt;

	while (count > nextcount) {
		make_new_node(nextcount / 64);
		if ((nextcount += 64) >= (unsigned int)nn)
			nextcount = 0xffffffff;
	}
	c = child[ROOT_P];
	buf = bitbuf; cnt = 0;
	while (c > 0) {
		c = child[c - (buf < 0)];
		buf <<= 1;
		if (++cnt == 16) {
			fillbuf(16);
			buf = bitbuf; cnt = 0;
		}
	}
	fillbuf((unsigned char)cnt);
	c = (~c) - N_CHAR;
	update_p(c);

	return (c << 6) + getbits(6);
}

