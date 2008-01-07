/*
 *  tracker/FilterParameters.h
 *
 *  Copyright 2008 Peter Barth
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 *  FilterParameters.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 25.11.07.
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
