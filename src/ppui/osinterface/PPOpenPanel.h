#ifndef __PPOPENPANEL_H__
#define __PPOPENPANEL_H__

#include "PPModalDialog.h"
#include "SimpleVector.h"

class PPOpenPanel : public PPModalDialog
{
protected:
	struct Descriptor
	{
		PPString extension;
		PPString description;

		Descriptor(const PPString& ext, const PPString& desc) :
			extension(ext), description(desc)
		{}
	};

	PPSimpleVector<Descriptor> items;

	PPSystemString fileName;

	char* caption;

public:
	PPOpenPanel(PPScreen* screen, const char* caption);
	virtual ~PPOpenPanel();
	
	// must contain pairs of extensions / description
	// terminated by TWO NULL pointers
	virtual void addExtensions(const char* extensions[])
	{
		for (pp_uint32 i = 0; extensions[i] != NULL; i+=2)
			addExtension(extensions[i], extensions[i+1]);
	}

	virtual void addExtension(const PPString& ext, const PPString& desc);

	virtual const PPSystemString& getFileName() { return fileName; }

	virtual ReturnCodes runModal();

};

#endif
