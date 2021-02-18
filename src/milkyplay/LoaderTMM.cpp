#include "Loaders.h"

const char* LoaderTMM::identifyModule(const mp_ubyte* buffer)
{
	// 4-chn TMM
	if(buffer[0] == 232)
		return "TM4";

	// Extended TMM
	if(!memcmp(buffer, "Magic:", 6))
		return "TMM";

	// Not magic
	return NULL;
}

mp_sint32 LoaderTMM::load(XMFileBase& f, XModule* module)
{
	mp_sint32 ret;
	mp_ubyte sig;

	sig = f.readByte();
	f.seek(-1, XMFile::SeekOffsetTypeCurrent);

	if(sig == 232) {
		LoaderMOD* mod = new LoaderMOD;
		module->type = XModule::ModuleType_TMM;
		ret = mod->load(f, module);
		delete mod;
	} else {
		LoaderXM* xm  = new LoaderXM;
		module->type = XModule::ModuleType_TMM;
		ret = xm->load(f, module);
		delete xm;
	}

	return ret;
}
