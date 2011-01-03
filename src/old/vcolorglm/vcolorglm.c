/****************************************************************
 *
 * Program: vcolorglm
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2004, <lipsia@cbs.mpg.de>
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
 * $Id: vcolorglm.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


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
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_cdf.h>
#include "gsl_utils.h"

#define ETMP     64   /* max number of temporary images for smoothness estim */
#define NSLICES 2500   /* max number of image slices */
#define MBETA    64   /* max number of covariates */

#define ABS(x) ((x) > 0 ? (x) : -(x))


typedef struct ListStruct {
    int ntimesteps;
    int nrows;
    int ncols;
    int nslices;
    int itr;
    VRepnKind repn;
    int zero[NSLICES];
    VString filename;
    VImageInfo info[NSLICES];
} ListInfo;


extern void gsl_sort_vector(gsl_vector *);
extern float VSmoothnessEstim(VImage *, int);
extern double gsl_sf_beta_inc(double, double, double);

extern int isnanf(float);
extern gsl_vector *GaussKernel(double);
extern void GaussMatrix(double, gsl_matrix_float *);
extern gsl_vector_float *VectorConvolve(gsl_vector_float *, gsl_vector_float *, gsl_vector *);

extern VAttrList GetListInfo(VString, ListInfo *);
extern VBoolean VReadBandDataFD(int, VImageInfo *, int, int, VImage *);
extern char *getLipsiaVersion();

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
** general linear regression
*/
VAttrList
VRegression(ListInfo *linfo, int nlists, VShort minval, VImage design, VFloat sigma, VLong itr) {
    VAttrList out_list;
    VImageInfo *xinfo;
    int nbands = 0, nslices = 0, nrows = 0, ncols = 0, slice, row, col, nr, nc;
    VImage src[NSLICES], res_image = NULL;
    VImage beta_image[MBETA], BCOV = NULL, KX_image = NULL;
    VImage res_map[ETMP];
    float  smooth_fwhm = 0, vx = 0, vy = 0, vz = 0;
    VFloat *float_pp, df;
    VRepnKind repn;
    float d, err;
    int   i, k, l, n, m = 0, nt, fd = 0, npix = 0;
    int   i0 = 0, i1 = 0;
    float u, sig, trace = 0, trace2 = 0, var = 0, sum = 0, nx = 0, mean = 0, sum2;
    float *ptr1, *ptr2;
    double x;
    gsl_matrix_float *X = NULL, *XInv = NULL, *SX = NULL;
    gsl_vector_float *y, *z, *beta, *ys;
    gsl_vector *kernel;
    gsl_matrix_float *S = NULL, *Vc = NULL, *F = NULL, *P = NULL, *Q = NULL;
    gsl_matrix_float *R = NULL, *RV = NULL;
    VBoolean smooth = TRUE;  /* no smoothness estimation */
    gsl_set_error_handler_off();
    /*
    ** read input data
    */
    nslices = nbands = nrows = ncols = 0;
    for(k = 0; k < nlists; k++) {
        n  = linfo[k].nslices;
        nr = linfo[k].nrows;
        nc = linfo[k].ncols;
        nt = linfo[k].ntimesteps;
        nbands += nt;
        if(nslices == 0)
            nslices = n;
        else if(nslices != n)
            VError(" inconsistent image dimensions, slices: %d %d", n, nslices);
        if(nrows == 0)
            nrows = nr;
        else if(nrows != nr)
            VError(" inconsistent image dimensions, rows: %d %d", nr, nrows);
        if(ncols == 0)
            ncols = nc;
        else if(ncols != nc)
            VError(" inconsistent image dimensions, cols: %d %d", nc, ncols);
    }
    fprintf(stderr, " num images: %d,  image dimensions: %d x %d x %d\n",
            nlists, nslices, nrows, ncols);
    /*
    ** get design dimensions
    */
    m = VImageNRows(design);      /* number of timesteps   */
    n = VImageNColumns(design);   /* number of covariates */
    fprintf(stderr, " ntimesteps=%d,   num covariates=%d\n", m, n);
    if(n >= MBETA)
        VError(" too many covariates (%d), max is %d", n, MBETA);
    if(m != nbands)
        VError(" design dimension inconsistency: %d %d", m, nbands);
    fprintf(stderr, " working...\n");
    /*
    ** read design matrix
    */
    X = gsl_matrix_float_alloc(m, n);
    for(k = 0; k < m; k++) {
        for(l = 0; l < n; l++) {
            x = VGetPixel(design, 0, k, l);
            fmset(X, k, l, (float)x);
        }
    }
    /*
    ** pre-coloring, set up K-matrix, S=K, V = K*K^T with K=S
    */
    S  = gsl_matrix_float_alloc(m, m);
    GaussMatrix((double)sigma, S);
    Vc = fmat_x_matT(S, S, NULL);
    /*
    ** compute pseudoinverse
    */
    SX = fmat_x_mat(S, X, NULL);
    XInv = fmat_PseudoInv(SX, NULL);
    /*
    ** get variance estimate
    */
    Q = fmat_x_mat(XInv, Vc, Q);
    F = fmat_x_matT(Q, XInv, F);
    BCOV = VCreateImage(1, n, n, VFloatRepn);
    float_pp = VImageData(BCOV);
    ptr1 = F->data;
    for(i = 0; i < n * n; i++)
        *float_pp++ = *ptr1++;
    gsl_matrix_float_free(Q);
    gsl_matrix_float_free(F);
    /*
    ** get effective degrees of freedom
    */
    R  = gsl_matrix_float_alloc(m, m);
    P = fmat_x_mat(SX, XInv, P);
    gsl_matrix_float_set_identity(R);
    gsl_matrix_float_sub(R, P);
    RV = fmat_x_mat(R, Vc, NULL);
    trace = 0;
    for(i = 0; i < m; i++)
        trace += fmget(RV, i, i);
    P = fmat_x_mat(RV, RV, P);
    trace2 = 0;
    for(i = 0; i < m; i++)
        trace2 += fmget(P, i, i);
    df = (trace * trace) / trace2;
    fprintf(stderr, " df= %.3f\n", df);
    /*
    ** create output images
    */
    xinfo = linfo[0].info;
    out_list = VCreateAttrList();
    res_image = VCreateImage(nslices, nrows, ncols, VFloatRepn);
    VFillImage(res_image, VAllBands, 0);
    VSetAttr(VImageAttrList(res_image), "name", NULL, VStringRepn, "RES/trRV");
    VSetAttr(VImageAttrList(res_image), "modality", NULL, VStringRepn, "RES/trRV");
    VSetAttr(VImageAttrList(res_image), "df", NULL, VFloatRepn, df);
    VSetAttr(VImageAttrList(res_image), "patient", NULL, VStringRepn, xinfo->patient);
    VSetAttr(VImageAttrList(res_image), "voxel", NULL, VStringRepn, xinfo->voxel);
    VSetAttr(VImageAttrList(res_image), "repetition_time", NULL, VLongRepn, itr);
    VSetAttr(VImageAttrList(res_image), "talairach", NULL, VStringRepn, xinfo->talairach);
    if(xinfo->fixpoint[0] != 'N')
        VSetAttr(VImageAttrList(res_image), "fixpoint", NULL, VStringRepn, xinfo->fixpoint);
    if(xinfo->ca[0] != 'N') {
        VSetAttr(VImageAttrList(res_image), "ca", NULL, VStringRepn, xinfo->ca);
        VSetAttr(VImageAttrList(res_image), "cp", NULL, VStringRepn, xinfo->cp);
        VSetAttr(VImageAttrList(res_image), "extent", NULL, VStringRepn, xinfo->extent);
    }
    VAppendAttr(out_list, "image", NULL, VImageRepn, res_image);
    for(i = 0; i < n; i++) {
        beta_image[i] = VCreateImage(nslices, nrows, ncols, VFloatRepn);
        VFillImage(beta_image[i], VAllBands, 0);
        VSetAttr(VImageAttrList(beta_image[i]), "patient", NULL, VStringRepn, xinfo->patient);
        VSetAttr(VImageAttrList(beta_image[i]), "voxel", NULL, VStringRepn, xinfo->voxel);
        VSetAttr(VImageAttrList(beta_image[i]), "repetition_time", NULL, VLongRepn, itr);
        VSetAttr(VImageAttrList(beta_image[i]), "talairach", NULL, VStringRepn, xinfo->talairach);
        if(xinfo->fixpoint[0] != 'N')
            VSetAttr(VImageAttrList(beta_image[i]), "fixpoint", NULL, VStringRepn, xinfo->fixpoint);
        if(xinfo->ca[0] != 'N') {
            VSetAttr(VImageAttrList(beta_image[i]), "ca", NULL, VStringRepn, xinfo->ca);
            VSetAttr(VImageAttrList(beta_image[i]), "cp", NULL, VStringRepn, xinfo->cp);
            VSetAttr(VImageAttrList(beta_image[i]), "extent", NULL, VStringRepn, xinfo->extent);
        }
        VSetAttr(VImageAttrList(beta_image[i]), "name", NULL, VStringRepn, "BETA");
        VSetAttr(VImageAttrList(beta_image[i]), "modality", NULL, VStringRepn, "BETA");
        VSetAttr(VImageAttrList(beta_image[i]), "beta", NULL, VShortRepn, i + 1);
        VSetAttr(VImageAttrList(beta_image[i]), "df", NULL, VFloatRepn, df);
        VAppendAttr(out_list, "image", NULL, VImageRepn, beta_image[i]);
    }
    VSetAttr(VImageAttrList(design), "name", NULL, VStringRepn, "X");
    VSetAttr(VImageAttrList(design), "modality", NULL, VStringRepn, "X");
    VAppendAttr(out_list, "image", NULL, VImageRepn, design);
    KX_image = Mat2Vista(SX);
    VSetAttr(VImageAttrList(KX_image), "name", NULL, VStringRepn, "KX");
    VSetAttr(VImageAttrList(KX_image), "modality", NULL, VStringRepn, "KX");
    VAppendAttr(out_list, "image", NULL, VImageRepn, KX_image);
    VSetAttr(VImageAttrList(BCOV), "name", NULL, VStringRepn, "BCOV");
    VSetAttr(VImageAttrList(BCOV), "modality", NULL, VStringRepn, "BCOV");
    VAppendAttr(out_list, "image", NULL, VImageRepn, BCOV);
    /*
    ** create temporary images for smoothness estimation
    */
    /* smoothness estim only for 3D images, i.e. CA/CP known */
    if(xinfo->ca[0] == 'N')
        smooth = FALSE;
    if(smooth) {
        i0 = 20;
        i1 = i0 + 30;
        if(i1 > m)
            i1 = m;
        for(i = i0; i < i1; i++) {
            if(i - i0 >= ETMP)
                VError(" too many tmp images");
            res_map[i - i0] = VCreateImage(nslices, nrows, ncols, VFloatRepn);
            VFillImage(res_map[i - i0], VAllBands, 0);
        }
    }
    /*
    ** process
    */
    ys   = gsl_vector_float_alloc(m);
    y    = gsl_vector_float_alloc(m);
    z    = gsl_vector_float_alloc(m);
    beta = gsl_vector_float_alloc(n);
    kernel = GaussKernel((double)sigma);
    for(k = 0; k < nlists; k++) {
        src[k] = VCreateImage(linfo[k].ntimesteps, nrows, ncols, linfo[k].repn);
        VFillImage(src[k], VAllBands, 0);
    }
    npix = 0;
    for(slice = 0; slice < nslices; slice++) {
        if(slice % 5 == 0)
            fprintf(stderr, " slice: %3d\r", slice);
        for(k = 0; k < nlists; k++) {
            if(linfo[k].zero[slice] == 0)
                goto next1;
            fd = open(linfo[k].filename, O_RDONLY);
            if(fd == -1)
                VError("could not open file %s", linfo[k].filename);
            nt = linfo[k].ntimesteps;
            if(! VReadBandDataFD(fd, &linfo[k].info[slice], 0, nt, &src[k]))
                VError(" error reading data");
            close(fd);
        }
        repn = linfo[0].repn;
        for(row = 0; row < nrows; row++) {
            for(col = 0; col < ncols; col++) {
                for(k = 0; k < nlists; k++)
                    if(VPixel(src[k], 0, row, col, VShort) < minval + 1)
                        goto next;
                npix++;
                /* read time series data */
                sum = sum2 = nx = 0;
                ptr1 = y->data;
                for(k = 0; k < nlists; k++) {
                    nt  = VImageNBands(src[k]);
                    for(i = 0; i < nt; i++) {
                        u = VPixel(src[k], i, row, col, VShort);
                        (*ptr1++) = u;
                        sum  += u;
                        sum2 += u * u;
                        nx++;
                    }
                }
                mean = sum / nx;
                sig = sqrt((double)((sum2 - nx * mean * mean) / (nx - 1.0)));
                if(sig < 0.001)
                    continue;
                /* centering and scaling, Seber, p.330 */
                ptr1 = y->data;
                for(i = 0; i < m; i++) {
                    u = ((*ptr1) - mean) / sig;
                    (*ptr1++) = u + 100.0;
                }
                /* S x y */
                ys = VectorConvolve(y, ys, kernel);
                /* compute beta's */
                fmat_x_vector(XInv, ys, beta);
                /* residuals */
                fmat_x_vector(SX, beta, z);
                err = 0;
                ptr1 = ys->data;
                ptr2 = z->data;
                for(i = 0; i < m; i++) {
                    d = ((*ptr1++) - (*ptr2++));
                    err += d * d;
                }
                /* sigma^2 */
                var = err / trace;
                /* write residuals output */
                VPixel(res_image, slice, row, col, VFloat) = (VFloat)var;
                /* save residuals of several timesteps for smoothness estimation */
                if(smooth) {
                    ptr1 = ys->data;
                    ptr2 = z->data;
                    err = 0;
                    for(i = i0; i < i1; i++) {
                        d = ((*ptr1++) - (*ptr2++));
                        err += d * d;
                        VPixel(res_map[i - i0], slice, row, col, VFloat) = d;
                    }
                    err = sqrt(err);
                    for(i = i0; i < i1; i++) {
                        d = VPixel(res_map[i - i0], slice, row, col, VFloat);
                        VPixel(res_map[i - i0], slice, row, col, VFloat) = d / err;
                    }
                }
                /* write beta output */
                ptr1 = beta->data;
                for(i = 0; i < n; i++)
                    VPixel(beta_image[i], slice, row, col, VFloat) = (VFloat)(*ptr1++);
next:
                ;
            }
        }
next1:
        ;
    }
    /*
    ** Smoothness estimation based on residual images
    */
    if(smooth) {
        smooth_fwhm = VSmoothnessEstim(res_map, i1 - i0);
        sscanf(xinfo->voxel, "%f %f %f", &vx, &vy, &vz);
        vx = (vx + vy + vz) / 3.0;    /* voxels should be isotropic */
        smooth_fwhm *= vx;
        fprintf(stderr, " smoothness: %f\n", smooth_fwhm);
        VSetAttr(VImageAttrList(res_image), "smoothness", NULL, VFloatRepn, smooth_fwhm);
        for(i = 0; i < n; i++) {
            VSetAttr(VImageAttrList(beta_image[i]), "smoothness", NULL, VFloatRepn, smooth_fwhm);
        }
        for(i = 0; i < i1 - i0; i++)
            VDestroyImage(res_map[i]);
    }
ende:
    if(npix == 0)
        VError(" no voxels above threshold %d found", minval);
    return out_list;
}





int
main(int argc, char *argv[]) {
    static VArgVector in_files;
    static VString out_filename;
    static VString filename;
    static VShort minval = 0;
    static VFloat fwhm = 4.0;
    static VOptionDescRec  options[] = {
        {"in", VStringRepn, 0, & in_files, VRequiredOpt, NULL, "Input files" },
        {"out", VStringRepn, 1, & out_filename, VRequiredOpt, NULL, "Output file" },
        {"design", VStringRepn, 1, (VPointer) &filename, VRequiredOpt, NULL, "Design file"},
        {
            "fwhm", VFloatRepn, 1, (VPointer) &fwhm, VOptionalOpt, NULL,
            "FWHM of temporal Gaussian filter in seconds"
        },
        {"minval", VShortRepn, 1, (VPointer) &minval, VOptionalOpt, NULL, "Signal threshold"}
    };
    FILE *fp = NULL, *f = NULL;
    VStringConst in_filename;
    VString ifilename;
    VAttrList list = NULL, list1 = NULL;
    VAttrList out_list = NULL, history_list = NULL;
    VAttrListPosn posn;
    VImage design = NULL;
    ListInfo *linfo;
    VLong itr = 0;
    VFloat sigma = 0, tr = 0;
    int  i, n, nimages;
    char prg[50];
    sprintf(prg, "vcolorglm V%s", getLipsiaVersion());
    fprintf(stderr, "%s\n", prg);
    /*
    ** parse command line
    */
    if(! VParseCommand(VNumber(options), options, & argc, argv)) {
        VReportUsage(argv[0], VNumber(options), options, NULL);
        exit(EXIT_FAILURE);
    }
    if(argc > 1) {
        VReportBadArgs(argc, argv);
        exit(EXIT_FAILURE);
    }
    /*
    ** read design matrix
    */
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
        if(VPixelRepn(design) != VFloatRepn /* && VPixelRepn(design) != VDoubleRepn */)
            continue;
        n++;
        break;
    }
    if(n == 0)
        VError(" design matrix not found ");
    /*
    ** get pre-coloring info
    */
    if(VGetAttr(VImageAttrList(design), "repetition_time", NULL, VLongRepn, &itr) != VAttrFound)
        VError(" TR info missing in header");
    tr = (float) itr / 1000.0;
    sigma = 0;
    if(tr > 0.001 && fwhm > 0.001) {
        fprintf(stderr, " TR: %.3f seconds\n", tr);
        sigma = fwhm / 2.35482;
        sigma /= tr;
        if(sigma < 0.1) {
            VWarning(" 'fwhm/sigma' too small (%.3f / %.3f), will be set to zero", fwhm, sigma);
            sigma = 0;
        }
    }
    /*
    ** Read each input file
    */
    nimages = in_files.number;
    linfo = (ListInfo *) VMalloc(sizeof(ListInfo) * nimages);
    for(i = 0; i < nimages; i++) {
        in_filename = ((VStringConst *) in_files.vector)[i];
        ifilename = VNewString(in_filename);
        fprintf(stderr, " file:  %s\n", ifilename);
        list = GetListInfo(ifilename, &linfo[i]);
        /* Create history */
        if(i == 0) {
            history_list = VReadHistory(&list);
            if(history_list == NULL)
                history_list = VCreateAttrList();
            VPrependHistory(VNumber(options), options, prg, &history_list);
        }
    }
    /*
    ** GLM
    */
    out_list = VRegression(linfo, nimages, minval, design, sigma, itr);
    /*
    **  Output:
    */
    VPrependAttr(out_list, "history", NULL, VAttrListRepn, history_list);
    f = VOpenOutputFile(out_filename, TRUE);
    if(!f)
        VError(" error opening outout file %s", out_filename);
    if(! VWriteFile(f, out_list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}


