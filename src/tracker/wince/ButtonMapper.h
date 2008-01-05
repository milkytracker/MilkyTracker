#ifndef BUTTONMAPPER__H
#define BUTTONMAPPER__H

struct TButtonMapping
{
	WORD keyModifiers;
	WORD virtualKeyCode;
};

enum EOrientation
{
	eOrientation90CW,
	eOrientation90CCW,
	eOrientationNormal	
};

extern TButtonMapping	mappings[];
extern EOrientation		orientation;
extern pp_int32			allowVirtualKeys;
extern pp_int32			hideTaskBar;
extern pp_int32			doublePixels;
extern pp_int32			dontTurnOffDevice;

void InitButtonRemapper();

#endif