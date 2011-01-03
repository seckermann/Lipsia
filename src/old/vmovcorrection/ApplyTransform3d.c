#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <via/via.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))
extern void InverseRotationMatrix(double pitch, double yaw, double roll, double ainv[3][3]);


void
VApplyTransform3d(VImage *src, int nslices, int nrows, int ncols, int i2, gsl_vector *vec) {
    static VImage tmp = NULL;
    VShort *ptr1, *ptr2;
    int b, r, c, bb, rr, cc, i;
    double bp, rp, cp, bx, rx, cx;
    double ainv[3][3];
    double w[8], wsum;
    double center[3];
    double shift[3], roll, pitch, yaw;
    double b1, b2, r1, r2, c1, c2, v;
    double deg, pi = 3.14159265;
    center[0] = nslices / 2;
    center[1] = nrows / 2;
    center[2] = ncols / 2;
    pitch  = gsl_vector_get(vec, 0);
    yaw    = gsl_vector_get(vec, 1);
    roll   = gsl_vector_get(vec, 2);
    shift[0] = gsl_vector_get(vec, 3);
    shift[1] = gsl_vector_get(vec, 4);
    shift[2] = gsl_vector_get(vec, 5);
    deg     =  180.0f / pi;
    pitch  /= deg;
    yaw    /= deg;
    roll   /= deg;
    InverseRotationMatrix(pitch, yaw, roll, ainv);
    /*
    ** trilinear resampling
    */
    if(tmp == NULL)
        tmp = VCreateImage(nslices, nrows, ncols, VShortRepn);
    VFillImage(tmp, VAllBands, 0);
    for(b = 0; b < nslices; b++) {
        if(VImageNRows(src[b]) < 2)
            continue;
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                v = VPixel(src[b], i2, r, c, VShort);
                VPixel(tmp, b, r, c, VShort) = v;
                bx = (double) b - shift[0];
                rx = (double) r - shift[1];
                cx = (double) c - shift[2];
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
                /* compute fractions of subcube */
                c1 = cp  - (double)cc;
                c2 = 1.0 - c1;
                r1 = rp  - (double)rr;
                r2 = 1.0 - r1;
                b1 = bp  - (double)bb;
                b2 = 1.0 - b1;
                if(c1 < 0 || r1 < 0 || b1 < 0)
                    continue;
                if(c2 < 0 || r2 < 0 || b2 < 0)
                    continue;
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
                VPixel(tmp, b, r, c, VShort) = (int)(v + 0.4999);
            }
        }
    }
    ptr1 = (VShort *) VImageData(tmp);
    for(b = 0; b < nslices; b++) {
        ptr2 = (VShort *) VPixelPtr(src[b], i2, 0, 0);
        for(i = 0; i < nrows * ncols; i++) {
            (*ptr2++) = (*ptr1++);
        }
    }
}

