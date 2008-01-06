/*
 *  SampleEditorResampler.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 06.01.08.
 *  Copyright 2008 milkytracker.net. All rights reserved.
 *
 */

#ifndef __SAMPLEEDITORRESAMPLER_H__
#define __SAMPLEEDITORRESAMPLER_H__

#include "BasicTypes.h"

class SampleEditorResampler
{
public:
	enum ResamplerTypes
	{
		ResamplerTypeNone,
		ResamplerTypeLinear,
		ResamplerTypeLagrange,
		ResamplerTypeSpline,
		ResamplerTypeFastSinc,
		ResamplerTypePreciseSinc
	};

private:
	class XModule& module;
	struct TXMSample& sample;
	ResamplerTypes type;

public:
	SampleEditorResampler(XModule& module, TXMSample& sample, ResamplerTypes type);
	virtual ~SampleEditorResampler();

	bool resample(float factor);
};

#endif
