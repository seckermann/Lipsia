#ifndef		ARFIT_SCHNEIDER_HEADER_INCLUDED
#define		ARFIT_SCHNEIDER_HEADER_INCLUDED


#include	"arfit.h"


/* non interface methods */
int				arqr( gsl_matrix *v, int p, int zero, gsl_matrix **R, gsl_vector **scale );
int 			arord( gsl_matrix *R, int m, int zero, int ne, int pmin, int pmax, gsl_vector **sbc, gsl_vector **fpe, gsl_vector **logdp, gsl_vector **np );
int				arconf( gsl_matrix *A, gsl_matrix *C, gsl_vector *w, gsl_matrix *th, int p, gsl_matrix **Aerr, gsl_vector **Werr );


#endif
