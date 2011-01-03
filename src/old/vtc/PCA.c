/*
** Principal components analysis,
** return <numcomponents> components
**
** G.Lohmann, Oct 2005
*/
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include "gsl_utils.h"

extern void printmat(gsl_matrix *);

/*
** get mean
*/
gsl_vector *
GComputeMean(gsl_matrix *src, gsl_vector *mean) {
    int i, j, n, dim;
    double sum;
    dim = src->size1;
    n   = src->size2;
    if(mean == NULL || mean->size != dim)
        mean = gsl_vector_alloc(dim);
    for(i = 0; i < dim; i++) {
        sum = 0;
        for(j = 0; j < n; j++) {
            sum += dmget(src, i, j);
        }
        dvset(mean, i, sum / (double)n);
    }
    return mean;
}



/*
** centering, subtract mean
*/
gsl_matrix *
GSubMean(gsl_matrix *src, gsl_vector *mean, gsl_matrix *dest) {
    int i, j, n, dim;
    double u;
    dim = src->size1;
    n   = src->size2;
    if(dest == NULL)
        dest = gsl_matrix_alloc(dim, n);
    for(i = 0; i < dim; i++) {
        for(j = 0; j < n; j++) {
            u = dmget(src, i, j) - dvget(mean, i);
            dmset(dest, i, j, u);
        }
    }
    return dest;
}


/*
** add mean
*/
gsl_matrix *
GAddMean(gsl_matrix *src, gsl_vector *mean, gsl_matrix *dest) {
    int i, j, n, dim;
    double u;
    dim = src->size1;
    n   = src->size2;
    if(dest == NULL)
        dest = gsl_matrix_alloc(dim, n);
    for(i = 0; i < dim; i++) {
        for(j = 0; j < n; j++) {
            u = dmget(src, i, j) + dvget(mean, i);
            dmset(dest, i, j, u);
        }
    }
    return dest;
}



/*
** get eigenvectors of covariance matrix
*/
gsl_matrix *
GEigenCovariance(gsl_matrix *A, gsl_matrix *E, gsl_vector *eval) {
    int m;
    gsl_matrix *cov = NULL;
    gsl_eigen_symmv_workspace *w = NULL;
    extern gsl_matrix *dcovariance(gsl_matrix *, gsl_matrix *, gsl_matrix *);
    cov = dcovariance(A, A, NULL);
    m = A->size1;
    if(E == NULL)
        E = gsl_matrix_alloc(m, m);
    w = gsl_eigen_symmv_alloc(m);
    gsl_eigen_symmv(cov, eval, E, w);
    gsl_eigen_symmv_sort(eval, E, GSL_EIGEN_SORT_VAL_DESC);
    gsl_matrix_free(cov);
    return E;
}



/*
** get eigenvectors of covariance matrix via SVD
*/
gsl_matrix *
GPCA_SVD(gsl_matrix *A, gsl_matrix *V, gsl_vector *w) {
    int i, j, m, n;
    gsl_matrix *U = NULL;
    gsl_vector *work = NULL;
    double u, mx, *ptr;
    n = A->size1;
    m = A->size2;
    if(V == NULL)
        V = gsl_matrix_alloc(n, n);
    U = gsl_matrix_alloc(m, n);
    for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            dmset(U, j, i, dmget(A, i, j));
        }
    }
    if(m > n) {
        work = gsl_vector_alloc(n);
        gsl_linalg_SV_decomp(U, V, w, work);
        gsl_vector_free(work);
    } else {
        gsl_linalg_SV_decomp_jacobi(U, V, w);
    }
    /* eigenvalues of covariance matrix are the squares of the singular values */
    mx = (double)(m - 1);
    ptr = w->data;
    for(i = 0; i < n; i++) {
        u = *ptr;
        *ptr++ = (u * u) / mx;
    }
    gsl_matrix_free(U);
    return V;
}




/*
** principal components
*/
gsl_matrix *
GPCA(gsl_matrix *A, gsl_matrix *E, gsl_matrix *B, int numcomponents) {
    int i, j, k, n, m;
    double u, v, sum;
    m = A->size1;
    n = A->size2;
    if(B == NULL) {
        B = gsl_matrix_alloc(numcomponents, n);
    } else if(B->size1 != numcomponents || B->size2 != n) {
        gsl_matrix_free(B);
        B = gsl_matrix_alloc(numcomponents, n);
    }
    gsl_matrix_set_zero(B);
    for(i = 0; i < n; i++) {
        for(j = 0; j < numcomponents; j++) {
            sum = 0;
            for(k = 0; k < m; k++) {
                u = dmget(E, k, j);
                v = dmget(A, k, i);
                sum += u * v;
            }
            dmset(B, j, i, sum);
        }
    }
    return B;
}


/*
** zero out last column vectors
*/
void
GZeroColumns(gsl_matrix *A, int numcomponents) {
    int i, j;
    for(i = numcomponents; i < A->size2; i++) {
        for(j = 0; j < A->size1; j++) {
            dmset(A, j, i, 0);
        }
    }
}




/*
**  PCA = E^T A
*/
VImage
VPCA(VImage src, VImage dest, int numcomponents, double *percent) {
    int i, j;
    double sum1, sum2;
    gsl_matrix *A = NULL, *B = NULL, *E = NULL;
    gsl_vector *mean = NULL, *ev = NULL;
    extern gsl_matrix *vista2gsl(VImage, gsl_matrix *);
    if(numcomponents < 0)
        numcomponents = VImageNRows(src);
    A   = vista2gsl(src, NULL);
    ev = gsl_vector_alloc(A->size1);
    mean = GComputeMean(A, NULL);   /* get mean */
    B    = GSubMean(A, mean, NULL);   /* centering */
    E    = GPCA_SVD(B, NULL, ev);   /* get pca using svd */
    /*
    E   = GEigenCovariance(B,NULL,ev);
    */
    GZeroColumns(E, numcomponents);
    A    = dmatT_x_mat(E, B, A);
    gsl_matrix_free(B);
    gsl_matrix_free(E);
    /* get percentage of variance explained by first eigenvectors */
    sum1 = 0;
    for(i = 0; i < A->size1; i++)
        sum1 += dvget(ev, i);
    sum2 = 0;
    for(i = 0; i < numcomponents; i++)
        sum2 += dvget(ev, i);
    *percent = sum2 / sum1;
    /* create vista output image */
    dest = VCreateImage(2, numcomponents, A->size2, VFloatRepn);
    VFillImage(dest, VAllBands, 0);
    for(i = 0; i < numcomponents; i++) {
        for(j = 0; j < A->size2; j++) {
            VPixel(dest, 0, i, j, VFloat) = (float) dmget(A, i, j);
        }
    }
    if(VImageNBands(src) > 1) {
        for(j = 0; j < A->size2; j++)
            VPixel(dest, 1, 0, j, VFloat) = VPixel(src, 1, 0, j, VFloat);
    }
    gsl_matrix_free(A);
    gsl_vector_free(ev);
    return dest;
}





/*
**   E E^T (A - mean)
*/
VImage
VPCAInverse(VImage src, VImage dest, int numcomponents, double *percent) {
    gsl_matrix *A = NULL, *B = NULL, *E = NULL;
    gsl_vector *mean = NULL, *ev = NULL;
    double sum1, sum2;
    int i;
    extern gsl_matrix *vista2gsl(VImage, gsl_matrix *);
    extern VImage gsl2vista(gsl_matrix *, VImage);
    if(numcomponents < 0)
        numcomponents = VImageNRows(src);
    A   = vista2gsl(src, NULL);
    ev = gsl_vector_alloc(A->size1);
    mean = GComputeMean(A, NULL);
    B   = GSubMean(A, mean, NULL);
    E   = GPCA_SVD(B, NULL, ev);
    /*
    E   = GEigenCovariance(B,NULL,ev);
    */
    GZeroColumns(E, numcomponents);
    A   = dmatT_x_mat(E, B, A);
    B   = dmat_x_mat(E, A, B);
    A   = GAddMean(B, mean, A);
    /* get percentage of variance explained by first eigenvectors */
    sum1 = 0;
    for(i = 0; i < A->size1; i++)
        sum1 += dvget(ev, i);
    sum2 = 0;
    for(i = 0; i < numcomponents; i++)
        sum2 += dvget(ev, i);
    *percent = sum2 / sum1;
    /* output */
    dest = gsl2vista(A, dest);
    gsl_matrix_free(A);
    gsl_matrix_free(B);
    gsl_matrix_free(E);
    gsl_vector_free(ev);
    return dest;
}



/*
**  PCA = E^T A
*/
VImage
VEigenvectors(VImage src, VImage dest, int numcomponents, double *percent) {
    int i, j;
    double sum1, sum2;
    gsl_matrix *A = NULL, *B = NULL, *E = NULL;
    gsl_vector *mean = NULL, *ev = NULL;
    extern gsl_matrix *vista2gsl(VImage, gsl_matrix *);
    if(numcomponents < 0)
        numcomponents = VImageNRows(src);
    A    = vista2gsl(src, NULL);
    ev = gsl_vector_alloc(A->size1);
    mean = GComputeMean(A, NULL);
    B    = GSubMean(A, mean, NULL);
    E    = GPCA_SVD(B, NULL, ev);
    /*
    E   = GEigenCovariance(B,NULL,ev);
    */
    dest = VCreateImage(2, E->size1, numcomponents, VFloatRepn);
    for(i = 0; i < E->size1; i++) {
        for(j = 0; j < numcomponents; j++) {
            VPixel(dest, 0, i, j, VFloat) = dmget(E, i, j);
        }
    }
    if(VImageNBands(src) > 1) {
        for(j = 0; j < A->size2; j++)
            VPixel(dest, 1, 0, j, VFloat) = VPixel(src, 1, 0, j, VFloat);
    }
    /* get percentage of variance explained by first eigenvectors */
    sum1 = 0;
    for(i = 0; i < A->size1; i++)
        sum1 += dvget(ev, i);
    sum2 = 0;
    for(i = 0; i < numcomponents; i++)
        sum2 += dvget(ev, i);
    *percent = sum2 / sum1;
    gsl_matrix_free(A);
    gsl_matrix_free(B);
    gsl_matrix_free(E);
    gsl_vector_free(ev);
    return dest;
}


/*
**  modes of variation, active shape models, cid=component id
*/
VImage
VarModes(VImage src, VImage dest, int cid, double *percent) {
    int i, j, nsamples, numcomponents = 0;
    double sum1, sum2;
    double u, v, e;
    gsl_matrix *A = NULL, *B = NULL, *E = NULL;
    gsl_vector *mean = NULL, *ev = NULL;
    extern gsl_matrix *vista2gsl(VImage, gsl_matrix *);
    A    = vista2gsl(src, NULL);
    ev   = gsl_vector_alloc(A->size1);
    mean = GComputeMean(A, NULL);
    B    = GSubMean(A, mean, NULL);
    E    = GPCA_SVD(B, NULL, ev);
    /* get percentage of variance explained by first eigenvector */
    numcomponents = 1;
    sum1 = 0;
    for(i = 0; i < A->size1; i++)
        sum1 += dvget(ev, i);
    sum2 = 0;
    for(i = 0; i < numcomponents; i++)
        sum2 += dvget(ev, i);
    *percent = sum2 / sum1;
    fprintf(stderr, " total variance:  %.2f\n", sum1);
    /* modes of variation */
    nsamples = 3;
    dest = VCreateImage(2, E->size1, nsamples, VFloatRepn);
    VFillImage(dest, VAllBands, 0);
    v = -2;
    for(j = 0; j < nsamples; j++) {
        for(i = 0; i < E->size1; i++) {
            u = dmget(E, i, cid);
            e = dvget(ev, cid);
            VPixel(dest, 0, i, j, VFloat) = dvget(mean, i) + v * u * sqrt(e);
        }
        v += 2;
    }
    gsl_matrix_free(A);
    gsl_matrix_free(B);
    gsl_matrix_free(E);
    gsl_vector_free(ev);
    return dest;
}
