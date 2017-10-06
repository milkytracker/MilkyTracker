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
 *  XModule.h
 *  MilkyPlay
 *
 *
 */
#ifndef __XMODULE_H__
#define __XMODULE_H__

#include "XMFile.h"

#define MP_MAXTEXT 32
#define MP_MAXORDERS 256

struct TXMHeader 
{
	char		sig[17];
	char		name[MP_MAXTEXT];
	char		whythis1a;
	char		tracker[MP_MAXTEXT];
	mp_uword	ver;
	mp_uint32	hdrsize;
	mp_uword	ordnum;
	mp_uword	restart;
	mp_uword	channum;
	mp_uword	patnum;
	mp_uword	insnum;
	mp_uword	smpnum;			// additional: number of samples in tune
	mp_uword	volenvnum;		// additional: number of volume envelopes
	mp_uword	panenvnum;		// additional: number of panning envelopes
	mp_uword	frqenvnum;		// additional: number of frequency envelopes (MDL)
	mp_uword	vibenvnum;		// additional: number of vibrato envelopes (AMS)
	mp_uword	pitchenvnum;	// additional: number of pitch envelopes (IT)
	mp_uword	freqtab;
	mp_ubyte	uppernotebound;	// additional: note limit if not zero
	mp_sbyte	relnote;		// additional: semitone adjust value
	mp_dword    flags;			// additional: some flags 
	mp_uword	tempo;
	mp_uword	speed;
	mp_uword	mainvol;
	mp_ubyte	ord[MP_MAXORDERS];
	mp_ubyte	pan[256];
};

// ** BEWARE :) **
// the order of the different elements concerning the envelope info 
// is not like in the xm format
struct TEnvelope 
{
	mp_uword	env[256][2];
	mp_ubyte	num,sustain,susloope,loops,loope,type,speed;
};

struct TXMInstrument
{
	enum Flags
	{
		IF_ITNOTEREMAPPING	= 0x01,
		IF_ITFADEOUT		= 0x02,
		IF_ITENVELOPES		= 0x04,
		IF_ITGOBALINSVOL	= 0x08
	};

	mp_uint32	size;
	char		name[MP_MAXTEXT];
	char		type;
	mp_uword	samp;
	mp_uint32	shsize;
	mp_sword	snum[120];		// -1 is an invalid/empty sample
	mp_uword	flags;			// not in .XM => from myself
								// Also holds NNA (bit 4-5), DCT (bit 6-7), DCA (bit 8-9)
	mp_ubyte	notemap[120];	// Impulse Tracker addition
	mp_uword	volfade;		// Impulse Tracker addition

	mp_uword	venvnum;		// Impulse Tracker envelopes can't be sample-based (different envelopes can map to the same sample) ...
	mp_uword	penvnum;		// ... (only when IF_ITENVELOPES flag is set)
	mp_uword	fenvnum;
	mp_uword	vibenvnum;		
	mp_uword	pitchenvnum;	// IT pitch envelope
	
	mp_uword	res;			// when bit 3 (= 8) of flags is set, take this as global instrument vol (Impulse Tracker)
	
	mp_ubyte	ifc;			// IT Initial Filter cutoff
	mp_ubyte	ifr;			// IT Initial Filter resonance
	//char		extra[20];
};

// some words about the samples:
// although the sample buffer is freely available for acess, I would not recommend
// accessing it directly if you don't exactly know what you are doing because
// to avoid sample clicks when looping samples, it uses a tricky double buffering
// technique between smoothed out sample loop areas and the original sample data
// If you need to modify sample data yourself, use the provided methods 
// getSampleValue and setSampleValue and call postProcessSamples when you're done
// modifying the sample, so the loop information is updated correctly
// Also call postProcessSamples when you're changing the loop information
struct TXMSample 
{
private:
	struct TLoopDoubleBuffProps
	{
		enum
		{
			StateUnused,
			StateUsed,
			StateDirty,
		};
		
		mp_uint32 samplesize;
		mp_ubyte state[4];
		mp_uint32 lastloopend;
	};

	enum 
	{
		LoopAreaBackupSize = 4,
		LoopAreaBackupSizeMaxInBytes = 8,		
		EmptySize = 8,
		LeadingPadding = sizeof(TLoopDoubleBuffProps) + LoopAreaBackupSizeMaxInBytes + EmptySize,
		TrailingPadding = 16,
		PaddingSpace = LeadingPadding+TrailingPadding
	};

	void restoreLoopArea();

public:
	mp_uint32	samplen;
	mp_uint32	loopstart;
	mp_uint32	looplen;
	mp_ubyte	flags;			// Bit 0: Use volume
								// Bit 1: Use panning
								// Bit 2: Use sample volume as channel global volume (.PLM modules)
								// Bit 3: Use the res field as global sample volume
								// Bit 4: Use IT style auto vibrato (not implemented yet)
	mp_ubyte	vol;
	mp_sbyte	finetune;
	mp_ubyte	type;			// In Addition to XM: Bit 5: One shot forward looping sample (MOD backward compatibility)
	mp_ubyte	pan;
	mp_sbyte	relnote;
	mp_uword	venvnum;
	mp_uword	penvnum;
	mp_uword	fenvnum;
	mp_uword	vibenvnum;
	mp_uword	pitchenvnum;
	mp_ubyte	vibtype, vibsweep, vibdepth, vibrate;
	mp_uword	volfade;
	mp_ubyte	res;			// when bit 3 (= 8) of flags is set, take this as global sample vol (Impulse Tracker)
	mp_sword	freqadjust;
	char		name[MP_MAXTEXT];
	mp_ubyte	terminate;
	mp_sbyte*   sample;

	static mp_uint32 getPaddedSize(mp_uint32 size)
	{
		return size+TXMSample::PaddingSpace;
	}

	static mp_ubyte* getPadStartAddr(mp_ubyte* mem)
	{
		return mem-TXMSample::LeadingPadding;
	}

	static mp_ubyte* allocPaddedMem(mp_uint32 size)
	{
		mp_ubyte* result = new mp_ubyte[getPaddedSize(size)];
		
		if (result == NULL)
			return NULL;
		
		// clear out padding space
		memset(result, 0, TXMSample::LeadingPadding);
		memset(result+size+TXMSample::LeadingPadding, 0, TXMSample::TrailingPadding);
		
		TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)result;
		loopBufferProps->samplesize = size;
		
		return result + TXMSample::LeadingPadding;
	}
	
	static void freePaddedMem(mp_ubyte* mem)
	{
		// behave safely on NULL
		if (mem == NULL)
			return;
			
		delete[] getPadStartAddr(mem);
	}

	static void copyPaddedMem(void* dst, const void* src, mp_uint32 size)
	{
		mp_ubyte* _src = ((mp_ubyte*)src) - TXMSample::LeadingPadding;
		mp_ubyte* _dst = ((mp_ubyte*)dst) - TXMSample::LeadingPadding;
		memcpy(_dst, _src, getPaddedSize(size));
	}
	
	static mp_uint32 getSampleSizeInBytes(mp_ubyte* mem)
	{
		TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr(mem);
		return loopBufferProps->samplesize;
	}

	static mp_uint32 getSampleSizeInSamples(mp_ubyte* mem)
	{
		TLoopDoubleBuffProps* loopBufferProps = (TLoopDoubleBuffProps*)getPadStartAddr(mem);
		return (loopBufferProps->state[1] & 16) ? (loopBufferProps->samplesize >> 1) : loopBufferProps->samplesize;
	}

	void smoothLooping();
	void restoreOriginalState();
	void postProcessSamples();

	// get sample value
	// values range from [-32768,32767] in case of a 16 bit sample
	// or from [-128,127] in case of an 8 bit sample
	mp_sint32 getSampleValue(mp_uint32 index);
	mp_sint32 getSampleValue(mp_ubyte* sample, mp_uint32 index);
	void setSampleValue(mp_uint32 index, mp_sint32 value);
	void setSampleValue(mp_ubyte* sample, mp_uint32 index, mp_sint32 value);
	
#ifdef MILKYTRACKER
	bool equals(const TXMSample& sample) const
	{
		if (this->sample != sample.sample)
			return false;

		if (samplen != sample.samplen)
			return false;

		return true;
	}

	bool isMinimizable() const
	{
		return (loopstart + looplen < samplen && (type & 3));
	}
#endif

	friend class XModule;
};

struct TXMPattern 
{
	mp_uint32	len;
	mp_ubyte	ptype;
	mp_uword	rows;
	mp_ubyte	effnum;
	mp_ubyte	channum;
	mp_uword	patdata;
	mp_ubyte*   patternData;
	
	mp_sint32 compress(mp_ubyte* dest) const;
	mp_sint32 decompress(mp_ubyte* src, mp_sint32 len);
#ifdef MILKYTRACKER
	bool saveExtendedPattern(const SYSCHAR* fileName) const;
	bool loadExtendedPattern(const SYSCHAR* fileName);

	bool saveExtendedTrack(const SYSCHAR* fileName, mp_uint32 channel) const;
	bool loadExtendedTrack(const SYSCHAR* fileName, mp_uint32 channel);	
	
	const TXMPattern& operator=(const TXMPattern& src);
#endif
};

//////////////////////////////////////////////////////////////////////////
// This is the class which handles a MilkyPlay module					//
//////////////////////////////////////////////////////////////////////////
class XModule
{
public:
	enum
	{
		IdentificationBufferSize	= 2048,
	};

	///////////////////////////////////////////////////////
	// this is our loader interface (abstract class)	 //
	// each loader has to conform to this interface		 //
	///////////////////////////////////////////////////////
	class LoaderInterface
	{
	public:
		// make GCC shut up
		virtual ~LoaderInterface() { }		
		// returns c-string which identifies the module, NULL if loader can't identify module
		// IMPORTANT: buffer MUST contain eIdentifyBufferSize bytes of the beginning of the file
		virtual const char* identifyModule(const mp_ubyte* buffer)			= 0;
		// try to load module (check with identifyModule first)
		virtual mp_sint32   load(XMFileBase& f, XModule* module)			= 0;
	};

	enum ModuleTypes
	{
		ModuleType_UNKNOWN,
		ModuleType_669,
		ModuleType_AMF,
		ModuleType_AMS,
		ModuleType_CBA,
		ModuleType_DBM,
		ModuleType_DIGI,
		ModuleType_DSM,
		ModuleType_DSm,
		ModuleType_DTM_1,
		ModuleType_DTM_2,
		ModuleType_FAR,
		ModuleType_GDM,
		ModuleType_GMC,
		ModuleType_IMF,
		ModuleType_IT,
		ModuleType_MDL,
		ModuleType_MOD,
		ModuleType_MTM,
		ModuleType_MXM,
		ModuleType_OKT,
		ModuleType_PLM,
		ModuleType_PSM,
		ModuleType_PTM,
		ModuleType_S3M,
		ModuleType_STM,
		ModuleType_SFX,
		ModuleType_UNI,
		ModuleType_ULT,
		ModuleType_XM,
		ModuleType_NONE = -1,
	};	
	
	class SampleLoader
	{
	protected:
		XMFileBase& f;
	
	public:
		SampleLoader(XMFileBase& file) :
			f(file)
		{
		}
		
		virtual ~SampleLoader()
		{
		}

		virtual mp_sint32 load_sample_8bits(void* p_dest_buffer, mp_sint32 compressedSize, mp_sint32 p_buffsize) = 0;
		virtual mp_sint32 load_sample_16bits(void* p_dest_buffer, mp_sint32 compressedSize, mp_sint32 p_buffsize) = 0;
	};	
	
private:
	struct TLoaderInfo
	{
		LoaderInterface* loader;
		ModuleTypes moduleType;
	};
	
public:
	enum
	{
		ST_DEFAULT			= 0x00,
		ST_DELTA			= 0x01,
		ST_UNSIGNED			= 0x02,
		ST_16BIT			= 0x04,
		ST_PACKING_MDL		= 0x08,
		ST_DELTA_PTM		= 0x10,
		ST_BIGENDIAN		= 0x20,
		ST_PACKING_IT		= 0x40,
		ST_PACKING_IT215	= 0x80,
		ST_PACKING_ADPCM	= 0x100
	};

	static const mp_sint32  periods[12];
	static const mp_sint32	sfinetunes[16];
	static const mp_sbyte	modfinetunes[16];
	
	static const mp_ubyte	numValidXMEffects;
	static const mp_ubyte	validXMEffects[];

	//////////////////////////////////////////////////////////////////////////
	// different stuff for importing different module types into own format //
	//////////////////////////////////////////////////////////////////////////
	static mp_sint32	FixedMUL(mp_sint32 a,mp_sint32 b) { return ((mp_sint32)(((mp_int64)(a)*(mp_int64)(b))>>16)); }
		
	///////////////////////////////////////////////////////
	// convert relative note + finetune into C4 speed    //
	///////////////////////////////////////////////////////
	static mp_sint32	getc4spd(mp_sint32 relnote, mp_sint32 finetune);
	///////////////////////////////////////////////////////
	// convert C4 speed into relative note + finetune    //
	///////////////////////////////////////////////////////
	static void			convertc4spd(mp_uint32 c4spd, mp_sbyte* finetune, mp_sbyte* relnote);
	
	static mp_uint32	amigaPeriodToNote(mp_uint32 period);

	///////////////////////////////////////////////////////
	// load sample into memory							 //
	///////////////////////////////////////////////////////
	static bool			loadSample(XMFileBase& f, void* buffer, 
								   mp_uint32 size, mp_uint32 length, 
								   mp_sint32 flags = ST_DEFAULT);
	
	///////////////////////////////////////////////////////
	// load a bunch of samples into memory				 //
	///////////////////////////////////////////////////////
	mp_sint32			loadModuleSample(XMFileBase& f, mp_sint32 index,
										 mp_sint32 flags8 = ST_DEFAULT, mp_sint32 flags16 = ST_16BIT,
										 mp_uint32 alternateSize = 0);
	
	mp_sint32			loadModuleSamples(XMFileBase& f, 
										  mp_sint32 flags8 = ST_DEFAULT, mp_sint32 flags16 = ST_16BIT);
	
	static void			convertXMVolumeEffects(mp_ubyte volume, mp_ubyte& eff, mp_ubyte& op);
	
	///////////////////////////////////////////////////////
	// Allocate sample memory and store pointer in pool  //
	// *Note* that this memory is always padded with 16  //
	// bytes at the start *AND* 16 bytes at the end.     //
	///////////////////////////////////////////////////////
	mp_ubyte*		allocSampleMem(mp_uint32 size);

	///////////////////////////////////////////////////////
	// Free sample memory
	///////////////////////////////////////////////////////
	void			freeSampleMem(mp_ubyte* mem, bool assertCheck = true);

#ifdef MILKYTRACKER
	void			insertSamplePtr(mp_ubyte* ptr);
	void			removeSamplePtr(mp_ubyte* ptr);
#endif

	///////////////////////////////////////////////////////
	//    Clean up! (Is called before loading a song)    //
	///////////////////////////////////////////////////////
	bool			cleanUp();

	///////////////////////////////////////////////////////
	// scan through samples and post process to avoid    //
	// interpolation clicks                              //
	///////////////////////////////////////////////////////
	void			postProcessSamples(bool heavy = false);

	///////////////////////////////////////////////////////
	// set default panning								 //
	///////////////////////////////////////////////////////
	void			setDefaultPanning();

private:
	// Identify module
	ModuleTypes		type;

	// Indicates whether a file is loaded or if it's just an empty song 
	bool			moduleLoaded;

	// each module comes with it's own sample-memory management (MILKYPLAY_MAXSAMPLES samples max.)
	mp_ubyte*		samplePool[MP_MAXSAMPLES];
	mp_uint32		samplePointerIndex;

	// song message retrieving
	char*			messagePtr;

	// subsong position table
	mp_ubyte		subSongPositions[256*2];
	mp_sint32		numSubSongs;

	// add nother envelope to a given list and increase size of array if necessary
	static bool		addEnvelope(TEnvelope*& envs,const TEnvelope& env,mp_uint32& numEnvsAlloc,mp_uint32& numEnvs);
	// fix broken envelopes (1 point envelope for example)
	static void		fixEnvelopes(TEnvelope* envs, mp_uint32 numEnvs);
	
	// holds available loader instances
	class LoaderManager
	{
	private:
		TLoaderInfo*	loaders;

		mp_uint32		numLoaders;
		mp_uint32		numAllocatedLoaders;

		mp_sint32		iteratorCounter;

		void registerLoader(LoaderInterface* loader, ModuleTypes type);
		
	public:
		LoaderManager();		
		~LoaderManager();
		
		TLoaderInfo* getFirstLoaderInfo();
		TLoaderInfo* getNextLoaderInfo();
	};

	friend class	LoaderManager;

	bool			validate();

public:
	
	// Module flags
	enum
	{
		MODULE_OLDS3MVOLSLIDES			= 1,
		MODULE_ST3NOTECUT				= 2,
		MODULE_ST3DUALCOMMANDS			= 4,
		MODULE_STMARPEGGIO				= 8,
		MODULE_XMARPEGGIO				= 16,
		MODULE_XMVOLCOLUMNVIBRATO		= 32,
		MODULE_XMNOTECLIPPING			= 64,
		MODULE_XMPORTANOTEBUFFER		= 128,
		MODULE_AMSENVELOPES				= 256,
		MODULE_PTNEWINSTRUMENT			= 512,
		MODULE_ST3NEWINSTRUMENT			= 1024,
		MODULE_OLDPTINSTRUMENTCHANGE	= 2048,
		MODULE_ITNOTEOFF				= 4096,
		MODULE_ITNEWEFFECTS				= 8192,
		MODULE_ITNEWGXX					= 16384,
		MODULE_ITLINKPORTAMEM			= 32768,
		MODULE_ITTEMPOSLIDE				= 65536,
	};
	
	enum
	{
		NOTE_LAST	= 120,
		NOTE_OFF	= 121,
		NOTE_CUT	= 122,
		NOTE_FADE	= 123,

		SubSongMarkEffect	= 0x1E,
		SubSongMarkOperand	= 0xFF,
	};
		
	TXMHeader		header;		// module header
	TXMInstrument*	instr;		// all instruments (256 of them)
	TXMSample*		smp;		// all samples (256 of them, only 255 can be used)
	TXMPattern*		phead;		// all pattern headers (256 of them)
	
	mp_uint32		messageBytesAlloc;
	char*			message;	// song message
	
	TEnvelope*		venvs;
	mp_uint32		numVEnvsAlloc;
	mp_uint32		numVEnvs; // should be equal to header.venvnum
	bool			addVolumeEnvelope(const TEnvelope& env) { return addEnvelope(venvs, env, numVEnvsAlloc, numVEnvs); }

	TEnvelope*		penvs;
	mp_uint32		numPEnvsAlloc;
	mp_uint32		numPEnvs; // should be equal to header.penvnum
 	bool			addPanningEnvelope(const TEnvelope& env) { return addEnvelope(penvs, env, numPEnvsAlloc, numPEnvs); }
	
	TEnvelope*		fenvs;
	mp_uint32		numFEnvsAlloc;
	mp_uint32		numFEnvs; // should be equal to header.fenvnum
	bool			addFrequencyEnvelope(const TEnvelope& env) { return addEnvelope(fenvs, env, numFEnvsAlloc, numFEnvs); }

	TEnvelope*		vibenvs;
	mp_uint32		numVibEnvsAlloc;
	mp_uint32		numVibEnvs; // should be equal to header.vibenvnum
	bool			addVibratoEnvelope(TEnvelope& env) { return addEnvelope(vibenvs, env, numVibEnvsAlloc, numVibEnvs); }

	TEnvelope*		pitchenvs;
	mp_uint32		numPitchEnvsAlloc;
	mp_uint32		numPitchEnvs; // should be equal to header.vibenvnum
	bool			addPitchEnvelope(TEnvelope& env) { return addEnvelope(pitchenvs, env, numPitchEnvsAlloc, numPitchEnvs); }
	
	///////////////////////////////////////////////////////
	// convert volume from range [0..64] to [0..255]     //
	///////////////////////////////////////////////////////
	static mp_sint32		vol64to255(mp_sint32 vol) { return ((vol>64?64:vol)*261120+65535)>>16; }

	///////////////////////////////////////////////////////
	// convert volume from range [0..255] to [0..64]     //
	///////////////////////////////////////////////////////
	static mp_uint32		vol255to64(mp_uint32 vol) { return (vol*64)/255; }

	///////////////////////////////////////////////////////
	// convert volume from range [0..127] to [0..255]    //
	///////////////////////////////////////////////////////
	static mp_sint32		vol127to255(mp_sint32 vol) { return ((vol>127?127:vol)*131588+65535)>>16; }

	///////////////////////////////////////////////////////
	// convert volume from range [0..128] to [0..255]    //
	///////////////////////////////////////////////////////
	static mp_sint32		vol128to255(mp_sint32 vol) { return ((vol>128?128:vol)*130560+65535)>>16; }
	
	static mp_sint32		pan15to255(mp_sint32 pan) { return pan>=0xF?0xFF:(pan<<4); }
	
	///////////////////////////////////////////////////
	// Allocate necessary memory for song structures //
	///////////////////////////////////////////////////
					XModule();
	
	///////////////////////////////////////////////////
	// Clean up										//
	///////////////////////////////////////////////////
					~XModule();

	///////////////////////////////////////////////////
	// Get type of module							 //
	///////////////////////////////////////////////////
	ModuleTypes		getType() const { return type; }

	///////////////////////////////////////////////////
	// identify module type 						 //
	// IMPORTANT: buffer MUST contain				 //
	// eIdentifyBufferSize bytes from the beginning  //
	// of the file									 //
	///////////////////////////////////////////////////
	static const char*	identifyModule(const mp_ubyte* buffer);
	
	///////////////////////////////////////////////////
	// generic module loader						 //
	///////////////////////////////////////////////////
	mp_sint32		loadModule(XMFileBase& f, bool scanForSubSongs = false);
	mp_sint32		loadModule(const SYSCHAR* fileName, bool scanForSubSongs = false);	 

	///////////////////////////////////////////////////
	// Module exporters								 //
	///////////////////////////////////////////////////
	mp_sint32		saveExtendedModule(const SYSCHAR* fileName);		// FT2 (.XM)
	mp_sint32		saveProtrackerModule(const SYSCHAR* fileName);   // Protracker compatible (.MOD)

	///////////////////////////////////////////////////
	// module loaded?								 //
	///////////////////////////////////////////////////
	bool			isModuleLoaded() const { return moduleLoaded; }
	
	///////////////////////////////////////////////////
	// string processing							 //
	///////////////////////////////////////////////////
	static void		convertStr(char* strIn, const char* strOut, mp_sint32 nLen, bool filter = true);

	void			getTitle(char* str, bool filter = true) const;
	void			getSignature(char* str, bool filter = true) const;
	void			getTracker(char* str, bool filter = true) const;
	
	///////////////////////////////////////////////////
	// dealing with song message				     //
	///////////////////////////////////////////////////
	
	// allocate empty song message
	void			allocateSongMessage(mp_uint32 initialSize = 512);
	
	// add one more line of text to the song message
	void			addSongMessageLine(const char* line);

	// start iterating text lines (get size of line)
	mp_sint32		getFirstSongMessageLineLength();
	// get next size text line
	mp_sint32		getNextSongMessageLineLength();
	// get line
	void			getSongMessageLine(char* line);
	
	void			buildSubSongTable();
	
	mp_sint32		getNumSubSongs() const { return numSubSongs; }
	
	mp_sint32		getSubSongPosStart(mp_sint32 i) const;
	mp_sint32		getSubSongPosEnd(mp_sint32 i) const;

	///////////////////////////////////////////////////
	// various post processing and analyzing		 //
	///////////////////////////////////////////////////
	// Remove stupid empty = 0xFE orders
	void			removeOrderSkips();
	// Remove unused patterns 
	mp_sint32		removeUnusedPatterns(bool evaluate);

	// these are located in ExporterXM.cpp
	mp_sint32		getNumUsedPatterns();
	mp_sint32		getNumUsedInstruments();

	// Analyse various things, for example if there is an old Protracker instrument change
	void			postLoadAnalyser();

	// MilkyTracker additions
	void			createEmptySong(bool clearPatterns = true, bool clearInstruments = true, mp_sint32 numChannels = 8);

	enum IsPTCompatibleErrorCodes
	{
		IsPTCompatibleErrorCodeNoError = 0,
		IsPTCompatibleErrorCodeTooManyInstruments,
		IsPTCompatibleErrorCodeLinearFrequencyUsed,
		IsPTCompatibleErrorCodeIncompatibleSamples,
		IsPTCompatibleErrorCodeIncompatibleInstruments,
		IsPTCompatibleErrorCodeIncompatiblePatterns
	};

	IsPTCompatibleErrorCodes isPTCompatible();
};	

#endif
