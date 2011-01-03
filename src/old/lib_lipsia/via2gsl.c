#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include "gsl_utils.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


gsl_matrix *
vista2gsl(VImage src, gsl_matrix *dest) {
    int i, j, n, m;
    n = VImageNRows(src);
    m = VImageNColumns(src);
    if(dest == NULL) {
        dest = gsl_matrix_alloc(n, m);
    } else if(dest->size1 != n || dest->size2 != m) {
        gsl_matrix_free(dest);
        dest = gsl_matrix_alloc(n, m);
    }
    for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            dmset(dest, i, j, (double)VGetPixel(src, 0, i, j));
        }
    }
    return dest;
}


VImage
gsl2vista(gsl_matrix *src, VImage dest) {
    int i, j, n, m;
    n = src->size1;
    m = src->size2;
    dest = VSelectDestImage("gsl2vista", dest, 1, n, m, VFloatRepn);
    for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            VPixel(dest, 0, i, j, VFloat) = dmget(src, i, j);
        }
    }
    return dest;
}


gsl_matrix_float *
fvista2gsl(VImage src, gsl_matrix_float *dest) {
    int i, j, n, m;
    n = VImageNRows(src);
    m = VImageNColumns(src);
    if(dest == NULL) {
        dest = gsl_matrix_float_alloc(n, m);
    } else if(dest->size1 != n || dest->size2 != m) {
        gsl_matrix_float_free(dest);
        dest = gsl_matrix_float_alloc(n, m);
    }
    for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            fmset(dest, i, j, (float)VGetPixel(src, 0, i, j));
        }
    }
    return dest;
}


VImage
fgsl2vista(gsl_matrix_float *src, VImage dest) {
    int i, j, n, m;
    n = src->size1;
    m = src->size2;
    dest = VSelectDestImage("gsl2vista", dest, 1, n, m, VFloatRepn);
    for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            VPixel(dest, 0, i, j, VFloat) = fmget(src, i, j);
        }
    }
    return dest;
}

