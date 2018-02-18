/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  XModule.cpp
 *  MilkyPlay module (based on a hybrid of the XM, MDL, AMS and IT formats)
 *
 *
 */
#include "XModule.h"
#include "Loaders.h"

#undef VERBOSE

#ifdef VERBOSE
	#include <stdio.h>
#endif

// heavy processing removes some of the nasty clicks found
// in 669 and PLM songs (found in 8 bit samples only)
void TXMSample::smoothLooping()
{
	if ((type&16) || (type & 3) != 1 || sample == NULL || samplen < 1024 || looplen <= 32)
		return;

	mp_sbyte* data = (mp_sbyte*)this->sample;
	
	const mp_sint32 blockSize = 8;
	
	mp_sint32 max,t;
	
	float v1 = data[loopstart];
	float v2 = data[loopstart+looplen];
	
	float avg = (v1+v2)*0.5f;
	
	// step 1: Fade to avg from what's coming before loopstart
	max = loopstart;
	if (max > blockSize) max = blockSize;
	for (t = 0; t < max; t++)
	{
		float ft = (float)t/(float)max;
		mp_sint32 index = loopstart - max + t;					
		mp_sint32 src = data[index];
		float final = src * (1.0f - ft) + (avg * ft);
		data[index] = (mp_sbyte)final;
	}
	
	// step 2: Fade from avg into what's coming after loopstart
	max = blockSize;
	for (t = 0; t < max; t++)
	{
		float ft = (float)t/(float)max;
		mp_sint32 index = loopstart + t;					
		mp_sint32 dst = data[index];
		float final = avg * (1.0f - ft) + (dst * ft);
		data[index] = (mp_sbyte)final;
	}
	
	// step 3
	for (t = 0; t < blockSize; t++)
	{
		mp_sint32 index = loopstart+looplen - blockSize + t;
		
		mp_sint32 src = data[index];
		
		float ft = (float)t/(float)blockSize;
		float final = src * (1.0f - ft) + (avg * ft);
		data[index] = (mp_sbyte)final;
	}
}

void TXMSample::restoreLoopArea()
{
	if (sample == NULL)
		return;
	
	mp_ubyte* originalSample = getPadStartAddr((mp_ubyte*)sample) + sizeof(TLoopDoubleBuffProps);
	mp_uint32 saveLen = (type&16) ? LoopAreaBackupSize * sizeof(mp_sword) : LoopAreaBackupSize;
	
	TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr((mp_ubyte*)sample);
	
	ASSERT(saveLen <= LeadingPadding / 2);

	// 16 bit sample
	if (type&16)
	{
		mp_sword* data = (mp_sword*)this->sample;
		
		// save "real" loop area back from double buffer to sample
		if ((loopBufferProps->state[1] & 16) == (type & 16))
			memcpy(data+loopBufferProps->lastloopend, originalSample, saveLen);
		else
		{
			// buffer has been 8 bit, now we're 16 bit => convert upwards
			for (mp_sint32 i = 0; i < LoopAreaBackupSize; i++)
				data[loopBufferProps->lastloopend+i] = ((mp_sbyte*)originalSample)[i] << 8;
		}
		loopBufferProps->state[0] = TLoopDoubleBuffProps::StateUnused;
	}
	// 8 bit sample
	else 
	{			
		mp_sbyte* data = (mp_sbyte*)this->sample;
	
		// save "real" loop area back from double buffer to sample
		if ((loopBufferProps->state[1] & 16) == (type & 16))
			memcpy(data+loopBufferProps->lastloopend, originalSample, saveLen);
		else
		{
			// buffer has been 16 bit, now we're 8 bit => convert downwards
			for (mp_sint32 i = 0; i < LoopAreaBackupSize; i++)
				data[loopBufferProps->lastloopend+i] = ((mp_sword*)originalSample)[i] >> 8;
		}
		loopBufferProps->state[0] = TLoopDoubleBuffProps::StateUnused;
	}
}

void TXMSample::restoreOriginalState()
{
	if (sample == NULL)
		return;
	
	TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr((mp_ubyte*)sample);
	
	if (loopBufferProps->state[0] == TLoopDoubleBuffProps::StateDirty ||
		loopBufferProps->state[0] == TLoopDoubleBuffProps::StateUsed)
	{
		restoreLoopArea();
	}
}

void TXMSample::postProcessSamples()
{
	mp_ubyte buffer[8];

	// Sanitize loop points
	if(loopstart > samplen)
		loopstart = samplen;
	if(looplen > samplen - loopstart)
		looplen = samplen - loopstart;

	mp_sint32 loopend = loopstart + looplen;
	mp_sint32 samplen = this->samplen;

	if (looplen == 0)
		type &= ~3;
	if (sample == NULL)
		return;
	
	mp_ubyte* originalSample = getPadStartAddr((mp_ubyte*)sample) + sizeof(TLoopDoubleBuffProps);
	mp_uint32 saveLen = (type&16) ? LoopAreaBackupSize * sizeof(mp_sword) : LoopAreaBackupSize;
	
	TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr((mp_ubyte*)sample);
	
	ASSERT(saveLen <= sizeof(buffer));
	ASSERT(saveLen <= LeadingPadding / 2);
	
	const mp_ubyte importantFlags = 3 + 16;
	
	if (loopBufferProps->state[0] == TLoopDoubleBuffProps::StateDirty ||
		(((loopBufferProps->lastloopend != loopend) ||
		  (loopBufferProps->state[1] != (type & importantFlags))) && 
		 loopBufferProps->state[0] == TLoopDoubleBuffProps::StateUsed))
	{
		restoreLoopArea();
	}
	
	// 16 bit sample
	if (type&16)
	{
		mp_sword* data = (mp_sword*)this->sample;
		
		if (!(type&3)) 
		{
			data[-1]=data[0];
			data[-2]=data[1];
			data[-3]=data[2];
			data[-4]=data[3];
			
			data[samplen]=data[samplen-1];
			data[samplen+1]=data[samplen-2];
			data[samplen+3]=data[samplen-3];
			data[samplen+4]=data[samplen-4];
		}
		else if ((type&3) && 
				 loopBufferProps->state[0] == TLoopDoubleBuffProps::StateUnused)
		{ 
			// forward loop
			if ((type&3) == 1)
			{
				// padding start
				data[-1] = data[loopend-1];
				data[-2] = data[loopend-2];
				data[-3] = data[loopend-3];
				data[-4] = data[loopend-4];
				
				// save portions after loopend, gets overwritten now
				memcpy(originalSample, data+loopend, saveLen);
				loopBufferProps->state[0] = TLoopDoubleBuffProps::StateUsed;
				loopBufferProps->state[1] = type & importantFlags;
				loopBufferProps->lastloopend = loopend;
				
				memcpy(buffer, data+loopstart, saveLen);
				memcpy(data+loopend, buffer, saveLen);	
			}
			else if ((type&3) == 2)
			{
				data[-1] = data[0];
				data[-2] = data[1];
				data[-3] = data[2];
				data[-4] = data[3];

				// save portions after loopend, gets overwritten now
				memcpy(originalSample, data+loopend, saveLen);
				loopBufferProps->state[0] = TLoopDoubleBuffProps::StateUsed;
				loopBufferProps->state[1] = type & importantFlags;
				loopBufferProps->lastloopend = loopend;
				
				data[loopend] = data[loopend-1];
				data[loopend+1] = data[loopend-2];
				data[loopend+2] = data[loopend-3];
				data[loopend+3] = data[loopend-4];
			}			
		}			
	}
	// 8 bit sample
	else 
	{			
		mp_sbyte* data = (mp_sbyte*)this->sample;
	
		if (!(type&3)) 
		{
			data[-1] = data[0];
			data[-2] = data[1];
			data[-3] = data[2];
			data[-4] = data[3];
			
			data[samplen] = data[samplen-1];
			data[samplen+1] = data[samplen-2];
			data[samplen+2] = data[samplen-3];
			data[samplen+3] = data[samplen-4];
		}
		else if ((type&3) && 
				 loopBufferProps->state[0] == TLoopDoubleBuffProps::StateUnused)
		{
			// forward loop
			if ((type&3) == 1)
			{
				// leading padding
				data[-1] = data[loopend-1];
				data[-2] = data[loopend-2];
				data[-3] = data[loopend-3];
				data[-4] = data[loopend-4];
				
				// save portions after loopend, gets overwritten now
				memcpy(originalSample, data+loopend, saveLen);
				loopBufferProps->state[0] = TLoopDoubleBuffProps::StateUsed;
				loopBufferProps->state[1] = type & importantFlags;
				loopBufferProps->lastloopend = loopend;
				
				memcpy(buffer, data+loopstart, saveLen);
				memcpy(data+loopend, buffer, saveLen);
			}
			// bidi loop
			else if ((type&3) == 2)
			{
				data[-1] = data[0];
				data[-2] = data[1];
				data[-3] = data[2];
				data[-4] = data[3];

				// save portions after loopend, gets overwritten now
				memcpy(originalSample, data+loopend, saveLen);
				loopBufferProps->state[0] = TLoopDoubleBuffProps::StateUsed;
				loopBufferProps->state[1] = type & importantFlags;
				loopBufferProps->lastloopend = loopend;
				
				data[loopend] = data[loopend-1];
				data[loopend+1] = data[loopend-2];
				data[loopend+2] = data[loopend-3];
				data[loopend+3] = data[loopend-4];
			}
		}
	}
}

// get sample value
// values range from [-32768,32767] in case of a 16 bit sample
// or from [-128,127] in case of an 8 bit sample
mp_sint32 TXMSample::getSampleValue(mp_uint32 index)
{
	if (type & 16)
	{
		if ((type & 3) && index >= loopstart+looplen && 
			index < loopstart+looplen+LoopAreaBackupSize)
		{
			TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr((mp_ubyte*)sample);
			if (loopBufferProps->state[0] == TLoopDoubleBuffProps::StateUnused)
				return *(((mp_sword*)sample)+index);
			
			mp_sword* buff = (mp_sword*)(getPadStartAddr((mp_ubyte*)sample) + sizeof(TLoopDoubleBuffProps));
			return *(buff + (index - (loopstart+looplen)));
		}
		else
			return *(((mp_sword*)sample)+index);
	}
	else
	{
		if ((type & 3) && index >= loopstart+looplen && 
			index < loopstart+looplen+LoopAreaBackupSize)
		{
			TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr((mp_ubyte*)sample);
			if (loopBufferProps->state[0] == TLoopDoubleBuffProps::StateUnused)
				return *(sample+index);

			mp_sbyte* buff = (mp_sbyte*)(getPadStartAddr((mp_ubyte*)sample) + sizeof(TLoopDoubleBuffProps));
			return *(buff + (index - (loopstart+looplen)));
		}
		else
			return *(sample+index);
	}
}

mp_sint32 TXMSample::getSampleValue(mp_ubyte* sample, mp_uint32 index)
{
	if (type & 16)
		return *(((mp_sword*)sample)+index);
	else
		return *(sample+index);
}

void TXMSample::setSampleValue(mp_uint32 index, mp_sint32 value)
{
	if (type & 16)
	{
		if ((type & 3) && index >= loopstart+looplen && 
			index < loopstart+looplen+LoopAreaBackupSize)
		{
			TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr((mp_ubyte*)sample);
			if (loopBufferProps->state[0] == TLoopDoubleBuffProps::StateUnused)
			{
				*(((mp_sword*)sample)+index) = (mp_sword)value;
				return;
			}

			mp_sword* buff = (mp_sword*)(getPadStartAddr((mp_ubyte*)sample) + sizeof(TLoopDoubleBuffProps));
			*(buff + (index - (loopstart+looplen))) = (mp_sword)value;
			
			loopBufferProps->state[0] = TLoopDoubleBuffProps::StateDirty;
		}
		else if ((type & 3) && index >= loopstart && 
				 index < loopstart+LoopAreaBackupSize)
		{
			TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr((mp_ubyte*)sample);
			if (loopBufferProps->state[0] == TLoopDoubleBuffProps::StateUsed)
				loopBufferProps->state[0] = TLoopDoubleBuffProps::StateUnused;
			
			*(((mp_sword*)sample)+index) = (mp_sword)value;
		}
		else
			*(((mp_sword*)sample)+index) = (mp_sword)value;
	}
	else
	{
		if ((type & 3) && index >= loopstart+looplen && 
			index < loopstart+looplen+LoopAreaBackupSize)
		{
			TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr((mp_ubyte*)sample);
			if (loopBufferProps->state[0] == TLoopDoubleBuffProps::StateUnused)
			{
				*(sample+index) = (mp_sbyte)value;
				return;
			}

			mp_sbyte* buff = (mp_sbyte*)(getPadStartAddr((mp_ubyte*)sample) + sizeof(TLoopDoubleBuffProps));
			*(buff + (index - (loopstart+looplen))) = (mp_sbyte)value;

			loopBufferProps->state[0] = TLoopDoubleBuffProps::StateDirty;
		}
		else if ((type & 3) && index >= loopstart && 
				 index < loopstart+LoopAreaBackupSize)
		{
			TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr((mp_ubyte*)sample);
			if (loopBufferProps->state[0] == TLoopDoubleBuffProps::StateUsed)
				loopBufferProps->state[0] = TLoopDoubleBuffProps::StateUnused;
			
			*(sample+index) = (mp_sbyte)value;
		}
		else
			*(sample+index) = (mp_sbyte)value;
	}
}

void TXMSample::setSampleValue(mp_ubyte* sample, mp_uint32 index, mp_sint32 value)
{
	if (type & 16)
	{
		*(((mp_sword*)sample)+index) = (mp_sword)value;
	}
	else
		*(sample+index) = (mp_sbyte)value;
}


#define FUNCTION_SUCCESS	MP_OK
#define FUNCTION_FAILED		MP_LOADER_FAILED

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// IT sample loading helper class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ITSampleLoader : public XModule::SampleLoader
{
private:
	mp_dword* source_buffer;			/* source buffer */
	mp_dword* source_position;			/* actual reading position */
	mp_ubyte source_remaining_bits;		/* bits remaining in read dword */
	
	bool it215;
	
public:
		ITSampleLoader(XMFileBase& file, bool isIt215 = false) :
		SampleLoader(file),
		source_buffer(NULL),
		source_position(NULL),
		source_remaining_bits(0),
		it215(isIt215)
	{
	}
	
	virtual ~ITSampleLoader() 
	{
		free_IT_compressed_block();
	}
	
	mp_dword read_n_bits_from_IT_compressed_block(mp_ubyte p_bits_to_read);
	
	mp_sint32 read_IT_compressed_block ();
	
	void free_IT_compressed_block ();
	
	// second parameter is ignored
	virtual mp_sint32 load_sample_8bits(void* p_dest_buffer, mp_sint32 compressedSize, mp_sint32 p_buffsize);
	
	// second parameter is ignored
	virtual mp_sint32 load_sample_16bits(void* p_dest_buffer, mp_sint32 compressedSize, mp_sint32 p_buffsize);
};

/* * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE

 -The following sample decompression code is based on CheeseTracker code which is based on xmp's code.(http://xmp.helllabs.org) which is based in openCP code. :D

* NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE */

mp_dword ITSampleLoader::read_n_bits_from_IT_compressed_block (mp_ubyte p_bits_to_read) {				
	
    mp_dword aux_return_value;
	
    mp_dword val;
    mp_ubyte *buffer=(mp_ubyte*)source_position;
	
    if ( p_bits_to_read <= source_remaining_bits ) {
		
    	val=buffer[3];
		val<<=8;
    	val|=buffer[2];
		val<<=8;
    	val|=buffer[1];
		val<<=8;
    	val|=buffer[0];
		
		aux_return_value = val & ((1 << p_bits_to_read) - 1);
		val >>= p_bits_to_read;
		source_remaining_bits -= p_bits_to_read;
		
		buffer[3]=val>>24;
    	buffer[2]=(val>>16)&0xFF;
    	buffer[1]=(val>>8)&0xFF;
    	buffer[0]=(val)&0xFF;
		
    } else {
    	aux_return_value=buffer[3];
		aux_return_value<<=8;
    	aux_return_value|=buffer[2];
		aux_return_value<<=8;
    	aux_return_value|=buffer[1];
		aux_return_value<<=8;
    	aux_return_value|=buffer[0];
		
		mp_dword nbits = p_bits_to_read - source_remaining_bits;
		//	aux_return_value = *source_position;
		source_position++;
        buffer+=4;
    	val=buffer[3];
		val<<=8;
    	val|=buffer[2];
		val<<=8;
    	val|=buffer[1];
		val<<=8;
    	val|=buffer[0];
		aux_return_value |= ((val & ((1 << nbits) - 1)) << source_remaining_bits);
		val >>= nbits;
		source_remaining_bits = 32 - nbits;
		buffer[3]=val>>24;
    	buffer[2]=(val>>16)&0xFF;
    	buffer[1]=(val>>8)&0xFF;
    	buffer[0]=(val)&0xFF;
		
    }
	
    return aux_return_value;
}

mp_sint32 ITSampleLoader::read_IT_compressed_block () {				

	mp_uword size;

	size=f.readWord();

	if (f.isEOF()) return FUNCTION_FAILED;

	mp_sint32 finalSize = 4 * ((size >> 2) + 2);
	source_buffer = (mp_dword*)new mp_ubyte[finalSize];

	if (source_buffer==NULL) return FUNCTION_FAILED;

	memset(source_buffer, 0, finalSize);

	mp_sint32 res = f.read(source_buffer, 1, size);
	if (res != size)
	{
		delete[] (mp_ubyte*)source_buffer;
		source_buffer = NULL;
		return FUNCTION_FAILED;
	}

	source_position = source_buffer;
	source_remaining_bits = 32;

	return FUNCTION_SUCCESS;
}

void ITSampleLoader::free_IT_compressed_block () {				


	if (source_buffer!=NULL) delete[] (mp_ubyte*)source_buffer;

	source_buffer = NULL;

}

mp_sint32 ITSampleLoader::load_sample_8bits(void* p_dest_buffer, mp_sint32 compressedSize, mp_sint32 p_buffsize) {
	
	mp_sbyte *dest_buffer;		/* destination buffer which will be returned */
   	mp_uword block_length;		/* length of compressed data block in samples */
	mp_uword block_position;		/* position in block */
	mp_ubyte bit_width;			/* actual "bit width" */
	mp_uword aux_value;			/* value read from file to be processed */
	mp_sbyte d1, d2;		/* integrator buffers (d2 for it2.15) */
	mp_sbyte *dest_position;		/* position in output buffer */
	mp_sbyte v;			/* sample value */

	dest_buffer = (mp_sbyte *) p_dest_buffer;

	if (dest_buffer==NULL) return FUNCTION_FAILED;

	memset (dest_buffer, 0, p_buffsize);

	dest_position = dest_buffer;		

	/* now unpack data till the dest buffer is full */
	
	while (p_buffsize) {
	/* read a new block of compressed data and reset variables */
		if ( read_IT_compressed_block() ) return FUNCTION_FAILED;


		block_length = (p_buffsize < 0x8000) ? p_buffsize : 0x8000;

		block_position = 0;

		bit_width = 9;		/* start with width of 9 bits */

		d1 = d2 = 0;		/* reset integrator buffers */

	/* now uncompress the data block */
		while ( block_position < block_length ) {

			aux_value = read_n_bits_from_IT_compressed_block(bit_width);			/* read bits */

			if ( bit_width < 7 ) { /* method 1 (1-6 bits) */

				if ( aux_value == (1 << (bit_width - 1)) ) { /* check for "100..." */

					aux_value = read_n_bits_from_IT_compressed_block(3) + 1; /* yes -> read new width; */
		    			bit_width = (aux_value < bit_width) ? aux_value : aux_value + 1;
							/* and expand it */
		    			continue; /* ... next value */
				}

			} else if ( bit_width < 9 ) { /* method 2 (7-8 bits) */

				mp_ubyte border = (0xFF >> (9 - bit_width)) - 4;
							/* lower border for width chg */

				if ( aux_value > border && aux_value <= (border + 8) ) {

					aux_value -= border; /* convert width to 1-8 */
					bit_width = (aux_value < bit_width) ? aux_value : aux_value + 1;
							/* and expand it */
		    			continue; /* ... next value */
				}


			} else if ( bit_width == 9 ) { /* method 3 (9 bits) */

				if ( aux_value & 0x100 ) {			/* bit 8 set? */

					bit_width = (aux_value + 1) & 0xff;		/* new width... */
		    			continue;				/* ... and next value */
				}

			} else { /* illegal width, abort */

				free_IT_compressed_block();
			
				return FUNCTION_FAILED;
			}

			/* now expand value to signed byte */
			if ( bit_width < 8 ) {

				mp_ubyte tmp_shift = 8 - bit_width;

				v=(aux_value << tmp_shift);
				v>>=tmp_shift;

			} else v = (mp_sbyte) aux_value;

			/* integrate upon the sample values */
			d1 += v;
	    		d2 += d1;

			/* ... and store it into the buffer */
			*(dest_position++) = it215 ? d2 : d1;
			block_position++;

		}

		/* now subtract block lenght from total length and go on */
		free_IT_compressed_block();
		p_buffsize -= block_length;
	}


	return FUNCTION_SUCCESS;
}

mp_sint32 ITSampleLoader::load_sample_16bits(void* p_dest_buffer, mp_sint32 compressedSize, mp_sint32 p_buffsize) {

	mp_sword *dest_buffer;		/* destination buffer which will be returned */
   	mp_uword block_length;		/* length of compressed data block in samples */
	mp_uword block_position;		/* position in block */
	mp_ubyte bit_width;			/* actual "bit width" */
	mp_dword aux_value;			/* value read from file to be processed */
	mp_sword d1, d2;		/* integrator buffers (d2 for it2.15) */
	mp_sword *dest_position;		/* position in output buffer */
	mp_sword v;			/* sample value */

	dest_buffer = (mp_sword *) p_dest_buffer;

	if (dest_buffer==NULL) return FUNCTION_FAILED;

	memset (dest_buffer, 0, p_buffsize*2);

	dest_position = dest_buffer;		

	while (p_buffsize) {
	/* read a new block of compressed data and reset variables */
		if ( read_IT_compressed_block() ) {

			return FUNCTION_FAILED;
		}


		block_length = (p_buffsize < 0x4000) ? p_buffsize : 0x4000;

		block_position = 0;

		bit_width = 17;		/* start with width of 9 bits */

		d1 = d2 = 0;		/* reset integrator buffers */

		while ( block_position < block_length ) {

			aux_value = read_n_bits_from_IT_compressed_block(bit_width);			/* read bits */

			if ( bit_width < 7 ) { /* method 1 (1-6 bits) */

				if ( (signed)aux_value == (1 << (bit_width - 1)) ) { /* check for "100..." */

					aux_value = read_n_bits_from_IT_compressed_block(4) + 1; /* yes -> read new width; */
		    			bit_width = (aux_value < bit_width) ? aux_value : aux_value + 1;
							/* and expand it */
		    			continue; /* ... next value */
				}

			} else if ( bit_width < 17 ) {

				mp_uword border = (0xFFFF >> (17 - bit_width)) - 8;

				if ( (signed)aux_value > border && (signed)aux_value <= (border + 16) ) {

					aux_value -= border; /* convert width to 1-8 */
					bit_width = (aux_value < bit_width) ? aux_value : aux_value + 1;
							/* and expand it */
		    			continue; /* ... next value */
				}


			} else if ( bit_width == 17 ) {

				if ( aux_value & 0x10000 ) {			/* bit 8 set? */

					bit_width = (aux_value + 1) & 0xff;		/* new width... */
		    			continue;				/* ... and next value */
				}

			} else { /* illegal width, abort */

			 	//ERROR("Sample has illegal BitWidth ");

				free_IT_compressed_block();
			
				return FUNCTION_FAILED;
			}

			/* now expand value to signed byte */
			if ( bit_width < 16 ) {

				mp_ubyte tmp_shift = 16 - bit_width;

				v=(aux_value << tmp_shift);
				v>>=tmp_shift;

			} else v = (mp_sword) aux_value;

			/* integrate upon the sample values */
			d1 += v;
	    		d2 += d1;

			/* ... and store it into the buffer */
			*(dest_position++) = it215 ? d2 : d1;
			block_position++;

		}

		/* now subtract block lenght from total length and go on */
		free_IT_compressed_block();
		p_buffsize -= block_length;
	}


	return FUNCTION_SUCCESS;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MDL sample loading helper class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MDLSampleLoader : public XModule::SampleLoader
{
private:
	mp_ubyte* tmpBuffer;
	mp_ubyte* dstBuffer;
	
	// MDL unpacking
	static mp_ubyte readbits(mp_ubyte* buffer,mp_uint32& bitcount,mp_uint32& bytecount,mp_sint32 numbits)
	{
		mp_ubyte val=0,bitti=0;
		for (mp_sint32 n=0;n<numbits;n++){
			val+=((buffer[bytecount]>>bitcount)&1)<<(bitti++);
			bitcount++;
			if (bitcount==8) {
				bitcount=0;
				bytecount++;
			}
		}
		return val;
	}
	
	// MDL unpacking
	static mp_ubyte depackbyte(mp_ubyte* packed,mp_uint32& bitcount,mp_uint32& bytecount) 
	{
		mp_ubyte b = 0;
		mp_ubyte sign = readbits(packed,bitcount,bytecount,1);
		mp_ubyte bit = readbits(packed,bitcount,bytecount,1);
		
		if (bit) {
			b = readbits(packed,bitcount,bytecount,3);
			goto next;
		}
		else b=8;
loop:;
		bit=readbits(packed,bitcount,bytecount,1);
		if (!bit) {
			b+=16;
			goto loop;
		}
		else b+=readbits(packed,bitcount,bytecount,4);
next:;
		if (sign) b^=255;
		
		return b;
	}
	
	void cleanUp()
	{
		if (tmpBuffer)
		{
			delete[] tmpBuffer;
			tmpBuffer = NULL;
		}
		
		if (dstBuffer)
		{
			delete[] dstBuffer;
			dstBuffer = NULL;
		}
	}
	
public:
		MDLSampleLoader(XMFileBase& file) :
		SampleLoader(file),
		tmpBuffer(NULL),
		dstBuffer(NULL)
	{
	}
	
	virtual ~MDLSampleLoader() 
	{
		cleanUp();
	}
	
	virtual mp_sint32 load_sample_8bits(void* p_dest_buffer, mp_sint32 compressedSize, mp_sint32 p_buffsize);	
	virtual mp_sint32 load_sample_16bits(void* p_dest_buffer, mp_sint32 compressedSize, mp_sint32 p_buffsize);
};

mp_sint32 MDLSampleLoader::load_sample_8bits(void* buffer, mp_sint32 size, mp_sint32 length)
{
	cleanUp();

	tmpBuffer = new mp_ubyte[size+32];
	memset(tmpBuffer,0,size+32);
	
	if (tmpBuffer == NULL)
		return FUNCTION_FAILED;

	f.read(tmpBuffer,1,size);
	
	dstBuffer = new mp_ubyte[length+64];
	
	if (dstBuffer == NULL)
		return FUNCTION_FAILED;
	
	memset(dstBuffer,0,length+64);
	
	mp_uint32 bitcount=0, bytecount=0;
	
	mp_sint32 i=0;
	
	while (bytecount < (unsigned)size && i < length) 
	{
		dstBuffer[i++]=depackbyte((mp_ubyte*)tmpBuffer,bitcount,bytecount);							
	}
	
	mp_sbyte b1=0;
	for (i = 0; i < length; i++) 
		dstBuffer[i] = b1+=dstBuffer[i];
	
	memcpy(buffer,dstBuffer,length);
	
	return FUNCTION_SUCCESS;
}

mp_sint32 MDLSampleLoader::load_sample_16bits(void* buffer, mp_sint32 size, mp_sint32 length)
{
	cleanUp();

	tmpBuffer = new mp_ubyte[size+32];
	memset(tmpBuffer,0,size+32);
	
	if (tmpBuffer == NULL)
		return FUNCTION_FAILED;
		
	f.read(tmpBuffer,1,size);

	dstBuffer = new mp_ubyte[length*2+64];
	
	if (dstBuffer == NULL)
		return FUNCTION_FAILED;
	
	memset(dstBuffer,0,length+64);
	
	mp_uint32 bitcount=0, bytecount=0;
	
	mp_sint32 i=0;
	
	while (bytecount<(unsigned)size && i < length*2) 
	{
		dstBuffer[i++]=readbits((mp_ubyte*)tmpBuffer,bitcount,bytecount,8);
		dstBuffer[i++]=depackbyte((mp_ubyte*)tmpBuffer,bitcount,bytecount);
	}
	
	mp_sbyte b1=0;
	for (i = 0; i < length; i++) 
		dstBuffer[i*2+1] = b1+=dstBuffer[i*2+1];
	
	mp_sword* dstBuffer16 = (mp_sword*)buffer;
	mp_ubyte* srcBuffer = (mp_ubyte*)dstBuffer;
	
	for (i = 0; i < length; i++)
		*dstBuffer16++ = LittleEndian::GET_WORD(srcBuffer+=2);
	
	return FUNCTION_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ADPCM sample loading helper class:
//
// 4-bit ADPCM is coding the delta values between a sample and
// the next in 4-bits (starting value is zero). The delta values are
// stored as a 16-byte table at the start of the sample data:
// [16-bytes delta values][(length+1)/2 bytes of 4-bit indexes...]
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ADPCMSampleLoader : public XModule::SampleLoader
{
private:
	mp_ubyte* tmpBuffer;
	
	void cleanUp()
	{
		if (tmpBuffer)
		{
			delete[] tmpBuffer;
			tmpBuffer = NULL;
		}
	}
	
public:
		ADPCMSampleLoader(XMFileBase& file) :
		SampleLoader(file),
		tmpBuffer(NULL)
	{
	}
	
	virtual ~ADPCMSampleLoader() 
	{
		cleanUp();
	}

	virtual mp_sint32 load_sample_8bits(void* p_dest_buffer, mp_sint32 compressedSize, mp_sint32 p_buffsize)
	{
		cleanUp();
	
		mp_sbyte deltaValues[16];
		
		// read delta values table
		f.read(deltaValues, 1, 16);
	
		// this is the actual size of the compressed size
		const mp_uint32 blockSize = (p_buffsize + 1) / 2;

		// allocate some memory for it
		tmpBuffer = new mp_ubyte[blockSize];
	
		// read compressed data
		f.read(tmpBuffer, 1, blockSize);
	
		mp_sbyte b1 = 0;
		
		mp_sbyte* srcPtr = (mp_sbyte*)p_dest_buffer;
		for (mp_uint32 i = 0; i < blockSize; i++)
		{
			mp_uint32 index = tmpBuffer[i] & 0xF;
			*srcPtr++ = b1+=deltaValues[index];
			index = tmpBuffer[i] >> 4;
			*srcPtr++ = b1+=deltaValues[index];
		}
	
		return FUNCTION_SUCCESS;
	}
	
	virtual mp_sint32 load_sample_16bits(void* p_dest_buffer, mp_sint32 compressedSize, mp_sint32 p_buffsize)
	{
		cleanUp();

		return FUNCTION_SUCCESS;
	}
};

mp_sint32 TXMPattern::compress(mp_ubyte* dest) const
{
	mp_sint32 patternSize = rows*channum*(2+effnum*2);

	mp_ubyte* srcPtr = patternData;
	mp_ubyte* dstPtr = dest;

	mp_sint32 i = 0;
	mp_sint32 len = 0;

	// retrieve buffer size only
	if (!dest)
	{
		while (i < patternSize)
		{
			if (*srcPtr > 0 && *srcPtr < 128)
			{
				srcPtr++;
				i++;
				len++;
			}
			else if (*srcPtr >= 128)
			{
				srcPtr++;
				i++;
				len+=2;
			}
			else
			{
				mp_sint32 j = 0;
				while (i < patternSize && j < 125 && *srcPtr == 0)
				{
					srcPtr++;
					j++;
					i++;
				}
				if (j == 1)
				{
					len++;
				}
				else
				{
					ASSERT(128+j < 255);
					len++;
				}
			}
		}
		
		return len;
	}

	// compress and store
	while (i < patternSize)
	{
		if (*srcPtr > 0 && *srcPtr < 128)
		{
			*dstPtr++ = *srcPtr++;
			i++;
			len++;
		}
		else if (*srcPtr >= 128)
		{
			*dstPtr++ = 255;
			*dstPtr++ = *srcPtr++;
			i++;
			len+=2;
		}
		else
		{
			mp_sint32 j = 0;
			while (i < patternSize && j < 125 && *srcPtr == 0)
			{
				srcPtr++;
				j++;
				i++;
			}
			if (j == 1)
			{
				*dstPtr++ = 0;
				len++;
			}
			else
			{
				*dstPtr++ = 128 + j;
				
				ASSERT(128+j < 255);
				
				len++;
			}
		}
	}

	return len;
}

mp_sint32 TXMPattern::decompress(mp_ubyte* src, mp_sint32 len)
{
	mp_sint32 patternSize = rows*channum*(2+effnum*2);

	mp_ubyte* srcPtr = src;
	mp_ubyte* dstPtr = patternData;

	mp_sint32 i = 0;
	mp_sint32 j = 0;
	while (i < len && j < patternSize)
	{
		if (*srcPtr < 128)
		{
			*dstPtr++ = *srcPtr++;
			i++;
			j++;
		}
		else if (*srcPtr == 255)
		{
			srcPtr++;
			*dstPtr++ = *srcPtr++;
			i+=2;
			j++;
		}
		else
		{
			mp_sint32 k = *srcPtr++ & 127;
			for (mp_sint32 l = 0; l < k; l++)
			{
				if (j < patternSize)
					*dstPtr++ = 0;
				else
				{
					ASSERT(false);
				}
				j++;
			}
			i++;
		}
	}
	
	return MP_OK;
}

#ifdef MILKYTRACKER

const TXMPattern& TXMPattern::operator=(const TXMPattern& src)
{
	if (this != &src)
	{
		delete[] patternData;
		const mp_uint32 size = (mp_uint32)src.channum*(2+(mp_uint32)src.effnum*2)*(mp_uint32)src.rows;
		patternData = new mp_ubyte[size];
		memcpy(patternData, src.patternData, size);
		channum = src.channum;
		effnum = src.effnum;
		len = src.len;
		patdata = src.patdata;
		ptype = src.ptype;
		rows = src.rows;
	}
	
	return *this;
}

#endif

// Constructor for loader manager (private)
XModule::LoaderManager::LoaderManager() :
		loaders(NULL),
		numLoaders(0),
		numAllocatedLoaders(0),
		iteratorCounter(-1)
{
#ifndef MP_XMONLY 
	registerLoader(new Loader669(), ModuleType_669);
	registerLoader(new LoaderAMF_1(), ModuleType_AMF);
	registerLoader(new LoaderAMF_2(), ModuleType_AMF);
	registerLoader(new LoaderAMSv1(), ModuleType_AMS);
	registerLoader(new LoaderAMSv2(), ModuleType_AMS);
	registerLoader(new LoaderCBA(), ModuleType_CBA);
	registerLoader(new LoaderDBM(), ModuleType_DBM);
	registerLoader(new LoaderDIGI(), ModuleType_DIGI);
	registerLoader(new LoaderDSMv1(), ModuleType_DSM);
	registerLoader(new LoaderDSMv2(), ModuleType_DSM);
	registerLoader(new LoaderDSm(), ModuleType_DSm);
	registerLoader(new LoaderDTM_1(), ModuleType_DTM_1);
	registerLoader(new LoaderDTM_2(), ModuleType_DTM_2);
	registerLoader(new LoaderFAR(), ModuleType_FAR);
	registerLoader(new LoaderGDM(), ModuleType_GDM);
	registerLoader(new LoaderIMF(), ModuleType_IMF);
	registerLoader(new LoaderIT(), ModuleType_IT);
	//registerLoader(new LoaderFNK(), funk format sucks
	registerLoader(new LoaderMDL(), ModuleType_MDL);
	registerLoader(new LoaderMTM(), ModuleType_MTM);
	registerLoader(new LoaderMXM(), ModuleType_MXM);
	registerLoader(new LoaderOKT(), ModuleType_OKT);
	registerLoader(new LoaderPLM(), ModuleType_PLM);
	registerLoader(new LoaderPSMv1(), ModuleType_PSM);
	registerLoader(new LoaderPSMv2(), ModuleType_PSM);
	registerLoader(new LoaderPTM(), ModuleType_PTM);
	registerLoader(new LoaderS3M(), ModuleType_S3M);
	registerLoader(new LoaderSTM(), ModuleType_STM);
	registerLoader(new LoaderSFX(), ModuleType_SFX);
	registerLoader(new LoaderUNI(), ModuleType_UNI);
	registerLoader(new LoaderULT(), ModuleType_ULT);	
	registerLoader(new LoaderXM(), ModuleType_XM);	
	// Game Music Creator may not be recognized perfectly
	registerLoader(new LoaderGMC(), ModuleType_GMC);
	// Last loader is MOD because there is a slight chance that other formats will be misinterpreted as 15 ins. MODs
	registerLoader(new LoaderMOD(), ModuleType_MOD);
#else
	registerLoader(new LoaderXM(), ModuleType_XM);	
#endif
}

XModule::LoaderManager::~LoaderManager()
{
	for (mp_uint32 i = 0; i < numLoaders; i++)
		delete loaders[i].loader;
		
	delete[] loaders;
}

void XModule::LoaderManager::registerLoader(LoaderInterface* loader, ModuleTypes type)
{
	if (numLoaders+1 > numAllocatedLoaders)
	{
		numAllocatedLoaders+=16;
		
		TLoaderInfo* newLoaders = new TLoaderInfo[numAllocatedLoaders];
		for (mp_uint32 i = 0; i < numLoaders; i++)
			newLoaders[i] = loaders[i];
			
		delete[] loaders;
		loaders = newLoaders;
	}
	
	loaders[numLoaders].loader = loader;
	loaders[numLoaders].moduleType = type;
	
	numLoaders++;
}

XModule::TLoaderInfo* XModule::LoaderManager::getFirstLoaderInfo()
{
	if (numLoaders)
	{
		iteratorCounter = 0;
		return loaders;
	}
	else return NULL;
}

XModule::TLoaderInfo* XModule::LoaderManager::getNextLoaderInfo()
{
	iteratorCounter++;
	if (iteratorCounter < (signed)numLoaders)
		return loaders+iteratorCounter;
	else
	{
		iteratorCounter = -1;
		return NULL;
	}
}

const mp_sint32 XModule::periods[12] = {1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,907};

const mp_sint32 XModule::sfinetunes[16] = {8363,8413,8463,8529,8581,8651,8723,8757,
										   7895,7941,7985,8046,8107,8169,8232,8280};

const mp_sbyte XModule::modfinetunes[16] = {0,16,32,48,64,80,96,112,-128,-112,-96,-80,-64,-48,-32,-16};

const mp_ubyte XModule::numValidXMEffects = 24;
const mp_ubyte XModule::validXMEffects[XModule::numValidXMEffects] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,20,21,25,27,29,33};

//////////////////////////////////////////////////////////////////////////
// various tools for loading and converting modules	     				//
//////////////////////////////////////////////////////////////////////////
mp_sint32 XModule::getc4spd(mp_sint32 relnote,mp_sint32 finetune)
{
	static const mp_sint32 table[] = {65536,69432,73561,77935,82570,87480,92681,98193,104031,110217,116771,123715,
						   65536,65565,65595,65624,65654,65684,65713,65743,65773,65802,65832,65862,65891,
						   65921,65951,65981,66010,66040,66070,66100,66130,66160,66189,66219,66249,66279,
						   66309,66339,66369,66399,66429,66459,66489,66519,66549,66579,66609,66639,66669,
						   66699,66729,66759,66789,66820,66850,66880,66910,66940,66971,67001,67031,67061,
						   67092,67122,67152,67182,67213,67243,67273,67304,67334,67365,67395,67425,67456,
						   67486,67517,67547,67578,67608,67639,67669,67700,67730,67761,67792,67822,67853,
						   67883,67914,67945,67975,68006,68037,68067,68098,68129,68160,68190,68221,68252,
						   68283,68314,68344,68375,68406,68437,68468,68499,68530,68561,68592,68623,68654,
						   68685,68716,68747,68778,68809,68840,68871,68902,68933,68964,68995,69026,69057,
						   69089,69120,69151,69182,69213,69245,69276,69307,69339,69370,69401};

	mp_sint32 c4spd = 8363;
	mp_sbyte xmfine = finetune;

	mp_sbyte octave = (relnote+96)/12;
	mp_sbyte note = (relnote+96)%12;
	
	mp_sbyte o2 = octave-8;
	
	if (xmfine<0)
	{
		xmfine+=(mp_sbyte)128;
		note--;
		if (note<0)
		{
			note+=12;
			o2--;
		}
	}

	if (o2>=0)
	{
		c4spd<<=o2;		
	}
	else
	{
		c4spd>>=-o2;
	}

	mp_sint32 f = FixedMUL(table[(mp_ubyte)note],c4spd);
	return (FixedMUL(f,table[(mp_ubyte)xmfine+12]));
}

void XModule::convertc4spd(mp_uint32 c4spd,mp_sbyte *finetune,mp_sbyte *relnote)
{
	mp_sint32 xmfine = 0;
	mp_sbyte cl = 0;
	mp_sbyte ch = 0;
	mp_uint32 ebp = 0xFFFFFFFF;
	mp_uint32 ebx = c4spd;
aloop:
	mp_uint32 c4s2 = ebx;
	mp_uint32 c4s = getc4spd((mp_sint32)cl-48,0);
	if (c4s<c4s2)
	{
		mp_sint32 s = c4s2;
		c4s2 = c4s;
		c4s = s;
	}
	mp_uint32 dc4 = c4s-c4s2;
	if (dc4<ebp)
	{
		ebp = dc4;
		ch = cl;
		cl++;
		if (cl<119) goto aloop; 
	}
	cl = 0;
aloop2:
	c4s2 = ebx;
	c4s = getc4spd((mp_sint32)ch-48,xmfine);
	if (c4s<c4s2)
	{
		mp_sint32 s = c4s2;
		c4s2 = c4s;
		c4s = s;
	}
	dc4 = c4s-c4s2;
	if (dc4<ebp)
	{
		ebp = dc4;
		cl = xmfine;
	}
	xmfine++;
	if (xmfine<256) goto aloop2;
	
	ch-=48;
	*finetune = (mp_sbyte)cl;
	*relnote = (mp_sbyte)ch;

}

mp_uint32 XModule::amigaPeriodToNote(mp_uint32 period) 
{
	for (mp_sint32 y = 0; y < 120; y++) 
	{
		mp_uint32 per = (periods[y%12]*16>>((y/12)))>>2;
		if (period >= per) 
			return y+1;
	}

	return 0;
}

////////////////////////////////////////////
// Load sample into given memory.		  //
// Sample size is in BYTES not in samples //
// Sample length is number of samples,    // 
////////////////////////////////////////////
bool XModule::loadSample(XMFileBase& f,void* buffer,mp_uint32 size,mp_uint32 length,mp_sint32 flags /* = ST_DEFAULT */)
{
	mp_ubyte* tmpBuffer = NULL;
	mp_ubyte* dstBuffer = NULL;
	
	// MDL style packing
	if (flags & ST_PACKING_MDL)
	{
	
		MDLSampleLoader sampleLoader(f);
		
		if (flags & ST_16BIT)
		{
			if (sampleLoader.load_sample_16bits(buffer, size, length)) 
				return false;
		}
		else
		{
			if (sampleLoader.load_sample_8bits(buffer, size, length)) 
				return false;
		}
		
		return true;
	}
	else if ((flags & ST_PACKING_IT) || (flags & ST_PACKING_IT215))
	{
		ITSampleLoader sampleLoader(f, (flags & ST_PACKING_IT215));

		if (flags & ST_16BIT)
		{
			if (sampleLoader.load_sample_16bits(buffer, -1, length)) 
				return false;
		}
		else
		{
			if (sampleLoader.load_sample_8bits(buffer, -1, length)) 
				return false;
		}
		
		return true;
	}
	else if (flags & ST_PACKING_ADPCM)
	{
		ADPCMSampleLoader sampleLoader(f);

		if (flags & ST_16BIT)
		{
			if (sampleLoader.load_sample_16bits(buffer, size, length)) 
				return false;
		}
		else
		{
			if (sampleLoader.load_sample_8bits(buffer, size, length)) 
				return false;
		}
		
		return true;
	}
	else
	{
		memset(buffer, 0, size);
		f.read(buffer,flags & ST_16BIT ? 2 : 1, length);
	}

	// 16 bit sample 
	if (flags & ST_16BIT)
	{
		mp_sword* dstPtr = (mp_sword*)buffer;
		mp_ubyte* srcPtr = (mp_ubyte*)buffer;

		// PTM delta storing
		if (flags & ST_DELTA_PTM)
		{
			mp_sbyte b1=0;
			for (mp_uint32 i = 0; i < length*2; i++) 
				srcPtr[i] = b1+=srcPtr[i];
		}

		mp_uint32 i;
		if (flags & ST_BIGENDIAN)
		{
			for (i = 0; i < length; i++)
				dstPtr[i] = BigEndian::GET_WORD(srcPtr+i*2);
		}
		else
		{
			for (i = 0; i < length; i++)
				dstPtr[i] = LittleEndian::GET_WORD(srcPtr+i*2);
		}
		
		// delta-storing
		if (flags & ST_DELTA)
		{
			mp_sword b1=0;
			for (i = 0; i < length; i++) 
				dstPtr[i] = b1+=dstPtr[i];
		}

		// unsigned sample data
		if (flags & ST_UNSIGNED)
		{
			for (i = 0; i < length; i++) 
				dstPtr[i] = (dstPtr[i]^32767);
		}
	}
	// 8 bit sample
	else
	{	
		mp_sbyte* smpPtr = (mp_sbyte*)buffer;
	
		// delta-storing
		if (flags & ST_DELTA)
		{
			mp_sbyte b1=0;
			for (mp_uint32 i = 0; i < length; i++) 
				smpPtr[i] = b1+=smpPtr[i];
		}

		// unsigned sample data
		if (flags & ST_UNSIGNED)
		{
			for (mp_uint32 i = 0; i < length; i++) 
				smpPtr[i] ^= 127;
		}
	}

	if (tmpBuffer)
		delete[] tmpBuffer;

	if (dstBuffer)
		delete[] dstBuffer;

	return true;
}

mp_sint32 XModule::loadModuleSample(XMFileBase& f, mp_sint32 index, 
									mp_sint32 flags8/* = ST_DEFAULT*/, mp_sint32 flags16/* = ST_16BIT*/,
									mp_uint32 alternateSize/* = 0*/)
{
	if (smp[index].type & 16)
	{
		mp_uint32 finalSize = alternateSize ? alternateSize : smp[index].samplen*2;
		if(finalSize < 8) finalSize = 8;
		
		smp[index].sample = (mp_sbyte*)allocSampleMem(finalSize);
		
		if (smp[index].sample == NULL)
		{
			return MP_OUT_OF_MEMORY;
		}
		
		if (!loadSample(f,smp[index].sample, finalSize, smp[index].samplen, flags16))
		{
			return MP_OUT_OF_MEMORY;
		}
	}
	else
	{
		mp_uint32 finalSize = alternateSize ? alternateSize : smp[index].samplen;
		if(finalSize < 4) finalSize = 4;
		
		smp[index].sample = (mp_sbyte*)allocSampleMem(finalSize);
		
		if (smp[index].sample == NULL)
		{
			return MP_OUT_OF_MEMORY;
		}
		
		if (!loadSample(f,smp[index].sample, finalSize, smp[index].samplen, flags8))
		{
			return MP_OUT_OF_MEMORY;
		}		
	}
	
	return MP_OK;
}

mp_sint32 XModule::loadModuleSamples(XMFileBase& f, mp_sint32 flags8/* = ST_DEFAULT*/, mp_sint32 flags16/* = ST_16BIT*/)
{
	for (mp_sint32 i = 0; i < header.smpnum; i++) 
	{		
		mp_sint32 res = loadModuleSample(f, i, flags8, flags16);
		if (res != MP_OK)
			return res;
	}	
	return MP_OK;
}

////////////////////////////////////////////
// Before using the sample postprocessing //
// please make sure that the memory       //
// allocated for the samples has another  //
// 16 bytes padding space				  // 
////////////////////////////////////////////
void XModule::postProcessSamples(bool heavy/* = false*/)
{
	for (mp_uint32 i = 0; i < header.smpnum; i++)
	{

		TXMSample* smp = &this->smp[i];

#ifdef VERBOSE
		printf("%i: %i, %i, %i, %x\n",i,smp->samplen, smp->loopstart, smp->looplen,smp->pan);
#endif

		if (smp->samplen == 0)
		{
			freeSampleMem((mp_ubyte*)smp->sample, false);
			smp->sample = NULL;
			continue;
		}
		
		if (heavy)
			smp->smoothLooping();
		
		smp->postProcessSamples();
	}

}

void XModule::setDefaultPanning()
{
	for (mp_sint32 i = 0; i < header.channum; i++)
	{
		if (i & 1) 
			header.pan[i] = 192;
		else
			header.pan[i] = 64;
	}
}

mp_ubyte* XModule::allocSampleMem(mp_uint32 size)
{
	// sample is always padded at start and end with 16 bytes
	for (mp_sint32 i = 0; i < (signed)samplePointerIndex; i++)
	{
		if (samplePool[i] == NULL)
		{
			samplePool[i] = TXMSample::allocPaddedMem(size);
			return samplePool[i];
		}
	}

	samplePool[samplePointerIndex] = TXMSample::allocPaddedMem(size);
	return samplePool[samplePointerIndex++];
}

void XModule::freeSampleMem(mp_ubyte* mem, bool assertCheck/* = true*/)
{
	bool found = false;
	for (mp_sint32 i = 0; i < (signed)samplePointerIndex; i++)
	{
		if (samplePool[i] == mem)
		{
			found = true;
			TXMSample::freePaddedMem(samplePool[i]);
			samplePool[i] = NULL;
			if (i == (signed)samplePointerIndex - 1)
			{
				samplePointerIndex--;
				break;
			}
		}
	}
	
	if (assertCheck)
	{
		ASSERT(found);
	}
}

#ifdef MILKYTRACKER
void XModule::insertSamplePtr(mp_ubyte* ptr)
{
	for (mp_sint32 i = 0; i < (signed)samplePointerIndex; i++)
	{
		if (samplePool[i] == NULL)
		{
			samplePool[i] = ptr;
			return;
		}
	}

	samplePool[samplePointerIndex++] = ptr;
}
void XModule::removeSamplePtr(mp_ubyte* ptr)
{
	for (mp_sint32 i = 0; i < (signed)samplePointerIndex; i++)
	{
		if (samplePool[i] == ptr)
		{
			samplePool[i] = NULL;
		}
	}
}
#endif

bool XModule::addEnvelope(TEnvelope*& envs, 
						  const TEnvelope& env,
						  mp_uint32& numEnvsAlloc,
						  mp_uint32& numEnvs)
{
	if (envs == NULL)
	{
		numEnvsAlloc = 8;

		envs = new TEnvelope[numEnvsAlloc];

		if (envs == NULL)
			return false;

		envs[numEnvs++] = env;
		return true;
	}
	else
	{
		if (numEnvs >= numEnvsAlloc)
		{
			
			numEnvsAlloc+=8;

			TEnvelope* tmpEnvs = new TEnvelope[numEnvsAlloc];

			if (tmpEnvs == NULL)
				return false;

			memcpy(tmpEnvs,envs,numEnvs*sizeof(TEnvelope));

			delete[] envs;
			
			envs = tmpEnvs;
		}
		
		envs[numEnvs++] = env;

	}
	
	return true;
}

// fix broken envelopes (1 point envelope for example)
void XModule::fixEnvelopes(TEnvelope* envs, mp_uint32 numEnvs)
{
	TEnvelope* env = envs;

	for (mp_uint32 i = 0; i < numEnvs; i++, env++)
	{
		// Check for envelope with single point (seen in a DBM module)
		if (env->num == 1)
		{
			// make sure point one starts at x-position 0
			env->env[0][0] = 0;
			// add second point with y-coordinate from point one
			env->env[1][1] = env->env[0][1];
			// x-coordinate is just a few ticks right from point one
			env->env[1][0] = env->env[0][0] + 64;
			env->num++;
		}		
	}
}

// better destructor, should be called before loading a song
// just in case loading of a module failed and someone tries to load another
// module right after that
bool XModule::cleanUp()
{
	if (venvs)
	{
		delete[] venvs;
		venvs = NULL;
		numVEnvsAlloc = numVEnvs = 0;
	}
	if (penvs)
	{
		delete[] penvs;
		penvs = NULL; 
		numPEnvsAlloc = numPEnvs = 0;
	}
	if (fenvs)
	{
		delete[] fenvs;
		fenvs = NULL;
		numFEnvsAlloc = numFEnvs = 0;
	}
	if (vibenvs)
	{
		delete[] vibenvs;
		vibenvs = NULL;
		numVibEnvsAlloc = numVibEnvs = 0;
	}
	if (pitchenvs)
	{
		delete[] pitchenvs;
		pitchenvs = NULL;
		numPitchEnvsAlloc = numPitchEnvs = 0;
	}

	if (message)
	{
		delete[] message;
		message = NULL;
		messageBytesAlloc = 0;
	}

	// release pattern memory
	mp_uint32 i;
#ifdef MILKYTRACKER
	for (i = 0; i < 256; i++)
#else
	for (i = 0; i < header.patnum; i++)
#endif
	{
		if (phead[i].patternData) 
		{
			delete[] phead[i].patternData;
			phead[i].patternData = NULL;
		}
		
	}

	// release sample-memory
	for (i = 0; i < samplePointerIndex; i++)
	{
		if (samplePool[i]) 
		{
			TXMSample::freePaddedMem(samplePool[i]);
			samplePool[i] = NULL;
		}
	}
	samplePointerIndex = 0;
	
	memset(&header,0,sizeof(TXMHeader));
	
	if (instr)
		memset(instr,0,sizeof(TXMInstrument)*256);
	
	if (smp)
		memset(smp,0,sizeof(TXMSample)*MP_MAXSAMPLES);
	
	if (phead)
		memset(phead,0,sizeof(TXMPattern)*256);

	// subsong stuff
	memset(subSongPositions, 0, sizeof(subSongPositions));
	numSubSongs = 0;

	moduleLoaded = false;

	return true;
}

XModule::XModule()
{
	// allocated necessary space for all possible patterns, instruments and samples
	phead = new TXMPattern[256];
	instr = new TXMInstrument[256];
	smp = new TXMSample[MP_MAXSAMPLES];

	type = ModuleType_NONE;

	// no module loaded (empty song)
	moduleLoaded = false;

	// initialise all sample pointers to NULL
	memset(samplePool,0,sizeof(samplePool));
	// reset current sample index
	samplePointerIndex = 0;

	memset(&header,0,sizeof(TXMHeader));

	if (instr)
		memset(instr,0,sizeof(TXMInstrument)*256);
	
	if (smp)
		memset(smp,0,sizeof(TXMSample)*MP_MAXSAMPLES);
	
	if (phead)
		memset(phead,0,sizeof(TXMPattern)*256);

	// subsong stuff
	memset(subSongPositions, 0, sizeof(subSongPositions));
	numSubSongs = 0;
	
	venvs = NULL;
	numVEnvsAlloc = numVEnvs = 0;

	penvs = NULL; 
	numPEnvsAlloc = numPEnvs = 0;

	fenvs = NULL;
	numFEnvsAlloc = numFEnvs = 0;

	vibenvs = NULL;
	numVibEnvsAlloc = numVibEnvs = 0;

	pitchenvs = NULL;
	numPitchEnvsAlloc = numPitchEnvs = 0;
	
	message = NULL;
	messageBytesAlloc = 0;
}

XModule::~XModule()
{
	cleanUp();

	delete[] phead;
	delete[] instr;
	delete[] smp;
}

const char* XModule::identifyModule(const mp_ubyte* buffer)
{
	// browse through all available loaders and find suitable
	LoaderManager loaderManager;
	TLoaderInfo* loaderInfo;
	loaderInfo = loaderManager.getFirstLoaderInfo();
	while (loaderInfo)
	{
		// if loader can identify module return ID
		const char* id = loaderInfo->loader->identifyModule(buffer);
		if (id)
		{
			return id;
		}
		
		loaderInfo = loaderManager.getNextLoaderInfo();
	}
	return NULL;
}

mp_sint32 XModule::loadModule(const SYSCHAR* fileName, bool scanForSubSongs/* = false*/)
{
	XMFile f(fileName);
	return f.isOpen() ? loadModule(f, scanForSubSongs) : -8; 
}

mp_sint32 XModule::loadModule(XMFileBase& f, bool scanForSubSongs/* = false*/)
{
	mp_ubyte buffer[IdentificationBufferSize];
	memset(buffer, 0, sizeof(buffer));

	f.setBaseOffset(f.pos());
	f.read(buffer, 1, sizeof(buffer));

	// browse through all available loaders and find suitable
	LoaderManager loaderManager;
	TLoaderInfo* loaderInfo;
	loaderInfo = loaderManager.getFirstLoaderInfo();
	while (loaderInfo)
	{
		// if loader can identify module take that loader
		if (loaderInfo->loader->identifyModule(buffer))
		{
			// try to load module
			f.seekWithBaseOffset(0);
			mp_sint32 err = loaderInfo->loader->load(f, this);
			if (err == MP_OK)
			{
				moduleLoaded = true;
				
				bool res = validate();

				if (!res)
					return MP_OUT_OF_MEMORY;
				
				type = loaderInfo->moduleType;
				if (scanForSubSongs)
					buildSubSongTable();
			}
			return err;
		}
		
		loaderInfo = loaderManager.getNextLoaderInfo();
	}
	
#ifdef MILKYTRACKER
	return MP_UNKNOWN_FORMAT;
#else
	return MP_UNSPECIFIED;
#endif

}

bool XModule::validate()
{
	if (header.channum == 0)
		header.channum++;

	if (header.insnum == 0)
		header.insnum++;
	
	/*for (mp_sint32 i = 0; i < header.ordnum; i++)
		if (header.ord[i] >= header.patnum)
			header.ord[i] = 0;*/
			
	// if we're not having any pattern just create an empty dummy pattern
	if (!header.patnum)
	{
		header.patnum++;
		
		phead[0].rows = 64;
#ifdef MILKYTRACKER
		phead[0].effnum = 2;
#else
		phead[0].effnum = 1;
#endif
		phead[0].channum = (mp_ubyte)header.channum;
		
		phead[0].patternData = new mp_ubyte[phead[0].rows*header.channum*(2+phead[0].effnum*2)];
		
		// out of memory?
		if (phead[0].patternData == NULL)
		{
			return false;
		}
		
		memset(phead[0].patternData,0,phead[0].rows*header.channum*(2+phead[0].effnum*2));
	}
		
	removeOrderSkips();
	
	if (!header.ordnum)
	{
		header.ordnum++;
		header.ord[0] = 0;
	}
	
	fixEnvelopes(venvs, numVEnvs);
	fixEnvelopes(penvs, numPEnvs);
	fixEnvelopes(fenvs, numFEnvs);
	fixEnvelopes(vibenvs, numVibEnvs);
	fixEnvelopes(pitchenvs, numPitchEnvs);
	
	return true;
}

void XModule::convertStr(char* strIn, const char* strOut, mp_sint32 nLen, bool filter)
{
	memset(strIn, 0, nLen);
	mp_sint32 i;
	for (i = 0; i < nLen; i++)
	{
		strIn[i] = strOut[i];
		
		// must be an asciiz string
		if (strIn[i] == '\0')
			break;
		
		// Filter non-viewable characters
		if (filter && (strIn[i]<32 || (unsigned)strIn[i]>127)) 
			strIn[i] = 32;
	}

	i = nLen-1;
	while (i>=0 && strIn[i]<=32) 
		i--;

	i++;
	strIn[i] = '\0';
}

void XModule::getTitle(char* str, bool filter /* = true */) const
{
	if (!moduleLoaded)
	{
		memset(str, 0, 33);
		return;
	}

	convertStr(str, (const char*)&header.name, 32, filter);
}

void XModule::getSignature(char* str, bool filter /* = true */) const
{
	if (!moduleLoaded)
	{
		memset(str, 0, 18);
		return;
	}

	convertStr(str, (const char*)&header.sig, 17, filter);
}

void XModule::getTracker(char* str, bool filter /* = true */) const
{
	if (!moduleLoaded)
	{
		memset(str, 0, 33);
		return;
	}

	convertStr(str, (const char*)&header.tracker, 32, filter);
}

///////////////////////////////////////////////////
// dealing with song messages				     //
///////////////////////////////////////////////////
void XModule::allocateSongMessage(mp_uint32 initialSize/* = 512*/)
{
	if (message)
	{
		delete[] message;
		message = NULL;
		messageBytesAlloc = 0;		
	}
	
	message = new char[initialSize];
	
	if (message)
	{
		memset(message, 0, initialSize);
		messageBytesAlloc = initialSize;
	}
}

// add one more line of text to songmessage
void XModule::addSongMessageLine(const char* line)
{
	
	if (!message)
	{
		allocateSongMessage();
		if (!message) 
			return;
	}

	mp_uint32 oSize = (mp_uint32)strlen(message) + 1;
	mp_uint32 nSize = (mp_uint32)strlen(line) + 1;

	mp_uint32 size = oSize + nSize + 2;
	
	if (size > messageBytesAlloc)
	{
		char* tempMessage = new char[size];
		if (tempMessage)
		{
			memset(tempMessage, 0, size);
			messageBytesAlloc = size;

			strcpy(tempMessage, message);
			delete[] message;
			message = tempMessage;
		}
		else 
			return;
	}

	// if this is not the first line in song message,
	// add CR to the previous line
	if (strlen(message) != 0)
	{		
		message[strlen(message)] = 0x0D;
		message[strlen(message)+1] = '\0';
	}

	strcat(message, line);
	
}

// start iterating text lines (get size of line)
mp_sint32 XModule::getFirstSongMessageLineLength()
{
	if (message == NULL)
		return -1;
	
	// no song message at all
	if (*message == '\0')
		return -1;

	messagePtr = message;
	
	mp_sint32 i = 0;
	while (messagePtr[i] != 0x0D && messagePtr[i] != '\0')
		i++;
		
	return i;
}

// get next size text line
mp_sint32 XModule::getNextSongMessageLineLength()
{
	
	if (message == NULL)
		return -1;	

	// advance to next line first
	while (*messagePtr != 0x0D && *messagePtr != '\0')
		messagePtr++;
		
	// we reached end of song message
	if (*messagePtr == '\0')
		return -1;
	
	ASSERT(*messagePtr == 0x0D);
	
	// skip CR
	messagePtr++;
	
	mp_sint32 i = 0;
	while (messagePtr[i] != 0x0D && messagePtr[i] != '\0')
		i++;
		
	return i;
}

// get line
void XModule::getSongMessageLine(char* line)
{
	mp_sint32 i = 0;
	while (messagePtr[i] != 0x0D && messagePtr[i] != '\0')
	{
		line[i] = messagePtr[i];
		i++;
	}
	
	line[i] = '\0';
}

// search for subsongs
void XModule::buildSubSongTable()
{
	if (!moduleLoaded)
		return;
		
	mp_ubyte* positionLookup = new mp_ubyte[header.ordnum*256];

	if (positionLookup == NULL)
		return;

	memset(positionLookup, 0, header.ordnum*256);

	// entire song = first subsong, starts at 0
	subSongPositions[numSubSongs*2] = 0;
	subSongPositions[numSubSongs*2+1] = 0;

	mp_ubyte pbreak = 0;
	mp_ubyte pbreakpos = 0;
	mp_ubyte pjump = 0;
	mp_ubyte pjumppos = 0, pjumprow = 0;

	mp_sint32 poscnt = 0, rowcnt = 0;
	mp_sint32 poscntMax = -1;

	while (true)
	{
		
		bool breakMain = false;
		
		while (!breakMain)
		{
			
			/*if (header.ord[poscnt]==254) 
			{ 
				while (header.ord[poscnt]==254) 
				{ 
					poscnt++; 
					if (poscnt>=header.ordnum) 
					{
						breakMain = true;
						break;
					}
				} 
			}*/
			
			if (!breakMain)
			{
				
				mp_sint32 ord = header.ord[poscnt];
				if (ord < header.patnum)
				{
					mp_ubyte* pattern = phead[ord].patternData;
					
					mp_sint32 r = rowcnt;
					
					mp_sint32 i = poscnt*256+r;
					
					if (!positionLookup[i])
						positionLookup[i]++;
					else
					{
						subSongPositions[numSubSongs*2+1] = poscntMax;
						numSubSongs++;

						breakMain = true;
						continue;
					}
					
					pbreak = pbreakpos = pjump = pjumppos = pjumprow = 0;
					
					for (mp_sint32 c = 0; c < phead[ord].channum; c++)
					{
						
						mp_sint32 slotSize = 2 + 2*phead[ord].effnum;
						
						mp_ubyte* slot = pattern + r*phead[ord].channum*slotSize + c*slotSize;
						
						for (mp_sint32 e = 0; e < phead[ord].effnum; e++)
						{
							mp_ubyte eff = slot[2+e*2];
							mp_ubyte eop = slot[2+e*2+1];
							
							switch (eff)
							{
								case 0x0B : 
								{
									pjump = 1;
									pjumppos = eop;
									pjumprow = 0;
									break;
								}
									
								case 0x0D : 
								{
									pbreak=1;
									pbreakpos = (eop>>4)*10+(eop&0xf);
									break;
								}
								
								case 0x0F:
								{
									if (eop == 0)
									{
										subSongPositions[numSubSongs*2+1] = poscntMax;
										numSubSongs++;
										breakMain = true;
										poscnt++;
										rowcnt = 0;
										goto skipChannels;
									}
									break;
								}
								
								case SubSongMarkEffect:
								{
									if (eop == SubSongMarkOperand)
									{
										subSongPositions[numSubSongs*2+1] = poscntMax;
										numSubSongs++;
										breakMain = true;
										continue;
									}
								}
								
								case 0x2B:
								{
									pjump = 1;
									pjumppos = eop;
									pjumprow = slot[2+((e+1)%phead[ord].effnum)*2+1];
									break;
								}
							}		
						} // effects
						
					} // channels
					
					if (poscnt > poscntMax)
						poscntMax = poscnt;

					// player logic
					// break pattern?
					if (pbreak&&(poscnt<(header.ordnum-1))) 
					{
						if (!pjump)
							poscnt++;
						rowcnt=pbreakpos-1;
					}
					else if (pbreak&&(poscnt==(header.ordnum-1))) 
					{
						if (!pjump)
							poscnt=0;
						rowcnt=pbreakpos-1;
					}
					
					// pattern jump?
					if (pjump) 
					{
						if (!pbreak)
							rowcnt = pjumprow-1;
						
						if (pjumppos < header.ordnum) 
							poscnt = pjumppos;
					}
					
					rowcnt++;
					
					// make sure we're getting the right pattern, position might
					// have changed because of position jumps or pattern breaks
					ord = header.ord[poscnt];
					if (rowcnt >= phead[ord].rows)
					{
						poscnt++;
					
						rowcnt = 0;
						
						if (poscnt >= header.ordnum)
						{
							poscnt = 0;
						}
						
					}
skipChannels:;
				}
				else
				{
					mp_sint32 i = poscnt*256;
					memset(positionLookup+i, 1, 256);
					poscnt++;					
					rowcnt = 0;
					if (poscnt >= header.ordnum)
						poscnt = 0;
				}
				
			}
			
		}
		
		if (numSubSongs >= 256)
			break;
		
		bool allPlayed = true;
		for (poscnt = 0; poscnt < header.ordnum; poscnt++)
		{
			mp_sint32 ord = header.ord[poscnt];
			
			if (ord < header.patnum)
			{
				bool played = false;
			
				mp_sint32 slotSize = 2 + 2*phead[ord].effnum;
				
				for (mp_sint32 i = 0; i < phead[ord].rows; i++)
				{
					if (positionLookup[poscnt*256+i])
					{
						played = true;
						break;
					}
				}
				
				if (!played)
				{
					bool empty = true;
					mp_sint32 offs = 0;
					mp_ubyte* pattern = phead[ord].patternData;
					for (mp_sint32 i = 0; i < phead[ord].rows*phead[ord].channum; i++)
					{
						if (pattern[offs])
						{
							empty = false;
							break;
						}
						offs+=slotSize;
					}
					if (empty)
					{
						memset(positionLookup+poscnt*256, 1, 256);
						played = true;
					}
				}
				
				if (!played)
				{
					subSongPositions[numSubSongs*2] = poscnt;
					// make it safe
					subSongPositions[numSubSongs*2+1] = poscnt;
					poscntMax = poscnt;
					//numSubSongs++;
					rowcnt = 0;
					allPlayed = false;
					break;
				}
			}
		}
		
		if (allPlayed)
			break;
		
	}
	
	delete[] positionLookup;
		
#if 0
	if (subSongPositions[(numSubSongs-1)*2+1] < header.ordnum - 1)
	{
		numSubSongs++;
		subSongPositions[(numSubSongs-1)*2] = subSongPositions[(numSubSongs-2)*2+1] + 1;
		subSongPositions[(numSubSongs-1)*2+1] = header.ordnum - 1;
	}
#endif
	
	//subSongPositions[3*2] = 3;
	//subSongPositions[3*2+1] = 10;

	//for (mp_sint32 i = 0; i < numSubSongs*2; i++)
	//	printf("%i: %i\n",i,subSongPositions[i]);
	//printf("\n\n");
	
	mp_sint32 i,j = 0,k = 0;
	mp_sint32 tempSubSongPositions[256*2];
	for (i = 0; i < numSubSongs*2; i++)	
		tempSubSongPositions[i] = subSongPositions[i];

	// find subsets of sub songs and merge them
	for (i = 0; i < numSubSongs; i++)
	{
		mp_sint32 start = tempSubSongPositions[i*2];
		mp_sint32 end = tempSubSongPositions[i*2+1];
		
		if (start != -1 && end != -1)
		{
			
			for (j = 0; j < numSubSongs; j++)
			{
				if (j != i)
				{
					if ((start <= tempSubSongPositions[j*2]) && 
						(end >= tempSubSongPositions[j*2+1]))
					{
						tempSubSongPositions[j*2] = tempSubSongPositions[j*2+1] = -1;
					}
					else if ((start <= tempSubSongPositions[j*2]) && 
							 (end >= tempSubSongPositions[j*2]))
					{
						end = tempSubSongPositions[j*2+1];
						tempSubSongPositions[j*2] = tempSubSongPositions[j*2+1] = -1;
					}
					else if ((start <= tempSubSongPositions[j*2+1]) && 
							 (end >= tempSubSongPositions[j*2+1]))
					{
						start = tempSubSongPositions[j*2];
						tempSubSongPositions[j*2] = tempSubSongPositions[j*2+1] = -1;
					}
				}
			}
		}
		
		tempSubSongPositions[i*2] = start;
		tempSubSongPositions[i*2+1] = end;
	}
	
	// cut out sets which have been merged/replaced
	for (i = 0; i < numSubSongs; i++)
	{
		if ((tempSubSongPositions[i*2] != -1) && (tempSubSongPositions[i*2+1] != -1))
		{
			subSongPositions[k*2] = (mp_ubyte)tempSubSongPositions[i*2];
			subSongPositions[k*2+1] = (mp_ubyte)tempSubSongPositions[i*2+1];
			k++;
		}	
	}
	
	//printf("subsongs initial: %i, after: %i\n", numSubSongs, k);
	
	numSubSongs = k;
	
	//for (mp_sint32 i = 0; i < numSubSongs*2; i++)
	//	printf("%i: %i\n",i,subSongPositions[i]);
	
	// one subsong is no subsong
	if (numSubSongs == 1)
		numSubSongs = 0;
	
}

// get subsong pos
mp_sint32 XModule::getSubSongPosStart(mp_sint32 i) const
{
	if (i >= 0 && i < numSubSongs)
		return subSongPositions[i*2];
		
	return 0;
}

mp_sint32 XModule::getSubSongPosEnd(mp_sint32 i) const
{
	if (i >= 0 && i < numSubSongs)
		return subSongPositions[i*2+1];
		
	return 0;
}

// MilkyTracker additions
void XModule::createEmptySong(bool clearPatterns/* = true*/, bool clearInstruments/* = true*/, mp_sint32 numChannels/* = 8*/)
{
	moduleLoaded = true;

	type = ModuleType_XM;

	mp_uint32 i;

	if (clearPatterns)
	{
#ifdef MILKYTRACKER
		for (i = 0; i < 256; i++)
#else
		for (i = 0; i < header.patnum; i++)
#endif
		{
			if (phead[i].patternData) 
			{
				delete[] phead[i].patternData;
				phead[i].patternData = NULL;
			}
			
		}
		memset(header.ord, 0, sizeof(header.ord));
		// song length
		header.ordnum = 1;
		header.patnum = 0;
		header.restart = 0;
	}

	if (clearInstruments)
	{
		if (venvs)
		{
			delete[] venvs;
			venvs = NULL;
			numVEnvsAlloc = numVEnvs = 0;
		}
		if (penvs)
		{
			delete[] penvs;
			penvs = NULL; 
			numPEnvsAlloc = numPEnvs = 0;
		}
		if (fenvs)
		{
			delete[] fenvs;
			fenvs = NULL;
			numFEnvsAlloc = numFEnvs = 0;
		}
		if (vibenvs)
		{
			delete[] vibenvs;
			vibenvs = NULL;
			numVibEnvsAlloc = numVibEnvs = 0;
		}
		if (pitchenvs)
		{
			delete[] pitchenvs;
			pitchenvs = NULL;
			numPitchEnvsAlloc = numPitchEnvs = 0;
		}
		
		if (message)
		{
			delete[] message;
			message = NULL;
			messageBytesAlloc = 0;
		}
		
		// release sample-memory
		for (i = 0; i < samplePointerIndex; i++)
		{
			if (samplePool[i]) 
			{
				TXMSample::freePaddedMem(samplePool[i]);
				samplePool[i] = NULL;
			}
		}

		samplePointerIndex = 0;
		
		if (instr)
			memset(instr,0,sizeof(TXMInstrument)*256);
		
		// some default values please
		if (smp)
		{
			memset(smp,0,sizeof(TXMSample)*MP_MAXSAMPLES);
		
			for (i = 0; i < MP_MAXSAMPLES; i++)
			{
				smp[i].vol = 0xff;
				smp[i].pan = 0x80;
				smp[i].flags = 3;
				smp[i].volfade = 65535;
			}
		}
		
		header.insnum = 128;
		header.smpnum = 128*16;
		header.volenvnum = 0;
		header.panenvnum = 0;
		header.frqenvnum = 0;
		header.vibenvnum = 0;
		header.pitchenvnum = 0;
	}
	
	// clear entire song
	if (clearPatterns && clearInstruments)
	{
		memset(&header,0,sizeof(TXMHeader));
		
		header.insnum = 128;
		header.smpnum = 128*16;

		header.ordnum = 1;
		header.patnum = 0;
		header.restart = 0;

		header.channum = numChannels;
		header.freqtab = 1;
		header.mainvol = 255;
		
		// number of patterns
		
		header.speed = 125;
		header.tempo = 6;

		if (message)
		{
			delete[] message;
			message = NULL;
			messageBytesAlloc = 0;
		}
	
		setDefaultPanning();
	}

	header.flags = XModule::MODULE_XMNOTECLIPPING | 
		XModule::MODULE_XMARPEGGIO | 
		XModule::MODULE_XMPORTANOTEBUFFER | 
		XModule::MODULE_XMVOLCOLUMNVIBRATO;

}

void XModule::removeOrderSkips()
{
	mp_sint32 newOrderListReloc[256];
	mp_ubyte newOrderList[256];
	
	mp_sint32 i,j;
	
	j = 0;
	for (i = 0; i < header.ordnum; i++)
	{
		if (header.ord[i] < header.patnum)
		{
			newOrderListReloc[i] = j; 
			newOrderList[j++] = header.ord[i];
		}
		else
		{
			newOrderListReloc[i] = -1; 
		}
	}
	
	mp_sint32 newLen = j;
	
	for (i = 0; i < header.ordnum; i++)
	{
		if (newOrderListReloc[i] == -1)
		{
			j = i;
			mp_sint32 reloc = 0;
			while (newOrderListReloc[j] == -1 && j < header.ordnum) j++;
			if (j != header.ordnum)
				reloc = newOrderListReloc[j];
			newOrderListReloc[i] = reloc;
		}
	}
	
	for (i = 0; i < header.patnum; i++)
	{
		if (phead[i].patternData)
		{
			mp_ubyte* data = phead[i].patternData;

			mp_sint32 slotSize = phead[i].effnum * 2 + 2;

			mp_sint32 patternSize = phead[i].channum * phead[i].rows;
			for (j = 0; j < patternSize; j++)
			{
				mp_ubyte* ptr = data+j*slotSize+2;
				for (mp_sint32 e = 0; e < phead[i].effnum; e++)
				{
					if (ptr[e*2] == 0x0B && ptr[e*2+1] < header.ordnum)
					{
						ptr[e*2+1] = newOrderListReloc[ptr[e*2+1]];
					}
				}
			}
		}
	}
	
	header.ordnum = newLen;
	
	memset(header.ord, 0, sizeof(header.ord));
	
	memcpy(header.ord, newOrderList, newLen);
	
}

mp_sint32 XModule::removeUnusedPatterns(bool evaluate)
{
	if (!header.patnum)
		return 0;

	mp_sint32 i,j;

	mp_ubyte* bitMap = new mp_ubyte[header.patnum];

	memset(bitMap, 0, sizeof(mp_ubyte)*header.patnum);

	mp_sint32 numUsedPatterns = 0;
	for (i = 0; i < header.ordnum; i++)
	{
		j = header.ord[i];

		// this must *should* be always the case
		if (j < header.patnum && !bitMap[j])
		{
			bitMap[j] = 1;
			numUsedPatterns++;
		}
	}

	if (!numUsedPatterns || numUsedPatterns == header.patnum)
	{
		delete[] bitMap;
		return 0;
	}

	mp_sint32 result = abs(header.patnum - numUsedPatterns);

	if (evaluate)
	{
		delete[] bitMap;
		return result;
	}

	mp_sint32* patRelocTable = new mp_sint32[header.patnum];

	for (i = 0, j = 0; i < header.patnum; i++)
	{
		if (bitMap[i])
		{
			patRelocTable[i] = j++;
		}
	}

	TXMPattern* tempPHeads = new TXMPattern[header.patnum];

	memcpy(tempPHeads, phead, header.patnum*sizeof(TXMPattern));

	memset(phead, 0, header.patnum*sizeof(TXMPattern));

	for (i = 0; i < header.patnum; i++)
	{
		if (bitMap[i])
		{
			j = patRelocTable[i];
			phead[j] = tempPHeads[i];
		}
		else 
		{
			delete[] tempPHeads[i].patternData;
			tempPHeads[i].patternData = NULL;
		}
	}

	for (i = 0; i < header.ordnum; i++)
	{
		header.ord[i] = (mp_ubyte)patRelocTable[header.ord[i]];
	}

	delete[] tempPHeads;
	delete[] patRelocTable;
	delete[] bitMap;

	header.patnum = numUsedPatterns;

	return result;
}

void XModule::postLoadAnalyser()
{
	mp_sint32 i,r,c;

	bool oldPTProbability = false;

	for (i = 0; i < header.patnum; i++)
	{
	
		if (phead[i].patternData)
		{
			mp_ubyte* data = phead[i].patternData;

			mp_sint32 slotSize = phead[i].effnum * 2 + 2;
			mp_sint32 rowSize = slotSize * phead[i].channum;

			for (c = 0; c < phead[i].channum; c++)
			{
				mp_sint32 insCycleCounter = 0;
				mp_sint32 lastCycleIns = -1;
				mp_sint32 lastIns = -1;
				bool hasCycled = false;
			
				for (r = 0; r < phead[i].rows; r++)
				{
					
					mp_ubyte* slot = data + r*rowSize + c*slotSize;

					if (!oldPTProbability)
					{
						if (slot[1] && !slot[0] && (mp_sint32)slot[1] != lastCycleIns)
						{
							insCycleCounter++;
							hasCycled = true;
							lastCycleIns = slot[1];
						}
						else if (slot[1] && slot[0] && hasCycled)
						{
							insCycleCounter = 0;
							hasCycled = false;
							lastCycleIns = -1;
						}
						
						if (insCycleCounter >= 3 && hasCycled)
						{
#ifdef VERBOSE
							printf("pattern:%i, channel:%i, row:%i\n",i,c,r);
#endif
							oldPTProbability = true;
						}
					
						// another try:
						if (lastIns != -1)
						{
							if (slot[0] && slot[1] && (slot[1] != lastIns) && (slot[2] == 0x03 || slot[2] == 0x05))
							{
#ifdef VERBOSE
								printf("pattern:%i, channel:%i, row:%i\n",i,c,r);
#endif
								oldPTProbability = true;
							}
						}
					}

					if (slot[1])
						lastIns = slot[1];					
					
				}
			}
		}
	}
	
	if (oldPTProbability)
		header.flags |= MODULE_OLDPTINSTRUMENTCHANGE;

#ifdef VERBOSE
	printf("%s: %i\n", header.name, oldPTProbability);
#endif
}

void XModule::convertXMVolumeEffects(mp_ubyte vol, mp_ubyte& effect, mp_ubyte& operand)
{
	effect = 0;
	operand = 0;

	if (vol>=0x10&&vol<=0x50) {
		effect = 0x0C;
		operand = XModule::vol64to255(vol-0x10);
	}
	
	if (vol>=0x60) {
		mp_ubyte eff = vol>>4;
		mp_ubyte op  = vol&0xf;
		/*printf("%x, %x\r\n",eff,op);
		getch();*/
		if (op) 
		{
			switch (eff) {
				case 0x6 : {
					effect=0x0A;
					operand=op;
				}; break;
				case 0x7 : {
					effect=0x0A;
					operand=op<<4;
				}; break;
				case 0x8 : {
					effect=0x3B;
					operand=op;
				}; break;
				case 0x9 : {
					effect=0x3A;
					operand=op;
				}; break;
				case 0xA : {
					effect=0x4;
					operand=op<<4;
				}; break;
				case 0xB : {
					effect=0x4;
					operand=op;
				}; break;
				case 0xC : {
					effect=0x8;
					operand=(mp_ubyte)XModule::pan15to255(op);
				}; break;
				case 0xD : {
					effect=0x19;
					operand=op;
				}; break;
				case 0xE : {
					effect=0x19;
					operand=op<<4;
				}; break;
				case 0xF : {
					effect=0x3;
					operand=op<<4;
				}; break;
					
			}
		}
		else
		{
			switch (eff) {
				case 0xB : {
					effect=0x4;
					operand=op;
				}; break;
				case 0xC : {
					effect=0x8;
					operand=(mp_ubyte)XModule::pan15to255(op);
				}; break;
				case 0xF : {
					effect=0x3;
					operand=op;
				}; break;
					
			}
		}
	}
	
}

XModule::IsPTCompatibleErrorCodes XModule::isPTCompatible()
{
	mp_sint32 i;
	
	// step 1: linear frequencies are used
	if (header.freqtab & 1)
		return IsPTCompatibleErrorCodeLinearFrequencyUsed;
	
	// step 2: find last used instrument, if greater than 31 => too many samples
	mp_sint32 insNum = header.insnum;
	for (i = header.insnum - 1; i > 0; i--)
	{
		mp_ubyte buffer[MP_MAXTEXT+1];
		
		convertStr(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(instr[i].name), MP_MAXTEXT, false);
		
		if (strlen((char*)buffer))
		{
			insNum = i+1;
			break;
		}
		
		if (instr[i].samp)
		{
			
			mp_sint32 lasts = -1;
#ifdef MILKYTRACKER
			for (mp_sint32 j = 0; j < 96; j++)
#else
			for (mp_sint32 j = 0; j < 120; j++)
#endif
			{
				mp_sint32 s = instr[i].snum[j];
				
				if (lasts != -1 && s != lasts)
					return IsPTCompatibleErrorCodeIncompatibleInstruments;
				
				lasts = s;
							
				if (s >= 0)
				{
					convertStr(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(smp[s].name), MP_MAXTEXT, false);
					if (strlen((char*)buffer) || (smp[s].sample && smp[s].samplen))
					{
						insNum = i+1;
						goto insFound;
					}					
				}
			}		
		}
	}
	
insFound:
	if (i == 0)
		insNum = 1;		
	
	if (insNum > 31)
		return IsPTCompatibleErrorCodeTooManyInstruments;
	
	// step 3: incompatible samples
	for (i = 0; i < MP_MAXSAMPLES; i++)
	{
		if (smp[i].samplen >= 0xffff || (smp[i].samplen &&
			((smp[i].type & 16) || (smp[i].type & 3) == 2 ||
			smp[i].relnote || smp[i].pan != 0x80)))
			return IsPTCompatibleErrorCodeIncompatibleSamples;
		
		if (smp[i].venvnum)
		{
			if (venvs[smp[i].venvnum-1].type & 1)
				return IsPTCompatibleErrorCodeIncompatibleInstruments;
		}
		
		if (smp[i].penvnum)
		{
			if (penvs[smp[i].penvnum-1].type & 1)
				return IsPTCompatibleErrorCodeIncompatibleInstruments;
		}
		
		if (smp[i].vibdepth && smp[i].vibrate)
			return IsPTCompatibleErrorCodeIncompatibleInstruments;
	}
	
	// step 4: incompatible patterns
	for (i = 0; i < header.patnum; i++)
	{
		mp_sint32 slotSize = phead[i].effnum * 2 + 2;
		mp_sint32 rowSizeSrc = slotSize*phead[i].channum;
	
		if (phead[i].rows != 64)
			return IsPTCompatibleErrorCodeIncompatiblePatterns;
	
		for (mp_sint32 r = 0; r < phead[i].rows; r++)
			for (mp_sint32 c = 0; c < header.channum; c++)
			{
				if (c < phead[i].channum)
				{
					mp_ubyte* src = phead[i].patternData + r*rowSizeSrc+c*slotSize;
				
					// check note range
					mp_ubyte note = *src;
					if (note)
					{
						note--;
						if (!(note >= 36 && note < 36+12*3))
							return IsPTCompatibleErrorCodeIncompatiblePatterns;
					}
					
					// check volume command
					mp_ubyte eff = *(src+2);
					if (eff)
						return IsPTCompatibleErrorCodeIncompatiblePatterns;
						
					// check for normal commands
					eff = *(src+4);
					if (eff > 0xF && (eff < 0x30 || eff >=0x3F) && eff != 0x20)
						return IsPTCompatibleErrorCodeIncompatiblePatterns;
				}
			}
	
	}

	return IsPTCompatibleErrorCodeNoError;
}
