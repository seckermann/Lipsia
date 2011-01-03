

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <via.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>


#define ABS(x) ((x) > 0 ? (x) : -(x))
#define PI 3.14159265

struct my_params {
    int    nslices;
    int    nrows;
    int    ncols;
};


extern double func2(gsl_vector *, void *params);



extern VImage s1, s2;
extern int   nbands1, nrows1, ncols1;
extern int   nbands2, nrows2, ncols2;
extern float slicedist;
extern float xminval, xmaxval;
extern float rot[3][3];
extern int   bmin, rmin, cmin;
extern int   bmax, rmax, cmax;
extern float ave1, ave2;
extern float center[3];
extern int   verbose;




void
VGetRefineTrans(VImage ref, VImage src, int minval, int maxval,
                float pitch, float roll, float yaw, float shiftcorr,
                float offset, VImage *transform) {
    long k, b, r, c, n = 6, step2;
    float sb, sr, sc, range;
    float theta0, phi0, psi0, step1;
    float theta1 = 0, phi1 = 0, psi1 = 0, sb1 = 0, sr1 = 0, sc1 = 0;
    float sb0, sr0, sc0;
    VFloat d, dmin, nx, u, deg;
    gsl_vector *p = NULL;
    struct my_params params;
    deg = (float) 180.0 / (float) PI;
    xminval = minval;
    xmaxval = maxval;
    s1 = ref;
    s2 = src;
    nbands1 = VImageNBands(ref);
    nrows1  = VImageNRows(ref);
    ncols1  = VImageNColumns(ref);
    nbands2 = VImageNBands(src);
    nrows2  = VImageNRows(src);
    ncols2  = VImageNColumns(src);
    if(VPixelRepn(ref) != VUByteRepn)
        VError(" ref must be ubyte");
    if(VPixelRepn(src) != VUByteRepn)
        VError(" src must be ubyte");
    if(offset < 1)
        offset = 1;
    slicedist = offset;    /* distance between slices */
    center[0] = (float) nbands1 * 0.5;    /* center of rotation */
    center[1] = (float) nrows1 * 0.5;
    center[2] = (float) ncols1 * 0.5;
    bmin = rmin = cmin = VRepnMaxValue(VShortRepn);
    bmax = rmax = cmax = 0;
    ave2 = nx = 0;
    for(b = 0; b < nbands2; b++) {
        for(r = 0; r < nrows2; r++) {
            for(c = 0; c < ncols2; c++) {
                u = VPixel(src, b, r, c, VUByte);
                if(u > xminval && u < xmaxval) {
                    if(r > rmax)
                        rmax = r;
                    if(c > cmax)
                        cmax = c;
                    if(r < rmin)
                        rmin = r;
                    if(c < cmin)
                        cmin = c;
                    ave2 += u;
                    nx++;
                }
            }
        }
    }
    ave2 /= nx;
    bmin = rmin = cmin = 0;
    bmax = nbands2;
    rmax = nrows2;
    cmax = ncols2;
    ave1 = nx = 0;
    step2 = nbands2 + 2;
    for(b = 0; b < nbands1; b += (int)offset) {
        for(r = 0; r < nrows1; r += step2) {
            for(c = 0; c < ncols1; c += step2) {
                u = VPixel(ref, b, r, c, VUByte);
                if(u > minval && u < xmaxval) {
                    ave1 += u;
                    nx++;
                }
            }
        }
    }
    ave1 /= nx;
    /*
    ** refine shift and rotation
    */
    range = 4;
    step1 = 2;
    psi1   = psi0   = pitch / deg;
    phi1   = phi0   = roll / deg;
    theta1 = theta0 = yaw / deg;
    sb0 = sb1 = VPixel((*transform), 0, 0, 0, VDouble);
    sr0 = sr1 = VPixel((*transform), 0, 1, 0, VDouble);
    sc0 = sc1 = VPixel((*transform), 0, 2, 0, VDouble);
    sb0 += shiftcorr;
    sb1 = sb0;
    n = 6;
    p = gsl_vector_calloc(n);
    gsl_vector_set(p, 0, sb1);
    gsl_vector_set(p, 1, sr1);
    gsl_vector_set(p, 2, sc1);
    gsl_vector_set(p, 3, psi0);
    gsl_vector_set(p, 4, psi0);
    gsl_vector_set(p, 5, theta0);
    d = func2(p, &params);
    if(verbose >= 1)
        fprintf(stderr, " %6.2f  %6.2f  %6.2f,  %6.2f  %6.2f  %6.2f,  corr: %7.4f\n",
                sb0, sr0, sc0, psi0 * deg, phi0 * deg, theta0 * deg, sqrt((double)ABS(d)));
    dmin = d;
    for(k = 0; k < 2; k++) {
        if(verbose >= 1)
            fprintf(stderr, " range: %g,  step: %g\n", range, step1);
        for(sb = -range + sb0; sb <= range + sb0; sb += step1) {
            for(sr = -range + sr0; sr <= range + sr0; sr += step1) {
                for(sc = -range + sc0; sc <= range + sc0; sc += step1) {
                    gsl_vector_set(p, 0, sb);
                    gsl_vector_set(p, 1, sr);
                    gsl_vector_set(p, 2, sc);
                    d = func2(p, &params);
                    if(d < dmin) {
                        dmin = d;
                        sb1 = sb;
                        sr1 = sr;
                        sc1 = sc;
                        if(verbose >= 1)
                            fprintf(stderr, " %6.2f  %6.2f  %6.2f,  %6.2f  %6.2f  %6.2f,  corr: %7.4f\n",
                                    sb, sr, sc, psi0 * deg, phi0 * deg, theta0 * deg, sqrt((double)ABS(dmin)));
                    }
                }
            }
        }
        range = 2;
        step1 = 1;
    }
    if(verbose >= 1)
        fprintf(stderr, " %6.2f  %6.2f  %6.2f,  %6.2f  %6.2f  %6.2f,  corr: %7.4f\n",
                sb1, sr1, sc1, psi1 * deg, phi1 * deg, theta1 * deg, sqrt((double)ABS(dmin)));
    /*
    ** output
    */
    VPixel((*transform), 0, 0, 0, VDouble) = sb1 - shiftcorr;
    VPixel((*transform), 0, 1, 0, VDouble) = sr1;
    VPixel((*transform), 0, 2, 0, VDouble) = sc1;
}




void
VRegRefine(VImage ref, VImage src, VImage transform, int band1, int band2) {
    VImage src1 = NULL, src2 = NULL;
    VString str;
    VUByte *src_pp, *dest_pp;
    int nbands1, nrows1, ncols1;
    int nbands3, i, b, bb, npix;
    float shiftcorr;
    float pitch, roll, yaw;
    float scaleb, scaler, scalec, offset;
    int minval = 50;
    int maxval = 252;
    float scale2d[2], shift2d[2];
    nbands1 = VImageNBands(ref);
    nrows1  = VImageNRows(ref);
    ncols1  = VImageNColumns(ref);
    fprintf(stderr, "\n\n refine...\n");
    if(VGetAttr(VImageAttrList(src), "voxel", NULL, VStringRepn, (VPointer) & str) != VAttrFound)
        VError(" attribute 'voxel' missing in input file.");
    sscanf(str, "%f %f %f", &scalec, &scaler, &scaleb);
    offset = scaleb;
    if(VGetAttr(VImageAttrList(transform), "rotation", NULL,
                VStringRepn, (VPointer) & str) != VAttrFound)
        VError(" attribute 'rotation' missing in transformation matrix.");
    sscanf(str, "%f %f %f", &pitch, &roll, &yaw);
    shift2d[0] = (float)nrows1 * 0.5 - scaler * (float)VImageNRows(src) * 0.5;
    shift2d[1] = (float)ncols1 * 0.5 - scalec * (float)VImageNColumns(src) * 0.5;
    scale2d[0] = scaler;
    scale2d[1] = scalec;
    src1 = VBiLinearScale2d(src, NULL, nrows1, ncols1, shift2d, scale2d);
    shiftcorr = (VImageNBands(src) - 1 - band2) * offset;
    /*
    ** reshuffle slices
    */
    nbands3 = band2 - band1 + 1;
    src2 = VCreateImage(nbands3, VImageNRows(src1), VImageNColumns(src1), VUByteRepn);
    VFillImage(src2, VAllBands, 0);
    npix = VImageNRows(src1) * VImageNColumns(src1);
    bb = nbands3 - 1;
    for(b = band1; b <= band2; b++) {
        dest_pp = (VUByte *) VPixelPtr(src2, bb, 0, 0);
        src_pp  = (VUByte *) VPixelPtr(src1, b, 0, 0);
        for(i = 0; i < npix; i++) {
            *dest_pp++ = *src_pp++;
        }
        bb--;
    }
    /* do refinement */
    VGetRefineTrans(ref, src2, minval, maxval, pitch, roll, yaw, shiftcorr, offset, &transform);
}

