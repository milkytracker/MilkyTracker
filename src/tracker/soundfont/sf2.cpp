// https://github.com/schnitzeltony/soundfont-cmdline-tools
// SPDX: GPL-2
//
// check the gitrepo for more info: https://github.com/schnitzeltony/soundfont-cmdline-tools
//
#include "iffdigest.h"
#include "sf2.h"
#include <iostream>
#include <string.h>

#include "ModuleEditor.h"
  
////////////////////////////////////////////////////////////////////////////////
// SF2 data constuctors
sfPresetHeader::sfPresetHeader()
{
  memset(this, 0, sizeof(sfPresetHeader));
}

sfPresetBag::sfPresetBag()
{
  memset(this, 0, sizeof(sfPresetBag));
}

sfModList::sfModList()
{
  memset(this, 0, sizeof(sfModList));
}

sfGenList::sfGenList()
{
  memset(this, 0, sizeof(sfGenList));
}

sfInst_t::sfInst_t()
{
  memset(this, 0, sizeof(sfInst_t));
}

sfInstBag::sfInstBag()
{
  memset(this, 0, sizeof(sfInstBag));
}

sfSample::sfSample()
{
  memset(this, 0, sizeof(sfSample));
}

////////////////////////////////////////////////////////////////////////////////
// Common helpers

void CheckForChunk( IFFChunk *ckParent, const char* id,
                    IFFChunk **ckFound, unsigned int elem_len, bool bMandatory,
                    bool &bOK, int level)
{
  IFFChunkIterator iter = ckParent->ck_find(iff_ckid(id));
  // indent
  for(int i=0; i<level; i++)
    if( getenv("SOUNDFONT_DEBUG") ) std::cout <<"  ";
  // set found chunk / msg
  if(iter != ckParent->ck_end()) {
    if(elem_len == 0) {
      if( getenv("SOUNDFONT_DEBUG") ) std::cout <<id<<" chunk found (size "<<(*iter).len()<<")\n";
    }
    else {
      if((*iter).len()%elem_len == 0) {
        if( getenv("SOUNDFONT_DEBUG") ) std::cout <<id<<" chunk found with "<<(*iter).len()/elem_len<< " elements\n";
      }
      else {
        std::cerr<<id<<" chunk not found but size "<<(*iter).len()<<
        "is not multiple of "<<elem_len<<"\n";
        bOK = false;
      }
    }
    *ckFound = &(*iter);
  }
  else {
    if(bMandatory) {
      std::cerr<<id<<" chunk not found!\n";
      bOK = false;
    }
    else {
      if( getenv("SOUNDFONT_DEBUG") ) std::cout <<id<<" chunk not found (optional).\n";
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Debug helper

static int nestedDepth = 0;

void displayChunkHierarchy(IFFChunk *ck)
{
  for (IFFChunkIterator iter=ck->ck_begin(); iter!=ck->ck_end(); ++iter) {
    char text[sizeof(iff_ckid_t)+1];
    memcpy(text, (*iter).id_str(), sizeof(iff_ckid_t));
    text[sizeof(iff_ckid_t)] = 0;
    for(int i=0; i<nestedDepth; i++) {
      std::cout<<"  ";
    }
    std::cout<<"ID:"<<text<<" Len:"<<(*iter).len()<<"\n";
    if((*iter).ck_begin() != (*iter).ck_end()) {
      nestedDepth++;
      displayChunkHierarchy(&(*iter));
      nestedDepth--;
    }
  }
}

void sf2NameToStr(std::string &strDest, const char *sf2Name)
{
  char _str[SF_NAME_LEN+1];
  memcpy(_str, sf2Name, SF_NAME_LEN);
  _str[SF_NAME_LEN] = 0;
  strDest = _str;
}


////////////////////////////////////////////////////////////////////////////////
// SF2Samples members

bool SF2Samples::Analyse(IFFChunk *_ck_sdta)
{
  bool bOK = true;
  ck_sdta = _ck_sdta;

  // smpl: 16Bit samples (We deny ROM samples -> smpl is mandatory)
  CheckForChunk(_ck_sdta, "smpl", &ck_smpl, 0, true, bOK, 1);
  // sm24: extent to 24Bit samples LSB
  CheckForChunk(_ck_sdta, "sm24", &ck_sm24, 0, false, bOK, 1);

  if(ck_sm24 && ck_smpl->len()/2 != ck_sm24->len()) {
    std::cout<<"Warning: Size of sm24 ("<<
      ck_sm24->len()<<") is not exactly half sizeof smpl ("<<ck_smpl->len()<<
      ")!";
      ck_sm24 = 0;
  }
  return bOK;
}

uint32_t SF2Samples::GetSampleCount()
{
  uint32_t ui32SampleCount = 0;
  if(ck_smpl) {
    ui32SampleCount = ck_smpl->len()/2;
  }
  return ui32SampleCount;
}

////////////////////////////////////////////////////////////////////////////////
// SF2Hydra members

// Check and keep pointers for all chunks
bool SF2Hydra::CheckAllChunks(SF2Samples *samples)
{
  bool bOK = true;
  // phdr: Preset header
  CheckForChunk(ck_pdta, "phdr", &ck_phdr, sizeof(sfPresetHeader_t), true, bOK, 1);
  // pbag: Preset zones
  CheckForChunk(ck_pdta, "pbag", &ck_pbag, sizeof(sfPresetBag_t), true, bOK, 1);
  // pmod: Preset zone modulators
  CheckForChunk(ck_pdta, "pmod", &ck_pmod, sizeof(sfModList_t), true, bOK, 1);
  // pgen: Preset zone generators
  CheckForChunk(ck_pdta, "pgen", &ck_pgen, sizeof(sfGenList_t), true, bOK, 1);
  // inst: instInfoVector
  CheckForChunk(ck_pdta, "inst", &ck_inst, sizeof(sfInst_t), true, bOK, 1);
  // ibag: Instrument zones
  CheckForChunk(ck_pdta, "ibag", &ck_ibag, sizeof(sfInstBag_t), true, bOK, 1);
  // imod: Instrument zone modulators
  CheckForChunk(ck_pdta, "imod", &ck_imod, sizeof(sfModList_t), true, bOK, 1);
  // igen: zone generators
  CheckForChunk(ck_pdta, "igen", &ck_igen, sizeof(sfGenList_t), true, bOK, 1);
  // shdr: Sample headers
  CheckForChunk(ck_pdta, "shdr", &ck_shdr, sizeof(sfSample_t), true, bOK, 1);

  if(bOK) {
    // Keep pointers / counters
    phdr_data = (sfPresetHeader_t*)ck_phdr->dataPtr();
    phdr_count = ck_phdr->len() / sizeof(sfPresetHeader_t);
    pbag_data = (sfPresetBag_t*)ck_pbag->dataPtr();
    pbag_count = ck_pbag->len() / sizeof(sfPresetBag_t);
    pmod_data = (sfModList_t*)ck_pmod->dataPtr();
    pmod_count = ck_pmod->len() / sizeof(sfModList_t);
    pgen_data = (sfGenList_t*)ck_pgen->dataPtr();
    pgen_count = ck_pgen->len() / sizeof(sfGenList_t);
    inst_data = (sfInst_t*)ck_inst->dataPtr();
    inst_count = ck_inst->len() / sizeof(sfInst_t);
    ibag_data = (sfInstBag_t*)ck_ibag->dataPtr();
    ibag_count = ck_ibag->len() / sizeof(sfInstBag_t);
    imod_data = (sfModList_t*)ck_imod->dataPtr();
    imod_count = ck_imod->len() / sizeof(sfModList_t);
    igen_data = (sfGenList_t*)ck_igen->dataPtr();
    igen_count = ck_igen->len() / sizeof(sfGenList_t);
    shdr_data = (sfSample_t*)ck_shdr->dataPtr();
    shdr_count = ck_shdr->len() / sizeof(sfSample_t);

    // At least one preset + terminal required
    if(phdr_count < 2) {
      std::cerr<<"Not enough presets in PHDR!\n";
      bOK = false;
    }
    // At least one instrument + terminal required
    if(inst_count < 2) {
      std::cerr<<"Not enough instInfoVector in INST!\n";
      bOK = false;
    }

    // Check for correct sizes terminal entries / pointer limoits / monotony
    if(bOK) {
      // phdr->pbag
      uint16_t terminal_expected = phdr_data[phdr_count-1].wPresetBagNdx;
      if(terminal_expected != pbag_count-1) {
        std::cout<<"PHDR terminal: wrong count for preset zones (pbag): Expected: "
          << pbag_count-1 << " Found: " << terminal_expected << "!\n";
        bOK = false;
      }
      else {
        int pbagIdxOld = -1;
        for(int i=0; i<phdr_count; i++) {
          if(phdr_data[i].wPresetBagNdx >= pbag_count) {
            std::cerr<<"PHDR["<<i<<"].wPresetBagNdx is out of limits!\n";
            bOK = false;
            break;
          }
          if(phdr_data[i].wPresetBagNdx <= pbagIdxOld) { // strictly monoton
            std::cerr<<"PHDR["<<i<<"].wPresetBagNdx is not monotonic!\n";
            bOK = false;
            break;
          }
          pbagIdxOld = phdr_data[i].wPresetBagNdx;
        }
      }

      // pbag->pmod
      terminal_expected = pbag_data[pbag_count-1].wModNdx;
      if(terminal_expected != pmod_count-1) {
        std::cout<<"PBAG terminal: wrong count for preset zone modulators (pmod): Expected: "
          << pmod_count-1 << " Found: " << terminal_expected << "!\n";
        bOK = false;
      }
      else {
        int pmodIdxOld = -1;
        for(int i=0; i<pbag_count; i++) {
          if(pbag_data[i].wModNdx >= pmod_count) {
            std::cerr<<"PBAG["<<i<<"].wModNdx is out of limits!\n";
            bOK = false;
            break;
          }
          if(pbag_data[i].wModNdx < pmodIdxOld) {
            std::cerr<<"PBAG["<<i<<"].wModNdx is not monotonic!\n";
            bOK = false;
            break;
          }
          pmodIdxOld = pbag_data[i].wModNdx;
        }
      }

      // pbag->pgen
      terminal_expected = pbag_data[pbag_count-1].wGenNdx;
      if(terminal_expected != pgen_count-1) {
        std::cout<<"PBAG terminal: wrong count for preset zone generators (pgen): Expected: "
          << pgen_count-1 << " Found: " << terminal_expected << "!\n";
        bOK = false;
      }
      else {
        int pgenIdxOld = -1;
        for(int i=0; i<pbag_count; i++) {
          if(pbag_data[i].wGenNdx >= pgen_count) {
            std::cerr<<"PBAG["<<i<<"].wGenNdx is out of limits!\n";
            bOK = false;
            break;
          }
          if(pbag_data[i].wGenNdx < pgenIdxOld) {
            std::cerr<<"PBAG["<<i<<"].wGenNdx is not monotonic!\n";
            bOK = false;
            break;
          }
          pgenIdxOld = pbag_data[i].wGenNdx;
        }
      }

      // inst->ibag
      terminal_expected = inst_data[inst_count-1].wInstBagNdx;
      if(terminal_expected != ibag_count-1) {
        std::cout<<"INST terminal: wrong count for instrument zones (ibag): Expected: "
          << ibag_count-1 << " Found: " << terminal_expected << "!\n";
        bOK = false;
      }
      else {
        int ibagIdxOld = -1;
        for(int i=0; i<inst_count; i++) {
          if(inst_data[i].wInstBagNdx >= ibag_count) {
            std::cerr<<"INST["<<i<<"].wInstBagNdx is out of limits!\n";
            bOK = false;
            break;
          }
          if(inst_data[i].wInstBagNdx <= ibagIdxOld) { // strictly monoton
            std::cerr<<"INST["<<i<<"].wInstBagNdx is not monotonic!\n";
            bOK = false;
            break;
          }
          ibagIdxOld = inst_data[i].wInstBagNdx;
        }
      }

      // ibag->imod
      terminal_expected = ibag_data[ibag_count-1].wInstModNdx;
      if(terminal_expected != imod_count-1) {
        std::cout<<"IBAG terminal: wrong count for instrument zone modulators (imod): Expected: "
          << imod_count-1 << " Found: " << terminal_expected << "!\n";
        bOK = false;
      }
      else {
        int imodIdxOld = -1;
        for(int i=0; i<ibag_count; i++) {
          if(ibag_data[i].wInstModNdx >= imod_count) {
            std::cerr<<"IBAG["<<i<<"].wInstModNdx is out of limits!\n";
            bOK = false;
            break;
          }
          if(ibag_data[i].wInstModNdx < imodIdxOld) {
            std::cerr<<"IBAG["<<i<<"].wInstModNdx is not monotonic!\n";
            bOK = false;
            break;
          }
          imodIdxOld = ibag_data[i].wInstModNdx;
        }
      }

      // ibag->igen
      terminal_expected = ibag_data[ibag_count-1].wInstGenNdx;
      if(terminal_expected != igen_count-1) {
        std::cout<<"IBAG terminal: wrong count for instrument zone generators (igen): Expected: "
          << igen_count-1 << " Found: " << terminal_expected << "!\n";
        bOK = false;
      }
      else {
        int igenIdxOld = -1;
        for(int i=0; i<ibag_count; i++) {
          if(ibag_data[i].wInstGenNdx >= igen_count) {
            std::cerr<<"IBAG["<<i<<"].wInstGenNdx is out of limits!\n";
            bOK = false;
            break;
          }
          if(ibag_data[i].wInstGenNdx < igenIdxOld) {
            std::cerr<<"IBAG["<<i<<"].wInstGenNdx is not monotonic!\n";
            bOK = false;
            break;
          }
          igenIdxOld = ibag_data[i].wInstGenNdx;
        }
      }

      // shdr -> sdta (smpl/sm24)
      for(uint16_t shdr=0; shdr<shdr_count; ++shdr) {
        uint32_t ui32SampleCount = samples->GetSampleCount();
        // Check buffer overflow only (ignore 46 trailing zeros)
        if(shdr_data[shdr].dwStart >= ui32SampleCount) {
          std::cerr<<"SHDR["<<shdr<<"].dwStart is out of limits!\n";
          bOK = false;
          break;
        }
        if(shdr_data[shdr].dwEnd >= ui32SampleCount) {
          std::cerr<<"SHDR["<<shdr<<"].dwEnd is out of limits!\n";
          bOK = false;
          break;
        }
        if(shdr_data[shdr].dwStartloop >= ui32SampleCount) {
          std::cerr<<"SHDR["<<shdr<<"].dwStartloop is out of limits!\n";
          bOK = false;
          break;
        }
        if(shdr_data[shdr].dwEndloop >= ui32SampleCount) {
          std::cerr<<"SHDR["<<shdr<<"].dwEndloop is out of limits!\n";
          bOK = false;
          break;
        }
        // Check for ROM samples
        if(shdr_data[shdr].sfSampleType & (1<<15)) {
          std::cerr<<"SHDR["<<shdr<<"].sfSampleType references ROM samples [not supported yet]\n";
          break;
        }
        // All samples except mono types are linked
        if(shdr_data[shdr].sfSampleType > 1) {
          // Check linked samples within limits (no terminal)
          if(shdr_data[shdr].wSampleLink >= shdr_count-1) {
            std::cerr<<"SHDR["<<shdr<<"].wSampleLink references SHDR enty out of limit!\n";
            bOK = false;
            break;
          }
        }
      }
    }
  }
  return bOK;
}

// Check, order and keep indexes for preset headers and their zones
bool SF2Hydra::AnalysePhdrPbag()
{
  bool bOK = true;
  presInfo_t *presInfoPrev = 0;
  // Put preset headers in order / keep pbag info per preset
  for(int phdrIdx=0; bOK && phdrIdx<phdr_count; phdrIdx++) {
    presInfo_t *presInfoCurr = 0;
    uint16_t pbagIdxCurr = phdr_data[phdrIdx].wPresetBagNdx;
    // Do not keep terminal
    if(phdrIdx < phdr_count-1) {
      // Store in nested maps to sort
      presInfoCurr =
        &bank_presets[phdr_data[phdrIdx].wBank][phdr_data[phdrIdx].wPreset];
      presInfoCurr->phdrIdx = phdrIdx;
      presInfoCurr->pbagIdxStart = pbagIdxCurr;
    }
    // For previous preset:
    // * Set preset zone (pbag) count
    // * collect preset zone modulators (pmod) indexes
    if(presInfoPrev) {
      uint16_t pbagIdxFirstPrev = phdr_data[phdrIdx-1].wPresetBagNdx;
      uint16_t pbagIdxFirstCurr = phdr_data[phdrIdx].wPresetBagNdx;
      presInfoPrev->pbagInfoVec.resize(pbagIdxFirstCurr-pbagIdxFirstPrev);
      // Now we know preset zone count for previous preset: Loop
      for(int pbagIdx=pbagIdxFirstPrev; pbagIdx<pbagIdxFirstCurr; pbagIdx++) {
        uint16_t pbagIdxVec = pbagIdx-pbagIdxFirstPrev;

        // Collect all preset zone modulators
        for(uint16_t pmodIdxPrev = pbag_data[pbagIdx].wModNdx;
            pmodIdxPrev<pbag_data[pbagIdx+1].wModNdx; pmodIdxPrev++) {
          // Do not store pointers to terminal
          if(pmodIdxPrev != pmod_count-1) {
            presInfoPrev->pbagInfoVec[pbagIdxVec].modIdxs.push_back(pmodIdxPrev);
            pmod_count_used++;
          }
        }
        // no instrument is default
        presInfoPrev->pbagInfoVec[pbagIdxVec].instOrSample = false;
        // Collect preset zone generators
        for(uint16_t pgenIdxPrev = pbag_data[pbagIdx].wGenNdx;
            pgenIdxPrev<pbag_data[pbagIdx+1].wGenNdx; pgenIdxPrev++) {
          // Do not store pointers to terminal
          if(pgenIdxPrev != pgen_count-1) {
            presInfoPrev->pbagInfoVec[pbagIdxVec].genIdxs.push_back(pgenIdxPrev);
            pgen_count_used++;
            // An instrument?
            if(pgen_data[pgenIdxPrev].sfGenOper == (SFGenerator)INSTRUMENT) {
              // Within instrument limits (no terminal)?
              if(pgen_data[pgenIdxPrev].genAmount <
                (genAmountType)(inst_count-1)) {
                // Note: We do not check for multiple globals / instInfoVector here
                presInfoPrev->pbagInfoVec[pbagIdxVec].instOrSample = true;
                presInfoPrev->pbagInfoVec[pbagIdxVec].instOrSampleIdx =
                  pgen_data[pgenIdxPrev].genAmount;
              }
              // Out of instrument limits
              else {
                std::cerr<<"PGEN["<<pgenIdxPrev<<
                  "].wInstGenNdx is out of limits: "<<
                  pgen_data[pgenIdxPrev].genAmount<<"!\n";
                bOK = false;
                break;
              }
            }
          }
        }
      }
    }
    presInfoPrev = presInfoCurr;
  }
  if( getenv("SOUNDFONT_DEBUG") ) std::cout<<"PMOD: used: "<<pmod_count_used<<" available: "<<pmod_count-1<<"\n";
  if( getenv("SOUNDFONT_DEBUG") ) std::cout<<"PGEN: used: "<<pgen_count_used<<" available: "<<pgen_count-1<<"\n";
  return bOK;
}

// Check and keep instInfoVector and their zones
bool SF2Hydra::AnalyseInstIbag()
{
  bool bOK = true;
  instInfoVector.resize(inst_count-1);
  instInfo_t *instInfoPrev = 0;
  // Put preset headers in order / keep ibag info per preset
  for(int instIdx=0; instIdx<inst_count; instIdx++) {
    instInfo_t *instInfoCurr = 0;
    // Do not keep terminal
    if(instIdx < inst_count-1) {
      // Store in array
      instInfoCurr = &instInfoVector[instIdx];
      instInfoCurr->ibagIdxStart = inst_data[instIdx].wInstBagNdx;
    }
    // For previous instrument:
    // * Set preset zone (ibag) count
    // * collect preset zone modulators (pmod) indexes
    if(instInfoPrev) {
      uint16_t ibagIdxFirstPrev = inst_data[instIdx-1].wInstBagNdx;
      uint16_t ibagIdxFirstCurr = inst_data[instIdx].wInstBagNdx;
      instInfoPrev->ibagInfoVec.resize(ibagIdxFirstCurr-ibagIdxFirstPrev);
      // Now we know instrument zone count for previous preset: Loop
      for(int ibagIdx=ibagIdxFirstPrev; ibagIdx<ibagIdxFirstCurr; ibagIdx++) {
        uint16_t ibagIdxVec = ibagIdx-ibagIdxFirstPrev;
        // Collect all instrument zone modulators
        for(uint16_t imodIdxPrev = ibag_data[ibagIdx].wInstModNdx;
            imodIdxPrev<ibag_data[ibagIdx+1].wInstModNdx;
            imodIdxPrev++) {
          // Do not store pointers to terminal
          if(imodIdxPrev != imod_count-1) {
            instInfoPrev->ibagInfoVec[ibagIdxVec].modIdxs.push_back(imodIdxPrev);
            imod_count_used++;
          }
        }
        // no sample is default
        instInfoPrev->ibagInfoVec[ibagIdxVec].instOrSample = false;
        // Collect instrument zone generators
        for(uint16_t igenIdxPrev = ibag_data[ibagIdx].wInstGenNdx;
            igenIdxPrev<ibag_data[ibagIdx+1].wInstGenNdx; igenIdxPrev++) {
          // Do not store pointers to terminal
          if(igenIdxPrev != igen_count-1) {
            instInfoPrev->ibagInfoVec[ibagIdxVec].genIdxs.push_back(igenIdxPrev);
            igen_count_used++;
            // A sample?
            if(igen_data[igenIdxPrev].sfGenOper == (SFGenerator)SAMPLEID) {
              // Within sample limits (no terminal)?
              if(igen_data[igenIdxPrev].genAmount <
                (genAmountType)(shdr_count-1)) {
                // Note: We do not check for multiple globals / instInfoVector here
                instInfoPrev->ibagInfoVec[ibagIdxVec].instOrSample = true;
                instInfoPrev->ibagInfoVec[ibagIdxVec].instOrSampleIdx =
                  igen_data[igenIdxPrev].genAmount;
              }
              // Out of sample limits
              else {
                std::cerr<<"IGEN["<<igenIdxPrev<<
                  "].wInstGenNdx is out of limits: "
                  <<igen_data[igenIdxPrev].genAmount<<"!\n";
                bOK = false;
                break;
              }
            }
          }
        }
      }
    }
    instInfoPrev = instInfoCurr;
  }
  if( getenv("SOUNDFONT_DEBUG") ) std::cout<<"IMOD: used: "<<imod_count_used<<" available: "<<imod_count-1<<"\n";
  if( getenv("SOUNDFONT_DEBUG") ) std::cout<<"IGEN: used: "<<igen_count_used<<" available: "<<igen_count-1<<"\n";
  return bOK;
}

bool SF2Hydra::CheckInstrumentOrSample(
    const sfGenList_t *gen_data,
    std::list<uint16_t> &genList,
    const sfGenEnumLink_t instSampleID,
    uint16_t& instSampleIdx)
{
  // No checks if instrument is last in list - we don't care
  bool bFound = false;
  for(std::list<uint16_t>::iterator iter=genList.begin();
      iter!=genList.end(); ++iter) {
    if(gen_data[*iter].sfGenOper == (SFGenerator)instSampleID) {
      bFound = true;
      instSampleIdx = *iter;
    }
  }
  // Not found -> global
  return !bFound;
}


// Main worker
bool SF2Hydra::Analyse(IFFChunk *_ck_pdta, SF2Samples *samples)
{
  ck_pdta = _ck_pdta;

  bool bOK = true;
  bOK = bOK && CheckAllChunks(samples);
  bOK = bOK && AnalysePhdrPbag();
  bOK = bOK && AnalyseInstIbag();

  return bOK;
}

const sfInst_t& SF2Hydra::getInstrument(uint16_t i)
{
  assert(i<inst_count-1);
  return inst_data[i];
}
const instInfo_t& SF2Hydra::getInstrumentInfo(uint16_t i)
{
  assert(i<inst_count-1);
  return instInfoVector[i];
}

const sfModList_t& SF2Hydra::getPresetModulator(uint16_t i)
{
  assert(i<pmod_count-1);
  return pmod_data[i];
}

const sfGenList_t& SF2Hydra::getPresetGenerator(uint16_t i)
{
  assert(i<pgen_count-1);
  return pgen_data[i];
}

const sfModList_t& SF2Hydra::getInstrumentModulator(uint16_t i)
{
  assert(i<imod_count-1);
  return imod_data[i];
}

const sfGenList_t& SF2Hydra::getInstrumentGenerator(uint16_t i)
{
  assert(i<igen_count-1);
  return igen_data[i];
}

const sfSample_t& SF2Hydra::getSample(uint16_t i)
{
  assert(i<shdr_count-1);
  return shdr_data[i];
}

////////////////////////////////////////////////////////////////////////////////
// SF2File members

// Main worker
bool SF2File::Analyse(IFFDigest *_digest)
{
  bool bOK = true;

  // TODO: Make IFFChunkIterator a IFFChunk??

  // Is it a soundfont?
  if(_digest->id() != iff_ckid("sfbk")) {
    std::cerr<<"Not a soundfont!\n";
    bOK = false;
  }

  // Check mandatory tompost chunks
  // Info
  IFFChunkIterator iter = _digest->ck_find(iff_ckid("INFO"));
  if(iter != _digest->ck_end()) {
    if( getenv("SOUNDFONT_DEBUG") ) std::cout <<"INFO chunk found\n";
    ck_info = &(*iter);
    // ROM samples are not supported
    if(iter->ck_find(iff_ckid("irom")) != iter->ck_end()) {
      if( getenv("SOUNDFONT_DEBUG") ) std::cout <<"INFO chunk contains irom chunk. ROM sample files are not supported (yet)\n";
      bOK = false;
    }
  }
  else {
    std::cerr<<"INFO chunk not found!\n";
    bOK = false;
  }

  // sample data
  iter = _digest->ck_find(iff_ckid("sdta"));
  if(iter != _digest->ck_end()) {
    if( getenv("SOUNDFONT_DEBUG") ) std::cout <<"sdta chunk found\n";
    if(!samples.Analyse(&(*iter))) {
      bOK = false;
    }
  }
  else {
    std::cerr<<"sdta chunk not found!\n";
    bOK = false;
  }

  // hydra data
  iter = _digest->ck_find(iff_ckid("pdta"));
  if(iter != _digest->ck_end()) {
    if( getenv("SOUNDFONT_DEBUG") ) std::cout <<"pdta chunk found\n";
    if(!hydra.Analyse(&(*iter), &samples)) {
      bOK = false;
    }
  }
  else {
    std::cerr<<"pdta chunk not found!\n";
    bOK = false;
  }

  return bOK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Milkytracker import-code which caches the last loaded SF in memory (hence the static vars)

SF2File SF2File::font;
PPString SF2File::fontFile = PPString();
unsigned int SF2File::sampleIndex;
char* SF2File::data = NULL;
struct stat SF2File::stbuf;
FILE* SF2File::f;
unsigned int SF2File::maxSamples = 0;
IFFDigest* SF2File::iff;

bool SF2File::loadSFSampleToEditor(const SYSCHAR* fileName, pp_uint32 MTinstrIndex, pp_uint32 MTsampleIndex, ModuleEditor *med )
{
	XModule *module = med->getModule();
	ModuleEditor::TEditorInstrument &instrument = *med->getInstrumentInfo(MTinstrIndex);
	// a milkyplay SampleLoader-class for SF2 does not make sense 
	// (it requires caching to choose samples within, and is tracker-specific, so it 
	//  does not belong in the milkyplay-library).
	std::string strName;
	bool alreadyLoaded = SF2File::fontFile.compareTo(fileName) == 0;
	if( !alreadyLoaded ){
	    maxSamples = 0;
		munmap(SF2File::data,SF2File::stbuf.st_size);
		SF2File::fontFile.replace(fileName);
		// Open soundfont
		f = fopen(fileName,"rb");
		if(!f){
			printf("soundfont: could not open %s\n",fileName);
			reset();
			return false;
		}
		int fd = fileno(f);
		// Parse soundfont
		fstat(fd, &SF2File::stbuf);
		// *TODO* maybe we need a available memorycheck for lowspec devices here
		SF2File::data = (char*)mmap(0, SF2File::stbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
		iff = new IFFDigest( (const char*)SF2File::data, SF2File::stbuf.st_size);
	}


	if( !SF2File::font.Analyse(iff) ){
		printf("soundfont: unsupported sf2 [try re-exporting in polyphone]\n");
		reset();
		return false;
	}

	SF2Hydra& hydra = SF2File::font.getHydra();
	maxSamples = hydra.getMaxSamples();
	//const sfInst_t& sfi = hydra.getInstrument(SF2inst);
	//const instInfo_t& ii = hydra.getInstrumentInfo(SF2inst);
	//sf2NameToStr(strName, sfi.achInstName);

	// now we import the sample
	TXMSample* dst = &module->smp[instrument.usedSamples[MTsampleIndex]];
	int relnote    = dst->relnote;
	const sfSample_t& sfsmp = hydra.getSample(sampleIndex % maxSamples );
	sf2NameToStr(strName, sfsmp.achSampleName);

	// set name
	memset(dst->name, 0, sizeof(dst->name));
	if (strlen(strName.c_str()) <= sizeof(dst->name))
	{
		memcpy(dst->name, strName.c_str(), strlen(strName.c_str()));
	} else memcpy(dst->name, strName.c_str(), sizeof(dst->name));
	
	// default values first	
	dst->flags = 3;
	dst->venvnum = instrument.volumeEnvelope+1;
	dst->penvnum = instrument.panningEnvelope+1;
	dst->fenvnum = dst->vibenvnum = 0;
	
	dst->vibtype = instrument.vibtype;
	dst->vibsweep = instrument.vibsweep;
	dst->vibdepth = instrument.vibdepth << 1;
	dst->vibrate = instrument.vibrate;
	dst->volfade = instrument.volfade << 1;
	dst->relnote = relnote == 0 ? 12 : relnote; // sfsmp.byOriginalPitch-60;
	dst->loopstart = sfsmp.dwStartloop-sfsmp.dwStart;
	dst->looplen  = sfsmp.dwEndloop - sfsmp.dwStartloop;
	dst->type = 16; // 16=16bits 0=8bit 
				
	// loop flag is defined per-instrument, so we do some poor man's guessing here for now
	// based on heuristics from the spec: 
	//   * dwStart must be less than dwStartloop-7, 
	//   * dwStartloop must be less than dwEndloop-31
	//   & dwEndloop must be less than dwEnd-7
	bool isLoop = sfsmp.dwStart < sfsmp.dwStartloop-7 && sfsmp.dwStartloop < sfsmp.dwEndloop-31 && sfsmp.dwEndloop < sfsmp.dwEnd;
	if( isLoop ){ // enable forward loop
		dst->type &= ~(3+32);
		dst->type |= 1;
	}
	
	ASSERT(dst);

	SF2Samples &sampleData = SF2File::font.getSamples(); // raw sampledata
	if (dst->sample)
	{
		module->freeSampleMem((mp_ubyte*)dst->sample);
		dst->sample = NULL;
	}					
	dst->samplen = (sfsmp.dwEnd - sfsmp.dwStart);
	dst->sample = (mp_sbyte*)module->allocSampleMem(dst->samplen*2);						

	int16_t* dataSmpl = 0;
	dataSmpl = new int16_t[dst->samplen];
	memset(dataSmpl, 0, dst->samplen*sizeof(int16_t));

	int bitDepth = 16;
	if (sfsmp.sfSampleType & 0x08 ) bitDepth = 24;  // SF2.4 introduced 24-bit samples
    if (sfsmp.sfSampleType & 0x10 ) bitDepth = 32;  // Rare case

	switch (bitDepth){
		case 16: // 16-bit signed integer
			{
				memcpy(dataSmpl,
						sampleData.smpl()->dataPtr() + sfsmp.dwStart*sizeof(int16_t),
						(sfsmp.dwEnd - sfsmp.dwStart)*sizeof(int16_t));
			}
			break;
		case 24: // 24-bit signed integer
			{
				int32_t* data24 = (int32_t*)sampleData.smpl()->dataPtr();
				for (int i = 0; i < dst->samplen; i++) {
					int32_t sample24 = data24[sfsmp.dwStart + i];
					dataSmpl[i] = (int16_t)((sample24 >> 8) & 0xFFFF); // Convert 24-bit to 16-bit
				}
			}
			break;
		case 32: // 32-bit floating-point or signed integer
			{
				if ((sfsmp.sfSampleType & 0x7F00) >> 8 == 4) {
					float* dataFloat = (float*)sampleData.smpl()->dataPtr();
					for (int i = 0; i < dst->samplen; i++) {
						float sampleFloat = dataFloat[sfsmp.dwStart + i];
						dataSmpl[i] = (int16_t)(sampleFloat * 32767.0f); // Convert float to 16-bit
					}
				} else {
					int32_t* data32 = (int32_t*)sampleData.smpl()->dataPtr();
					for (int i = 0; i < dst->samplen; i++) {
						int32_t sample32 = data32[sfsmp.dwStart + i];
						dataSmpl[i] = (int16_t)(sample32 >> 16); // Convert 32-bit to 16-bit
					}
				}
			}
			break;
		default:
			printf("Unsupported sample type: %d\n", sfsmp.sfSampleType);
			reset();
			return false;
			break;
	}

	for (int i = 0; i < dst->samplen; i++) {
		((mp_sbyte*)dst->sample)[i*2] = dataSmpl[i] & 0xFF;
		((mp_sbyte*)dst->sample)[i*2+1] = (dataSmpl[i] >> 8) & 0xFF;
	}

	delete[] dataSmpl; // Don't forget to free the memory	

	if( !alreadyLoaded ){
		reset();
	}
	
	return true;
}

bool SF2File::isLoaded(){
	return fontFile.length() > 0; // *TODO* probably a better way of doing this
}

void SF2File::reset(){
  if( f != NULL ){
	  fclose(f);
	  f = NULL;
  }
}

