#ifndef __MIDITOOLS_H__
#define __MIDITOOLS_H__

static inline int vol126to255(int vol, int amplify)
{
	int finalVol = (((vol <= 126 ? (vol*133300)>>16 : 255)) * amplify) / 100;

	return finalVol <= 255 ? finalVol : 255;
}

#endif
