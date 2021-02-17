#include "Loaders.h"

const char* LoaderTMM::identifyModule(const mp_ubyte* buffer)
{
	// check for .TMM module first
	if (!memcmp(buffer,"Magic:",6))
	{
		return "TMM";
	}

	// this is not an .TMM
	return NULL;
}

mp_sint32 LoaderTMM::load(XMFileBase& f, XModule* module)
{
	LoaderXM* xm  = new LoaderXM;
	module->type = XModule::ModuleType_TMM;
	mp_sint32 ret = xm->load(f, module);
	delete xm;

	return ret;
}
