// https://github.com/schnitzeltony/soundfont-cmdline-tools
// SPDX: GPL-2
//
//  iffdigest.h  C++ classes for a simple (in-memory) IFF/RIFF file parser.

#ifndef __IFFPARSER_H
#define __IFFPARSER_H

#include <cstddef>
#include <list>

typedef unsigned int iff_ckid_t;

//  utility functions 

// cast (the first 4 chars of) a string to a 32-bit int chunk ID
static inline iff_ckid_t iff_ckid(const char* id) { return *((const int*)id); }

enum IFFFormat { IFF_FMT_IFF85, IFF_FMT_RIFF, IFF_FMT_ERROR };

// gcc auto endian
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define __BIG_ENDIAN__
#endif

static inline unsigned int
swap_u32(unsigned int i)
{
  return ((i&0xff)<<24) | ((i&0xff00) <<8) | ((i&0xff0000)>>8) | (i>>24);
}

static inline unsigned int
u32_be(unsigned int i)
{
#ifdef __BIG_ENDIAN__
  return i;
#else
  return swap_u32(i);
#endif
}

static inline unsigned int
u32_le(unsigned int i)
{
#ifdef __BIG_ENDIAN__
  return swap_u32(i);
#else
  return i;
#endif
}

static inline unsigned int
u32(unsigned int i, enum IFFFormat fmt)
{
  if(fmt==IFF_FMT_RIFF)  return u32_le(i);
  else return u32_be(i);
}


class IFFChunk;
typedef std::list<IFFChunk>::iterator IFFChunkIterator;

class IFFChunkList: public std::list<IFFChunk> {
public:
  IFFChunkIterator findNextChunk(IFFChunkIterator from, iff_ckid_t ckid);
  IFFChunkIterator findChunk(iff_ckid_t ckid);
};

class IFFChunk {
protected:
  iff_ckid_t ckid;
  char tnull; 		// terminating null for id
  enum { IFF_CHUNK_DATA, IFF_CHUNK_LIST } ctype;
  const char* data;
  unsigned int length;
  IFFChunkList subchunks;
  
public:
  IFFChunk(unsigned int id, const char* d, unsigned int l) 
    : ckid(id), tnull('\0'), ctype(IFF_CHUNK_DATA), data(d), length(l) {}
  IFFChunk(unsigned int id, IFFChunkList sc) 
    : ckid(id), tnull('\0'), ctype(IFF_CHUNK_LIST), data(0), length(0), subchunks(sc) {}
  IFFChunk(const IFFChunk& ck);
  void operator=(const IFFChunk& ck);
  const char* id_str() const { return (const char*)(&ckid); }
  iff_ckid_t id() const { return ckid; }
  inline bool operator==(const char* id) const { return ckid==iff_ckid(id); }
  inline bool operator!=(const char* id) const { return ckid!=iff_ckid(id); }
  inline bool operator==(iff_ckid_t id) const { return ckid==id; }
  inline bool operator!=(iff_ckid_t id) const { return ckid!=id; }
  const char* dataPtr() const { return data; }
  unsigned int len() const { return length; }
  inline IFFChunkIterator ck_begin() { return subchunks.begin(); }
  inline IFFChunkIterator ck_end() { return subchunks.end(); }
  inline IFFChunkIterator ck_find(iff_ckid_t id) { return subchunks.findChunk(id); }
  inline IFFChunkIterator ck_findNext(IFFChunkIterator i, iff_ckid_t id) { return subchunks.findNextChunk(i, id); }
  bool writeData(std::size_t &len, char* outData,
    const enum IFFFormat iffFormat, const std::size_t maxLen); // outData=0 calc length only
};

class IFFDigest {
protected:
  enum IFFFormat ftype;
  iff_ckid_t fid;
  char tnull;
  IFFChunkList chunks;
  const char* contents;

public:
  IFFDigest(const char* data, unsigned int dlen);

  inline enum IFFFormat iffvariant() const { return ftype; }
  inline bool valid() const { return ftype != IFF_FMT_ERROR; }
  inline iff_ckid_t id() const { return fid; }
  inline const char* id_str() const { return (const char*)(&fid); }

  inline IFFChunkIterator ck_begin() { return chunks.begin(); }
  inline IFFChunkIterator ck_end() { return chunks.end(); }
  inline IFFChunkIterator ck_find(iff_ckid_t id) { return chunks.findChunk(id); }
  inline IFFChunkIterator ck_findNext(IFFChunkIterator i, iff_ckid_t id) { return chunks.findNextChunk(i, id); }
};

// functions for decoding byte-order-specific ints
static inline unsigned short iff_u16_le(const char* ptr) 
  { return ((const unsigned char*)ptr)[0]|(((const unsigned char*)ptr)[1]<<8);}
static inline unsigned short iff_u16_be(const char* ptr) 
  { return ((const unsigned char*)ptr)[1]|(((const unsigned char*)ptr)[0]<<8);}
static inline signed short iff_s16_le(const char* ptr) 
  { return (signed short)iff_u16_le(ptr); }
static inline signed short iff_s16_be(const char* ptr) 
  { return (signed short)iff_u16_be(ptr); }
static inline unsigned int iff_u32_le(const char* ptr) 
  { return ((const unsigned char*)ptr)[0] |(((const unsigned char*)ptr)[1]<<8)
    |(((const unsigned char*)ptr)[2]<<16) |(((const unsigned char*)ptr)[3]<<24);}
static inline unsigned int iff_u32_be(const char* ptr) 
  { return ((const unsigned char*)ptr)[3] |(((const unsigned char*)ptr)[2]<<8)
    |(((const unsigned char*)ptr)[1]<<16) |(((const unsigned char*)ptr)[0]<<24);}
static inline signed int iff_s32_le(const char* ptr) 
  { return (signed int)iff_u32_le(ptr); }
static inline signed int iff_s32_be(const char* ptr) 
  { return (signed int)iff_u32_be(ptr); }

static inline unsigned short 
iff_u16_le(const unsigned short& s) { return iff_u16_le((const char*)&s); }
static inline unsigned short 
iff_u16_be(const unsigned short& s) { return iff_u16_be((const char*)&s); }
static inline signed short 
iff_s16_le(const signed short& s) { return iff_s16_le((const char*)&s); }
static inline signed short 
iff_s16_be(const signed short& s) { return iff_s16_be((const char*)&s); }
static inline unsigned int 
iff_u32_le(const unsigned int& s) { return iff_u32_le((const char*)&s); }
static inline unsigned int 
iff_u32_be(const unsigned int& s) { return iff_u32_be((const char*)&s); }
static inline signed int 
iff_s32_le(const signed int& s) { return iff_s32_le((const char*)&s); }
static inline signed int 
iff_s32_be(const signed int& s) { return iff_s32_be((const char*)&s); }

#endif
