/*
** general linear regression using pre-coloring
**
**
** G.Lohmann, Nov 2004
*/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/headerinfo.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>

#include <gsl_utils.h>

#define TINY 1.0e-6

#define NSLICES 2000   /* max number of image slices */
#define MBETA     50   /* max number of covariates */

#define SQR(x) ((x)*(x))
#define ABS(x) ((x) > 0 ? (x) : -(x))

extern gsl_vector *GaussKernel(double);
extern void GaussMatrix(double, gsl_matrix_float *);
extern gsl_vector_float *VectorConvolve(gsl_vector_float *, gsl_vector_float *, gsl_vector *);
extern void getLipsiaVersion(char*,size_t);
gsl_vector_float *VGlobalMean(VAttrList, VShort, VImage);


/*
** convert a gsl matrix to a 2D vista raster image
*/
VImage
Mat2Vista(gsl_matrix_float *A) {
    VImage dest = NULL;
    int i, j;
    float x;
    dest = VCreateImage(1, A->size1, A->size2, VFloatRepn);
    for(i = 0; i < A->size1; i++) {
        for(j = 0; j < A->size2; j++) {
            x = fmget(A, i, j);
            VPixel(dest, 0, i, j, VFloat) = x;
        }
    }
    return dest;
}


/*
** compute pseudoinverse: V * D^-1 * U^T
*/
void
PseudoInverse(gsl_matrix *U, gsl_matrix *V, gsl_vector *w, gsl_matrix_float *C) {
    int k, l, j, n, m;
    double x, u, wmin, wmax, tiny = TINY;
    n = C->size1;
    m = C->size2;
    gsl_matrix_float_set_zero(C);
    wmax = 0;
    for(j = 0; j < n; j++) {
        u = ABS(dvget(w, j));
        if(u > wmax)
            wmax = u;
    }
    wmin = wmax * tiny;
    if(wmin < 1.0e-8)
        wmin = 1.0e-8;
    for(k = 0; k < n; k++) {
        for(l = 0; l < m; l++) {
            for(j = 0; j < n; j++) {
                if(dvget(w, j) != 0) {
                    x = fmget(C, k, l);
                    u = dvget(w, j);
                    if(ABS(u) > wmin)
                        x += dmget(V, k, j) * dmget(U, l, j) / u;
                    fmset(C, k, l, x);
                }
            }
        }
    }
}


/*
** general linear regression
*/
VImage
VResiduals(VAttrList list, VShort minval, VImage design, VFloat sigma, VLong itr,
           VImage mask, VBoolean global_mean) {
    VImage dest = NULL;
    VAttrListPosn posn;
    int nslices = 0, nbands = 0, nrows = 0, ncols = 0, slice, row, col;
    VImage xsrc, src[NSLICES];
    VFloat *float_pp;
    float  d;
    int    i, k, l, n, m = 0, npix = 0;
    float  x, sig, sum = 0, nx = 0, mean = 0, sum2;
    float  sst, sse, ssr, u, v;
    float  *ptr1, *ptr2;
    double *double_pp;
    gsl_matrix_float *X = NULL, *XInv = NULL, *SX = NULL;
    gsl_vector_float *y, *z, *beta, *ys, *gmean = NULL;
    gsl_vector *kernel = NULL;
    gsl_matrix *U = NULL, *V = NULL;
    gsl_vector *w;
    gsl_matrix_float *S = NULL;
    gsl_set_error_handler_off();
    /* get image dimensions */
    m = k = nbands = nrows = ncols = nslices = 0;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & xsrc);
        if(VPixelRepn(xsrc) != VShortRepn) {
            VDeleteAttr(&posn);
            continue;
        }
        if(VImageNBands(xsrc) > nbands)
            nbands = VImageNBands(xsrc);
        if(VImageNRows(xsrc) > nrows)
            nrows = VImageNRows(xsrc);
        if(VImageNColumns(xsrc) > ncols)
            ncols = VImageNColumns(xsrc);
        src[k] = xsrc;
        k++;
    }
    nslices = k;
    /*
    ** create dest image mit varianz-aufklaerung
    */
    dest = VCreateImage(nslices, nrows, ncols, VFloatRepn);
    VFillImage(dest, VAllBands, 0);
    VCopyImageAttrs(src[0], dest);
    VSetAttr(VImageAttrList(dest), "modality", NULL, VStringRepn, "conimg");
    /*
    ** get design dimensions
    */
    m = VImageNRows(design);      /* number of timesteps   */
    n = VImageNColumns(design);   /* number of covariates */
    fprintf(stderr, "# ntimesteps=%d,   num covariates=%d\n", m, n);
    if(n >= MBETA)
        VError(" too many covariates (%d), max is %d", n, MBETA);
    if(m != nbands)
        VError(" design dimension inconsistency: %d %d", m, nbands);
    /*
    ** global mean per time step
    */
    if(global_mean) {
        gmean = VGlobalMean(list, minval, mask);
        n++;
    }
    /*
    ** read design matrix
    */
    X = gsl_matrix_float_calloc(m, n);
    for(k = 0; k < m; k++) {
        for(l = 0; l < n; l++) {
            x = VGetPixel(design, 0, k, l);
            fmset(X, k, l, x);
        }
    }
    /* add global mean to design matrix */
    if(global_mean) {
        for(k = 0; k < m; k++) {
            x = gsl_vector_float_get(gmean, k);
            fmset(X, k, n - 1, x);
        }
    }
    /*
    ** pre-coloring, set up K-matrix, S=K, V = K*K^T with K=S
    */
    S  = gsl_matrix_float_calloc(m, m);
    GaussMatrix((double)sigma, S);
    /*
    ** compute pseudoinverse: singular value decomp, and V * D^-1 * U^T
    */
    U    = gsl_matrix_calloc(m, n);
    V    = gsl_matrix_calloc(n, n);
    XInv = gsl_matrix_float_calloc(n, m);
    w    = gsl_vector_calloc(n);
    SX = fmat_x_mat(S, X, NULL);
    float_pp  = SX->data;
    double_pp = U->data;
    for(i = 0; i < m * n; i++)
        *double_pp++ = *float_pp++;
    gsl_linalg_SV_decomp_jacobi(U, V, w);
    PseudoInverse(U, V, w, XInv);
    gsl_matrix_free(U);
    gsl_matrix_free(V);
    /*
    ** process
    */
    ys   = gsl_vector_float_calloc(m);
    y    = gsl_vector_float_calloc(m);
    z    = gsl_vector_float_calloc(m);
    beta = gsl_vector_float_calloc(n);
    if(sigma > TINY)
        kernel = GaussKernel((double)sigma);
    npix = 0;
    for(slice = 0; slice < nslices; slice++) {
        if(slice % 5 == 10)
            fprintf(stderr, "# slice: %3d\r", slice);
        if(VImageNRows(src[slice]) < 2)
            continue;
        for(row = 0; row < nrows; row++) {
            for(col = 0; col < ncols; col++) {
                if(VPixel(src[slice], 0, row, col, VShort) < minval)
                    goto next;
                npix++;
                /* read time series data */
                sum = sum2 = nx = 0;
                ptr1 = y->data;
                for(i = 0; i < m; i++) {
                    x = VPixel(src[slice], i, row, col, VShort);
                    (*ptr1++) = x;
                    sum  += x;
                    sum2 += x * x;
                    nx++;
                }
                mean = sum / nx;
                sig = sqrt((double)((sum2 - nx * mean * mean) / (nx - 1.0)));
                /* centering and scaling, Seber, p.330 */
                ptr1 = y->data;
                for(i = 0; i < m; i++) {
                    x = ((*ptr1) - mean) / sig;
                    (*ptr1++) = x + 100.0;
                }
                /* S x y */
                if(sigma > TINY)
                    ys = VectorConvolve(y, ys, kernel);
                else
                    gsl_vector_float_memcpy(ys, y);
                /* compute beta's */
                fmat_x_vector(XInv, ys, beta);
                /* residuals */
                fmat_x_vector(SX, beta, z);
                /* subtract  */
                ptr1 = ys->data;
                ptr2 = z->data;
                for(i = 0; i < m; i++) {
                    d = ((*ptr1++) - (*ptr2++));
                    d = (d * 2000.0 + 10000.0);
                    if(isnan(d))
                        d = 0;
                    VPixel(src[slice], i, row, col, VShort) = (VShort)d;
                }
                /*  multiple coeff of determination R^2 */
                ptr1 = ys->data;
                sum = 0;
                for(i = 0; i < m; i++)
                    sum += (*ptr1++);
                mean = sum / (float)m;
                ptr1 = ys->data;
                ptr2 = z->data;
                sst = ssr = sse = 0;
                for(i = 0; i < m; i++) {
                    u = *ptr1++;
                    v = *ptr2++;
                    sst += SQR(u - mean);
                    ssr += SQR(v - mean);
                    sse += (u - v) * (u - v);
                }
                d = 1.0 - sse / sst;
                d = ssr / sst;
                VPixel(dest, slice, row, col, VFloat) = 100.0 * d;
next:
                ;
            }
        }
    }
    if(npix == 0)
        VError(" no voxels above threshold %d found", minval);
    return dest;
}





int
main(int argc, char *argv[]) {
    static VString  filename;
    static VString  mask_filename = "";
    static VShort   minval = 2000;
    static VFloat   fwhm = 4.0;
    static VBoolean global_mean = FALSE;
    static VOptionDescRec  options[] = {
        {"design", VStringRepn, 1, (VPointer) &filename, VRequiredOpt, NULL, "Design file"},
        {
            "mask", VStringRepn, 1, (VPointer) &mask_filename, VOptionalOpt, NULL,
            "Mask for global covariate"
        },
        {"fwhm", VFloatRepn, 1, (VPointer) &fwhm, VOptionalOpt, NULL, "fwhm"},
        {"global", VBooleanRepn, 1, (VPointer) &global_mean, VOptionalOpt, NULL, "global mean as confound"},
        {"minval", VShortRepn, 1, (VPointer) &minval, VRequiredOpt, NULL, "Signal threshold"}
    };
    FILE *fp = NULL, *in_file = NULL, *out_file = NULL;
    VAttrList list = NULL, list1 = NULL, mlist = NULL;
    VAttrListPosn posn;
    VImage design = NULL, dest = NULL, mask = NULL;
    VLong itr = 0;
    VFloat sigma = 0, tr = 0;
    int  n;
	char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vresiduals V%s", ver);
	fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    /* read design matrix */
    fp = VOpenInputFile(filename, TRUE);
    list1 = VReadFile(fp, NULL);
    if(! list1)
        VError("Error reading design file");
    fclose(fp);
    n = 0;
    for(VFirstAttr(list1, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & design);
        if(VPixelRepn(design) != VFloatRepn && VPixelRepn(design) != VDoubleRepn)
            continue;
        n++;
        break;
    }
    if(n == 0)
        VError(" design matrix not found ");
    /* read mask */
    if(strlen(mask_filename) > 2) {
        fp = VOpenInputFile(mask_filename, TRUE);
        mlist = VReadFile(fp, NULL);
        if(! mlist)
            VError("Error reading mask file");
        fclose(fp);
        n = 0;
        for(VFirstAttr(mlist, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & mask);
            if(VPixelRepn(mask) != VBitRepn)
                continue;
            n++;
            break;
        }
        if((VPixelRepn(mask) != VBitRepn) || (n == 0))
            VError(" mask image not found ");
    }
    /* get pre-coloring info */
    if(VGetAttr(VImageAttrList(design), "repetition_time", NULL, VLongRepn, &itr) != VAttrFound)
        VError(" TR info missing in header");
    tr = (float) itr / 1000.0;
    if(tr < 0.0001)
        VError(" repetition time unknown");
    fprintf(stderr, "# TR: %.3f seconds\n", tr);
    sigma = fwhm / sqrt(8 * log(2))  / tr;
    if(fwhm < TINY)
        sigma = 0;
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    dest = VResiduals(list, minval, design, sigma, itr, mask, global_mean);
    VAppendAttr(list, "var_explained", NULL, VImageRepn, dest);
    /* output */
    VHistory(VNumber(options), options, prg_name, &list, &list);
    if(! VWriteFile(out_file, list))
        exit(1);
    fprintf(stderr, "# %s: done.\n", argv[0]);
    exit(0);
}


