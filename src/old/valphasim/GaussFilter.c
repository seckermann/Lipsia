/*
** 3D spatial gaussian filter
**
** G.Lohmann, MPI-CBS, Jan 2007
*/
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define NSLICES 256


/* Gaussian function */
double
xsgauss(double x, double sigma) {
    double y, z, a = 2.506628273;
    z = x / sigma;
    y = exp(-z * z * 0.5) / (sigma * a);
    return y;
}


VImage
VSGaussKernel(double sigma) {
    int    i, dim, n;
    double x, u, step;
    VImage kernel = NULL;
    double sum;
    if(sigma < 0.0001)
        return NULL;
    dim  = 3.0 * sigma + 1;
    n    = 2 * dim + 1;
    step = 1;
    kernel = VCreateImage(1, 1, n, VFloatRepn);
    sum = 0;
    x = -(float)dim;
    for(i = 0; i < n; i++) {
        u = xsgauss(x, sigma);
        sum += u;
        VPixel(kernel, 0, 0, i, VFloat) = u;
        x += step;
    }
    /* normalize */
    for(i = 0; i < n; i++) {
        u = VPixel(kernel, 0, 0, i, VFloat);
        u /= sum;
        VPixel(kernel, 0, 0, i, VFloat) = u;
    }
    return kernel;
}



VImage
VSConvolveCol(VImage src, VImage dest, VImage kernel) {
    int b, r, c, nbands, nrows, ncols;
    int c0, c1, cc;
    float sum, x;
    VFloat *float_pp;
    int dim, d;
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    nbands = VImageNBands(src);
    dest = VSelectDestImage("VConvolveCol", dest, nbands, nrows, ncols, VFloatRepn);
    dim  = VImageNColumns(kernel);
    d    = dim / 2;
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = d; c < ncols - d; c++) {
                float_pp = (VFloat *) VImageData(kernel);
                sum = 0;
                c0 = c - d;
                c1 = c + d;
                for(cc = c0; cc <= c1; cc++) {
                    x = VPixel(src, b, r, cc, VFloat);
                    sum += x * (*float_pp++);
                }
                VPixel(dest, b, r, c, VFloat) = sum;
            }
        }
    }
    return dest;
}


VImage
VSConvolveRow(VImage src, VImage dest, VImage kernel) {
    int b, r, c, nbands, nrows, ncols;
    int r0, r1, rr;
    float sum, x;
    VFloat *float_pp;
    int d, dim;
    dim = VImageNColumns(kernel);
    d = dim / 2;
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    nbands = VImageNBands(src);
    dest = VSelectDestImage("VConvolveRow", dest, nbands, nrows, ncols, VFloatRepn);
    for(b = 0; b < nbands; b++) {
        for(r = d; r < nrows - d; r++) {
            for(c = 0; c < ncols; c++) {
                float_pp = (VFloat *) VImageData(kernel);
                sum = 0;
                r0 = r - d;
                r1 = r + d;
                for(rr = r0; rr <= r1; rr++) {
                    x = VPixel(src, b, rr, c, VFloat);
                    sum += x * (*float_pp++);
                }
                VPixel(dest, b, r, c, VFloat) = sum;
            }
        }
    }
    return dest;
}


VImage
VSConvolveBand(VImage src, VImage dest, VImage kernel) {
    int b, r, c, nbands, nrows, ncols;
    int b0, b1, bb;
    float sum, x;
    VFloat *float_pp;
    int d, dim;
    dim = VImageNColumns(kernel);
    d = dim / 2;
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    nbands = VImageNBands(src);
    dest = VSelectDestImage("VConvolveBand", dest, nbands, nrows, ncols, VFloatRepn);
    for(b = d; b < nbands - d; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                float_pp = (VFloat *) VImageData(kernel);
                sum = 0;
                b0 = b - d;
                b1 = b + d;
                for(bb = b0; bb <= b1; bb++) {
                    x = VPixel(src, bb, r, c, VFloat);
                    sum += x * (*float_pp++);
                }
                VPixel(dest, b, r, c, VFloat) = sum;
            }
        }
    }
    return dest;
}



VImage
VGauss3d(VImage src, VImage dest, VImage kernel) {
    static VImage tmp = NULL;
    dest = VSConvolveCol(src, dest, kernel);
    tmp  = VSConvolveRow(dest, tmp, kernel);
    dest = VSConvolveBand(tmp, dest, kernel);
    return dest;
}


void
VZeroBorders(VImage src, VImage kernel) {
    int b, r, c, nbands, nrows, ncols, d, dim;
    /* zero out borders */
    nbands = VImageNBands(src);
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    dim = VImageNColumns(kernel);
    d = dim / 2;
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c <= d; c++)
                VPixel(src, b, r, c, VBit) = 0;
            for(c = ncols - d; c < ncols; c++)
                VPixel(src, b, r, c, VBit) = 0;
        }
    }
    for(b = 0; b < nbands; b++) {
        for(c = 0; c < ncols; c++) {
            for(r = 0; r <= d; r++)
                VPixel(src, b, r, c, VBit) = 0;
            for(r = nrows - d; r < nrows; r++)
                VPixel(src, b, r, c, VBit) = 0;
        }
    }
    for(r = 0; r < nrows; r++) {
        for(c = 0; c < ncols; c++) {
            for(b = 0; b <= d; b++)
                VPixel(src, b, r, c, VBit) = 0;
            for(b = nbands - d; b < nbands; b++)
                VPixel(src, b, r, c, VBit) = 0;
        }
    }
}
