/****************************************************************
 *
 * Program: vfdr
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
 * $Id: vfdr.c 3394 2008-08-28 09:12:39Z lohmann $
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
#include <via.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_errno.h>


#define ABS(x) ((x) > 0 ? (x) : -(x))


VDictEntry SIGNDict[] = {
    { "pos",  1 },
    { "neg", -1 },
    { NULL }
};


extern double p2t(double, double);
extern double t2z(double, double);
extern float t2z_approx(float, float);
extern double t2p(double, double);
extern double p2z(double);
extern double z2p(double);
extern char *getLipsiaVersion();

double
VFDR(VImage src, VShort sign, VFloat alpha) {
    int i, j = 0, n, tmap = 0;
    double t, df = 0, z, p, ix, nx, tiny = 1.0e-6;
    VFloat *float_pp;
    gsl_vector *v = NULL;
    VString buf;
    extern void gsl_sort_vector(gsl_vector *);
    gsl_set_error_handler_off();
    if(VGetAttr(VImageAttrList(src), "modality", NULL,
                VStringRepn, (VPointer) & buf) == VAttrFound) {
        tmap = 0;
        if(strcmp(buf, "tmap") == 0)
            tmap = 1;
        if(strcmp(buf, "zmap") == 0)
            tmap = 2;
        if(tmap < 1)
            VError(" image must be 'tmap' or 'zmap', not %s", buf);
    } else
        VError(" attribute 'modality' not found");
    if(tmap == 1) {
        if(VGetAttr(VImageAttrList(src), "df", NULL, VDoubleRepn, (VPointer) &df) != VAttrFound)
            VError(" degrees of freedom unknown");
    }
    /*
    ** count number of voxels
    */
    n = 0;
    float_pp = VImageData(src);
    for(i = 0; i < VImageNPixels(src); i++) {
        z = *float_pp++;
        if(ABS(z) < tiny)
            continue;
        n++;
    }
    v = gsl_vector_calloc(n);
    /*
    ** convert to t to p
    */
    if(tmap == 1) {
        float_pp = VImageData(src);
        j = 0;
        for(i = 0; i < VImageNPixels(src); i++) {
            t = *float_pp++;
            if(ABS(t) < tiny)
                continue;
            if(sign > 0 && t < 0)
                p = 1;
            else if(sign < 0 && t > 0)
                p = 1;
            else
                p = t2p(t, df);
            gsl_vector_set(v, j, p);
            j++;
        }
    }
    /*
    **  convert z to p
    */
    if(tmap == 2) {
        float_pp = VImageData(src);
        j = 0;
        for(i = 0; i < VImageNPixels(src); i++) {
            z = *float_pp++;
            if(ABS(z) < tiny)
                continue;
            if(sign > 0 && z < 0)
                p = 1;
            else if(sign < 0 && z > 0)
                p = 1;
            else
                p = z2p(z);
            gsl_vector_set(v, j, p);
            j++;
        }
    }
    /*
    ** sort p-vals
    */
    gsl_sort_vector(v);
    /*
    ** get FDR threshold
    */
    i  = n;
    ix = (double)i;
    nx = (double)n;
    p  = gsl_vector_get(v, i - 1);
    while(i > 0 && p > ix * alpha / nx) {
        i--;
        ix = (double)i;
        if(i < 1)
            break;
        p = gsl_vector_get(v, i - 1);
    }
    fprintf(stderr, "   significant:  %d of %d\n", i, n);
    if(i < 1)
        return 0;
    z = t = 0;
    if(tmap == 2) {
        z = p2z(p);
        if(sign < 1)
            z = -z;
        fprintf(stderr, " FDR threshold:  p= %.5f,  z= %.5f\n", p, z);
    } else {
        t = p2t(p, df);
        if(sign < 1)
            t = -t;
        fprintf(stderr, " FDR threshold:  p= %.5f,  t= %.5f\n", p, t);
    }
    return p;
}


VImage
ApplyThreshold(VImage src, VString filename, VShort sign, double p0, VFloat minsize) {
    VImage dest = NULL, tmp = NULL, label_image = NULL;
    VString buf, str;
    int i, k, nl, msize = 0, tmap = 0;
    float x0, x1, x2;
    double z, p, df = 0;
    VBit *bin_pp;
    VFloat *float_pp;
    if(p0 >= 0.5) {
        VFillImage(src, VAllBands, 0);
        return src;
    }
    /*
    ** check modality, and df
    */
    if(VGetAttr(VImageAttrList(src), "modality", NULL,
                VStringRepn, (VPointer) & buf) == VAttrFound) {
        tmap = 0;
        if(strcmp(buf, "tmap") == 0)
            tmap = 1;
        if(strcmp(buf, "zmap") == 0)
            tmap = 2;
        if(tmap < 1)
            VError(" image must be 'tmap' or 'zmap', not %s", buf);
    } else
        VError(" attribute 'modality' not found");
    if(tmap == 1) {
        if(VGetAttr(VImageAttrList(src), "df", NULL, VDoubleRepn, (VPointer) &df) != VAttrFound)
            VError(" degrees of freedom unknown");
        fprintf(stderr, " df= %g\n", df);
    }
    /*
    ** get voxel size
    */
    if(VGetAttr(VImageAttrList(src), "voxel", NULL, VStringRepn, (VPointer) &str) != VAttrFound)
        VError(" attribute 'voxel' not found");
    sscanf(str, "%f %f %f", &x0, &x1, &x2);
    msize = (int)(minsize / (x0 * x1 * x2) + 0.5);
    if(msize < 1 && minsize > 0) {
        VWarning(" parameter 'minsize' has no effect (0 voxels). Units are in mm^3 !");
    }
    /*
    ** thresholding
    */
    tmp = VCreateImage(VImageNBands(src), VImageNRows(src), VImageNColumns(src), VBitRepn);
    VFillImage(tmp, VAllBands, 0);
    bin_pp   = VImageData(tmp);
    float_pp = VImageData(src);
    for(i = 0; i < VImageNPixels(src); i++) {
        z = *float_pp;
        *bin_pp = 0;
        if(ABS(z) > 0) {
            if(tmap == 1)
                p = t2p(z, df);
            else
                p = z2p(z);
            if(p <= p0) {
                if((sign > 0 && z > 0) || (sign < 0 && z < 0))
                    *bin_pp = 1;
            }
        }
        bin_pp++;
        float_pp++;
    }
    if(msize > 0) {
        label_image = VLabelImage3d(tmp, NULL, (int)26, VShortRepn, &nl);
        if(nl < 1)
            VWarning(" no voxels found, perhaps areas too small");
        tmp = VDeleteSmall(label_image, tmp, msize);
        fprintf(stderr, " areas with less than %d voxels deleted.\n", msize);
    }
    dest = VCopyImage(src, NULL, VAllBands);
    VSetAttr(VImageAttrList(dest), "FDR_threshold", NULL, VFloatRepn, (VFloat)p0);
    bin_pp   = VImageData(tmp);
    float_pp = VImageData(dest);
    k = 0;
    for(i = 0; i < VImageNPixels(src); i++) {
        if(*bin_pp == 0)
            *float_pp = 0;
        else
            k++;
        float_pp++;
        bin_pp++;
    }
    fprintf(stderr, " significant voxels: %d\n", k);
    return dest;
}



int
main(int argc, char *argv[]) {
    static VShort  sign = 1;
    static VFloat  alpha = 0.05;
    static VFloat  minsize = 81;
    static VString filename = "";
    static VOptionDescRec  options[] = {
        {"out", VStringRepn, 1, (VPointer) &filename, VOptionalOpt, NULL, "Thresholded output file"},
        {
            "sign", VShortRepn, 1, (VPointer) &sign, VOptionalOpt, SIGNDict,
            "Whether to analyze positive or negative values"
        },
        {"alpha", VFloatRepn, 1, (VPointer) &alpha, VOptionalOpt, NULL, "Threshold"},
        {"minsize", VFloatRepn, 1, (VPointer) &minsize, VOptionalOpt, NULL, "Minimum area size in mm^3"}
    };
    FILE *in_file = NULL, *fp = NULL;
    VAttrList list = NULL, out_list = NULL;
    VAttrListPosn posn;
    VImage src = NULL, dest = NULL;
    double p0 = 0;
    char prg_name[50];
    sprintf(prg_name, "vfdr V%s", getLipsiaVersion());
    fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, NULL);
    /* process */
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(VPixelRepn(src) != VFloatRepn)
            continue;
        p0 = VFDR(src, sign, alpha);
        if(strlen(filename) > 2)
            dest = ApplyThreshold(src, filename, sign, p0, minsize);
        break;
    }
    if(dest) {
        fp = VOpenOutputFile(filename, TRUE);
        out_list = VCreateAttrList();
        VHistory(VNumber(options), options, prg_name, &list, &out_list);
        VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
        if(! VWriteFile(fp, out_list))
            exit(1);
        fclose(fp);
    }
    fprintf(stderr, "%s: done.\n", argv[0]);
    exit(0);
}
