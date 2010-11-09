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


/**
 **  The bicubic spline code is based on 
 **  Michael J. Aramini, "Efficient Image Magnification by Bicubic 
 **  Spline Interpolation" 
 **  http://members.bellatlantic.net/~vze2vrva/design.html
 **/

/*
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
*/

#include "interpolator.hh"

#include <algorithm>
#include <float.h>
#include <math.h>


CNnMag::CNnMag(int numerator, int denominator):
	CMagBase(numerator, denominator)
{
	
}

void CNnMag::scale(const QScaleImage& in_image, QScaleImage& result) const
{
	for (int y = 0; y < result.height(); ++y) {
		int iy = int(float(y * in_image.height() ) / result.height() /* + 0.5 */);
		for (int x = 0; x < result.width(); ++x) {
			int ix = int(float(x * in_image.width() ) / result.width() /* + 0.5 */);
			result(x,y) = in_image(ix, iy);
		}
	}
}

CBilinMag::CBilinMag(int numerator, int denominator):
	CMagBase(numerator, denominator)
{
}

void CBilinMag::scale(const QScaleImage& in_image, QScaleImage& result) const
{
	for (int ry = 0; ry < result.height(); ++ry) {
		float y = float(ry * in_image.height() ) / result.height();
		
		int iy = (int)y;  float dy = y - iy; float my = 1.0 - dy; 
		
		for (int rx = 0; rx < result.width(); ++rx) {
			float x = float(rx *  in_image.width() ) / result.width();
		
			int ix = (int)x;  float dx = x - ix; float mx = 1.0 - dx; 
			
			result(rx,ry) = 
				my * (mx * RgbPixel(in_image(ix, iy  ))+ 
				      dx * RgbPixel(in_image(ix + 1, iy  ))) +
				dy * (mx * RgbPixel(in_image(ix, iy+1)) + 
				      dx * RgbPixel(in_image(ix + 1, iy+1)));
		}
	}
}

CMagBase::CMagBase(int numerator, int denominator):
	m_numerator(numerator),
	m_denominator(denominator)
{
	m_scale = 0;
}
	
CMagBase::~CMagBase()
{
}
inline int CMagBase::get_numerator()const
{
	return m_numerator;
}

inline double CMagBase::get_scale()const
{
	if (m_scale!=0)
		return m_scale;
	
	return m_numerator/m_denominator;
}

inline int CMagBase::get_denominator()const
{
	return m_denominator;
}

QScaleImage *CMagBase::operator ()(const QScaleImage& in_image)const
{
	int out_width  = (int)(in_image.width() * get_scale()); 
	int out_height = (int)(in_image.height() * get_scale()); 
	
	QScaleImage *result = new QScaleImage(out_width, out_height);
	scale(in_image, *result);
	return result; 
}

float CBicubSplineMag::C0(float t)
{
	return -m_param * t * t * t + m_param * t * t;
}

float CBicubSplineMag::C1(float t)
{
	return -(m_param + 2.0f) * t * t * t + (2.0f * m_param + 3.0f) * t * t - m_param * t;
}

float CBicubSplineMag::C2(float t)
{
	return (m_param + 2.0f) * t * t * t - (m_param + 3.0f) * t * t + 1.0f;
}

float CBicubSplineMag::C3(float t)
{
	return m_param * t * t * t - 2.0f * m_param * t * t + m_param * t;
}

CBicubSplineMag::CBicubSplineMag(int numerator, int denominator,float param):
	CMagBase(numerator, denominator),
	m_c0(numerator),
	m_c1(numerator),
	m_c2(numerator),
	m_c3(numerator),
	m_param(param)
{
	for (int i = 0; i < get_numerator(); i++) {
		float x = (float)((i * get_denominator()) % get_numerator()) / (float)get_numerator();
		m_c0[i] = C0(x);
		m_c1[i] = C1(x);
		m_c2[i] = C2(x);
		m_c3[i] = C3(x);
	}
}


void CBicubSplineMag::scale(const QScaleImage& in_image, QScaleImage& result) const
{
	int index = 0; 
	
	//vector <RgbPixel> temp(in_image.width()+3); 
	float *temp = (float *)calloc(in_image.width()+3, sizeof(float));	
	//fill(temp.begin(),temp.end(),RgbPixel());

	//int testsize = (int)(result.width()/10);
	
	for (int k = 0; k < result.height(); k++) {
		index = (k * get_denominator()) / get_numerator() - 1;
		
		int c_idx = k % get_numerator(); 
		int  j; 
		for (j = 0; j < in_image.width(); j++) {
			temp[j]  = RgbPixel(in_image(j, index  )).getRed() * m_c3[c_idx]; 
			temp[j] += RgbPixel(in_image(j, index+1)).getRed() * m_c2[c_idx];
			temp[j] += RgbPixel(in_image(j, index+2)).getRed() * m_c1[c_idx]; 
			temp[j] += RgbPixel(in_image(j, index+3)).getRed() * m_c0[c_idx];
		}
		
		for (int j = 0; j < result.width(); j++) {
			
			int c_idx = j % get_numerator();
			int idx = (j * get_denominator()) / get_numerator() - 1;
			
			/* result(j,k) = 
				temp[idx  ] * m_c3[c_idx] + 
				temp[idx+1] * m_c2[c_idx] + 
				temp[idx+2] * m_c1[c_idx] + 
				temp[idx+3] * m_c0[c_idx]; */
			float buf = 
				temp[idx  ] * m_c3[c_idx] + 
				temp[idx+1] * m_c2[c_idx] + 
				temp[idx+2] * m_c1[c_idx] + 
				temp[idx+3] * m_c0[c_idx];
			result(j,k) = RgbPixel(buf, buf, buf);

			/*
			if (j<testsize) {		
                          int rot=qRed(result(j,k));
			  int gruen=qGreen(result(j,k));
			  int blau=qBlue(result(j,k));
			  if (rot==0 || blau==0 || gruen==0) {
                            if (!(rot==0 && blau==0 && gruen==0)) {
                               if (rot==0) {
                                 if (blau>0)  result(j,k)= RgbPixel(qRgb(blau, blau, blau));
		                 if (gruen>0) result(j,k)= RgbPixel(qRgb(gruen,gruen,gruen));
                               }
                               if (blau==0) {
                                 if (rot>0)   result(j,k)= RgbPixel(qRgb(rot, rot, rot));
		                 if (gruen>0) result(j,k)= RgbPixel(qRgb(gruen,gruen,gruen));
                               }
                               if (gruen==0) {
                                 if (blau>0) result(j,k)= RgbPixel(qRgb(blau, blau, blau));
		                 if (rot>0)  result(j,k)= RgbPixel(qRgb(rot,rot,rot));
                               }
			    }
                          } 
			} */
		}
	}
}

CBicub6Mag::CBicub6Mag(int numerator, int denominator):
	CMagBase(numerator, denominator),
	m_c0(numerator),
	m_c1(numerator),
	m_c2(numerator),
	m_c3(numerator),
	m_c4(numerator),
	m_c5(numerator)
{
	for (int i = 0; i < get_numerator(); i++) {
		float x = (float)((i * get_denominator()) % get_numerator()) / (float)get_numerator();
		float b1=x*x, b2=b1*x, b3=1.2*b2, b4=0.6*b2, b5=0.2*b2,
			b6=1.4*b1, b7=0.4*b1, b8=0.8*x, b9=0.2*x;
		m_c0[i] = -b5 + 0.2*b1;
		m_c1[i] = b4 - b7 - b9;
		m_c2[i] = -b3 + b6 + b8;
		m_c3[i] = b3 - 2.2*b1 + 1.0;
		m_c4[i] = -b4 + b6 - b8;
		m_c5[i] = b5 - b7 + b9;
	}
}

void CBicub6Mag::scale(const QScaleImage& in_image, QScaleImage& result) const
{
	int index = 0; 
	
	//vector <RgbPixel> temp(in_image.width()+5); 
	float *temp = (float *)calloc(in_image.width()+5, sizeof(float));
	//fill(temp.begin(),temp.end(),RgbPixel());

	//int testsize = (int)(result.width()/10);
		
	for (int k = 0; k < result.height(); k++) {
		index = (k * get_denominator()) / get_numerator() - 2;
		
		int c_idx = k % get_numerator();
		int  j;
		for (j = 0; j < in_image.width(); j++) {
			temp[j]  = RgbPixel(in_image(j, index  )).getRed() * m_c5[c_idx];
			temp[j] += RgbPixel(in_image(j, index+1)).getRed() * m_c4[c_idx];
			temp[j] += RgbPixel(in_image(j, index+2)).getRed() * m_c3[c_idx];
			temp[j] += RgbPixel(in_image(j, index+3)).getRed() * m_c2[c_idx];
			temp[j] += RgbPixel(in_image(j, index+4)).getRed() * m_c1[c_idx];
			temp[j] += RgbPixel(in_image(j, index+5)).getRed() * m_c0[c_idx];
		}
		
		for (int j = 0; j < result.width(); j++) {
			
			int c_idx = j % get_numerator();
			int idx = (j * get_denominator()) / get_numerator() - 2;
			
			/* result(j,k) = 
				temp[idx  ] * m_c5[c_idx] + 
				temp[idx+1] * m_c4[c_idx] + 
				temp[idx+2] * m_c3[c_idx] + 
				temp[idx+3] * m_c2[c_idx] +
				temp[idx+4] * m_c1[c_idx] + 
				temp[idx+5] * m_c0[c_idx]; */
			float buf = 
				temp[idx  ] * m_c5[c_idx] + 
				temp[idx+1] * m_c4[c_idx] + 
				temp[idx+2] * m_c3[c_idx] + 
				temp[idx+3] * m_c2[c_idx] +
				temp[idx+4] * m_c1[c_idx] + 
				temp[idx+5] * m_c0[c_idx];
			result(j,k) = RgbPixel(buf, buf, buf);

			/*
			if (j<testsize) {	
                          int rot=qRed(result(j,k));
			  int gruen=qGreen(result(j,k));
			  int blau=qBlue(result(j,k));
			  if (rot==0 || blau==0 || gruen==0) {
                            if (!(rot==0 && blau==0 && gruen==0)) {
                               if (rot==0) {
                                 if (blau>0)  result(j,k)= RgbPixel(qRgb(blau, blau, blau));
		                 if (gruen>0) result(j,k)= RgbPixel(qRgb(gruen,gruen,gruen));
                               }
                               if (blau==0) {
                                 if (rot>0)   result(j,k)= RgbPixel(qRgb(rot, rot, rot));
		                 if (gruen>0) result(j,k)= RgbPixel(qRgb(gruen,gruen,gruen));
                               }
                               if (gruen==0) {
                                 if (blau>0) result(j,k)= RgbPixel(qRgb(blau, blau, blau));
		                 if (rot>0)  result(j,k)= RgbPixel(qRgb(rot,rot,rot));
                               }
			    }
                          } 
                        } */
		}
	}
}



CBSplineMag::CBSplineMag(int numerator, int denominator):
	CMagBase(numerator, denominator),
	m_c0(numerator),
	m_c1(numerator),
	m_c2(numerator),
	m_c3(numerator)
{

		z1 = -2.0 + sqrt(3.0);
		for (int i = 0; i < get_numerator(); i++) {
		float x = (float)((i * get_denominator()) % get_numerator()) / (float)get_numerator();
		float b1=x*x, b2=b1*x, b3=1.0/6.0, b4=b3*b2, b5=0.5*b2, b6=0.5*b1, b7=0.5*x;
		m_c0[i] = b4;
		m_c1[i] = -b5 + b6 + b7 + b3;
		m_c2[i] = b5 - b1 + (2.0/3.0);
		m_c3[i] = -b4 + b6 - b7 + b3;

	}
}

void CBSplineMag::scale(const QScaleImage& in_image, QScaleImage& result) const
{
	int cext;
	float zexp, eps=FLT_EPSILON;
	float c0;
	int index = 0;
		

	HelpField coeff(in_image.width(), in_image.height(), 2);
	//QScaleImage coeff = QScaleImage(in_image.width()+3, in_image.height()+3);
	//vector <RgbPixel> temp(in_image.width()+3); 
	//RgbPixel *temp = (RgbPixel *)calloc(in_image.width()+3, sizeof(RgbPixel));
	float *temp = (float *)calloc(in_image.width()+3, sizeof(float));
	//fill(temp.begin(),temp.end(),RgbPixel());
	float **c=(float **)calloc(in_image.width(), sizeof(float *));
	for(int a=0; a<in_image.width(); a++)
		c[a]=(float *)calloc(in_image.height(), sizeof(float));
        
                        
	for(int i=0; i<in_image.height(); i++){
		for(int j=0; j<in_image.width(); j++){
			c[j][i] = RgbPixel(in_image(j, i)).getRed()*6.0;
		}
	}
					
	/* Prefiltering */
	if(eps > 0.0){
		cext = (int)ceilf(log(eps)/log(fabs(z1)));
		if(cext > in_image.width()){
			cext = in_image.width();
		}
	} else{
		cext = in_image.width();
	}
	//cout << "z1: " << z1 << endl;
	/* horizontal computation of coefficients */                
	for(int i = 0; i<in_image.height(); i++){
	/* causal initialization */
	zexp = z1;
	c0 = c[0][i];
	for(int j = 1; j<cext; j++){
		c0 += zexp * c[j][i];
		zexp *= z1;
	}
	c[0][i] = c0;
	/* causal filtering */
	for(int k = 1; k<in_image.width(); k++){
		c[k][i] = c[k-1][i] * z1 + c[k][i];
	}
	/* anticausal initialization */
	c[in_image.width()-1][i] = (z1 / (z1*z1 - 1))*(z1 * c[in_image.width()-2][i] + c[in_image.width()-1][i]);
	/*anticausal filtering */
	for(int l = in_image.width()-2; 0<=l; l--){
		c[l][i] = z1 *(c[l+1][i]-c[l][i]);
	}
	}
        
	for(int i=0; i<in_image.height(); i++){
		for(int j=0; j<in_image.width(); j++){
			c[j][i] *= 6.0;
		}
	}
	
	if(eps > 0.0){
		cext = (int)ceilf(log(eps)/log(fabs(z1)));
		if(cext > in_image.height()){
			cext = in_image.height();
		}
	} else{
		cext = in_image.height();
	}
        
	/* vertical computation of coefficients */                
	for(int i = 0; i<in_image.width(); i++){
                /* causal initialization */
                zexp = z1;
                c0 = c[i][0];
	for(int j = 1; j<cext; j++){
		c0 += zexp * c[i][j];
					zexp *= z1;
		
	}
	c[i][0] = c0;
	/* causal filtering */
	for(int k = 1; k<in_image.height(); k++){
		c[i][k] = c[i][k-1] * z1 + c[i][k];
	}
	/* anticausal initialization */
	c[i][in_image.height()-1] = (z1 / (z1*z1 - 1))*(z1 * c[i][in_image.height()-2] + c[i][in_image.height()-1]);
	/*anticausal filtering */
	for(int l = in_image.height()-2; 0<=l; l--){
		c[i][l] = z1 *(c[i][l+1]-c[i][l]);
	}
	}
        
	for(int i=0; i<in_image.height(); i++){
		for(int j=0; j<in_image.width(); j++){
			//coeff(j,i) = RgbPixel(c[j][i], c[j][i], c[j][i]);
			coeff.setVal(j,i,c[j][i]);
		}
	}

	/* Filtering */
	for (int k = 0; k < result.height(); k++) {
		index = (k * get_denominator()) / get_numerator() - 1;
		
		int c_idx = k % get_numerator();
		int  j;
		for (j = 0; j < in_image.width(); j++) {
			/*if(k>0 && j>0 && k<result.height()-1 && j<in_image.width()-1){
				temp[j+1]  = c[j][index] * m_c3[c_idx];
				temp[j+1] += c[j][index+1] * m_c2[c_idx];
				temp[j+1] += c[j][index+2] * m_c1[c_idx];
				temp[j+1] += c[j][index+3] * m_c0[c_idx];
			} else{
				temp[j+1]  = RgbPixel(coeff.getPixel(j, index  )).getRed() * m_c3[c_idx];
				temp[j+1] += RgbPixel(coeff.getPixel(j, index+1)).getRed() * m_c2[c_idx];
				temp[j+1] += RgbPixel(coeff.getPixel(j, index+2)).getRed() * m_c1[c_idx];
				temp[j+1] += RgbPixel(coeff.getPixel(j, index+3)).getRed() * m_c0[c_idx];
			}*/
			temp[j+1]  = coeff.getVal(j, index  ) * m_c3[c_idx];
			temp[j+1] += coeff.getVal(j, index+1) * m_c2[c_idx];
			temp[j+1] += coeff.getVal(j, index+2) * m_c1[c_idx];
			temp[j+1] += coeff.getVal(j, index+3) * m_c0[c_idx];
		}
                /*temp[in_image.width()] = temp[in_image.width()-1];
                temp[in_image.width()+1] = temp[in_image.width()-2];
                temp[in_image.width()+2] = temp[in_image.width()-3];
                temp[0] = temp[1];*/
		
		for (int j = 0; j < result.width(); j++) {
			
			int c_idx = j % get_numerator();
			int idx = (j * get_denominator()) / get_numerator() - 1;
			/*result(j,k) = 
				temp[idx  ] * m_c3[c_idx] + 
				temp[idx+1] * m_c2[c_idx] + 
				temp[idx+2] * m_c1[c_idx] + 
				temp[idx+3] * m_c0[c_idx];*/
			float buf = 
				temp[idx+1] * m_c3[c_idx] + 
				temp[idx+2] * m_c2[c_idx] + 
				temp[idx+3] * m_c1[c_idx] + 
				temp[idx+4] * m_c0[c_idx];
			result(j,k) = RgbPixel(buf, buf, buf);

		}
	}
	free(c);
	
}

