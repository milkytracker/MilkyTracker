#include "FXInterface.h"

class TCBSplineTest : public FXInterface
{
private:
	float* buffer;

public:
	TCBSplineTest(int width, int height);
	~TCBSplineTest();

	void render(unsigned short* vscreen, unsigned int pitch);
	void update(float syncFrac);

};
