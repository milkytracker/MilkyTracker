#include "unlha32.h"

#include "lharc.h"

#undef CHAR_BIT
#undef UCHAR_MAX
#undef USHRT_MAX
#undef SHRT_MAX
#undef SHRT_MIN

#include "slidehuf.h"
#include "header.inl"
#include "lhext.inl"
#include "extract.inl"
#include "slide.inl"
#include "maketbl.inl"
#include "dhuf.inl"
#include "huf.inl"
#include "shuf.inl"
#include "larc.inl"

CLhaArchive::CLhaArchive(/*pp_uint8* lpStream*/StreamerBase& streamer, pp_uint32 dwMemLength, IDNotifier* notifier) :
	m_lpStream(streamer),
	m_notifier(notifier)
//---------------------------------------------------------------------------------
{
	// File Read
	//m_lpStream = lpStream;
	m_dwStreamLen = dwMemLength;
	m_dwStreamPos = 0;
	// File Write
	m_lpOutputFile = 0;
	m_dwOutputLen = 0;
	m_pDecoderData = NULL;
}


CLhaArchive::~CLhaArchive()
//-------------------------
{
	if (m_lpOutputFile)
	{
		delete[] m_lpOutputFile;
		m_lpOutputFile = NULL;
	}
	if (m_pDecoderData)
	{
		delete[] m_pDecoderData;
		m_pDecoderData = NULL;
	}
}


bool CLhaArchive::IsArchive()
//---------------------------
{
	LzHeader hdr;
	pp_uint32 pos = 0;

	if (!get_header(pos, &hdr)) return false;
#ifdef LHADEBUG
	Log("LHA Archive\n");
#endif
	return true;
}


bool CLhaArchive::ExtractFile()
//-----------------------------
{
	LzHeader hdr;

	//if (!m_lpStream) return false;

	if (!m_pDecoderData)
	{
		m_pDecoderData = new pp_uint8[65536]; // 64K of data - should be enough
		if (!m_pDecoderData) return false;
	}

	// Init misc tables
	InitDecodeTables();
	InitHufTables();
	make_crctable();

	// extract each files
	while (get_header(m_dwStreamPos, &hdr))
	{
	#ifdef LHADEBUG
		Log("%d bytes packed in %s\n", hdr.packed_size, hdr.name);
	#endif
		blocksize = 0;
		long pos = m_dwStreamPos;
		extract_one(m_dwStreamPos, &hdr);
		
		if (m_notifier && m_lpOutputFile && m_dwOutputLen)
		{
			if (m_notifier->identify(m_lpOutputFile, m_dwOutputLen))
				break;
		}
		
		m_dwStreamPos = pos + hdr.packed_size;
	}

#ifdef LHADEBUG
	if (m_lpOutputFile)
	{
		Log("%d bytes extracted\n", m_dwOutputLen);
	}
#endif

	return (m_lpOutputFile) ? true : false;
}


int CLhaArchive::lharead(void *p, int sz1, int sz2, pp_uint32 &fp)
//------------------------------------------------------------
{
	int sz = sz1 * sz2;
	int bytesavailable = m_dwStreamLen - fp;
	if (sz > bytesavailable) sz = bytesavailable;
	if ((sz <= 0) || (!p) /*|| (!m_lpStream)*/) return 0;
	//memcpy(p, m_lpStream + fp, sz);
	
	m_lpStream.read(p, fp, sz);
	
	fp += sz;
	return sz;
}


#define CRCPOLY  0xA001  // CRC-16
#define UPDATE_CRC(c) \
	crc = crctable[(crc ^ (c)) & 0xFF] ^ (crc >> CHAR_BIT)

void CLhaArchive::make_crctable()
//-------------------------------
{
	for (unsigned int i = 0; i < 256; i++)
	{
		unsigned int r = i;
		for (unsigned int j = 0; j < CHAR_BIT; j++)
			if (r & 1)
				r = (r >> 1) ^ CRCPOLY;
			else
				r >>= 1;
		crctable[i] = r;
	}
}


unsigned short CLhaArchive::calccrc(unsigned char *p , int n)
//-----------------------------------------------------------
{
	while (n-- > 0) UPDATE_CRC(*p++);
	return crc;
}



void CLhaArchive::fwrite_crc(unsigned char *p, int n, pp_uint8* &fp)
//---------------------------------------------------------------
{
#ifdef LHADEBUG
	Log("Writing %d bytes", n);
#endif
	calccrc(p, n);
	if (fp)
    {
		int len = m_dwOutputLen - (int)(fp - m_lpOutputFile);
		if (n > len) n = len;
		for (int i=0; i<n; i++)
		{
			*fp++ = p[i];
		}
    }
#ifdef LHADEBUG
	Log("...\n");
#endif
}


// Shift bitbuf n bits left, read n bits
void CLhaArchive::fillbuf(unsigned char n)
//----------------------------------------
{
	while (n > bitcount)
	{
		n -= bitcount;
		bitbuf = (bitbuf << bitcount) + (subbitbuf >> (CHAR_BIT - bitcount));
		subbitbuf = 0;
		if (compsize != 0)
		{
			compsize--;
			if (LzInterface.infile < m_dwStreamLen) subbitbuf = (unsigned char)m_lpStream[LzInterface.infile++];
		}
		bitcount = CHAR_BIT;
	}
	bitcount -= n;
	bitbuf = (bitbuf << n) + (subbitbuf >> (CHAR_BIT - n));
	subbitbuf <<= n;
}


void CLhaArchive::init_getbits()
//------------------------------
{
	bitbuf = 0;  subbitbuf = 0;  bitcount = 0;
	fillbuf(2 * CHAR_BIT);
}


unsigned short CLhaArchive::getbits(unsigned int n)
//-------------------------------------------------
{
	unsigned short x;
	n &= 0xff;
	x = bitbuf >> (2 * CHAR_BIT - n);  fillbuf(n);
	return x;
}

