#ifndef FILTER_H
#define FILTER_H

#define PRGB2SHORT(r,g,b) ((((r))<<11)+(((g))<<5)+((b))) 
#define RGB2SHORT(r,g,b) ((((r>>3))<<11)+(((g>>2))<<5)+((b>>3))) 

class Filter 
{
private:
	static void		applyRadialToSector(unsigned short *buffer,int width, int height,int pitch,
										int xCenter,int yCenter,int xBound,int yBound,
										int radius, int style = 0);
public:
	static	void	applyRadial(unsigned short *buffer,int width, int height,int pitch,
								int xCenter,int yCenter,
								int radius, int style = 0);

	static	void	applyHorizontal(unsigned short *src, 
								    unsigned short *dst, 
									int width, int height, int pitch, 
									int boxw);

	static	void	applyVertical(unsigned short *src, 
								  unsigned short *dst, 
								  int width, int height, int pitch, 
								  int boxw);

	static	void	applyBoxed(unsigned short *srcImage, 
							   unsigned short *dstImage,
							   int width, int height, int pitch, 
							   int boxw);

	static  void	stylize(unsigned short* srcImage, 
							unsigned short* tmpImage,
							int width, int height, int srcPitch, int tmpPitch,
							int minr, int ming, int minb, 
							int maxr, int maxg, int maxb);

	static  void	glow(unsigned short* srcImage,
						 int width, int height, int pitch, 
						 unsigned short* glowBuffer1,
						 unsigned short* glowBuffer2,
						 unsigned int cellSizeShift,
						 unsigned int scale = 65536*2, unsigned int boxw = 4);

};

#endif
