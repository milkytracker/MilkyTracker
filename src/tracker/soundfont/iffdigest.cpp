// https://github.com/schnitzeltony/soundfont-cmdline-tools
// SPDX: GPL-2
//
#include "iffdigest.h"
#include <algorithm>
#include <string.h>
#include <iostream>

static IFFChunkList
parseChunks(const char* mem, enum IFFFormat fmt, unsigned int len);

// compare a chunk ID (unsigned int) to a (4-letter) string.

static inline bool
ckid_cmp(iff_ckid_t id, const char* tag)
{
  return (id == iff_ckid(tag));
}

//  ----- IFFChunkList methods 

IFFChunkIterator
IFFChunkList::findNextChunk(IFFChunkIterator from, iff_ckid_t ckid)
{
  return find(++from, end(), ckid);
}

IFFChunkIterator
IFFChunkList::findChunk(iff_ckid_t ckid)
{
  return find(begin(), end(), ckid);
}

//  ----- IFFChunk methods 
//  ----- IFFChunk methods 

IFFChunk::IFFChunk(const IFFChunk& ck)
 : ckid(ck.ckid), tnull('\0'), ctype(ck.ctype), data(0), length(0) {
  if(ctype == IFF_CHUNK_DATA) {
    length=ck.length; data=ck.data;
  } else {
    subchunks = ck.subchunks;
  } 
}


void
IFFChunk::operator=(const IFFChunk& ck)
{
  ckid = ck.ckid;
  tnull = '\0';
  ctype = ck.ctype;
  switch(ctype) {
  case IFF_CHUNK_DATA:
    data = ck.data; length = ck.length; break;
  case IFF_CHUNK_LIST:
    subchunks = ck.subchunks;  data = 0; length = 0;
  }
}

bool
IFFChunk::writeData(std::size_t &len, char* outData,
  const enum IFFFormat iffFormat, const std::size_t maxLen)
{
  bool bOk = true;
  std::size_t listLenStart = len;
  std::size_t lenAdd;
  if(ctype == IFF_CHUNK_LIST) {
    // 'LIST'
    lenAdd = 4;
    if(outData) {
      if(maxLen >= len+lenAdd) {
        memcpy(outData+len, "LIST", lenAdd);
      }
      else {
        std::cerr<<"Cannot write LIST - max data length exceeded!\n";
        bOk = false;
      }
    }
    len += lenAdd;
    listLenStart = len;
    // LISTlength unknown here -> set below
    len += lenAdd;
  }

  // chunk ID
  lenAdd = sizeof(iff_ckid_t);
  if(outData) {
    if(maxLen >= len+lenAdd) {
      memcpy(outData+len, &ckid, lenAdd);
    }
    else {
      std::cerr<<"Cannot write chunk ID - max data length exceeded!\n";
      bOk = false;
    }
  }
  len+=lenAdd;

  if(ctype == IFF_CHUNK_DATA) {
    // chunk len
    unsigned int chunkLen;
    lenAdd = sizeof(chunkLen);
    if(outData) {
      if(maxLen >= len+lenAdd) {
        chunkLen = u32(length, iffFormat);
        memcpy(outData+len, &chunkLen, lenAdd);
      }
      else {
        std::cerr<<"Cannot write chunk length - max data length exceeded!\n";
        bOk = false;
      }
    }
    len+=lenAdd;

    // chunk data
    if(outData) {
      if(maxLen >= len+length) {
        memcpy(outData+len, data, length);
      }
      else {
        std::cerr<<"Cannot write chunk data - max data length exceeded!\n";
        bOk = false;
      }
    }
    len += length;
  }

  // Write all sub chunks recursively
  IFFChunkIterator subChunkIterator;
  for(subChunkIterator = ck_begin();
      subChunkIterator != ck_end(); subChunkIterator++) {
    (*subChunkIterator).writeData(len, outData, iffFormat, maxLen);
  }

  // ListLen
  unsigned int listLen;
  lenAdd = sizeof(listLen);
  if(ctype == IFF_CHUNK_LIST && outData) {
    if(maxLen >= listLenStart+lenAdd) {
      // length itself is not part of length -> '- sizeof(unsigned int)'
      listLen = u32(len - listLenStart - lenAdd, iffFormat);
      memcpy(outData+listLenStart, &listLen, lenAdd);
    }
    else {
      std::cerr<<"Cannot write list length - max data length exceeded!\n";
      bOk = false;
    }
  }
  return bOk;
}

static IFFChunk
parseChunk(const char* mem, enum IFFFormat fmt, unsigned int *clen)
{
  unsigned int len;
  len = u32(((unsigned int*)mem)[1], fmt);
  *clen = ((len+8)+1)&0xfffffffe;
  if(ckid_cmp(((unsigned int*)mem)[0], "LIST")) {
    IFFChunkList sc = parseChunks(mem+12, fmt, len-4);
    return IFFChunk( ((unsigned int*)mem)[2], sc);
  } else {
    return IFFChunk(*((unsigned int*)mem), mem+8, len);
  }
}

static IFFChunkList
parseChunks(const char* mem, enum IFFFormat fmt, unsigned int len)
{
  IFFChunkList result;

  while(len>0) {
    unsigned int cl;
    IFFChunk c = parseChunk(mem, fmt, &cl);
    mem+=cl; len -= cl;
    result.push_back(c);
  }
  return result;
}

IFFDigest::IFFDigest(const char* data, unsigned int dlen)
 : tnull('\0'), contents(data)
{
  if(ckid_cmp(((unsigned int*)data)[0], "FORM")) {
    ftype = IFF_FMT_IFF85;
  } 
  else if(ckid_cmp(((unsigned int*)data)[0], "RIFF")) {
    ftype = IFF_FMT_RIFF;
  } else {
    // maybe throw an exception here?
    ftype = IFF_FMT_ERROR;
    return;
  }
  unsigned int len = u32(((unsigned int*)data)[1], ftype)-4;
  if(len>dlen-12) {
    // illegal length in top level; maybe throw an exception here?
    ftype = IFF_FMT_ERROR;
    return;
  }
  fid = ((unsigned int*)data)[2];
  chunks = parseChunks(data+12, ftype, len);
}
