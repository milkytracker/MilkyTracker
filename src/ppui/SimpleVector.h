/*
 *  ppui/SimpleVector.h
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

#ifndef SIMPLEVECTOR__H
#define SIMPLEVECTOR__H

#include "BasicTypes.h"

template<class Type>
class PPSimpleVector
{
private:
	pp_int32 numValuesAllocated;
	Type** values;
	pp_int32 numValues;
	bool destroy;

	void reallocate()
	{
		Type** values = new Type*[numValuesAllocated];
		for (pp_int32 i = 0; i < numValues; i++)
		{
			values[i] = this->values[i];
		}
		delete[] this->values;		
		this->values = values;
	}

	// no copy construction please
	PPSimpleVector(const PPSimpleVector&);
	PPSimpleVector& operator=(const PPSimpleVector&);
	
public:
	PPSimpleVector(pp_int32 initialSize = 0, bool destroy = true)
	{
		this->destroy = destroy;

		if (initialSize == 0)
			initialSize = 16;

		numValuesAllocated = initialSize;

		if (initialSize)
			values = new Type*[initialSize];
		else
			values = 0;

		numValues = 0;
	}

	~PPSimpleVector()
	{	
		if (values)
		{
			if (destroy)
				for (pp_int32 i = 0; i < numValues; i++)
					delete values[i];

			delete[] values;
		}
	}

	PPSimpleVector* clone()
	{
		PPSimpleVector* clonedVector = new PPSimpleVector(numValuesAllocated, true);		
		for (pp_int32 i = 0; i < numValues; i++)
		{
			clonedVector->values[i] = new Type(*values[i]);
		}		
		clonedVector->numValues = numValues;
		return clonedVector;
	}

	void clear()
	{
		if (values)
		{
			if (destroy)
				for (pp_int32 i = 0; i < numValues; i++)
					delete values[i];

			numValues = 0;
		}
	}

	Type* removeNoDestroy(pp_int32 index)
	{
		if (!numValues)
			return NULL;
			
		if (index < 0 || index >= numValues)
			return NULL;
			
		Type* result = values[index];

		for (pp_int32 i = index; i < numValues-1; i++)
			values[i] = values[i+1];
			
		numValues--;
		
		if (numValuesAllocated - numValues > 16)
		{
			numValuesAllocated-=16;
			reallocate();
		}
		
		return result;
	}

	bool remove(pp_int32 index)
	{
		if (!numValues)
			return false;
			
		if (index < 0 || index >= numValues)
			return false;
			
		if (destroy)
			delete values[index];

		for (pp_int32 i = index; i < numValues-1; i++)
			values[i] = values[i+1];
			
		numValues--;
		
		if (numValuesAllocated - numValues > 16)
		{
			numValuesAllocated-=16;
			reallocate();
		}
		
		return true;
	}

	void add(Type* value)
	{
		if (numValues >= numValuesAllocated)
		{			
			numValuesAllocated += 16;
			reallocate();
		}
		values[numValues++] = value;
	}

	// handle with care
	void replace(pp_int32 index, Type* value)
	{
		if (index < 0 || index >= numValues)
			return;

		if (destroy)
			delete values[index];

		values[index] = value;
	}

	Type* get(pp_int32 index) const
	{
		if (index < numValues)
		{
			return values[index];
		}
		else
			return 0;
	}

	pp_int32 size() const { return numValues; }

	bool isEmpty() const { return numValues == 0; }

	// -- sorting --------------------------------------------------------------
	struct SortRule
	{
		virtual pp_int32 compare(const Type& left, const Type& right) const = 0;
	};
	
private:
	static pp_int32 partition(Type** a, pp_int32 left, pp_int32 right, const SortRule& sortRule, bool descending = false)
	{
		const pp_int32 sign = descending ? -1 : 1;
	
		pp_int32 first=left, pivot=right--;
		while(left<=right)
		{
			while(sortRule.compare(*a[left], *a[pivot])*sign < 0/*a[left]<a[pivot]*/)
				left++;
			
			while((right>=first)&&(sortRule.compare(*a[right], *a[pivot])*sign >= 0/*a[right]>=a[pivot]*/))
				right--;
			
			if(left<right)
			{
				swap(a, left,right);
				left++;
			}
		}
		if(left!=pivot)
		swap(a, left,pivot);
			
		return left;
	}
			
	static void swap(Type** a, pp_int32 i, pp_int32 j)
	{
		Type* temp=a[i];
		a[i]=a[j];
		a[j]=temp;
	}
			
	static void sortInternal(Type** array, pp_int32 left, pp_int32 right, const SortRule& sortRule, bool descending = false)
	{
		pp_int32 p;
		
		if(left>=right)
			return;
		
		p = partition(array, left, right, sortRule, descending);
		
		sortInternal(array, left,p-1, sortRule, descending);
		sortInternal(array, p+1, right, sortRule, descending);
	}

public:
	void sort(const SortRule& sortRule, pp_int32 l = 0, pp_int32 r = -1, const bool descending = false)
	{
		if (r == -1)
			r = size()-1;
	
		// no need to sort
		if (l == 0 && r <= 1)
			return;
		
		sortInternal(values, l, r, sortRule, descending);
	}	
};

#endif
