#ifndef _IMAGE_H_
#define _IMAGE_H_

namespace converter
{


/*
 * TODO: this one really should be a template !! (a matter of time ...)
 */

class Image
{

public:
	Image( int width, int height, float pixelValues[] );
	virtual ~Image();
	virtual float getPixelValue( int x, int y );

private:
	float *pixels;
	int    width;
	int    height;
};

};

#endif /*_IMAGE_H_*/
