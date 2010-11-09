/* -*- linux-c -*-
**  
** Copyrigh (C) 1999 Max-Planck-Institute of Cognitive Neurosience
**                    Gert Wollny <wollny@cns.mpg.de>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __interpolator_hh
#define __interpolator_hh

#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <qimage.h> 
#include <stdlib.h>

using namespace std; 

class HelpField {
	float **c;
	int w, h;
public:
	HelpField():c(0), w(0), h(0){};
	
	HelpField(int x, int y, int ):w(x), h(y){
		c=(float **)calloc(x, sizeof(float *));
		for(int a=0; a<x; a++)
			c[a]=(float *)calloc(y, sizeof(float));
	}
	
	void setVal(int x, int y, float val){
		c[x][y] = val;
	}
	
	float getVal(int x, int y){
		if(x >= 0 && x<w && y>=0 && y<h)
			return c[x][y];
		else
			return 0.0;
	}
};



/** helper class for sane representation of RGB data and translation to QT QRgb format*/

class RgbPixel {
	float r;
	float g; 
	float b;
		
public:	
	/** construct a black pixel as standart */
	RgbPixel():r(0), g(0), b(0){};
	
	/** construct a pixel from three color components 
	    \param fr red
	    \param fg green 
	    \param fb blue
	 */
	RgbPixel(float fr, float fg, float fb):
		r(fr), g(fg), b(fb){};
	
	/** construct a Pixel from an QT QRgb value
	    \param rgb the input color value
	    \remark this constructor has to be called explicitely to avoid 
	    ambiugity 
	*/
	
	explicit  RgbPixel(QRgb rgb):
		r((rgb & 0xFF0000) >> 16),
		g((rgb & 0x00FF00) >> 8),
		b(rgb & 0x0000FF){
	}

	float getRed() {
                return r;
	}

	
	/** conversion operator that creates an QT QRgb value from the pixel 
	    \returns the QRgb value by translating the r, g, and b values and clamping to 
	    the range [0,255]
	*/
	operator QRgb() const {
		int ib = int(b + 0.5); if (ib > 255) ib = 255;  if (ib < 0) ib = 0; 
		int ig = int(g + 0.5); if (ig > 255) ig = 255;  if (ig < 0) ig = 0; 
		int ir = int(r + 0.5); if (ir > 255) ir = 255;  if (ir < 0) ir = 0; 		
		
		QRgb res =  ib | (ig << 8 ) | (ir  << 16 );
			
		return res; 
	}
	
	/** inplace componentwise addition of the color values of two pixels
	    \param other the other pixel
	    \returns a reference to this pixel with the value of the other added
	*/
	RgbPixel& operator += (const RgbPixel& other) {
		r += other.r; 
		b += other.b; 
		g += other.g;
		return *this;
	}

	/** inplace componentwise multiplication with float value
	    \param float value
	    \returns a reference to this pixel multiplied by the float value
	*/
	RgbPixel& operator *= (float f) {
		r *= f; 
		b *= f; 
		g *= f;
		return *this;
	}

	
	/** componentwise addition of two pixels
	    \param a one pixel
	    \param b the other pixel
	    \returns the sum of both pixels
	*/
	friend RgbPixel operator + (RgbPixel a, RgbPixel b) {
		RgbPixel res = a; 
		res += b;
		return res; 
	}
	
	/** componentwise multiplication of a pixels with a scalar
	    \param f scalar value
	    \param rgb pixel 
	    \returns product of pixel and float 
	*/
	friend RgbPixel operator * (float f, RgbPixel rgb) {
		RgbPixel res = rgb; 
		res *= f; 
		return res; 
	}
	/** componentwise multiplication of a pixels with a scalar
	    \param rgb pixel 
	    \param f scalar value
	    \returns product of pixel and float 
	*/
	friend RgbPixel operator * (RgbPixel rgb,float f) {
		RgbPixel res = rgb; 
		res *= f; 
		return res; 
	}
};


/** interpolation types */
enum interpol_type {ip_nn,      /*!< nearest neighbor */
                    ip_bilin,   /*!< bi-linear        */
                    ip_bicub,   /*!< bicubic          */
                    ip_bicub_6, /*!< bicubic over six samples */
                    ip_bspline  /*!< b-spline interpolation */
};


/** little helper class to make a saner access to QImage */
class QScaleImage: public QImage {
public:	
	/** Create a new 32-bit color image with 
	    \param w width
	    \param h height
	*/
	QScaleImage(int w, int h):
		QImage(w,h,32),
		m_w(w),
		m_h(h)
	{
	}
	
	/** read only access to the image pixels, returns 0, if x and y are out of bounds
	    \param x pixel column to access
	    \param y pixel row to access
	    \returns color of requested pixel or 0 if the coordinates are out of bounds
	*/
	const QRgb operator()( unsigned int x, unsigned int y) const {
		if (x < m_w && y < m_h)
			return pixel(x,y);
		else 
			return 0;
	}
	
	/** Read-Write access to the image without bounds checking 
	    \param x pixel column to access
	    \param y pixel row to access
	    \returns color of requested pixel
	 */
	QRgb& operator()(int x, int y){
		return ((QRgb*)scanLine(y))[x]; 
	}
private: 
	unsigned int m_w, m_h; 
};

 

/** The base class for the image magnification filter. It can handle C2DImage class images.
    \remark probably this one could be templated to use abitrary types of 2D images 
 */
    
class CMagBase {
public:
	/**
	 * Default constructor
	 * */
	CMagBase() {};
	
	/** Construct the scaler and initialize it 
	 \param numerator numerator of the scaling ratio 
	 \param denominator denominator of the scaling ratio
	*/
	
	CMagBase(int numerator, int denominator);
	
	/** virtual destructor just to make sure it is virtual for all child classes */
	virtual ~CMagBase();
	
	/** The scaling operator 
	    \param in_image input image to be scaled 
	    \returns the newly created scaled image that has to be deleted by the caller
	*/
	QScaleImage *operator ()(const QScaleImage& in_image)const;
	
	void setScalingRatio(int numerator, int denominator) {
		m_numerator = numerator;
		m_denominator = denominator;
	};
	
	void setScalingRatio(double scale) {
		m_scale = scale;
	};
	
protected:
	/** access operator since proteced member variables are evil
	    \returns the numerator
	*/
	int get_numerator()const; 
	
	/** \returns the denominator */
	int get_denominator()const; 
	
	/** \returns the scale factor */
	double get_scale() const;
private:	
	
	/** the actual scaling function that has to be implemented per interpolation type
	    \param in_image the input image
	    \param the result image that has already the requested size
	*/
	virtual void scale(const QScaleImage& in_image, QScaleImage& result) const = 0; 
	
	int m_numerator; 
	int m_denominator;
	double m_scale;		// alternate scale for nn-alg
};

/// This one implements a nearest neighbor base magnifier 

class CNnMag: public CMagBase {
public:
	/**
	 * Default constructor
	 * */
	CNnMag() {};
	
	/** Construct the scaler and initialize it 
	 \param numerator numerator of the scaling ratio 
	 \param denominator denominator of the scaling ratio
	*/

	CNnMag(int numerator, int denominator);
private:	
	virtual void  scale(const QScaleImage& in_image, QScaleImage& result) const;
	
};

/// This one implements a bilinear interpolation based magnifier 
class CBilinMag: public CMagBase {
public:
	/**
	 * Default constructor
	 * */
	CBilinMag() {};
	/** Construct the scaler and initialize it 
	 \param numerator numerator of the scaling ratio 
	 \param denominator denominator of the scaling ratio
	*/

	CBilinMag(int numerator, int denominator);
private:	
	virtual void  scale(const QScaleImage& in_image, QScaleImage& result) const;
};


/// This one implements a bicubic spline interpolation based magnifier 
class CBicubSplineMag: public CMagBase {

public:	
	/**
	 * Default constructor
	 * */
	CBicubSplineMag() {};
	
	/** Construct the scaler and initialize it 
	 \param numerator numerator of the scaling ratio 
	 \param denominator denominator of the scaling ratio
	 \param a special spline parameter -1 <= param <= 0
	*/
	CBicubSplineMag(int numerator, int denominator, float param);

	void setParam(float param) {m_param = param;};
private:
	virtual void scale(const QScaleImage& in_image, QScaleImage& result) const;


	float C0(float t);
	float C1(float t);
	float C2(float t);
	float C3(float t);
	
	typedef vector<float> TFloatVector;
	TFloatVector m_c0; 
	TFloatVector m_c1; 
	TFloatVector m_c2; 
	TFloatVector m_c3; 
	float m_param;
	
};


/* dmoeller 02-22-06 */
class CBicub6Mag: public CMagBase{

public:
	/**
	 * Default constructor
	 * */
	CBicub6Mag() {};
	/** Construct the scaler and initialize it 
	 \param numerator numerator of the scaling ratio 
	 \param denominator denominator of the scaling ratio
	*/
	CBicub6Mag(int numerator, int denominator);
private:
	virtual void scale(const QScaleImage& in_image, QScaleImage& result) const;
	typedef vector<float> TFloatVector;
	TFloatVector m_c0; 
	TFloatVector m_c1; 
	TFloatVector m_c2; 
	TFloatVector m_c3;
	TFloatVector m_c4; 
	TFloatVector m_c5;
};


/* dmoeller 03-28-06 */
class CBSplineMag: public CMagBase{

public:
	/**
	 * Default constructor
	 * */
	CBSplineMag() {};
	/** Construct the scaler and initialize it 
	 \param numerator numerator of the scaling ratio 
	 \param denominator denominator of the scaling ratio
	*/
	CBSplineMag(int numerator, int denominator);
private:
	virtual void scale(const QScaleImage& in_image, QScaleImage& result) const;
	float z1;
	typedef vector<float> TFloatVector;
	TFloatVector m_c0; 
	TFloatVector m_c1; 
	TFloatVector m_c2; 
	TFloatVector m_c3;
	float m_param;
};


#endif

