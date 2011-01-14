/****************************************************************
 *
 * Program: vwilcoxon
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2001, <lipsia@cbs.mpg.de>
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
 * $Id: vwilcoxon.c 3581 2009-06-04 07:33:10Z proeger $
 *
 *****************************************************************/

#include "viaio/Vlib.h"
#include "viaio/file.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include <viaio/VImage.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_cdf.h>


#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))


/* two-tailed significance levels */
extern double LevelOfSignificanceWXMPSR(double Winput, long int N);
extern double p2z(double);
extern void getLipsiaVersion(char*,size_t);


VImage
VWilcoxonTest(VImage *src, VImage dest, int n) {
    int i, m, k, b, r, c, nslices, nrows, ncols;
    int sumpos, sumneg, w;
    double wx, u, z, p, tiny = 1.0e-8;
    double *ptr1, *ptr2;
    float sum, nx, smooth = 0;
    float *table = NULL;
    gsl_vector *vec1 = NULL, *vec2 = NULL;
    gsl_permutation *perm = NULL, *rank = NULL;
    extern void gsl_sort_vector_index(gsl_permutation *, gsl_vector *);
    extern float *getTable(int);
    gsl_set_error_handler_off();
    if(src[0] == NULL)
        VError(" no input images found");
    nslices = VImageNBands(src[0]);
    nrows   = VImageNRows(src[0]);
    ncols   = VImageNColumns(src[0]);
    dest = VCopyImage(src[0], NULL, VAllBands);
    VFillImage(dest, VAllBands, 0);
    VSetAttr(VImageAttrList(dest), "num_images", NULL, VShortRepn, (VShort)n);
    VSetAttr(VImageAttrList(dest), "patient", NULL, VStringRepn, "wilcoxon_test");
    VSetAttr(VImageAttrList(dest), "modality", NULL, VStringRepn, "zmap");
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
    m = 0;
    for(i = 1; i <= n; i++)
        m += i;
    if(n >= 18) {
        table = getTable(n);
        for(i = 0; i < m; i++) {
            p = table[i];
            p *= 0.5;
            if(p < tiny)
                p = tiny;
            z = p2z(p);
            if(z < 0)
                z = 0;
            table[i] = z;
        }
    } else {
        table = (float *) VMalloc(sizeof(float) * m);
        for(i = 0; i < m; i++) {
            wx = i;
            p = LevelOfSignificanceWXMPSR(wx, (long int)n);
            p *= 0.5;
            if(p < tiny)
                p = tiny;
            z = p2z(p);
            if(z < 0)
                z = 0;
            table[i] = z;
        }
    }
    vec1 = gsl_vector_calloc(n);
    vec2 = gsl_vector_calloc(n);
    perm = gsl_permutation_alloc(n);
    rank = gsl_permutation_alloc(n);
    for(b = 0; b < nslices; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                k = 0;
                ptr1 = vec1->data;
                ptr2 = vec2->data;
                for(i = 0; i < n; i++) {
                    u = VPixel(src[i], b, r, c, VFloat);
                    if(isinf(u) || isnan(u))
                        goto next;
                    if(ABS(u) > tiny)
                        k++;
                    *ptr1++ = ABS(u);
                    *ptr2++ = u;
                }
                if(k < n / 2)
                    continue;
                gsl_sort_vector_index(perm, vec1);
                gsl_permutation_inverse(rank, perm);
                sumpos = sumneg = 0;
                ptr2 = vec2->data;
                for(i = 0; i < n; i++) {
                    u = *ptr2++;
                    if(u > 0)
                        sumpos += rank->data[i];
                    else if(u < 0)
                        sumneg += rank->data[i];
                }
                w = sumpos;
                if(sumpos > sumneg)
                    w = sumneg;
                if(w >= m || w < 0)
                    z = 0;
                else
                    z = table[w];
                if(sumneg > sumpos)
                    z = -z;
                VPixel(dest, b, r, c, VFloat) = z;
next:
                ;
            }
        }
    }
    return dest;
}



int main(int argc, char *argv[]) {
    static VArgVector in_files;
    static VString out_filename;
    static VOptionDescRec options[] = {
        {"in", VStringRepn, 0, & in_files, VRequiredOpt, NULL, "Input files" },
        {"out", VStringRepn, 1, & out_filename, VRequiredOpt, NULL, "Output file" }
    };
    FILE *fp = NULL;
    VStringConst in_filename;
    VAttrList list, out_list;
    VAttrListPosn posn;
    VString str;
    VImage src, *src1, dest = NULL;
    int i, n, npix;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vwilcoxon V%s", ver);
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
    /*
    ** read images
    */
    n = in_files.number;
    src1 = (VImage *) VCalloc(n, sizeof(VImage));
    for(i = 0; i < n; i++) {
        src1[i] = NULL;
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
            if(VGetAttr(VImageAttrList(src), "modality", NULL,
                        VStringRepn, (VPointer) & str) == VAttrFound) {
                if(strcmp(str, "conimg") != 0)
                    continue;
            }
            if(i == 0)
                npix = VImageNPixels(src);
            else if(npix !=  VImageNPixels(src))
                VError(" inconsistent image dimensions");
            src1[i] = src;
            break;
        }
        if(src1[i] == NULL)
            VError(" no contrast image found in %s", in_filename);
    }
    /*
    ** wilcoxon test
    */
    dest = VWilcoxonTest(src1, dest, n);
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
