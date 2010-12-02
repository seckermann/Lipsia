#ifndef VCHECK_H
#define VCHECK_H

/* From the std. library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Qt Include Files */
#include <qwidget.h>

class VCheck
{
	//    Q_OBJECT
public:
	VImage VReadDesignFile( VString desfile = NULL, int verbose = 0 );
	int VCheckRawDataFile( VString rawfile = NULL, int tc_minlength = 200 );
	VImage VCheckBetaFile( VString betafile = NULL );
};

#endif // VCHECK
