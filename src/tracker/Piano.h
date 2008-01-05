#ifndef PIANO__H
#define PIANO__H

class PianoBitmapBase
{
protected:
	PianoBitmapBase()
	{	
	}

public:

	virtual const pp_uint8* getBitmap() = 0;
	virtual const pp_uint8* getBitmapLUT() = 0;

	virtual pp_int32 getBitmapWidth() = 0;
	virtual pp_int32 getBitmapHeight() = 0;
	virtual pp_int32 getBitmapLUTWidth() = 0;
	
	virtual const pp_int32* getDIVLUT() = 0;
	virtual const pp_int32* getMODLUT() = 0;
};

class PianoBitmapSmall : public PianoBitmapBase
{
private:
	enum
	{
		width = 448,
		height = 25,
		octaveWidth = 56
	};
	
	PianoBitmapSmall()
	{
		const pp_int32 lutWidth = getBitmapLUTWidth();
		const pp_int32 width = getBitmapWidth();

		for (pp_int32 i = 0; i < width; i++)
		{
			DIVLUT[i] = (i/lutWidth)*12;
			MODLUT[i] = (i%lutWidth)*3;
		}	
	}

	static PianoBitmapSmall* instance;

	static const pp_uint8 PIANO_LUT[];
	static const pp_uint8 PIANO[];

	pp_int32 DIVLUT[width];
	pp_int32 MODLUT[width];

public:
	static PianoBitmapSmall* getInstance()
	{
		if (instance == NULL)
			instance = new PianoBitmapSmall();
		return instance;
	}

	virtual const pp_uint8* getBitmap() { return PIANO; }
	virtual const pp_uint8* getBitmapLUT() { return PIANO_LUT; }

	virtual pp_int32 getBitmapWidth() { return width; }
	virtual pp_int32 getBitmapHeight() { return height; }
	virtual pp_int32 getBitmapLUTWidth() { return octaveWidth; }

	virtual const pp_int32* getDIVLUT() { return DIVLUT; }
	virtual const pp_int32* getMODLUT() { return MODLUT; }
};

#ifndef __LOWRES__
class PianoBitmapLarge : public PianoBitmapBase
{
private:
	enum
	{
		width = 448*2,
		height = 25*2,
		octaveWidth = 56*2
	};
	
	PianoBitmapLarge()
	{
		const pp_int32 lutWidth = getBitmapLUTWidth();
		const pp_int32 width = getBitmapWidth();

		for (pp_int32 i = 0; i < width; i++)
		{
			DIVLUT[i] = (i/lutWidth)*12;
			MODLUT[i] = (i%lutWidth)*3;
		}	
	}

	static PianoBitmapLarge* instance;

	static const pp_uint8 PIANO_LUT[];
	static const pp_uint8 PIANO[];

	pp_int32 DIVLUT[width];
	pp_int32 MODLUT[width];

public:
	static PianoBitmapLarge* getInstance()
	{
		if (instance == NULL)
			instance = new PianoBitmapLarge();
		return instance;
	}

	virtual const pp_uint8* getBitmap() { return PIANO; }
	virtual const pp_uint8* getBitmapLUT() { return PIANO_LUT; }

	virtual pp_int32 getBitmapWidth() { return width; }
	virtual pp_int32 getBitmapHeight() { return height; }
	virtual pp_int32 getBitmapLUTWidth() { return octaveWidth; }

	virtual const pp_int32* getDIVLUT() { return DIVLUT; }
	virtual const pp_int32* getMODLUT() { return MODLUT; }
};
#else
class PianoBitmapLarge : public PianoBitmapBase
{
private:
	PianoBitmapLarge()
	{
	}

	static PianoBitmapLarge* instance;

public:
	static PianoBitmapLarge* getInstance()
	{
		if (instance == NULL)
			instance = new PianoBitmapLarge();
		return instance;
	}

	virtual const pp_uint8* getBitmap() { return NULL; }
	virtual const pp_uint8* getBitmapLUT() { return NULL; }

	virtual pp_int32 getBitmapWidth() { return 0; }
	virtual pp_int32 getBitmapHeight() { return 0; }
	virtual pp_int32 getBitmapLUTWidth() { return 0; }

	virtual const pp_int32* getDIVLUT() { return NULL; }
	virtual const pp_int32* getMODLUT() { return NULL; }
};
#endif

#endif
