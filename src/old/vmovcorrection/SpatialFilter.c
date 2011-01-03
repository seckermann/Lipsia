/****************************************************************
 *
 * vpreprocess: SpatialFilter.c
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id: SpatialFilter.c 3190 2008-04-01 16:06:57Z karstenm $
 *
 *****************************************************************/

#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define NSLICES 2000
#define ABS(x) ((x) < 0 ? -(x) : (x))


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
    VFillImage(dest, VAllBands, 0);
    dim  = VImageNColumns(kernel);
    d    = dim / 2;
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = d; c < ncols - d; c++) {
                float_pp = (VFloat *) VImageData(kernel);
                sum = 0;
                c0 = c - d;
                c1 = c + d;
                if(c0 < 0)
                    c0 = 0;
                if(c1 >= ncols)
                    c1 = ncols - 1;
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
                if(r0 < 0)
                    r0 = 0;
                if(r1 >= nrows)
                    r1 = nrows - 1;
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


void
VSpatialFilter1(VAttrList list) {
    VAttrListPosn posn;
    VImage src[NSLICES], xsrc = NULL, tmp2d = NULL;
    int b, i, size;
    int n, nslices, nrows, ncols;
    double sigma = 0;
    extern VImage VGaussianConv(VImage, VImage, VBand, double, int);
    /* get image dimensions */
    n = i = nrows = ncols = 0;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(i >= NSLICES)
            VError(" too many slices");
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & xsrc);
        if(VPixelRepn(xsrc) != VShortRepn)
            continue;
        if(VImageNBands(xsrc) > n)
            n = VImageNBands(xsrc);
        if(VImageNRows(xsrc) > nrows)
            nrows = VImageNRows(xsrc);
        if(VImageNColumns(xsrc) > ncols)
            ncols = VImageNColumns(xsrc);
        src[i] = xsrc;
        i++;
    }
    nslices = i;
    /*
    ** 2D gauss filtering
    */
    sigma = 0.5;
    size = (int)(6.0 * sigma + 1.5);
    if((size & 1) == 0)
        size++;
    for(b = 0; b < nslices; b++) {
        if(VImageNRows(src[b]) < 2)
            continue;
        tmp2d  = VGaussianConv(src[b], tmp2d, VAllBands, sigma, size);
        src[b] = VCopyImagePixels(tmp2d, src[b], VAllBands);
    }
    VDestroyImage(tmp2d);
}
