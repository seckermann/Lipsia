/****************************************************************
 *
 * vregr.c
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Karsten Mueller, 2001, <lipsia@cbs.mpg.de>
 * Many parts of this file are taken from "vtimecourse"
 * written by Gabriele Lohmann
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
 * $Id: vregr.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "viaio/headerinfo.h"
#include <ctype.h>
#include <fftw3.h>

extern VBoolean VReadBlockData(FILE *, VImageInfo *, int, int, VImage *);
extern void VTal2Pixel(float *, float *, float *, float, float, float, int *, int *, int *);
extern double *Orthogonalize(double *, double *, double *, int);
extern double *VConvolveRegr(double *, int *);

void
VglmGetRegr(VImageInfo *imageInfo, FILE *in_file, FILE *fp2, int *ptrvi, VString reportfile, VString regressor, char *voxelstr, char *extentstr, char *castr) {
    double *regr = NULL, *regr1 = NULL, *regr2 = NULL;
    int *regr1i = NULL;
    int z0 = 0, y0 = 0, x0 = 0;
    int b0, b1, r0, r1, c0, c1, b, r, c;
    int rr, cc, bb, i, j, nobj, nobj1, regrnb;
    int relev_pixel = 0, rrws = 0, regr1length = 0;
    double sum, pos = 0, neg = 0, quad = 0, N = 0, N1 = 0;
    float voxel[3], ca[3], extent[3];
    float uu = 0, vv = 0, ww = 0;
    VShort *pp;
    VFloat u = 0;
    VImage image = NULL, mask = NULL, zmap = NULL, dest = NULL;
    VBoolean ret, sw;
    VAttrList list;
    FILE *fp = NULL, *fp1 = NULL;
    char buf[256];
    size_t n = 256;
    /* Initialization */
    regrnb = 1;
    regr = (double *) malloc(sizeof(double) * ptrvi[5]);
    for(j = 0; j < ptrvi[5]; j++)
        regr[j] = 0;
    if(strlen(regressor) > 2) {
        regrnb = 2;
        regr1 = (double *) malloc(sizeof(double) * ptrvi[5]);
        for(j = 0; j < ptrvi[5]; j++)
            regr1[j] = 0;
        regr1i = (int *) malloc(sizeof(int) * ptrvi[5]);
        for(j = 0; j < ptrvi[5]; j++)
            regr1i[j] = 0;
        if(ptrvi[15] == 1) {
            regrnb = 3;
            regr2 = (double *) malloc(sizeof(double) * ptrvi[5]);
            for(j = 0; j < ptrvi[5]; j++)
                regr2[j] = 0;
        }
    }
    /* Coordinate Type */
    switch(ptrvi[9]) {
    case 0:   /* voxel ccord */
        z0 = ptrvi[13];
        y0 = ptrvi[12];
        x0 = ptrvi[11];
        break;
    case 1: /* mm coord */
        if(!voxelstr)
            VError(" attribute 'voxel' missing");
        sscanf(voxelstr, "%f %f %f", &uu, &vv, &ww);
        voxel[0] = uu;
        voxel[1] = vv;
        voxel[2] = ww;
        z0 = rint((double)((double)ptrvi[13] / voxel[2]));
        y0 = rint((double)((double)ptrvi[12] / voxel[1]));
        x0 = rint((double)((double)ptrvi[11] / voxel[0]));
        break;
    case 2: /* Talairach */
        if(!voxelstr)
            VError(" attribute 'voxel' missing");
        sscanf(voxelstr, "%f %f %f", &uu, &vv, &ww);
        voxel[0] = uu;
        voxel[1] = vv;
        voxel[2] = ww;
        if(!extentstr || !strcmp(extentstr, "N"))
            VError(" attribute 'extent' missing");
        sscanf(extentstr, "%f %f %f", &uu, &vv, &ww);
        extent[0] = uu;
        extent[1] = vv;
        extent[2] = ww;
        ca[0] = ca[1] = ca[2] = 0;
        if(!castr || !strcmp(castr, "N"))
            VError(" attribute 'ca' missing");
        sscanf(castr, "%f %f %f", &uu, &vv, &ww);
        ca[0]  = uu;
        ca[1]  = vv;
        ca[2]  = ww;
        uu = ptrvi[11];
        vv = ptrvi[12];
        ww = ptrvi[13];
        VTal2Pixel(ca, voxel, extent, uu, vv, ww, &z0, &y0, &x0);
        break;
    default:
        VError("illegal coordinate system: %d", ptrvi[9]);
    }
    /* Check the coordiantes */
    if(z0 < 0 || z0 >= ptrvi[0])
        VError(" illegal z-address (%d), max number of slices= %d", z0, ptrvi[0]);
    if(y0 < 0 || y0 >= ptrvi[6])
        VError(" illegal y-address (%d), max number of rows= %d", y0, ptrvi[6]);
    if(x0 < 0 || x0 >= ptrvi[7])
        VError(" illegal x-address (%d), max number of columns= %d", x0, ptrvi[7]);
    /* create mask */
    mask = VCreateImage(ptrvi[0], ptrvi[6], ptrvi[7], VBitRepn);
    VFillImage(mask, VAllBands, 0);
    /* Different types of neighbourhood */
    switch(ptrvi[10]) {
    case 0:  /* single pixel */
        VPixel(mask, z0, y0, x0, VBit) = (VBit)1;
        break;
    case 1: /* 8-neighbours  (2D)  */
        VPixel(mask, z0, y0, x0, VBit) = (VBit)1;
        b = z0;
        for(r = y0 - 1; r <= y0 + 1; r++) {
            for(c = x0 - 1; c <= x0 + 1; c++) {
                if(r < 0 || r >= ptrvi[6])
                    continue;
                if(c < 0 || c >= ptrvi[7])
                    continue;
                if(zmap != NULL) {
                    u = VPixel(zmap, b, r, c, VFloat);
                    if(u >= pos || u <= neg) {
                        VPixel(mask, b, r, c, VBit) = 1;
                    }
                } else
                    VPixel(mask, b, r, c, VBit) = 1;
            }
        }
        break;
    case 2: /* 26-neighbours  (3D) */
        VPixel(mask, z0, y0, x0, VBit) = 1;
        for(b = z0 - 1; b <= z0 + 1; b++) {
            for(r = y0 - 1; r <= y0 + 1; r++) {
                for(c = x0 - 1; c <= x0 + 1; c++) {
                    if(b < 0 || b >= ptrvi[0])
                        continue;
                    if(r < 0 || r >= ptrvi[6])
                        continue;
                    if(c < 0 || c >= ptrvi[7])
                        continue;
                    if(zmap != NULL) {
                        u = VPixel(zmap, b, r, c, VFloat);
                        if(u >= pos || u <= neg) {
                            VPixel(mask, b, r, c, VBit) = 1;
                        }
                    } else
                        VPixel(mask, b, r, c, VBit) = 1;
                }
            }
        }
        break;
    default:
        VError(" illegal type");
    }
    /* get bounding box in mask image */
    b0 = ptrvi[0];
    b1 = 0;
    r0 = ptrvi[6];
    r1 = 0;
    c0 = ptrvi[7];
    c1 = 0;
    for(b = 0; b < ptrvi[0]; b++) {
        for(r = 0; r < ptrvi[6]; r++) {
            for(c = 0; c < ptrvi[7]; c++) {
                if(VPixel(mask, b, r, c, VBit) == 0)
                    continue;
                if(b0 > b)
                    b0 = b;
                if(r0 > r)
                    r0 = r;
                if(c0 > c)
                    c0 = c;
                if(b1 < b)
                    b1 = b;
                if(r1 < r)
                    r1 = r;
                if(c1 < c)
                    c1 = c;
            }
        }
    }
    /* Loop over all objects */
    for(nobj1 = 0; nobj1 < ptrvi[4]; nobj1++) {
        if(nobj1 < b0 || nobj1 > b1)
            continue;
        /* Check if an input file has a "zero-object" */
        sw = 0;
        nobj = nobj1 + ptrvi[3];
        if(imageInfo[nobj].nbands < 2)
            sw = 1;
        /* Create image for source (functional) data */
        rrws  = r1 - r0 + 1; /* Number of rows of the bounding box */
        image = VCreateImage(ptrvi[5], rrws, ptrvi[7], VShortRepn);
        pp = VPixelPtr(image, 0, 0, 0);
        memset(pp, 0, ptrvi[5]*ptrvi[7]*rrws * VPixelSize(image));
        /* if it is a non-zero object ONLY */
        if(sw == 0) {
            /* Load the 3 columns into the image */
            ret = VReadBlockData(in_file, &imageInfo[nobj], (int) r0, (int) rrws, &image);
            if(!ret)
                VError("An error has occured while reading block data");
            /* Load the relevant_pixels */
            for(rr = 0; rr < rrws; rr++) {
                for(cc = 0; cc < ptrvi[7]; cc++) {
                    if(cc < c0 || cc > c1)
                        continue;
                    if(VPixel(mask, nobj1, rr + r0, cc, VBit) == 0)
                        continue;
                    /* Fill the regressor */
                    i = 0;
                    for(bb = 0; bb < ptrvi[5]; bb++) {
                        u = (double)VPixel(image, bb, rr, cc, VShort);
                        regr[bb] += u;
                        if(u > ptrvi[2])
                            i++;
                    }
                    relev_pixel++;
                    if(i != ptrvi[5])
                        VWarning("Some values of the regressor are less than the selected minval");
                }    /* end of column (cc)*/
            }      /* end of row (rr)*/
        }        /* if sw==0, i.e. if the object is non-zero in all files */
        /* Free memory */
        VDestroyImage(image);
    } /*  end of objects */
    /* Mean value */
    if(relev_pixel == 0)
        VError("There are no functional data for the selected voxel");
    if(relev_pixel > 1) {
        for(i = 0; i < ptrvi[5]; i++)
            regr[i] /= (double)relev_pixel;
    }
    /* Input of additionally specifies regressor */
    if(regr1) {
        fp = fopen(regressor, "r");
        if(fp == NULL)
            VError(" err opening %s", regressor);
        i = 0;
        regr1length = 0;
        while(fgets(buf, n, fp)) {
            if(strlen(buf) < 1)
                continue;
            if(buf[0] == '#')
                continue;
            if(buf[0] == 'X') {
                regr1[i]  = 0;
                regr1i[i] = 0;
            } else {
                regr1[i]  = atof(strtok(buf, " "));
                regr1i[i] = 1;
                regr1length++;
            }
            i++;
        }
        fclose(fp);
        if(i != ptrvi[5])
            VError("lengths of regressor and timesteps do not coincide");
        if(ptrvi[18])
            regr1 = VConvolveRegr(regr1, ptrvi);
    }
    /* normalize regressors */
    if(ptrvi[14] == 1) {
        sum = 0;
        quad = 0;
        for(bb = 0; bb < ptrvi[5]; bb++) {
            u = regr[bb];
            sum  += u;
            quad += u * u;
        }
        N = (double)ptrvi[5];
        /* quad=sqrt(quad-sum*sum/N); */
        quad = sqrt((N * quad - sum * sum) / (N * (N - 1)));
        sum = sum / N;
        for(bb = 0; bb < ptrvi[5]; bb++) {
            u = (regr[bb] - sum) / quad;
            regr[bb] = u;
        }
        if(regr1) {
            sum = 0;
            quad = 0;
            i = 0;
            for(bb = 0; bb < ptrvi[5]; bb++) {
                if(regr1i[bb] == 1) {
                    u = regr1[bb];
                    sum  += u;
                    quad += u * u;
                    i++;
                }
            }
            if(i != regr1length)
                VError("This should not occur: i!=regr1length");
            N1 = (double)regr1length;
            /* quad=sqrt(quad-sum*sum/N1); */
            quad = sqrt((N1 * quad - sum * sum) / (N1 * (N1 - 1)));
            sum = sum / N1;
            for(bb = 0; bb < ptrvi[5]; bb++) {
                if(regr1i[bb] == 1) {
                    u = (regr1[bb] - sum) / quad;
                    regr1[bb] = u;
                } else {
                    regr1[bb] = 0;
                }
            }
        }
    }
    if(ptrvi[15] == 1) {
        if(ptrvi[14] != 1)
            VError("PPI requires normalization of regressors using '-norm true'");
        if(!regr1)
            VError("PPI requires regressor using the option '-regr'");
        if(!regr2)
            VError("regr2 is not defined");
        for(bb = 0; bb < ptrvi[5]; bb++) {
            regr2[bb] = regr[bb] * regr1[bb];
        }
        if(ptrvi[16] == 1)
            regr2 = Orthogonalize(regr, regr1, regr2, ptrvi[5]);
    }
    /* Output */
    if(fp2 == NULL)
        VError("error opening out file");
    if(ptrvi[1] == 0) {
        if(ptrvi[17] < 10 || ptrvi[17] > 100000)
            VError("TR invalid in input file");
        dest = VCreateImage(1, ptrvi[5], regrnb + 1, VFloatRepn);
        VSetAttr(VImageAttrList(dest), "name", NULL, VStringRepn, "X");
        VSetAttr(VImageAttrList(dest), "modality", NULL, VStringRepn, "X");
        VSetAttr(VImageAttrList(dest), "repetition_time", NULL, VLongRepn, (VLong)ptrvi[17]);
        VSetAttr(VImageAttrList(dest), "ntimesteps", NULL, VLongRepn, (VLong)ptrvi[5]);
        VSetAttr(VImageAttrList(dest), "derivatives", NULL, VLongRepn, (VLong) - 1);
        VSetAttr(VImageAttrList(dest), "nsessions", NULL, VLongRepn, (VLong)1);
        VSetAttr(VImageAttrList(dest), "designtype", NULL, VLongRepn, (VLong)1);
        list = VCreateAttrList();
        for(bb = 0; bb < ptrvi[5]; bb++) {
            if(regrnb > 0)
                VPixel(dest, 0, bb, 0, VFloat) = (VFloat)regr[bb];
            if(regrnb > 1)
                VPixel(dest, 0, bb, 1, VFloat) = (VFloat)regr1[bb];
            if(regrnb > 2)
                VPixel(dest, 0, bb, 2, VFloat) = (VFloat)regr2[bb];
            VPixel(dest, 0, bb, regrnb, VFloat) = (VFloat)1;
        }
        VAppendAttr(list, "image", NULL, VImageRepn, dest);
        if(! VWriteFile(fp2, list))
            exit(1);
    } else {
        if(ptrvi[1] == 1) {
            for(bb = 0; bb < ptrvi[5]; bb++) {
                if(regrnb == 1)
                    fprintf(fp2, "%14.7f \n", regr[bb]);
                if(regrnb == 2)
                    fprintf(fp2, "%14.7f %14.7f \n", regr[bb], regr1[bb]);
                if(regrnb == 3)
                    fprintf(fp2, "%14.7f %14.7f %14.7f \n", regr[bb], regr1[bb], regr2[bb]);
            }
        } else {
            /* Old vglm2 design file */
            fprintf(fp2, "#\n# Design file for vglm2, created by vrfc\n#\n\n\n");
            fprintf(fp2, "regr: ");
            for(bb = 0; bb < ptrvi[5]; bb++)
                fprintf(fp2, "%1.7f ", regr[bb]);
            fprintf(fp2, "\n");
            if(regr1) {
                fprintf(fp2, "regr: ");
                for(bb = 0; bb < ptrvi[5]; bb++)
                    fprintf(fp2, "%1.7f ", regr1[bb]);
                fprintf(fp2, "\n");
                if(regr2 && ptrvi[15] == 1) {
                    fprintf(fp2, "regr: ");
                    for(bb = 0; bb < ptrvi[5]; bb++)
                        fprintf(fp2, "%1.7f ", regr2[bb]);
                    fprintf(fp2, "\n");
                }
            }
            fprintf(fp2, "\n");
            fprintf(fp2, "\nmodel.nscan              = %d", ptrvi[5]);
            fprintf(fp2, "\nmodel.conditions_nb      = 0");
            fprintf(fp2, "\nmodel.regressors_nb      = %d", regrnb);
            fprintf(fp2, "\nmodel.volterra           = 0");
            fprintf(fp2, "\nmodel.HF_cut             = -1");
            fprintf(fp2, "\nmodel.stochastics_flag   = 0");
            fprintf(fp2, "\nmodel.stochastics_ne     = 0");
            fprintf(fp2, "\nmodel.stochastics_soa    = 0");
            fprintf(fp2, "\nmodel.stochastics_sm     = 0");
            fprintf(fp2, "\nmodel.parametrics_type   = none");
            fprintf(fp2, "\nmodel.parametrics_etype  = 0");
            fprintf(fp2, "\nmodel.parametrics_name   = 0");
            fprintf(fp2, "\nmodel.parametrics_cst    = 0");
            fprintf(fp2, "\n\n\n");
            fprintf(fp2, "conditions(1).names      = 0\n");
            fprintf(fp2, "conditions(1).types      = 0\n");
            fprintf(fp2, "conditions(1).onsets     = 0\n");
            fprintf(fp2, "conditions(1).var_dur    = 0\n");
            fprintf(fp2, "conditions(1).parametric = 0\n");
            fprintf(fp2, "conditions(1).bf_ev      = 0\n");
            fprintf(fp2, "conditions(1).win_len    = 0\n");
            fprintf(fp2, "conditions(1).win_ord    = 0\n");
            fprintf(fp2, "conditions(1).bf_ep      = 0\n");
            fprintf(fp2, "conditions(1).fct_nb     = 0\n");
            fprintf(fp2, "conditions(1).eplength   = 0\n");
            fprintf(fp2, "conditions(1).conv       = 0\n");
            fprintf(fp2, "conditions(1).deriv      = 0\n");
            if(regr1) {
                if(regr2 && ptrvi[15] == 1) {
                    fprintf(fp2, "regressors(1).names      = timecourse regressor ppi\n");
                    fprintf(fp2, "regressors(1).regress    = 1 2 3\n\n");
                } else {
                    fprintf(fp2, "regressors(1).names      = timecourse regressor\n");
                    fprintf(fp2, "regressors(1).regress    = 1 2\n\n");
                }
            } else {
                fprintf(fp2, "regressors(1).names      = timecourse\n");
                fprintf(fp2, "regressors(1).regress    = 1\n\n");
            }
            fprintf(fp2, "\n");
        }
    }
    /* Report regressor */
    if(strlen(reportfile) > 2) {
        fp1 = fopen(reportfile, "w");
        if(fp1 == NULL)
            VError("error opening report file %s", reportfile);
        fprintf(fp1, "# Report file for regressor\n");
        fprintf(fp1, "# Timesteps: %d\n", ptrvi[5]);
        fprintf(fp1, "#\n");
        if(ptrvi[9] == 1)
            fprintf(fp1, "# input addr  : %d %d %d  (mm)\n", ptrvi[11], ptrvi[12], ptrvi[13]);
        if(ptrvi[9] == 2)
            fprintf(fp1, "# input addr  : %d %d %d  (talairach)\n", ptrvi[11], ptrvi[12], ptrvi[13]);
        fprintf(fp1, "# voxel addr  : %d %d %d\n", x0, y0, z0);
        if(ptrvi[10] == 0)
            fprintf(fp1, "# type        : %s\n", "single voxel");
        if(ptrvi[10] == 1)
            fprintf(fp1, "# type        : %s\n", "8adj");
        if(ptrvi[10] == 2)
            fprintf(fp1, "# type        : %s\n", "26adj");
        fprintf(fp1, "#\n");
        fprintf(fp1, "#------------------------------------------\n\n");
        for(bb = 0; bb < ptrvi[5]; bb++)
            fprintf(fp1, "%14.7f \n", regr[bb]);
        fclose(fp1);
    }
}

