/*
 *  tracker/FilterParameters.h
 *
 *  Copyright 2009 Peter Barth
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
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.11.07.
 *
 */
 
#ifndef __FILTERPARAMETERS_H__
#define __FILTERPARAMETERS_H__

class FilterParameters
{
public:
	union Parameter
	{
		float floatPart;
		pp_int32 intPart;
		
		Parameter()
		{
			memset(this, 0, sizeof(Parameter));
		}

		Parameter(const Parameter& src)		
		{
			memcpy(this, &src, sizeof(Parameter));
		}

		explicit Parameter(float value) :
			floatPart(value)
		{
		}

		explicit Parameter(pp_int32 value) :
			intPart(value)
		{
		}
	};

private:
	Parameter* parameters;
	pp_int32 numParameters;
	
public:
	FilterParameters(pp_int32 numParameters) :
		numParameters(numParameters),
		parameters(new Parameter[numParameters])
	{
	}

	// Construction
	FilterParameters(const FilterParameters& par) 
	{
		numParameters = par.numParameters;
		parameters = new Parameter[numParameters];		
		for (pp_int32 i = 0; i < numParameters; i++)
			parameters[i] = par.parameters[i];
	}
	
	~FilterParameters()
	{
		delete[] parameters;
	}
		
	void setParameter(pp_int32 index, const Parameter& par)
	{
		if (index >= 0 && index < numParameters)
			parameters[index] = par;
	}

	Parameter getParameter(pp_int32 index) const
	{
		if (index >= 0 && index < numParameters)
			return parameters[index];
		
		return Parameter();
	}
	
	pp_int32 getNumParameters() const { return numParameters; }
};

#endif
