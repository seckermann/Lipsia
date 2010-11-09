#ifndef VISTA2MAT_H
#define VISTA2MAT_H

#include <gsl/gsl_matrix.h>

gsl_matrix_float* VFloat2Mat_gsl(VImage);
gsl_matrix_float* VShort2Mat_gsl(VImage*,int);

#endif

