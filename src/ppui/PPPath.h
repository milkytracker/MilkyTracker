/*
 *  ppui/PPPath.h
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
 *  PPPath.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 12.10.06.
 *
 */

#ifndef __PPPATH_H__
#define __PPPATH_H__

#include "BasicTypes.h"

class PPPathEntry
{
public:
	enum Type
	{
		Nonexistent,
		Hidden,
		File,
		Directory
	};


protected:	
	PPSystemString	name;	
	Type			type;
	pp_uint32		size;
	
public:
	PPPathEntry() { }

	virtual ~PPPathEntry() { }

	virtual void create(const PPSystemString& path, const PPSystemString& name)
	{
		this->name = name;
		type = Nonexistent;
		size = 0;
	}

	virtual const PPSystemString& getName()	const { return name; }
	
	virtual bool isFile() const { return type == File; }
	virtual bool isDirectory() const { return type == Directory; }
	virtual pp_uint32 getSize() const { return size; }
	virtual bool isHidden() const { return type == Hidden; }
	virtual bool isDrive() const { return false; }
	virtual bool isParent() const 
	{ 
		static const PPSystemString temp("..");
		return name.compareTo(temp) == 0;
	}
	
	virtual bool compareTo(const PPPathEntry& src) const
	{
		if (name.compareTo(src.name) != 0)
			return false;
	
		return (type == src.type && size == src.size);
	}

	virtual PPPathEntry* clone() const
	{
		// check if this is the correct type
		PPPathEntry* result = new PPPathEntry();
		
		result->name = name;	
		result->type = type;
		result->size = size;
		
		return result;
	}

	class PathSortRuleInterface
	{
	public:
		virtual pp_int32 compare(const PPPathEntry& left, const PPPathEntry& right) const = 0;
	};
	
	class PathSortByFileRule : public PathSortRuleInterface
	{
	public:
		virtual pp_int32 compare(const PPPathEntry& left, const PPPathEntry& right) const
		{
			// no drives
			if (!left.isDrive() && !right.isDrive())
			{
				if (left.isFile() && right.isFile())
				{
					return left.getName().compareToNoCase(right.getName());
				}
				else if (left.isDirectory() && right.isDirectory())
				{
					if (!left.isParent() && !right.isParent())
						return left.getName().compareToNoCase(right.getName());
					else if (left.isParent())
						return -1;
					else
						return 1;
				}
				else
				{
					if (left.isDirectory() && right.isFile())
						return -1;
					else
						return 1;
				}
			}
			// drives
			else if (left.isDrive() && right.isDrive())
			{
				return left.getName().compareToNoCase(right.getName());
			}
			else
			{
				if (left.isDrive() && !right.isDrive())
					return 1;
				else
					return -1;
			}
		}
	};
	
	class PathSortBySizeRule : public PathSortRuleInterface
	{
	public:
		virtual pp_int32 compare(const PPPathEntry& left, const PPPathEntry& right) const
		{
			// both are files
			if (!left.isDirectory() && !right.isDirectory())
			{
				if (left.getSize() != right.getSize())
					return left.getSize() - right.getSize();
				else
					return left.getName().compareToNoCase(right.getName());
			}
			// directory
			else if (left.isDirectory() && right.isDirectory())
			{
				PathSortByFileRule comparator;
				return comparator.compare(left, right);
			}
			else
			{
				if (left.isDirectory() && !right.isDirectory())
					return -1;
				else
					return 1;
			}
		}
	};
	
	class PathSortByExtRule : public PathSortRuleInterface
	{
	public:
		virtual pp_int32 compare(const PPPathEntry& left, const PPPathEntry& right) const
		{
			// both are files
			if (!left.isDirectory() && !right.isDirectory())
			{
				pp_int32 diff = left.getName().compareExtensions(right.getName());
			
				if (diff == 0)			
					return left.getName().compareToNoCase(right.getName());
				else
					return diff;
			}
			// directory
			else if (left.isDirectory() && right.isDirectory())
			{
				PathSortByFileRule comparator;
				return comparator.compare(left, right);
			}
			else
			{
				if (left.isDirectory() && !right.isDirectory())
					return -1;
				else
					return 1;
			}
		}
	};

private:
	static pp_int32 partition(PPPathEntry** a, pp_int32 left, pp_int32 right, const PathSortRuleInterface& sortRule, bool descending = false)
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
			
	static void swap(PPPathEntry** a, pp_int32 i, pp_int32 j)
	{
		PPPathEntry* temp=a[i];
		a[i]=a[j];
		a[j]=temp;
	}
			
	static void sortInternal(PPPathEntry** array, pp_int32 left, pp_int32 right, const PathSortRuleInterface& sortRule, bool descending = false)
	{
		pp_int32 p;
		
		if(left>=right)
			return;
		
		p = partition(array, left, right, sortRule, descending);
		
		sortInternal(array, left,p-1, sortRule, descending);
		sortInternal(array, p+1, right, sortRule, descending);
		
		/*const pp_int32 sign = descending ? -1 : 1;
		pp_int32 i,j;
		PPPathEntry* x;
		PPPathEntry* y;
		i=l; j=r; x=array[(l+r)/2];
		do 
		{
			while (sortRule.compare(*array[i], *x)*sign < 0) i++;
			while (sortRule.compare(*x, *array[j])*sign < 0 && j > 0) j--;
			if (i <= j) 
			{
				y=array[i]; array[i]=array[j]; array[j]=y;
				i++; j--;
			}
		} while (i<=j);
		if (l<j) sortInternal(array, l, j, sortRule, descending);
		if (i<r) sortInternal(array, i, r, sortRule, descending);*/
	}

public:
	static void sort(PPPathEntry** array, pp_int32 l, pp_int32 r, const PathSortRuleInterface& sortRule, bool descending = false)
	{
		// no need to sort
		if (l == 0 && r <= 1)
			return;
		
		sortInternal(array, l, r, sortRule, descending);
	}
	
};

class PPPath
{
public:
	virtual ~PPPath() {}
	virtual const PPSystemString getCurrent() = 0;
	
	virtual bool change(const PPSystemString& path) = 0;
	virtual bool stepInto(const PPSystemString& directory) = 0;
	
	virtual const PPPathEntry* getFirstEntry() = 0;
	virtual const PPPathEntry* getNextEntry() = 0;	
	
	virtual bool canGotoHome() const = 0;
	virtual void gotoHome() = 0;
	virtual bool canGotoRoot() const = 0;
	virtual void gotoRoot() = 0;
	virtual bool canGotoParent() const = 0;
	virtual void gotoParent() = 0;
	
	virtual char getPathSeparatorAsASCII() const = 0;
	virtual const PPSystemString getPathSeparator() const = 0;

	virtual bool fileExists(const PPSystemString& fileName) const = 0;
};

#endif


