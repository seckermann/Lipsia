#include "Image.h"



namespace converter {

Image::Image(int _width, int _height, float _pixelValues[]) {
	pixels = &_pixelValues[0];
	width  = _width;
	height = _height;
}

Image::~Image() {
}

float Image::getPixelValue(int x, int y) {
	return pixels[(width * y) + x];
}

};
