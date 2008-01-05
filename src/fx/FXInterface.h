#ifndef FXINTERFACE__H
#define FXINTERFACE__H

class FXInterface
{
protected:
	int		width, height;

public:
	virtual ~FXInterface() {}

	virtual void render(unsigned short* vscreen, unsigned int pitch) = 0;

	virtual void update(float syncFrac) = 0;
};

#endif
