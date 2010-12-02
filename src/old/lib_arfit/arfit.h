#ifndef     ARFIT_HEADER_INCLUDED
#define     ARFIT_HEADER_INCLUDED


#include    "arfit_base.h"


/* arfit methods */
void    arfit_vector( gsl_vector *input, int pmin, int pmax, int zero, arfit_selector selector, arfit_algorithm algorithm, arfit_mode mode, double threshold, arfit_output **output );
void    arfit_matrix( gsl_matrix *input, int pmin, int pmax, int zero, arfit_selector selector, arfit_algorithm algorithm, arfit_mode mode, double threshold, arfit_output **output );
void    arfit_granger( gsl_matrix *input, int pmin, int pmax, int zero, arfit_selector selector, arfit_algorithm algorithm, arfit_mode mode, double threshold, double *output );


#endif
