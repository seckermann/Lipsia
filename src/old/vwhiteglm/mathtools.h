#ifndef MATHTOOLS_H
#define MATHTOOLS_H

#include <gsl/gsl_matrix.h>
#include <viaio/Vlib.h>
#include "gsl_utils.h"

#define ABS(x) ((x) >= 0) ? (x) : -(x)
#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#define MAX(a,b) ((a) > (b)) ? (a) : (b)
#define ROUND(x) (int)(((x) >= 0) ? ((x) + 0.5) : ((x) - 0.5))

void prewhite(gsl_matrix_float** pinvX,
         gsl_matrix_float** invM,
         gsl_matrix_float* X,
         int* dim);

void dfs(VFloat** Dfs,
         gsl_matrix_float* pinvX,
         gsl_matrix_float* X,
         gsl_matrix_float* con,
         int* dim);

void whitecov(VImage* rho_vol,
        gsl_matrix_float* Y,
        gsl_matrix_float* invM,
        gsl_matrix_float* pinvX,
        gsl_matrix_float* X,
        int* dim,
        double doubleslice);

void whitecov2(VImage** effect_image,
        VImage** sdeffect_image,
        VImage rho_vol,
        gsl_matrix_float* Y,
        gsl_matrix_float* X,
        gsl_matrix_float* con,
        VFloat* Dfs,
        int* dim,
        int slice);

#endif
