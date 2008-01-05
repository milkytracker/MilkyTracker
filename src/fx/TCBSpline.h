#ifndef TCBSPLINE__H
#define TCBSPLINE__H

#include "Math3d.h"

class TCBSpline
{
public:
	struct TKey
	{
		int			ti;
		VectorFloat v;
		float		t,c,b;
	};

private:
	TKey*	keys;

	int		numKeys;

	VectorFloat getLastPos(int i);
	VectorFloat getNextPos(int i);
	VectorFloat calcTi(int i);
	VectorFloat calcTo(int i);
	int			findKey(int curTime,int startIndex, int endIndex);

public:

	TCBSpline(int numKeys);
	~TCBSpline();

	void		setKey(int index, VectorFloat& v, int time, float tension = 0, float continuity = 0, float bias = 0);
	void		setKey(int index, TKey& key) { if (index < numKeys) keys[index] = key; }
	TKey*		getKey(int index) { if (index < numKeys) return &keys[index]; else return &keys[0];}
	VectorFloat	getPos(float curTime);
		
};

#endif

