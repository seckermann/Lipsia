/****************************************************************
**
** Definition of showVImage class
**
****************************************************************/

#ifndef VLTOOLS_H
#define VLTOOLS_H

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
#include "prefs.h"

class VLTools
{
	//    Q_OBJECT
public:
	//public slots:
	void vlhInterpolate( VImage &image, double nbands_mult = 0, double nrows_mult = 0, double ncols_mult = 0, int type = 0 );
	void vlhInflate( VImage &image, double nbands_mult = 0, double nrows_mult = 0, double ncols_mult = 0 );
	void interpolate( VImage &mysrc, VImage &myfnc, double ncols_mult = 0, double nrows_mult = 0, double nbands_mult = 0, double scaleb = 0, double scaler = 0, double scalec = 0, int whichimage = 0, int intpolswitch = 0 );
	void VTal3Pixel( int &x, int &y, int &z, double *voxel = 0, double *extent = 0, double *ca = 0, int files = 0, double *pixelmult = 0 );
	void VPixel3Tal( double &x, double &y, double &z, double *extent = 0, double *ca = 0 , double *cp = 0, int files = 0, double *pixelmult = 0 );
	prefs *GetRadiometricMax( VImage fnc = 0, prefs *pr = NULL, int npixels = 0 );
	prefs *vlhContrast( prefs *pr = NULL, VImage Src = 0 );

};

#endif // VLTOOLS_H
