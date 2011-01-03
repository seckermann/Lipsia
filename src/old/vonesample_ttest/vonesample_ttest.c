/****************************************************************
 *
 * Program: vonesample_ttest
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2007, <lipsia@cbs.mpg.de>
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
 * $Id: vonesample_ttest.c 3581 2009-06-04 07:33:10Z proeger $
 *
 *****************************************************************/

#include "viaio/Vlib.h"
#include "viaio/file.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include <viaio/VImage.h>

#include <gsl/gsl_cdf.h>
#include <gsl/gsl_errno.h>


#include <math.h>
#include <stdio.h>
#include <string.h>

#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))


extern double t2z(double, double);
extern float t2z_approx(float, float);
extern char *getLipsiaVersion();

void
avevar(float *data, int n, float *a, float *v) {
    int j;
    float ave, var, nx, s, u;
    nx = (float)n;
    ave = 0;
    for(j = 0; j < n; j++)
        ave += data[j];
    ave /= nx;
    var = u = 0;
    for(j = 0; j < n; j++) {
        s = data[j] - ave;
        u   += s;
        var += s * s;
    }
    var = (var - u * u / nx) / (nx - 1);
    *v = var;
    *a = ave;
}


VImage
OneSampleTest(VImage *src, VImage dest, int n, VShort type) {
    int    i, k, b, r, c, nslices, nrows, ncols;
    float  *data = NULL, ave, var, sqr_nx;
    float  u, t, z, df, nx, sum, smooth = 0;
    float  tiny = 1.0e-10;
    gsl_set_error_handler_off();
    nslices = VImageNBands(src[0]);
    nrows   = VImageNRows(src[0]);
    ncols   = VImageNColumns(src[0]);
    dest = VCopyImage(src[0], NULL, VAllBands);
    VFillImage(dest, VAllBands, 0);
    VSetAttr(VImageAttrList(dest), "num_images", NULL, VShortRepn, (VShort)n);
    VSetAttr(VImageAttrList(dest), "patient", NULL, VStringRepn, "one_sample_ttest");
    if(type == 0)
        VSetAttr(VImageAttrList(dest), "modality", NULL, VStringRepn, "tmap");
    if(type == 1)
        VSetAttr(VImageAttrList(dest), "modality", NULL, VStringRepn, "zmap");
    VSetAttr(VImageAttrList(dest), "df", NULL, VShortRepn, (n - 1));
    /* get smoothness estimates */
    sum = nx = 0;
    for(i = 0; i < n; i++) {
        if(VGetAttr(VImageAttrList(src[i]), "smoothness", NULL, VFloatRepn, &smooth) == VAttrFound) {
            sum += smooth;
            nx++;
        }
    }
    if(nx > 1) {
        VSetAttr(VImageAttrList(dest), "smoothness", NULL, VFloatRepn, sum / nx);
    }
    data = (float *) VCalloc(n, sizeof(float));
    for(b = 0; b < nslices; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                k = 0;
                for(i = 0; i < n; i++) {
                    data[i] = 0;
                    u = VPixel(src[i], b, r, c, VFloat);
                    if(isnan(u) || isinf(u))
                        continue;
                    if(ABS(u) > tiny)
                        k++;
                    data[i] = u;
                }
                if(k < n - 3)
                    continue;
                nx = (float)k;
                sqr_nx = sqrt(nx);
                df = nx - 1;
                avevar(data, n, &ave, &var);
                if(var < tiny)
                    continue;
                t = sqr_nx * ave / sqrt(var);
                switch(type) {
                case 0:
                    VPixel(dest, b, r, c, VFloat) = t;
                    break;
                case 1:
                    /* z = t2z_approx(t,df); */
                    z = t2z((double)t, (double)df);
                    if(t < 0)
                        z = -z;
                    VPixel(dest, b, r, c, VFloat) = (VFloat)z;
                    break;
                default:
                    VError(" illegal type");
                }
            }
        }
    }
    return dest;
}


VDictEntry TypeDict[] = {
    { "tmap", 0 },
    { "zmap", 1 },
    { NULL }
};


int main(int argc, char *argv[]) {
    static VArgVector in_files;
    static VString out_filename;
    static VShort type = 1;
    static VOptionDescRec options[] = {
        {"in", VStringRepn, 0, & in_files, VRequiredOpt, NULL, "Input files" },
        {"type", VShortRepn, 1, (VPointer) &type, VOptionalOpt, TypeDict, "output type"},
        {"out", VStringRepn, 1, & out_filename, VRequiredOpt, NULL, "Output file" }
    };
    FILE *fp = NULL;
    VStringConst in_filename;
    VAttrList list, out_list;
    VAttrListPosn posn;
    VString str;
    VImage src, *src1, dest = NULL;
    int i, n = 0, npix = 0;
    char prg_name[50];
    sprintf(prg_name, "vonesample_ttest V%s", getLipsiaVersion());
    fprintf(stderr, "%s\n", prg_name);
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
    if(type < 0 || type > 1)
        VError(" illegal type");
    /*
    ** read images
    */
    n = in_files.number;
    src1 = (VImage *) VCalloc(n, sizeof(VImage));
    for(i = 0; i < n; i++) {
        in_filename = ((VStringConst *) in_files.vector)[i];
        fprintf(stderr, " %3d:  %s\n", i, in_filename);
        fp = VOpenInputFile(in_filename, TRUE);
        list = VReadFile(fp, NULL);
        if(! list)
            VError("Error reading image");
        fclose(fp);
        src1[i] = NULL;
        for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & src);
            if(VPixelRepn(src) != VFloatRepn)
                continue;
            if(VGetAttr(VImageAttrList(src), "modality", NULL, VStringRepn, &str) == VAttrFound) {
                if(strcmp(str, "conimg") != 0)
                    continue;
            }
            if(i == 0)
                npix = VImageNPixels(src);
            else if(npix != VImageNPixels(src))
                VError(" inconsistent image dimensions");
            src1[i] = src;
            break;
        }
        if(src1[i] == NULL)
            VError(" no contrast image found in %s", in_filename);
    }
    /* one-sample t-test */
    dest = OneSampleTest(src1, dest, n, type);
    /*
    ** output
    */
    out_list = VCreateAttrList();
    VHistory(VNumber(options), options, prg_name, &list, &out_list);
    VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
    fp = VOpenOutputFile(out_filename, TRUE);
    if(! VWriteFile(fp, out_list))
        exit(1);
    fclose(fp);
    fprintf(stderr, "%s: done.\n", argv[0]);
    exit(0);
}
