// https://github.com/schnitzeltony/soundfont-cmdline-tools
// SPDX: GPL-2
//
// sf2.h  Soundfont type definitions.

#ifndef __SF2_H
#define __SF2_H

#include "iffdigest.h"
#include <stdint.h>
#include <cassert>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

#include "BasicTypes.h"
#include "XModule.h"

class ModuleEditor;                     // forward declartion
	
// Used SF2 'spec' @ http://freepats.zenvoid.org/sf2/sfspec24.pdf

////////////////////////////////////////////////////////////////////////////////
// SF2 data structures

#pragma pack(push, 1) // exact fit - no padding

#define SF_NAME_LEN 20

typedef struct sfPresetHeader // PHDR elements
{
  char achPresetName[SF_NAME_LEN];
  uint16_t wPreset;
  uint16_t wBank;
  uint16_t wPresetBagNdx;
  uint32_t dwLibrary;
  uint32_t dwGenre;
  uint32_t dwMorphology;
  sfPresetHeader();
} sfPresetHeader_t;

typedef struct sfPresetBag  // PBAG elements
{
  uint16_t wGenNdx;
  uint16_t wModNdx;
  sfPresetBag();
} sfPresetBag_t;

// For now we are not interested in all contents so just ensure proper size
#define SFModulator uint16_t
#define SFGenerator uint16_t
#define SFTransform uint16_t
#define genAmountType uint16_t
#define SFSampleLink uint16_t

typedef struct sfModList // PMOD / IMOD elements
{
  SFModulator sfModSrcOper;
  SFGenerator sfModDestOper;
  uint16_t modAmount;
  SFModulator sfModAmtSrcOper;
  SFTransform sfModTransOper;
  sfModList();
} sfModList_t;

typedef struct sfGenList // PGEN / IGEN elements
{
  SFGenerator sfGenOper;
  genAmountType genAmount;
  sfGenList();
} sfGenList_t;

typedef struct sfInst_t // INST elements
{
  char achInstName[SF_NAME_LEN];
  uint16_t wInstBagNdx;
  sfInst_t();
} sfInst_t;

typedef struct sfInstBag // IBAG elements
{
  uint16_t wInstGenNdx;
  uint16_t wInstModNdx;
  sfInstBag();
} sfInstBag_t;

typedef struct sfSample  // SHDR elements
{
  char achSampleName[20];
  uint32_t dwStart;
  uint32_t dwEnd;
  uint32_t dwStartloop;
  uint32_t dwEndloop;
  uint32_t dwSampleRate;
  uint8_t byOriginalPitch;
  char chPitchCorrection;
  uint16_t wSampleLink;
  SFSampleLink sfSampleType;
  sfSample();
} sfSample_t;

#pragma pack(pop) // back to padding

typedef enum // just those we are intersted in
{
  INSTRUMENT = 41,  // for PGEN
  SAMPLEID = 53     // for IGEN
} sfGenEnumLink_t;

////////////////////////////////////////////////////////////////////////////////
 // forwards
class IFFChunk;
class IFFDigest;

////////////////////////////////////////////////////////////////////////////////
// global debug helper
void displayChunkHierarchy(IFFChunk *ck);

// global helper
void sf2NameToStr(std::string &strDest, const char *sf2Name);

////////////////////////////////////////////////////////////////////////////////
class SF2Samples
{
public:
  SF2Samples() :
    ck_sdta(0), ck_smpl(0), ck_sm24(0) {};
  bool Analyse(IFFChunk *_ck_sdta);
  uint32_t GetSampleCount();
  IFFChunk *sdta() { return ck_sdta; }
  IFFChunk *smpl() { return ck_smpl; }
  IFFChunk *sm24() { return ck_sm24; }

protected:
  IFFChunk *ck_sdta;  // root
  IFFChunk *ck_smpl;
  IFFChunk *ck_sm24;
};

////////////////////////////////////////////////////////////////////////////////
// Hyda structs & classes

// Common (presets / instInfoVector)
typedef struct {
  // modIdxs / genIdxs store indexes to valid data: pointers to terminators
  // are ignored
  std::list<uint16_t> modIdxs;
  std::list<uint16_t> genIdxs;
  // generator specific data
  bool instOrSample;
  uint16_t instOrSampleIdx;
} bagInfo_t;

// Presets
typedef struct {
  int phdrIdx;
  int pbagIdxStart;
  // pbagInfoVec:
  //   size: number of pbags
  //   list of idxs: modifiers / generators
  std::vector<bagInfo_t> pbagInfoVec;
} presInfo_t;

// Keep presets sorted
typedef std::map<uint16_t, presInfo_t> presetMap_t;
typedef presetMap_t::iterator presetMapIter_t;
typedef std::map<uint16_t, presetMap_t> bankPresetMap_t;
typedef bankPresetMap_t::iterator bankPresetMapIter_t;

// instInfoVector
typedef struct {
  int ibagIdxStart;
  std::vector<bagInfo_t> ibagInfoVec;
} instInfo_t;

typedef std::vector<instInfo_t> instInfoVector_t;

class SF2Hydra
{
public:
  SF2Hydra() :
    ck_pdta(0), ck_phdr(0), ck_pbag(0), ck_pmod(0), ck_pgen(0),
    ck_inst(0), ck_ibag(0), ck_imod(0), ck_igen(0), ck_shdr(0),
    phdr_data(0), phdr_count(0), pbag_data(0), pbag_count(0),
    pmod_data(0), pmod_count(0), pmod_count_used(0),
    pgen_data(0), pgen_count(0), pgen_count_used(0),
    inst_data(0), inst_count(0), ibag_data(0), ibag_count(0),
    imod_data(0), imod_count(0), imod_count_used(0),
    igen_data(0), igen_count(0), igen_count_used(0),
    shdr_data(0), shdr_count(0) {};
  bool Analyse(IFFChunk *_ck_pdta, SF2Samples *samples);
  // Preset getters
  inline bankPresetMapIter_t bank_begin() { return bank_presets.begin(); }
  inline bankPresetMapIter_t bank_end() { return bank_presets.end(); }
  inline presetMapIter_t preset_begin(const bankPresetMapIter_t &bi) {
    return bi->second.begin(); }
  inline presetMapIter_t preset_end(const bankPresetMapIter_t &bi) {
    return bi->second.end(); }
  inline const sfPresetHeader_t& getPresetHeader(const presetMapIter_t &pi) {
    return phdr_data[pi->second.phdrIdx]; }
  inline const presInfo_t& getPresetInfo(const presetMapIter_t &pi) {
    return pi->second; }
  // Preset modulator (PMOD) getter
  const sfModList_t& getPresetModulator(uint16_t i);
  // Preset generator (PGEN) getter
  const sfGenList_t& getPresetGenerator(uint16_t i);
  // Instrument getter (as array - we need index access)
  inline uint16_t instrument_count() { return inst_count-1; /* no terminal */}
  const sfInst_t& getInstrument(uint16_t i);
  const instInfo_t& getInstrumentInfo(uint16_t i);
  // Instrument modulator (IMOD) getter
  const sfModList_t& getInstrumentModulator(uint16_t i);
  // Preset generator (IGEN) getter
  const sfGenList_t& getInstrumentGenerator(uint16_t i);
  // Sample (SHDR) getter
  const sfSample_t& getSample(uint16_t i);

  uint16_t getMaxSamples(){ return shdr_count-1; }

private:
  bool CheckAllChunks(SF2Samples *samples);
  bool AnalysePhdrPbag();
  bool AnalyseInstIbag();
  // return true if instrument/sample was found (=> global for first zone)
  bool CheckInstrumentOrSample(
    const sfGenList_t *gen_data,
    std::list<uint16_t> &genList,
    const sfGenEnumLink_t instSampleID,
    uint16_t& instSampleIdx);

  // Chunks
  IFFChunk *ck_pdta;  // root
  IFFChunk *ck_phdr;
  IFFChunk *ck_pbag;
  IFFChunk *ck_pmod;
  IFFChunk *ck_pgen;
  IFFChunk *ck_inst;
  IFFChunk *ck_ibag;
  IFFChunk *ck_imod;
  IFFChunk *ck_igen;
  IFFChunk *ck_shdr;

  // data array pinters an len counters (..count: terminal included!)
  // preset header
  sfPresetHeader_t *phdr_data;
  int phdr_count;
  bankPresetMap_t bank_presets;

  // preset zones
  sfPresetBag_t *pbag_data;
  uint16_t pbag_count;

  // preset zone modulators
  sfModList_t *pmod_data;
  uint16_t pmod_count;
  uint16_t pmod_count_used;

  // preset zone generators
  sfGenList_t *pgen_data;
  uint16_t pgen_count;
  uint16_t pgen_count_used;

  // instInfoVector
  sfInst_t *inst_data;
  uint16_t inst_count;
  instInfoVector_t instInfoVector;

  // instrument zones
  sfInstBag_t *ibag_data;
  uint16_t ibag_count;

  // instrument zone modulators
  sfModList_t *imod_data;
  uint16_t imod_count;
  uint16_t imod_count_used;

  // instrument zone generators
  sfGenList_t *igen_data;
  uint16_t igen_count;
  uint16_t igen_count_used;

  // sample headers
  sfSample_t *shdr_data;
  uint16_t shdr_count;
};

////////////////////////////////////////////////////////////////////////////////
class SF2File
{
public:
  SF2File() :
    digest(0), ck_info(0) {};
  bool Analyse(IFFDigest *_digest);
  IFFChunk *getInfo() { return ck_info; }
  SF2Samples &getSamples() { return samples; }
  SF2Hydra &getHydra() { return hydra; }

  // milkytracker funcs
  static bool loadSFSampleToEditor(const SYSCHAR* fileName, pp_uint32 MTinstrIndex, pp_uint32 MTsampleIndex, ModuleEditor *med );
  static bool isLoaded();
  static void reset();

  // static vars (restricts keeping/caching max 1 soundfont in memory)
  static SF2File  font;
  static PPString fontFile;
  static unsigned int maxSamples;
  static FILE* f;
  static unsigned int sampleIndex;
  static char* data;
  static struct stat stbuf;
  static IFFDigest *iff;

protected:
  IFFDigest *digest;

  IFFChunk *ck_info;  // No detailed analysis yet
  SF2Samples samples;
  SF2Hydra hydra;
};

#endif
