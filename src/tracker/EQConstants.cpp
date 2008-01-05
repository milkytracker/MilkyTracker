#include "EQConstants.h"

const float EQConstants::EQ3bands[3] = 
{
	80.0f,
	2500.0f,
	12000.0f
};

const float EQConstants::EQ3bandwidths[3] = 
{
	2500.0f*0.25f,
	2500.0f*0.5f,
	2500.0f
};

const float EQConstants::EQ10bands[10] = 
{
	31.25f,
	62.5f,
	125.0f,
	250.0f,
	500.0f,
	1000.0f,
	2000.0f,
	4000.0f,
	8000.0f,
	16000.0f
};

const float EQConstants::EQ10bandwidths[10] = 
{
	15.625f*0.25f,
	31.25f*0.25f,
	62.5f*0.25f,
	125.0f*0.25f,
	250.0f*0.25f,
	500.0f*0.25f,
	1000.0f*0.25f,
	2000.0f*0.25f,
	4000.0f*0.25f,
	5000.0f*0.25f
};
