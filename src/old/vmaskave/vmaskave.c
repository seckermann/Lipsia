/****************************************************************
 *
 * vmaskave:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 1998, <lipsia@cbs.mpg.de>
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
 * $Id: vmaskave.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/os.h>
#include <viaio/VImage.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <gsl/gsl_errno.h>

extern void VRoiInfo(VImage, VImage, VShort, double *, double *, double *);
extern void getLipsiaVersion(char*,size_t);

void
ttest(double ave1, double var1, double nx1,
      double ave2, double var2, double nx2,
      double *t, double *nu, double *p) {
    double t1, df, prob, x, a, b;
    extern double gsl_sf_beta_inc(double, double, double);
    gsl_set_error_handler_off();
    t1 = (ave1 - ave2) / sqrt(var1 / nx1 + var2 / nx2);
    df = (nx1 - 1.0);
    a = 0.5 * df;
    b = 0.5;
    x = df / (df + (t1 * t1));
    prob = gsl_sf_beta_inc(a, b, x);
    /* one-tailed t-test */
    prob *= 0.5;
    *t  = t1;
    *nu = df;
    *p  = prob;
}


int main(int argc, char *argv[]) {
    static VArgVector in_files;
    static VString mask_filename;
    static VString report_filename = "";
    static VBoolean in_found;
    static VShort id = 0;
    static VBoolean verbose = FALSE;
    static VOptionDescRec options[] = {
        {
            "in", VStringRepn, 0, & in_files, & in_found, NULL,
            "Input files"
        },
        {
            "mask", VStringRepn, 1, & mask_filename, VRequiredOpt, NULL,
            "File containing mask image"
        },
        {
            "id", VShortRepn, 1, & id, VOptionalOpt, NULL,
            "Blob id (use 0 to specify all blobs)"
        },
        {
            "report", VStringRepn, 1, & report_filename, VOptionalOpt, NULL,
            "File containing output report"
        },
        {
            "verbose", VBooleanRepn, 1, & verbose, VOptionalOpt, NULL,
            "Whether to report t-test results"
        },
    };
    FILE *f, *fp;
    VStringConst in_filename;
    VAttrList list;
    VAttrListPosn posn;
    VImage src = NULL, mask = NULL;
    VUByte *ubyte_pp;
    int maxlabel, label;
    double nx, mean, var;
    int nbands, nrows, ncols;
    int i, k, npixels;
    double ave1 = 0, var1 = 0, t, p, nu, sum1, sum2, nx1;
    float *sumb, *sumr, *sumc, *size;
    int b, r, c;
    VString buffer;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vmaskave V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments: */
    if(! VParseCommand(VNumber(options), options, & argc, argv) ||
            ! VIdentifyFiles(VNumber(options), options, "in", & argc, argv, 0))
        goto Usage;
    if(argc > 1) {
        VReportBadArgs(argc, argv);
Usage:
        VReportUsage(argv[0], VNumber(options), options, NULL);
        exit(EXIT_FAILURE);
    }
    /* Read mask image: */
    fp = VOpenInputFile(mask_filename, TRUE);
    list = VReadFile(fp, NULL);
    if(! list)
        VError("Error reading mask image");
    fclose(fp);
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & mask);
        if(VPixelRepn(mask) != VUByteRepn)
            VError(" mask image must be of type ubyte");
        break;
    }
    if(mask == NULL)
        VError("mask image not found");
    /*
    ** get number of roi's
    */
    nbands = VImageNBands(mask);
    nrows = VImageNRows(mask);
    ncols = VImageNColumns(mask);
    npixels = nbands * nrows * ncols;
    maxlabel = 0;
    ubyte_pp = VPixelPtr(mask, 0, 0, 0);
    for(i = 0; i < npixels; i++) {
        if(*ubyte_pp > maxlabel)
            maxlabel = *ubyte_pp;
        ubyte_pp++;
    }
    /*
    ** get center of gravity of each roi
    */
    sumb = (float *) VMalloc(sizeof(float) * (maxlabel + 1));
    sumr = (float *) VMalloc(sizeof(float) * (maxlabel + 1));
    sumc = (float *) VMalloc(sizeof(float) * (maxlabel + 1));
    size = (float *) VMalloc(sizeof(float) * (maxlabel + 1));
    for(i = 0; i <= maxlabel; i++)
        sumb[i] = sumr[i] = sumc[i] = size[i] = 0;
    ubyte_pp = VImageData(mask);
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                i = *ubyte_pp;
                if(i > 0) {
                    sumb[i] += b;
                    sumr[i] += r;
                    sumc[i] += c;
                    size[i]++;
                }
                ubyte_pp++;
            }
        }
    }
    if(strlen(report_filename) > 1) {
        fp = fopen(report_filename, "w");
        if(fp == 0)
            VError(" error opening report file %s", report_filename);
    } else
        fp = stderr;
    fprintf(fp, "\n roi   x    y    z\n");
    fprintf(fp, "-------------------\n");
    for(i = 1; i <= maxlabel; i++) {
        if(size[i] > 0) {
            sumb[i] /= size[i];
            sumr[i] /= size[i];
            sumc[i] /= size[i];
            fprintf(fp, " %3d  %3.0f  %3.0f  %3.0f\n", i, sumc[i], sumr[i], sumb[i]);
        }
    }
    fprintf(fp, "\n\n");
    VFree(sumb);
    VFree(sumr);
    VFree(sumc);
    VFree(size);
    /*
    ** For each input file and each label:
    */
    fprintf(fp, "\n image  roi    mean   sigma    size\n");
    fprintf(fp, "-----------------------------------\n");
    for(label = 1; label <= maxlabel; label++) {
        if(id > 0 && label != id)
            continue;
        sum1 = sum2 = nx1 = 0;
        for(i = 0; i < in_files.number; i++) {
            in_filename = ((VStringConst *) in_files.vector)[i];
            /* Read its contents: */
            if(strcmp(in_filename, "-") == 0)
                f = stdin;
            else {
                f = fopen(in_filename, "r");
                if(! f)
                    VError("Failed to open input file %s", in_filename);
            }
            if(!(list = VReadFile(f, NULL)))
                exit(EXIT_FAILURE);
            fclose(f);
            k = 0;
            for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
                if(VGetAttrRepn(& posn) != VImageRepn)
                    continue;
                VGetAttrValue(& posn, NULL, VImageRepn, & src);
                if(VPixelRepn(src) != VFloatRepn)
                    continue;
                if(VGetAttr(VImageAttrList(src), "modality", NULL,
                            VStringRepn, (VPointer) & buffer) != VAttrFound)
                    continue;
                if(strcmp(buffer, "conimg") != 0)
                    continue;
                VRoiInfo(src, mask, label, &mean, &var, &nx);
                sum1 += mean;
                sum2 += mean * mean;
                nx1++;
                VDestroyImage(src);
                fprintf(fp, " %3d   %3d   %6.3f  %6.3f  %6.0f\n", i, label, mean, var, nx);
                k++;
            }
            if(k == 0)
                VWarning(" no contrast map found in %s", in_filename);
        }
        if(nx1 > 1.0f) {
            ave1 = sum1 / nx1;
            var1 = (sum2 - nx1 * ave1 * ave1) / (nx1 - 1.0f);
        }
        if(verbose) {
            ttest(ave1, var1, nx1, (double) 0, (double) 0, nx1, &t, &nu, &p);
            fprintf(fp, " mean: %7.3f   t: %7.3f  df: %g   p: %f\n\n", ave1, t, nu, p);
        }
    }
    if(strlen(report_filename) > 1)
        fclose(fp);
    fprintf(stderr, "%s: done.\n", argv[0]);
    exit(EXIT_SUCCESS);
}



void
VRoiInfo(VImage src, VImage mask, VShort id,
         double *mean, double *sigma, double *nx) {
    int nbands, nrows, ncols;
    int i, npixels;
    VUByte *ubyte_pp;
    VFloat *float_pp;
    double sum1, sum2, var;
    if(VPixelRepn(src) != VFloatRepn)
        VError(" image must be in float repn");
    nbands = VImageNBands(src);
    nrows = VImageNRows(src);
    ncols = VImageNColumns(src);
    if(nbands != VImageNBands(mask))
        VError(" mask and src image must have the same number of slices");
    if(nrows != VImageNRows(mask))
        VError(" mask and src image must have the same number of rows");
    if(ncols != VImageNColumns(mask))
        VError(" mask and src image must have the same number of columns");
    npixels = nbands * nrows * ncols;
    ubyte_pp = VImageData(mask);
    float_pp = VImageData(src);
    (*nx) = sum1 = sum2 = 0;
    for(i = 0; i < npixels; i++) {
        if(*ubyte_pp == id) {
            sum1 += *float_pp;
            sum2 += ((*float_pp) * (*float_pp));
            (*nx)++;
        }
        ubyte_pp++;
        float_pp++;
    }
    if((*nx) > 1.0f) {
        (*mean) = sum1 / (*nx);
        var = (sum2 - (*nx) * (*mean) * (*mean)) / ((*nx) - 1.0f);
        (*sigma) = sqrt(var);
    }
}

