/*
** get reference time course in region of interest (ROI)
**
*/

#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_sort.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))


void
ROI(VImage *src, VImage mask, int start, int minval, gsl_vector *array1) {
    int i, j, b, r, c, len;
    int nslices, nrows, ncols;
    float bx, rx, cx, sum = 0, nx = 0;
    nslices = VImageNBands(mask);
    nrows   = VImageNRows(mask);
    ncols   = VImageNColumns(mask);
    bx = rx = cx = nx = 0;
    for(b = 0; b < nslices; b++) {
        if(VImageNRows(src[b]) < 2)
            continue;
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                if(VGetPixel(mask, b, r, c) < 0.5)
                    continue;
                if(VPixel(src[b], 0, r, c, VShort) < minval)
                    continue;
                bx += b;
                rx += r;
                cx += c;
                nx++;
            }
        }
    }
    if(nx < 1)
        VError(" empty roi");
    bx /= nx;
    rx /= nx;
    cx /= nx;
    fprintf(stderr, " ROI centre of gravity:  %.2f  %.2f  %.2f,  num voxels: %.2f\n", cx, rx, bx, nx);
    len = array1->size;
    gsl_vector_set_zero(array1);
    i = 0;
    for(j = start; j < start + len; j++) {
        sum = nx = 0;
        for(b = 0; b < nslices; b++) {
            if(VImageNRows(src[b]) < 2)
                continue;
            for(r = 0; r < nrows; r++) {
                for(c = 0; c < ncols; c++) {
                    if(VGetPixel(mask, b, r, c) < 0.5)
                        continue;
                    if(VPixel(src[b], 0, r, c, VShort) < minval)
                        continue;
                    sum += VPixel(src[b], j, r, c, VShort);
                    nx++;
                }
            }
        }
        if(nx > 0)
            gsl_vector_set(array1, i, sum / nx);
        else
            VError(" empty mask at timestep %d", i);
        i++;
    }
}

