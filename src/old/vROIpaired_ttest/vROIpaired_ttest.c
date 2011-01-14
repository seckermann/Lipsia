/****************************************************************
 *
 * Program: vROIpaired_ttest
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
 * $Id: vROIpaired_ttest.c 3181 2008-04-01 15:19:44Z karstenm $
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

extern void VGetTalCoord(VImage, float, float, float, float *, float *, float *);
extern double t2z(double, double);
extern float t2z_approx(float, float);
extern double t2p(double, double);
extern void getLipsiaVersion(char*,size_t);


/*
** check if IDs of ROIs match across masks
*/
int
CheckROI(Volumes *volumes, int m, int nROI) {
    int i;
    Volume vol;
    int *table;
    if(m < 2)
        return 1;    /* no problem if just one mask */
    table = (int *) VCalloc(nROI + 1, sizeof(int));
    for(i = 0; i < m; i++) {
        for(vol = volumes[i]->first; vol != NULL; vol = vol->next) {
            table[vol->label]++;
        }
    }
    for(i = 1; i <= nROI; i++) {
        if(table[i] != m)
            VError(" ROI %d missing in at least one mask", i + 1);
    }
    return 1;
}



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
VROIpaired_ttest(VImage *src1, VImage *src2, VImage *mask, int n, int nmask, FILE *fp) {
    VString str;
    int i, j, id, b, r, c, c0, c1, nROI = 0;
    float ave1 = 0, ave2 = 0, var1 = 0, var2 = 0;
    float sum1 = 0, sum2 = 0, u, mx;
    float t, z, p, df, sd, cov, *data1 = NULL, *data2 = NULL;
    float tiny = 1.0e-8;
    float xa, ya, za, xx, yy, zz, voxelsize = 1;
    double mean[3];
    Volumes *volumes;
    Volume vol;
    VTrack tc;
    VBoolean found = FALSE;
    gsl_set_error_handler_off();
    /*
    ** get ROIs from mask
    */
    fprintf(stderr, "\n List of ROIs:\n");
    fprintf(fp, "\n List of ROIs:\n");
    volumes = (Volumes *) VCalloc(nmask, sizeof(Volumes));
    nROI = 0;
    for(i = 0; i < nmask; i++) {
        fprintf(stderr, "\n Mask %2d:\n", i + 1);
        fprintf(fp, "\n Mask %2d:\n", i + 1);
        volumes[i] = VImage2Volumes(mask[i]);
        voxelsize = 1;
        if(VGetAttr(VImageAttrList(mask[i]), "voxel", NULL,
                    VStringRepn, (VPointer) & str) == VAttrFound) {
            sscanf(str, "%f %f %f", &xa, &ya, &za);
            voxelsize = xa * ya * za;
        }
        fprintf(stderr, " ROI              addr               size(mm^3)\n");
        fprintf(stderr, "-----------------------------------------------\n");
        fprintf(fp, " ROI              addr               size(mm^3)\n");
        fprintf(fp, "-----------------------------------------------\n");
        for(vol = volumes[i]->first; vol != NULL; vol = vol->next) {
            VolumeCentroid(vol, mean);
            if(nROI < vol->label)
                nROI = vol->label;
            b = mean[0];
            r = mean[1];
            c = mean[2];
            xx = mean[2];
            yy = mean[1];
            zz = mean[0];
            VGetTalCoord(src1[0], zz, yy, xx, &xa, &ya, &za);
            id = vol->label;
            fprintf(stderr, " %2d    %7.2f  %7.2f  %7.2f    %7.0f\n",
                    id, xa, ya, za, voxelsize *(double)VolumeSize(vol));
            fprintf(fp, " %2d    %7.2f  %7.2f  %7.2f    %7.0f\n",
                    id, xa, ya, za, voxelsize *(double)VolumeSize(vol));
        }
    }
    fprintf(stderr, "\n\n");
    fprintf(fp, "\n\n");
    /* check consistency */
    if(nROI < 1)
        VError(" no ROIs found");
    CheckROI(volumes, nmask, nROI);
    /*
    ** process each ROI
    */
    fprintf(stderr, "\n");
    fprintf(stderr, "  ROI          mean          t        z       p   \n");
    fprintf(stderr, " --------------------------------------------------\n");
    if(fp) {
        fprintf(fp, "\n");
        fprintf(fp, "  ROI          mean          t        z       p   \n");
        fprintf(fp, " --------------------------------------------------\n");
    }
    df = n - 1;
    data1 = (float *) VCalloc(n, sizeof(float));
    data2 = (float *) VCalloc(n, sizeof(float));
    for(id = 1; id <= nROI; id++) {
        for(i = 0; i < n; i++) {
            j = 0;
            if(nmask > 1)
                j = i;
            found = FALSE;
            for(vol = volumes[j]->first; vol != NULL; vol = vol->next) {
                if(vol->label != id)
                    continue;
                found = TRUE;
                sum1 = sum2 = mx = 0;
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
                            sum1 += u;
                            u = VPixel(src2[i], b, r, c, VFloat);
                            if(ABS(u) < tiny)
                                continue;
                            sum2 += u;
                            mx++;
                        }
                    }
                }
                if(mx < 1) {
                    VWarning(" no voxels in ROI %d of mask %d", id, i);
                    data1[i] = data2[i] = 0;
                    continue;
                }
                data1[i] = sum1 / mx;
                data2[i] = sum2 / mx;
            }
            if(!found)
                goto next;
        }
        avevar(data1, n, &ave1, &var1);
        avevar(data2, n, &ave2, &var2);
        if(var1 < tiny || var2 < tiny) {
            VWarning(" no variance in ROI %d", id);
            continue;
        }
        z = t = p = 0;
        cov = 0;
        for(j = 0; j < n; j++)
            cov += (data1[j] - ave1) * (data2[j] - ave2);
        cov /= df;
        sd = sqrt((var1 + var2 - 2.0 * cov) / (df + 1.0));
        if(sd < tiny)
            continue;
        t = (ave1 - ave2) / sd;
        if(ABS(t) < tiny)
            p = z = 0;
        else {
            p = t2p((double)t, (double)df);
            z = t2z((double)t, (double)df);
            if(t < 0)
                z = -z;
        }
        fprintf(stderr, " %3d   %8.4f (%.3f)  %7.3f  %7.3f  %7.4f\n",
                id, (ave1 - ave2), sd, t, z, p);
        if(fp)
            fprintf(fp, " %3d  %8.4f (%.3f)  %7.3f  %7.3f  %7.4f\n",
                    id, (ave1 - ave2), sd, t, z, p);
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
    static VArgVector mask_file;
    static VArgVector report_file;
    static VOptionDescRec options[] = {
        { "in1", VStringRepn, 0, & in_files1, VRequiredOpt, NULL, "Input files 1" },
        { "in2", VStringRepn, 0, & in_files2, VRequiredOpt, NULL, "Input files 2" },
        { "report", VStringRepn, 0, & report_file, VRequiredOpt, NULL, "Report file" },
        { "mask", VStringRepn, 0, & mask_file, VRequiredOpt, NULL, "Mask file(s)" }
    };
    FILE *fp = NULL;
    VStringConst in_filename;
    VAttrList list1, list2, list3;
    VAttrListPosn posn;
    VString str;
    VImage src, *src1, *src2, *mask;
    int i, nimages = 0, mimages = 0;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vROIpaired_ttest V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    if(!VParseCommand(VNumber(options), options, & argc, argv)) {
        VReportUsage(argv[0], VNumber(options), options, NULL);
        exit(0);
    }
    /* get number of input images */
    nimages = in_files1.number;
    if(in_files2.number != nimages)
        VError(" inconsistent number of files: in1: %d, in2: %d ",
               nimages, in_files2.number);
    mimages = mask_file.number;
    if(nimages != mimages && mimages > 1)
        VError(" inconsistent number of files, images: %d, masks: %d", nimages, mimages);
    /* images 1 */
    src1 = (VImage *) VMalloc(sizeof(VImage) * nimages);
    for(i = 0; i < nimages; i++) {
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
    src2 = (VImage *) VMalloc(sizeof(VImage) * nimages);
    for(i = 0; i < nimages; i++) {
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
    for(i = 0; i < nimages; i++) {
        fprintf(stderr, "%3d:  %s  %s\n", i,
                ((VStringConst *) in_files1.vector)[i],
                ((VStringConst *) in_files2.vector)[i]);
    }
    fprintf(stderr, "\n");
    /* mask images */
    mask = (VImage *) VMalloc(sizeof(VImage) * mimages);
    for(i = 0; i < mimages; i++) {
        mask[i] = NULL;
        in_filename = ((VStringConst *) mask_file.vector)[i];
        fp = VOpenInputFile(in_filename, TRUE);
        if(!fp)
            VError("Error opening file %s", in_filename);
        list3 = VReadFile(fp, NULL);
        if(! list3)
            VError("Error reading file", in_filename);
        fclose(fp);
        for(VFirstAttr(list3, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & src);
            if(VPixelRepn(src) != VUByteRepn)
                mask[i] = VConvertImageCopy(src, NULL, VAllBands, VUByteRepn);
            else
                mask[i] = src;
            break;
        }
        if(mask[i] == NULL)
            VError(" mask %d not found", i);
    }
    /* open report_file */
    fp = NULL;
    in_filename = ((VStringConst *) report_file.vector)[0];
    fp = fopen(in_filename, "w");
    if(!fp)
        VError(" error opening file %s", in_filename);
    /* paired t-test in ROIs */
    VROIpaired_ttest(src1, src2, mask, nimages, mimages, fp);
    exit(0);
}
