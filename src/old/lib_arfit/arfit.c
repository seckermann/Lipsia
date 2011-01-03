#include    "arfit.h"
#include    "gsl_matrix_wrapper.h"
#include    "arfit_schneider.h"
#include    "arfit_schloegl.h"
#include    <gsl/gsl_sf_log.h>


/* arfit_vector function
    performs an arfit algortihm on univariate timedata
*/
void arfit_vector(gsl_vector *input, int pmin, int pmax, int zero, arfit_selector selector, arfit_algorithm algorithm, arfit_mode mode, double threshold, arfit_output **output) {
    /* check parameters */
    if(!input)
        return;
    gsl_matrix *input_mat = gsl_matrix_alloc(input->size, 1);
    arfit_matrix(input_mat, pmin, pmax, zero, selector, algorithm, mode, threshold, output);
    gsl_matrix_free(input_mat);
}

/* arfit_matrix function
    performs an arfit algortihm on multivariate timedata
*/
void arfit_matrix(gsl_matrix *input, int pmin, int pmax, int zero, arfit_selector selector, arfit_algorithm algorithm, arfit_mode mode, double threshold, arfit_output **output) {
    /* check parameters */
    if(!input)
        return;
    arfit_input     *arinput    = NULL;
    if(*output)
        arfit_output_free(*output);
    arfit_input_alloc(arinput);
    arinput->pmin       = pmin;
    arinput->pmax       = pmax;
    arinput->selector   = selector;
    arinput->zero       = zero;
    arinput->v          = gsl_matrix_alloc(input->size1, input->size2);
    arinput->threshold  = threshold;
    gsl_matrix_memcpy(arinput->v, input);
    if(algorithm == arfit_algorithm_schneider)
        arfit_schneider(arinput, output);
    else if(algorithm == arfit_algorithm_schloegl)
        arfit_schloegl(arinput, mode, output);
    arfit_input_free(arinput);
}

/* arfit_granger function
    computes granger coefficients
*/
void arfit_granger(gsl_matrix *input, int pmin, int pmax, int zero, arfit_selector selector, arfit_algorithm algorithm, arfit_mode mode, double threshold, double *output) {
    /* Granger Kausalität

    function g = granger(X,pmin,pmax);

    [wx,Ax,Cx,SBCx,FPEx,thx]  = arfit2(X(:,1),pmin,pmax);
    [wy,Ay,Cy,SBCy,FPEy,thy]  = arfit2(X(:,2),pmin,pmax);
    [wxy,Axy,Cxy,SBCxy,FPExy] = arfit2(X,pmin,pmax);


    g(1) = log(abs(Cy)/abs(Cxy(2,2)));
    g(2) = log(abs(Cx)/abs(Cxy(1,1)));
    g(3) = log(abs(Cxy(1,1))*abs(Cxy(2,2))/det(Cxy));

    */
    int     i;
    double  aCy, aCx, aCxy11, aCxy22, det;
    /*check for invalid or non-bivariate input*/
    if(input == NULL || input->size2 != 2) {
        fprintf(stderr, "Invalid input matrix. Must be of size (n,2), 2 columns each representing a timecourse\n");
        return;
    }
    arfit_output *pout1 = NULL, *pout2 = NULL, *pout3 = NULL;
    gsl_matrix   *mat1  = gsl_matrix_alloc(input->size1, 1);
    gsl_matrix   *mat2  = gsl_matrix_alloc(input->size1, 1);
    for(i = 0; i < input->size1; ++i) {
        gsl_matrix_set(mat1, i, 0, gsl_matrix_get(input, i, 0));
        gsl_matrix_set(mat2, i, 0, gsl_matrix_get(input, i, 1));
    }
    arfit_matrix(mat1, pmin, pmax, zero, selector, algorithm, mode, threshold, &pout1);
    arfit_matrix(mat2, pmin, pmax, zero, selector, algorithm, mode, threshold, &pout2);
    arfit_matrix(input, pmin, pmax, zero, selector, algorithm, mode, threshold, &pout3);
    if(pout3->iprocessed == 1) {
        aCx     = fabs(gsl_matrix_get(pout1->C, 0, 0));
        aCy     = fabs(gsl_matrix_get(pout2->C, 0, 0));
        aCxy11  = fabs(gsl_matrix_get(pout3->C, 0, 0));
        aCxy22  = fabs(gsl_matrix_get(pout3->C, 1, 1));
        det     = gsl_matrix_det(pout3->C);
        if(det < 0.0) {
            fprintf(stderr, "\nWarning! det(Cxy) < 0. abs(det(Cxy)) used. Results might not be usefull.\n");
            det = fabs(det);
        }
        output[ 0 ] = gsl_sf_log(aCy / aCxy22);
        output[ 1 ] = gsl_sf_log(aCx / aCxy11);
        output[ 2 ] = gsl_sf_log((aCxy11 * aCxy22) / det);
        output[ 3 ] = det;
    } else {
        output[ 0 ] = 0.0;
        output[ 1 ] = 0.0;
        output[ 2 ] = 0.0;
        output[ 3 ] = 0.0;
    }
    gsl_matrix_free(mat2);
    gsl_matrix_free(mat1);
    arfit_output_free(pout3);
    arfit_output_free(pout2);
    arfit_output_free(pout1);
}
