#ifndef     ARFIT_SCHLOEGL_HEADER_INCLUDED
#define     ARFIT_SCHLOEGL_HEADER_INCLUDED


#include    "arfit.h"

int mvar( gsl_matrix *Y, int pmax, arfit_mode mode, gsl_matrix **ARF, gsl_matrix **RFC, gsl_matrix **PE, gsl_matrix **DC, int *varargout );


#endif
