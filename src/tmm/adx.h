typedef struct {
	int s1,s2;
} PREV;

#define	BASEVOL	0x11e0

static void
convert(short *out, unsigned char *in, PREV *prev)
{
	int scale = ((in[0]<<8)|(in[1]));
	int i;
	int s0,s1,s2,d;
//	int over=0;
	//printf("scale = %d\n", scale);

	in+=2;
	s1 = prev->s1;
	s2 = prev->s2;
	for(i=0;i<16;i++) {
		d = in[i]>>4;
		if (d&8) d-=16;
		s0 = (BASEVOL*d*scale + 0x7298*s1 - 0x3350*s2)>>14;
	//	if (abs(s0)>32767) over=1;
		if (s0>32767) s0=32767;
		else if (s0<-32768) s0=-32768;

		*out++=s0;
		s2 = s1;
		s1 = s0;

		d = in[i]&15;
		if (d&8) d-=16;
		s0 = (BASEVOL*d*scale + 0x7298*s1 - 0x3350*s2)>>14;
	//	if (abs(s0)>32767) over=1;
		if (s0>32767) s0=32767;
		else if (s0<-32768) s0=-32768;
		*out++=s0;
		s2 = s1;
		s1 = s0;
	}
	prev->s1 = s1;
	prev->s2 = s2;
}
