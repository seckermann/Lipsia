#ifndef     ARFIT_ERROR_HEADER_INCLUDED
#define     ARFIT_ERROR_HEADER_INCLUDED


#include    "arfit.h"

/* error computation methods */
double          arfit_mse( const arfit_output *output );
double          arfit_msy( const arfit_input *input, int sample );
double          arfit_rev( const arfit_input *input, int sample, const arfit_output *output );
double          arfit_gof( const arfit_input *input, int sample, const arfit_output *output );


#endif
