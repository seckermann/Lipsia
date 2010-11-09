/****************************************************************
**
** Definition of showVImage class
**
****************************************************************/

#ifndef SHOWVIMAGE_H
#define SHOWVIMAGE_H

/* Std. Include Files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Vista Include Files */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* Qt Include Files */
#include <qwidget.h>
#include <qcolor.h>
#include "prefs.h"

class VLShow {
	//    Q_OBJECT
  public:
    //    void vlhColorMap( QColor *, QColor *, int, int, VImage *, int, int, int );
	  	void vlhColorMap( QColor *&rgbfarbe, QColor *&rgbfarbeoverlay, int acoltype, int coltype, VImage src, VImage *fnc, prefs *pr);
  		/*
	  	void vlhCreateLegend( QPixmap &cpm, QColor *rgbfarbeoverlay, double ppmax, double pmax, 
  							  double nnmax, double nmax, bool equidistantColorTable = false) {
  			
  			vlhCreateLegend(cpm, rgbfarbeoverlay, ppmax, pmax, nnmax, nmax, equidistantColorTable);
  		};
  		*/
  		void vlhCreateLegend( QPixmap &cpm, QColor *rgbfarbeoverlay, double ppmax, double pmax, 
  							  double nnmax, double nmax, bool equidistantColorTable = false, 
  							  QColor background = QColor(190, 190, 190), 
  							  QColor textColor = QColor(0,0,0));

  		
  		
  		QImage vlhCreateImage ( QWidget *, VImage *, VImage *, int, int, int, int, int, int, int, VArgVector, double *, QColor *, int, int, int, int, int, int, int *, int, int, double, double, double, double, double, double, int );


};

#endif // SHOWVIMAGE_H
