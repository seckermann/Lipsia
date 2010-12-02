#ifndef IMAGEDATA_H_
#define IMAGEDATA_H_

/**
 * Structure which contains alle the informations of an image needed
 * for rendering it.
 */
typedef struct sImageData {
	float minwert;
	float maxwert;
	float anamean;
	float anaalpha;
	VRepnKind pixel_repn;
	VString rep;
	VImage image;
	float brightness;
	float contrast;
	bool  isanatomy;
	int minwert1;
	int maxwert1;
} ImageData;

#endif /*IMAGEDATA_H_*/
