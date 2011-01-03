/*
**  covariance matrix, input vectors are columns of A and B
**
** G.Lohmann, Nov. 2005
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include "gsl_utils.h"


gsl_matrix *
dcovariance(gsl_matrix *A, gsl_matrix *B, gsl_matrix *Cov) {
    int i, j, k, m;
    double x, y, u, v, sum;
    gsl_vector *mean1 = NULL, *mean2 = NULL;
    m = A->size2;  /* each column of A,B is one data point */
    if(A->size2 != B->size2) {
        fprintf(stderr, "dcovariance: inconsistent dimensions: %d %d\n", A->size2, B->size2);
        exit(0);
    }
    if(Cov == NULL) {
        Cov = gsl_matrix_calloc(A->size1, B->size1);
    } else {
        if(Cov->size1 != A->size1 || Cov->size2 != B->size1) {
            fprintf(stderr, "dcovariance: inconsistent dimensions (%d %d), (%d %d).\n",
                    Cov->size1, A->size1, Cov->size2, B->size1);
            exit(0);
        }
    }
    mean1 = gsl_vector_calloc(A->size1);
    for(i = 0; i < A->size1; i++) {
        sum = 0;
        for(j = 0; j < A->size2; j++)
            sum += dmget(A, i, j);
        u = sum / (float)A->size2;
    }
    mean2 = gsl_vector_calloc(B->size1);
    for(i = 0; i < B->size1; i++) {
        sum = 0;
        for(j = 0; j < B->size2; j++)
            sum += dmget(B, i, j);
        u = sum / (float)B->size2;
        dvset(mean2, i, u);
    }
    for(i = 0; i < Cov->size1; i++) {
        u = dvget(mean1, i);
        for(j = 0; j < Cov->size2; j++) {
            v = dvget(mean2, j);
            sum = 0;
            for(k = 0; k < m; k++) {
                x = dmget(A, i, k);
                y = dmget(B, j, k);
                sum += (x - u) * (y - v);
            }
            sum /= (float)(m - 1);
            dmset(Cov, i, j, sum);
        }
    }
    gsl_vector_free(mean1);
    gsl_vector_free(mean2);
    return Cov;
}



gsl_matrix *
dTcovariance(gsl_matrix *A, gsl_matrix *B, gsl_matrix *Cov) {
    gsl_matrix *AA = NULL, *BB = NULL;
    AA = dtranspose(A, AA);
    BB = dtranspose(B, BB);
    Cov = dcovariance(AA, BB, Cov);
    gsl_matrix_free(AA);
    gsl_matrix_free(BB);
    return Cov;
}




