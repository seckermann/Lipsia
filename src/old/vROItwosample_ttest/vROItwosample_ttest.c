/****************************************************************
 *
 * Program: vROItwosample_ttest
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
 * $Id: vROItwosample_ttest.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


#include "viaio/Vlib.h"
#include "viaio/file.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include <viaio/VImage.h>
#include <via.h>

#include <gsl/gsl_cdf.h>
#include <gsl/gsl_errno.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SQR(x) ((x)*(x))

extern void VGetTalCoord(VImage, float, float, float, float *, float *, float *);
extern double t2z(double, double);
extern float t2z_approx(float, float);
extern double t2p(double, double);
extern void getLipsiaVersion(char*,size_t);




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


void
VROItwosample_ttest(VImage *src1, VImage *src2, VImage mask, int n1, int n2, FILE *fp) {
    VString str;
    int i, j, id, b, r, c, c0, c1;
    float ave1 = 0, ave2 = 0, var1 = 0, var2 = 0;
    float sum = 0, nx = 0, u;
    float t, z, p, df, nx1, nx2, *data1 = NULL, *data2 = NULL;
    float tiny = 1.0e-8;
    float xa, ya, za, xx, yy, zz, voxelsize;
    double mean[3];
    Volumes volumes;
    Volume vol;
    VTrack tc;
    gsl_set_error_handler_off();
    voxelsize = 1;
    if(VGetAttr(VImageAttrList(mask), "voxel", NULL,
                VStringRepn, (VPointer) & str) == VAttrFound) {
        sscanf(str, "%f %f %f", &xa, &ya, &za);
        voxelsize = xa * ya * za;
    }
    data1 = (float *) VCalloc(n1, sizeof(float));
    data2 = (float *) VCalloc(n2, sizeof(float));
    nx1 = (double)n1;
    nx2 = (double)n2;
    fprintf(stderr, "\n");
    fprintf(stderr, "  ROI              addr            mm^3       t        z        p   \n");
    fprintf(stderr, " -------------------------------------------------------------------\n");
    if(fp) {
        fprintf(fp, "\n");
        fprintf(fp, "  ROI              addr            mm^3       t        z        p   \n");
        fprintf(fp, " -------------------------------------------------------------------\n");
    }
    volumes = VImage2Volumes(mask);
    for(vol = volumes->first; vol != NULL; vol = vol->next) {
        VolumeCentroid(vol, mean);
        xx = mean[2];
        yy = mean[1];
        zz = mean[0];
        VGetTalCoord(src1[0], zz, yy, xx, &xa, &ya, &za);
        id = vol->label;
        for(i = 0; i < n1; i++) {
            sum = nx = 0;
            for(j = 0; j < VolumeNBuckets(vol); j++) {
                for(tc = VFirstTrack(vol, j); VTrackExists(tc); tc = VNextTrack(tc)) {
                    b  = tc->band;
                    r  = tc->row;
                    c0 = tc->col;
                    c1 = c0 + tc->length;
                    for(c = c0; c < c1; c++) {
                        u = VPixel(src1[i], b, r, c, VFloat);
                        if(ABS(u) < tiny)
                            continue;
                        sum += u;
                        nx++;
                    }
                }
            }
            if(nx < 1) {
                VWarning(" group1: no voxels in ROI %d", id);
                goto next;
            }
            data1[i]  = sum / nx;
        }
        for(i = 0; i < n2; i++) {
            sum = nx = 0;
            for(j = 0; j < VolumeNBuckets(vol); j++) {
                for(tc = VFirstTrack(vol, j); VTrackExists(tc); tc = VNextTrack(tc)) {
                    b  = tc->band;
                    r  = tc->row;
                    c0 = tc->col;
                    c1 = c0 + tc->length;
                    for(c = c0; c < c1; c++) {
                        u = VPixel(src2[i], b, r, c, VFloat);
                        if(ABS(u) < tiny)
                            continue;
                        sum += u;
                        nx++;
                    }
                }
            }
            if(nx < 1) {
                VWarning(" group2: no voxels in ROI %d", id);
                goto next;
            }
            data2[i]  = sum / nx;
        }
        avevar(data1, n1, &ave1, &var1);
        avevar(data2, n2, &ave2, &var2);
        if(var1 < tiny || var2 < tiny) {
            VWarning(" no variance in ROI %d", id);
            continue;
        }
        z = t = p = 0;
        t = (ave1 - ave2) / sqrt(var1 / nx1 + var2 / nx2);
        df = SQR(var1 / nx1 + var2 / nx2) / (SQR(var1 / nx1) / (nx1 - 1.0) + SQR(var2 / nx2) / (nx2 - 1.0));
        p = t2p((double)t, (double)df);
        z = t2z((double)t, (double)df);
        if(t < 0)
            z = -z;
        fprintf(stderr, " %3d  %7.2f %7.2f %7.2f   %7.0f   %7.3f  %7.3f  %7.4f\n",
                id, xa, ya, za, nx * voxelsize, t, z, p);
        if(fp)
            fprintf(fp, " %3d  %7.2f %7.2f %7.2f   %7.0f   %7.3f  %7.3f  %7.4f\n",
                    id, xa, ya, za, nx * voxelsize, t, z, p);
next:
        ;
    }
    fprintf(stderr, "\n");
    if(fp) {
        fprintf(fp, "\n");
        fclose(fp);
    }
}




int main(int argc, char *argv[]) {
    static VArgVector in_files1;
    static VArgVector in_files2;
    static VArgVector report_file;
    static VArgVector mask_file;
    static VOptionDescRec options[] = {
        { "in1", VStringRepn, 0, & in_files1, VRequiredOpt, NULL, "Input files 1" },
        { "in2", VStringRepn, 0, & in_files2, VRequiredOpt, NULL, "Input files 2" },
        { "report", VStringRepn, 0, & report_file, VRequiredOpt, NULL, "Report file" },
        { "mask", VStringRepn, 0, & mask_file, VRequiredOpt, NULL, "Mask file" }
    };
    FILE *fp = NULL;
    VStringConst in_filename;
    VAttrList list1, list2, list3;
    VAttrListPosn posn;
    VString str;
    VImage src, *src1, *src2, mask = NULL;
    int i, n1, n2;
   char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vROItwosample_ttest V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    if(!VParseCommand(VNumber(options), options, & argc, argv)) {
        VReportUsage(argv[0], VNumber(options), options, NULL);
        exit(EXIT_FAILURE);
    }
    /* ini */
    n1 = in_files1.number;
    n2 = in_files2.number;
    if(mask_file.number > 1)
        VError(" multiple masks not allowed");
    /* images 1 */
    src1 = (VImage *) VMalloc(sizeof(VImage) * n1);
    for(i = 0; i < n1; i++) {
        src1[i] = NULL;
        in_filename = ((VStringConst *) in_files1.vector)[i];
        fp = VOpenInputFile(in_filename, TRUE);
        if(!fp)
            VError("Error opening file %s", in_filename);
        list1 = VReadFile(fp, NULL);
        if(! list1)
            VError("Error reading file %s", in_filename);
        fclose(fp);
        for(VFirstAttr(list1, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & src);
            if(VPixelRepn(src) != VFloatRepn)
                continue;
            if(VGetAttr(VImageAttrList(src), "modality", NULL, VStringRepn, &str) == VAttrFound) {
                if(strcmp(str, "conimg") != 0)
                    continue;
            }
            src1[i] = src;
            break;
        }
        if(src1[i] == NULL)
            VError(" no contrast image found in %s", in_filename);
    }
    /* images 2 */
    src2 = (VImage *) VMalloc(sizeof(VImage) * n2);
    for(i = 0; i < n2; i++) {
        src2[i] = NULL;
        in_filename = ((VStringConst *) in_files2.vector)[i];
        fp = VOpenInputFile(in_filename, TRUE);
        if(!fp)
            VError("Error opening file %s", in_filename);
        list2 = VReadFile(fp, NULL);
        if(! list2)
            VError("Error reading file %s", in_filename);
        fclose(fp);
        for(VFirstAttr(list2, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & src);
            if(VPixelRepn(src) != VFloatRepn)
                continue;
            if(VGetAttr(VImageAttrList(src), "modality", NULL, VStringRepn, &str) == VAttrFound) {
                if(strcmp(str, "conimg") != 0)
                    continue;
            }
            src2[i] = src;
            break;
        }
        if(src2[i] == NULL)
            VError(" no contrast image found in %s", in_filename);
    }
    fprintf(stderr, " group 1:\n");
    for(i = 0; i < n1; i++)
        fprintf(stderr, "%3d:  %s\n", i, ((VStringConst *) in_files1.vector)[i]);
    fprintf(stderr, "\n group 2:\n");
    for(i = 0; i < n2; i++)
        fprintf(stderr, "%3d:  %s\n", i, ((VStringConst *) in_files2.vector)[i]);
    fprintf(stderr, "\n");
    /* mask image */
    in_filename = ((VStringConst *) mask_file.vector)[0];
    fp = VOpenInputFile(in_filename, TRUE);
    list3 = VReadFile(fp, NULL);
    if(! list3)
        VError("Error reading image");
    fclose(fp);
    for(VFirstAttr(list3, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(VPixelRepn(src) != VUByteRepn)
            mask = VConvertImageCopy(src, NULL, VAllBands, VUByteRepn);
        else
            mask = src;
        break;
    }
    if(mask == NULL)
        VError(" mask mot found");
    /* open report_file */
    fp = NULL;
    in_filename = ((VStringConst *) report_file.vector)[0];
    fp = fopen(in_filename, "w");
    if(!fp)
        VError(" error opening file %s", in_filename);
    /* twosample t-test in ROIs */
    VROItwosample_ttest(src1, src2, mask, n1, n2, fp);
    exit(0);
}
