#ifndef TEXTURE__H
#define TEXTURE__H

class Texture
{
public:
	static void	createSplineTexture(unsigned char* tex,
									int numBlocks = 10000,
									int blockSize = 16);

	static void createFlareTexture(unsigned char* tex,
								   int r,int g,int b,
								   float pw = 4.0f, 
								   unsigned int size = 256);

	static void createPlasmaTexture(unsigned char* tex,
									unsigned int size = 256,
									int smooth = 3,
									int r = 255, int g = 255, int b = 255);

	static void convert24to16(unsigned short* dstImage, unsigned char* srcImage, int size = 256*256, unsigned int shifter = 0);

	static void blur24(unsigned char* tex, unsigned int width, unsigned int height, unsigned int passes = 1);
};

#endif
