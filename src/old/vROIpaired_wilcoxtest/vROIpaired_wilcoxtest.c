/****************************************************************
 *
 * Program: vROIpaired_wilcoxtest
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
 * $Id: vROIpaired_wilcoxtest.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include "viaio/Vlib.h"
#include "viaio/file.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include <viaio/VImage.h>
#include <via.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_errno.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))


extern void VGetTalCoord(VImage, float, float, float, float *, float *, float *);
extern float *getTable(int id);
extern double LevelOfSignificanceWXMPSR(double, long int);
extern double FDR(gsl_vector *, VShort, VFloat);
extern double p2z(double);
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
VROIPairedWilcoxTest(VImage *src1, VImage *src2, VImage *mask, int n, int nmask, FILE *fp) {
    VString str;
    int i, j, m, id, b, r, c, c0, c1, nROI = 0;
    float xa = 0, ya = 0, za = 0, xx, yy, zz;
    float sum1 = 0, sum2 = 0, nx = 0;
    float *data1 = NULL, *data2 = NULL;
    float voxelsize;
    double mean[3];
    Volumes *volumes;
    Volume vol;
    VTrack tc;
    VBoolean found = FALSE;
    int sumpos, sumneg, w;
    double wx, u, v, z, p, tiny = 1.0e-10;
    double *ptr1, *ptr2;
    float *table = NULL;
    gsl_vector *vec1 = NULL, *vec2 = NULL;
    gsl_permutation *perm = NULL, *rank = NULL;
    gsl_vector *zvals = NULL;
    extern void gsl_sort_vector_index(gsl_permutation *, gsl_vector *);
    gsl_set_error_handler_off();
    /*
    ** get tables
    */
    m = 0;
    for(i = 1; i <= n; i++)
        m += i;
    if(n > 18) {
        table = getTable(n);
        for(i = 0; i < m; i++) {
            p = table[i];
            p *= 0.5;
            table[i] = p;
        }
    } else {
        table = (float *) VMalloc(sizeof(float) * m);
        for(i = 0; i < m; i++) {
            for(i = 0; i < m; i++) {
                wx = i;
                p = LevelOfSignificanceWXMPSR(wx, (long int)n);
                p *= 0.5;
                table[i] = p;
            }
        }
    }
    /*
    ** alloc data
    */
    vec1  = gsl_vector_calloc(n);
    vec2  = gsl_vector_calloc(n);
    perm  = gsl_permutation_alloc(n);
    rank  = gsl_permutation_alloc(n);
    zvals = gsl_vector_calloc(n);
    data1 = (float *) VCalloc(n, sizeof(float));
    data2 = (float *) VCalloc(n, sizeof(float));
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
    fprintf(stderr, "  ROI       z       p   \n");
    fprintf(stderr, " -----------------------\n");
    if(fp) {
        fprintf(fp, "\n");
        fprintf(fp, "  ROI       z       p   \n");
        fprintf(fp, " -----------------------\n");
    }
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
                sum1 = sum2 = nx = 0;
                for(j = 0; j < VolumeNBuckets(vol); j++) {
                    for(tc = VFirstTrack(vol, j); VTrackExists(tc); tc = VNextTrack(tc)) {
                        b  = tc->band;
                        r  = tc->row;
                        c0 = tc->col;
                        c1 = c0 + tc->length;
                        for(c = c0; c < c1; c++) {
                            u = VPixel(src1[i], b, r, c, VFloat);
                            v = VPixel(src2[i], b, r, c, VFloat);
                            if(ABS(u) < tiny || ABS(v) < tiny)
                                continue;
                            sum1 += u;
                            sum2 += v;
                            nx++;
                        }
                    }
                }
                if(nx < 1)
                    VError(" no voxels in ROI %d, image %d", id, i + 1);
                data1[i]  = sum1 / nx;
                data2[i]  = sum2 / nx;
            }
            if(!found)
                goto next;
        }
        ptr1 = vec1->data;
        ptr2 = vec2->data;
        for(i = 0; i < n; i++) {
            u = data1[i];
            v = data2[i];
            *ptr1++ = ABS(u - v);
            *ptr2++ = u - v;
        }
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
        if(w >= m)
            p = 0;
        else
            p = table[w];
        if(p < tiny)
            p = tiny;
        z = p2z(p);
        if(sumneg > sumpos)
            z = -z;
        gsl_vector_set(zvals, id - 1, z);
        fprintf(stderr, " %3d   %7.3f  %7.4f\n", id, z, p);
        if(fp)
            fprintf(fp, " %3d   %7.3f  %7.4f\n", id, z, p);
next:
        ;
    }
    fprintf(stderr, "\n");
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
	sprintf(prg_name, "vROIpaired_wilcoxtest V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    if(!VParseCommand(VNumber(options), options, & argc, argv)) {
        VReportUsage(argv[0], VNumber(options), options, NULL);
        exit(0);
    }
    /* ini */
    nimages = in_files1.number;
    if(in_files2.number != nimages)
        VError(" inconsistent number of images: %d %d ",
               nimages, in_files2.number);
    mimages = mask_file.number;
    if(nimages != mimages && mimages > 1)
        VError(" inconsistent number of files, images: %d, masks: %d", nimages, mimages);
    /* images 1 */
    src1 = (VImage *) VCalloc(nimages, sizeof(VImage));
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
    src2 = (VImage *) VCalloc(nimages, sizeof(VImage));
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
    /* paired wilcoxon-test in ROIs */
    VROIPairedWilcoxTest(src1, src2, mask, nimages, mimages, fp);
    exit(0);
}
