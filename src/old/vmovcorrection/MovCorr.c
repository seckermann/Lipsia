/*
** motion correction using 6 degrees of freedom
**
** (3 translational + 3 rotational)
**
** G.Lohmann, MPI-CBS, April 2007
*/
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SQR(x) ((x)*(x))

extern void InverseRotationMatrix(double pitch, double yaw, double roll, double ainv[3][3]);
extern void VApplyTransform3d(VImage *, int, int, int, int, gsl_vector *);

int verbose = 0;

#define NSLICES 2500
VImage src[NSLICES];

struct my_params {
    int    i1;           /* reference time step */
    int    i2;           /* current time step */
    int    nslices;
    int    nrows;
    int    ncols;
    int    step;
};


/*
** goal function to minimize
*/
double
my_f(const gsl_vector *vec, void *params) {
    struct my_params *par = params;
    double t[3];
    double ainv[3][3], pitch, yaw, roll;
    int    b, r, c, b0, bb, rr, cc, nslices, nrows, ncols;
    int    i1, i2, step, q;
    double u, v, w[8], wsum;
    double cx, rx, bx;
    double sum, nx;
    double center[3];
    double c1, c2, r1, r2, b1, b2;
    double bp, rp, cp;
    double deg, pi = 3.14159265;
    double xmax = VRepnMaxValue(VDoubleRepn);
    double ssx, ssy, sxx, syy, sxy, tiny = 1.0e-6;
    xmax    = 1;
    i1      = par->i1;
    i2      = par->i2;
    nslices = par->nslices;
    nrows   = par->nrows;
    ncols   = par->ncols;
    step    = par->step;
    center[0] = (double)nslices * 0.5;
    center[1] = (double)nrows * 0.5;
    center[2] = (double)ncols * 0.5;
    deg    =  180.0f / pi;
    pitch  = gsl_vector_get(vec, 0) / deg;
    yaw    = gsl_vector_get(vec, 1) / deg;
    roll   = gsl_vector_get(vec, 2) / deg;
    t[0]   = gsl_vector_get(vec, 3);
    t[1]   = gsl_vector_get(vec, 4);
    t[2]   = gsl_vector_get(vec, 5);
    InverseRotationMatrix(pitch, yaw, roll, ainv);
    sum = nx = 0;
    sxx = syy = sxy = ssx = ssy = 0;
    b0 = 0;
    if(nslices > 12)
        b0 = 1;
    for(b = b0; b < nslices - b0; b++) {
        if(VImageNRows(src[b]) < 2)
            continue;
        q = 1;
        if(b % 2 == 0)
            q = 0;
        for(r = q + 2; r < nrows - 2; r += step) {
            for(c = q + 2; c < ncols - 2; c += step) {
                u = VPixel(src[b], i1, r, c, VShort);
                bx = (double) b - t[0];
                rx = (double) r - t[1];
                cx = (double) c - t[2];
                bx -= center[0];
                rx -= center[1];
                cx -= center[2];
                bp = ainv[0][0] * bx + ainv[0][1] * rx + ainv[0][2] * cx;
                rp = ainv[1][0] * bx + ainv[1][1] * rx + ainv[1][2] * cx;
                cp = ainv[2][0] * bx + ainv[2][1] * rx + ainv[2][2] * cx;
                bp += center[0];
                rp += center[1];
                cp += center[2];
                if(cp > -0.01 && cp < 0)
                    cp = 0;
                if(rp > -0.01 && rp < 0)
                    rp = 0;
                if(bp > -0.01 && bp < 0)
                    bp = 0;
                bb = (int)bp;
                rr = (int)rp;
                cc = (int)cp;
                /* check subcube */
                if(rr < 0 || rr >= nrows - 1)
                    continue;
                if(cc < 0 || cc >= ncols - 1)
                    continue;
                if(bb < 0 || bb >= nslices)
                    continue;
                if(VImageNRows(src[bb]) < 2)
                    continue;
                /* compute fractions of subcube */
                c1 = cp  - (double)cc;
                c2 = 1.0 - c1;
                r1 = rp  - (double)rr;
                r2 = 1.0 - r1;
                b1 = bp  - (double)bb;
                b2 = 1.0 - b1;
                if(b1 < 0) {
                    b1 = 0;
                    b2 = 1;
                }
                if(b2 < 0) {
                    b1 = 1;
                    b2 = 0;
                }
                w[0] = b2 * r2 * c2;
                w[1] = b2 * r2 * c1;
                w[2] = b2 * r1 * c2;
                w[3] = b2 * r1 * c1;
                w[4] = b1 * r2 * c2;
                w[5] = b1 * r2 * c1;
                w[6] = b1 * r1 * c2;
                w[7] = b1 * r1 * c1;
                v = wsum = 0;
                if(bb >= 0 && bb < nslices) {
                    v += w[0] * VPixel(src[bb], i2, rr, cc, VShort);
                    v += w[1] * VPixel(src[bb], i2, rr, cc + 1, VShort);
                    v += w[2] * VPixel(src[bb], i2, rr + 1, cc, VShort);
                    v += w[3] * VPixel(src[bb], i2, rr + 1, cc + 1, VShort);
                    wsum += w[0] + w[1] + w[2] + w[3];
                }
                if(bb + 1 >= 0 && bb + 1 <= nslices - 1) {
                    v += w[4] * VPixel(src[bb + 1], i2, rr, cc, VShort);
                    v += w[5] * VPixel(src[bb + 1], i2, rr, cc + 1, VShort);
                    v += w[6] * VPixel(src[bb + 1], i2, rr + 1, cc, VShort);
                    v += w[7] * VPixel(src[bb + 1], i2, rr + 1, cc + 1, VShort);
                    wsum += w[4] + w[5] + w[6] + w[7];
                }
                if(wsum < 0.999 && wsum > 0)
                    v += (1.0 - wsum) * v;
                ssx += u;
                ssy += v;
                sxx += u * u;
                syy += v * v;
                sxy += u * v;
                nx++;
            }
        }
    }
    if(nx < 10)
        return xmax;
    u = nx * sxx - ssx * ssx;
    v = nx * syy - ssy * ssy;
    sum = 0;
    if(u * v > tiny)
        sum = (nx * sxy - ssx * ssy) / sqrt(u * v);
    return 1.0 - sum;
}



/*
** motion correction using 6 degrees of freedom
*/
VImage
VMotionCorrection3d(VAttrList list, VLong i0, VLong step, VLong maxiter) {
    VAttrListPosn posn;
    VImage xsrc = NULL, motion = NULL;
    int i, j;
    int n, nslices, nrows, ncols;
    double f0 = 0;
    const gsl_multimin_fminimizer_type *T =
        gsl_multimin_fminimizer_nmsimplex;
    gsl_multimin_fminimizer *s = NULL;
    gsl_vector *stepsizes = NULL, *x = NULL;
    gsl_multimin_function minex_func;
    int status, iter;
    double size;
    size_t np = 6;
    struct my_params params;
    /*
    ** get image dimensions
    */
    n = i = nrows = ncols = nslices = 0;
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
    if(i0 < 0 || i0 >= n)
        VError(" illegal choice of ref scan (%d), must be in [%d,%d]", i0, 0, n - 1);
    /*
    ** ini minimization method
    */
    np = 6;  /* six degrees of freedom */
    stepsizes = gsl_vector_calloc(np);
    gsl_vector_set_all(stepsizes, 1.0);
    x = gsl_vector_calloc(np);
    minex_func.f = &my_f;
    minex_func.n = np;
    minex_func.params = (void *)&params;
    s = gsl_multimin_fminimizer_alloc(T, np);
    /*
    ** alloc storage for motion parameters
    */
    motion = VCreateImage(1, n, np, VDoubleRepn);
    VFillImage(motion, VAllBands, 0);
    /*
    ** set parameters of reference time step
    */
    params.i1      = (int)i0;
    params.nslices = (int)nslices;
    params.nrows   = (int)nrows;
    params.ncols   = (int)ncols;
    params.step    = (int)step;
    /*
    ** for each time step...
    */
    fprintf(stderr, "# estimating motion parameters...\n");
    for(i = 0; i < n; i++) {
        if(i % 10 == 0)
            fprintf(stderr, "#   %6d  of %6d\r", i, n);
        /* set parameter of current time step */
        params.i2 = i;
        /* optimization using simplex method */
        if(verbose) {
            gsl_vector_set_zero(x);
            f0 = my_f(x, &params);
        }
        gsl_vector_set_zero(x);
        gsl_vector_set_all(stepsizes, 1.0);
        gsl_multimin_fminimizer_set(s, &minex_func, x, stepsizes);
        for(iter = 0; iter < maxiter; iter++) {
            status = gsl_multimin_fminimizer_iterate(s);
            if(status)
                break;
            size = gsl_multimin_fminimizer_size(s);
            status = gsl_multimin_test_size(size, 1e-6);
            if(status == GSL_SUCCESS)
                break;
            if(verbose) {
                fprintf(stderr, " %5d:  %8.6f  %8.6f,  %8.5f  %8.5f  %8.5f,  %8.5f  %8.5f  %8.5f\n", iter, f0, s->fval,
                        gsl_vector_get(s->x, 0), gsl_vector_get(s->x, 1), gsl_vector_get(s->x, 2),
                        gsl_vector_get(s->x, 3), gsl_vector_get(s->x, 4), gsl_vector_get(s->x, 5));
            }
        }
        /* save motion parameters */
        for(j = 0; j < np; j++)
            VPixel(motion, 0, i, j, VDouble) = gsl_vector_get(s->x, j);
    }
    return motion;
}



void
VApplyMotionCorrection3d(VAttrList list, VImage motion, VString reportfile) {
    VAttrListPosn posn;
    VImage xsrc = NULL;
    FILE *fp = NULL;
    VString buf;
    int i, j, n = 0, np = 6;
    int nslices, nrows, ncols;
    double t[3], pitch, yaw, roll;
    double u, xmax, vox_x, vox_y, vox_z, voz;
    gsl_vector *x;
    x = gsl_vector_calloc(np);
    verbose = 0;
    if(strlen(reportfile) > 1) {
        fp = fopen(reportfile, "w");
        if(!fp)
            VError(" error opening report file");
    } else if(verbose)
        fp = stderr;
    if(fp) {
        fprintf(fp, "#   scan             shift              pitch    yaw    roll \n");
        fprintf(fp, "#------------------------------------------------------------\n");
    }
    n = i = nrows = ncols = nslices = 0;
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
    fprintf(stderr, "# doing motion correction...\n");
    vox_x = vox_y = vox_z = voz = 1;
    if(VGetAttr(VImageAttrList(src[0]), "voxel", NULL, VStringRepn, &buf) == VAttrFound)
        sscanf(buf, "%lf %lf %lf", &vox_x, &vox_y, &vox_z);
    else
        VWarning(" parameter 'voxel' missing in header");
    voz = vox_z / vox_x;
    for(i = 0; i < n; i++) {
        xmax = 0;
        for(j = 0; j < np; j++) {
            u = VPixel(motion, 0, i, j, VDouble);
            if(ABS(u) > xmax)
                xmax = ABS(u);
            gsl_vector_set(x, j, u);
        }
        /* print output */
        if(fp) {
            pitch = VPixel(motion, 0, i, 0, VDouble);
            yaw   = VPixel(motion, 0, i, 1, VDouble);
            roll  = VPixel(motion, 0, i, 2, VDouble);
            t[0]  = VPixel(motion, 0, i, 3, VDouble);
            t[1]  = VPixel(motion, 0, i, 4, VDouble);
            t[2]  = VPixel(motion, 0, i, 5, VDouble);
            fprintf(fp, "  %5d    %7.3f %7.3f %7.3f    %7.3f %7.3f %7.3f\n",
                    i, t[2]*vox_x, t[1]*vox_y, t[0]*vox_z, pitch * voz, yaw, roll);
        }
        if(xmax < 0.0005)
            continue;
        /* apply motion correction */
        VApplyTransform3d(src, nslices, nrows, ncols, i, x);
    }
    if(fp)
        fclose(fp);
}
