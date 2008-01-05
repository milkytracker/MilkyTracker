/*
 *  FilterParameters.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 25.11.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
 
#ifndef __FILTERPARAMETERS_H__
#define __FILTERPARAMETERS_H__

class FilterParameters
{
private:
	float* parameters;
	pp_int32 numParameters;
	
public:
	FilterParameters(pp_int32 numParameters) :
		numParameters(numParameters)
	{
		parameters = new float[numParameters];
		for (pp_int32 i = 0; i < numParameters; i++)
			parameters[i] = 0.0f;
	}

	// Construction
	FilterParameters(const FilterParameters& par) 
	{
		numParameters = par.numParameters;
		parameters = new float[numParameters];		
		for (pp_int32 i = 0; i < numParameters; i++)
			parameters[i] = par.parameters[i];
	}
	
	~FilterParameters()
	{
		delete[] parameters;
	}
		
	void setParameter(pp_int32 index, float par)
	{
		if (index >= 0 && index < numParameters)
			parameters[index] = par;
	}

	float getParameter(pp_int32 index) const
	{
		if (index >= 0 && index < numParameters)
			return parameters[index];
		
		return 0.0f;
	}
	
	pp_int32 getNumParameters() const { return numParameters; }
};

#endif
