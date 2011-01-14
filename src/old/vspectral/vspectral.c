/****************************************************************
 *
 * Program: vspectral
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Karsten Mueller, 2005, <lipsia@cbs.mpg.de>
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
 * $Id: vspectral.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MIN(x,min) ((min) < (x) ? (min) : (x))


VDictEntry lag_dict[] = {
    {"Parzen"   ,   1 },
    {"Naeve"    ,   2 },
    {"Tukey"    ,   3 },
    {"Jones"    ,   4 },
    { NULL }
};

VDictEntry choice_dict[] = {
    {"voxel"    ,   1 },
    {"sine"     ,   2 },
    {"halfsine" ,   3 },
    {"file"     ,   4 },
    { NULL }
};

VDictEntry hemi_dict[] = {
    {"both"    ,   1 },
    {"left"    ,   2 },
    {"right"   ,   3 },
    { NULL }
};

extern double klag(double, double);
extern void CovarianceSequence(VImage *, double *, double *, int, int);
extern double p2t(double, double);
extern double p2z(double);
extern void getLipsiaVersion(char*,size_t);

int
main(int argc, char *argv[]) {
    /* Command line options: */
    static VString filename = "0";
    static VString zmapfile = "0";
    static VString report = "0";
    static VDouble freqsec = 11;
    static VDouble p = 0.05;
    static VFloat pos = 3.09;
    static VLong choice = 2;
    static VLong swd = -1;
    static VLong lag = 1;
    static VLong first = 0;
    static VLong last = 1000;
    static VLong hemi = 1;
    static VShort addr[3];
    static VBoolean posphase = FALSE;
    static VBoolean t11 = TRUE;
    static VOptionDescRec  options[] = {
        {"choice", VLongRepn, 1 , &choice, VOptionalOpt, choice_dict, "Choice of reference function" },
        {"addr", VShortRepn, 3, &addr, VOptionalOpt, NULL, "Voxel coordinates" },
        {"file", VStringRepn, 1, (VPointer) &filename, VOptionalOpt, NULL, "File containing reference timecourse" },
        {"zmap", VStringRepn, 1, (VPointer) &zmapfile, VOptionalOpt, NULL, "File containing zmap for reference timecourse" },
        {"pos", VFloatRepn, 1, (VPointer)   &pos, VOptionalOpt, NULL, "Postive threshold for zmap"},
        {"sw", VLongRepn, 1 , &swd, VOptionalOpt, 0, "Size of spectral window. Default: 1/5 of timeline" },
        {"freq", VDoubleRepn, 1 , &freqsec, VRequiredOpt, 0, "Length of a full cycle in timesteps" },
        {"lag", VLongRepn, 1 , &lag, VOptionalOpt, lag_dict, "Type of lag window generator" },
        {"p", VDoubleRepn, 1 , &p, VOptionalOpt, 0, "p-value for confidence interval" },
        {"time", VBooleanRepn, 1 , &t11, VOptionalOpt, 0, "Phase lag in milliseconds" },
        {"posphase", VBooleanRepn, 1 , &posphase, VOptionalOpt, 0, "Positive phase lag only" },
        {"first", VLongRepn, 1 , &first, VOptionalOpt, 0, "First selected slice" },
        {"last", VLongRepn, 1 , &last, VOptionalOpt, 0, "Last selected slice" },
        {"hemi", VLongRepn, 1 , &hemi, VOptionalOpt, hemi_dict, "Selected hemisphere" },
        {"report", VStringRepn, 1, (VPointer) &report, VOptionalOpt, NULL, "Report file with reference tc"},
    };
    FILE *in_file, *out_file, *fp, *fp1, *fp2;
    VAttrList in_list, out_list, listz;
    VAttrListPosn posn;
    VString voxel = NULL, extent = NULL, orientation = NULL, mpilvista = NULL;
    VImage  src = NULL, dest = NULL, phase = NULL, destCI = NULL, phaseCI = NULL;
    VImage  zmap = NULL, mask = NULL, label_image = NULL;
    VImage *imgptr = NULL;
    VFloat *floatpt = NULL, v = 0;
    VShort *shortpt = NULL;
    VBit   *bitpt = NULL;
    int     xrow, xcolumn, xslice, bands = 0, rows = 0, cols = 0;
    int     i = 0, j = 0, k = 0, s, t, np, nn = 0, npixels = 0, nl;
    double *time = NULL, *timetmp = NULL, *ptr = NULL, *cov2 = NULL;
    double  mean = 0, u = 0, u2 = 0, tr = 0, edf = 0, sum = 0, quadsum = 0;
    double  nu = 0, zvalue = 0, tvalue = 0, piii = 6.2831853;
    double  sin(double);
    char   *token = NULL, *tmptoken = NULL;
    char    buf[128];
  char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vspectral V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments and identify files: */
    VParseFilterCmd(VNumber(options), options, argc, argv, & in_file, &out_file);
    /* Read the input file */
    in_list = VReadFile(in_file, NULL);
    if(!in_list)
        exit(1);
    /* Create attribute list for output images */
    out_list = VCreateAttrList();
    /* Make History */
    VHistory(VNumber(options), options, prg_name, &in_list, &out_list);
    /* check input file  */
    i = 0;
    for(VFirstAttr(in_list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(VPixelRepn(src) == VShortRepn) {
            if(voxel == NULL)
                VGetAttr(VImageAttrList(src), "voxel", NULL, VStringRepn, (VPointer) & voxel);
            if(extent == NULL)
                VGetAttr(VImageAttrList(src), "extent", NULL, VStringRepn, (VPointer) & extent);
            if(orientation == NULL)
                VGetAttr(VImageAttrList(src), "orientation", NULL, VStringRepn, (VPointer) & orientation);
            if(mpilvista == NULL)
                VGetAttr(VImageAttrList(src), "MPIL_vista_0", NULL, VStringRepn, (VPointer) & mpilvista);
            if(bands < 2 && VImageNBands(src) > 1)
                bands = (int)VImageNBands(src);
            else {
                if(bands > 1 && VImageNBands(src) > 1 && bands != VImageNBands(src))
                    VError("Different bands in images");
            }
            if(rows < 2 && VImageNRows(src) > 1)
                rows  = (int)VImageNRows(src);
            else {
                if(rows > 1 && VImageNRows(src) > 1 && rows != VImageNRows(src))
                    VError("Different rows in images");
            }
            if(cols < 2 && VImageNColumns(src) > 1)
                cols  = (int)VImageNColumns(src);
            else {
                if(cols > 1 && VImageNColumns(src) > 1 && cols != VImageNColumns(src))
                    VError("Different cols in images");
            }
            i++;
        }
    }
    /* Check input data */
    if(bands < 2 || rows < 2 || cols < 2)
        VError("Check image dimensions and data");
    if(i == 0)
        VError("No functional slices");
    else
        fprintf(stderr, "Functional slices: %d, Timespeps: %d\n", i, bands);
    npixels = i * rows * cols;
    /* for '-choice voxel' only */
    if(choice == 1) {
        /*
          strA = (char *)VMalloc(sizeof(char)*100);
          strcpy(strA,(const char *) str);
          xslice  = atoi(strtok(strA, " "));
          xrow    = atoi(strtok(NULL," "));
          xcolumn = atoi(strtok(NULL," "));
        */
        xslice  = addr[2];
        xrow    = addr[1];
        xcolumn = addr[0];
        if(xslice  >= i)
            VError("Choose a slice between 0 and %d", i - 1);
        if(xrow    >= rows)
            VError("Choose a row   between 0 and %d", rows - 1);
        if(xcolumn >= cols)
            VError("Choose a col   between 0 and %d", cols - 1);
        if(strlen(filename) > 2)
            VError("Do not use a reference-file with '-choice voxel'");
        /* Create Mask und set reference voxel */
        mask = VCreateImage(i, rows, cols, VBitRepn);
        bitpt = (VBit *) VPixelPtr(mask, 0, 0, 0);
        memset(bitpt, (VBit)0, sizeof(VBit) * npixels);
        VPixel(mask, xslice, xrow, xcolumn, VBit) = 1;
        /* Read zmap */
        if(strlen(zmapfile) > 3) {
            fp = VOpenInputFile(zmapfile, TRUE);
            listz = VReadFile(fp, NULL);
            if(! listz)
                VError("Error reading zmap");
            fclose(fp);
            for(VFirstAttr(listz, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
                if(VGetAttrRepn(& posn) != VImageRepn)
                    continue;
                VGetAttrValue(& posn, NULL, VImageRepn, & zmap);
                if(VPixelRepn(zmap) != VFloatRepn)
                    continue;
                break;
            }
            if(zmap == NULL)
                VError(" zmap not found");
            if(VPixelRepn(zmap) != VFloatRepn)
                VError(" zmap not found");
            if(VImageNBands(zmap) != i || VImageNRows(zmap) != rows || VImageNColumns(zmap) != cols)
                VError("Check dimensions of zmap");
            /* Apply mask */
            bitpt   = VPixelPtr(mask, 0, 0, 0);
            floatpt = VPixelPtr(zmap, 0, 0, 0);
            for(s = 0; s < npixels; s++) {
                v = *floatpt;
                if(v >= pos /* || v <= neg */)
                    *bitpt = 1;
                floatpt++;
                bitpt++;
            }
            /* label3d */
            label_image = (VImage) VLabelImage3d(mask, NULL, (int)26, VShortRepn, &nl);
            k = VPixel(label_image, xslice, xrow, xcolumn, VShort);
            bitpt   = VPixelPtr(mask, 0, 0, 0);
            shortpt = VPixelPtr(label_image, 0, 0, 0);
            for(s = 0; s < npixels; s++) {
                *bitpt = 0;
                t = (int)(*shortpt);
                if(t == k)
                    *bitpt = 1;
                shortpt++;
                bitpt++;
            }
        }
        /* Allocate memory for timecourse */
        if(time == NULL)
            time = (double *) malloc(sizeof(double) * bands);
        for(j = 0; j < bands; j++)
            time[j] = (double)0;
        /* load reference timecourse from raw data */
        i = 0;
        nl = 0;
        for(VFirstAttr(in_list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & src);
            if(VPixelRepn(src) == VShortRepn) {
                for(s = 0; s < rows; s++) {
                    for(t = 0; t < cols; t++) {
                        if(VPixel(mask, i, s, t, VBit) == 1) {
                            if(VImageNRows(src) != rows || VImageNColumns(src) != cols || VImageNBands(src) < 2)
                                VError("Check zmap and input data");
                            sum = 0;
                            quadsum = 0;
                            for(j = 0; j < bands; j++) {
                                u = (double)VPixel(src, j, s, t, VShort);
                                sum += u;
                                quadsum += u * u;
                            }
                            quadsum = sqrt(quadsum - sum * sum / (double)bands);
                            for(j = 0; j < bands; j++)
                                time[j] += (((double)VPixel(src, j, s, t, VShort) - sum / (double)bands) / quadsum);
                            nl++;
                        }
                    }
                }
                i++;
            }
        }
        /* divide */
        if(nl == 0)
            VError("No pixel found in mask");
        else
            fprintf(stderr, "mask contains %d voxels\n", nl);
        for(j = 0; j < bands; j++)
            time[j] /= (double)nl;
    } else {
        if(strlen(zmapfile) > 3)
            VError("Specify zmap only for '-choice voxel'");
    }
    /* Keine Voxel-Zeitreihe, sondern was anderes */
    if(choice != 1) {
        /* allocate time vector */
        if(time == NULL)
            time = (double *) malloc(sizeof(double) * bands);
        /* Mathematical reference function used */
        if(choice == 2) {
            for(j = 0; j < bands; j++)
                time[j] = sin((double)j * piii / freqsec);
        }
        if(choice == 3) {
            for(j = 0; j < bands; j++)
                time[j] = -ABS(sin((double)j * piii / freqsec / 2));
        }
        /* Reference file used */
        if(choice == 4) {
            if(strlen(filename) < 3)
                VError("A reference-file has to be specified with '-choice file'");
            fp2 = fopen(filename, "r");
            if(fp2 == NULL)
                VError(" err opening %s", filename);
            while(fgets(buf, 128, fp2)) {
                if(strlen(buf) < 2)
                    continue;
                if(buf[0] == '#')
                    continue;
                if(nn >= bands)
                    VError("The length of the reference-timecourse has to be %d\n", bands);
                time[nn] = (double)atof(strtok(buf, " "));
                nn++;
            }
            fclose(fp2);
            if(nn != bands)
                VError("The length of the reference-timecourse must be %d\n", bands);
        } else {
            if(strlen(filename) > 2)
                VError("Specify a reference file for '-choice file' only");
        }
    }
    /* mean of reference signal */
    for(j = 0; j < bands; j++)
        mean += time[j];
    mean /= (double)bands;
    /* covariance sequence of reference signal */
    if(swd == -1)
        swd = (int)(bands / 10);
    if(cov2 == NULL)
        cov2 = (double *) malloc(sizeof(double) * (swd + 1));
    for(s = 0; s <= swd; s++) {
        u = 0;
        for(t = 0; t < bands - s; t++) {
            u += (time[t] - mean) * (time[t + s] - mean);
        }
        cov2[s]  = u / (double)bands;
    }
    /* NSSM of reference signal */
    u = 0;
    for(t = -swd; t <= swd; t++) {
        u += klag((double)t / (double)swd, (double)lag) * cov2[ABS(t)] * cos((double)(piii / freqsec) * (double)t);
    }
    u2 = u / piii;
    /* report file */
    if(strlen(report) > 2) {
        fp1 = fopen(report, "w");
        for(j = 0; j < bands; j++)
            fprintf(fp1, "%f\n", time[j]);
        fclose(fp1);
    }
    /* Get tr */
    if(mpilvista != NULL) {
        token = strtok(mpilvista, " ");
        tmptoken = strpbrk(token, "=");
        tmptoken++;
        tr = (double) atoi(tmptoken);
    }
    /* Equivalent degrees of freedom */
    for(t = -swd; t <= swd; t++)
        edf += klag((double)t / (double)swd, (double)lag);
    edf = 2 * bands / edf;
    p  /= 2;
    zvalue = p2z((double)(p));
    nu     = 2.0 * edf - 2.0;
    tvalue = p2t((double)(p), nu);
    /* Some messages */
    fprintf(stderr, "Repetition time %1.0f msec.\n", tr);
    fprintf(stderr, "Size of spectral window: %d timesteps.\n", swd * 2);
    fprintf(stderr, "Equivalent degrees of freedom %1.1f\n", edf);
    if(edf < 20 || nu < 10)
        VWarning("Effective degrees of freedom too small");
    /* Allocate destination images */
    dest    = VCreateImage(i, rows, cols, VFloatRepn);
    floatpt = (VFloat *) VPixelPtr(dest, 0, 0, 0);
    memset(floatpt, (VFloat)0, sizeof(VFloat) * npixels);
    destCI  = VCreateImage(i, rows, cols, VFloatRepn);
    floatpt = (VFloat *) VPixelPtr(destCI, 0, 0, 0);
    memset(floatpt, (VFloat)0, sizeof(VFloat) * npixels);
    phase   = VCreateImage(i, rows, cols, VFloatRepn);
    floatpt = (VFloat *) VPixelPtr(phase, 0, 0, 0);
    memset(floatpt, (VFloat)0, sizeof(VFloat) * npixels);
    phaseCI = VCreateImage(i, rows, cols, VFloatRepn);
    floatpt = (VFloat *) VPixelPtr(phaseCI, 0, 0, 0);
    memset(floatpt, (VFloat)0, sizeof(VFloat) * npixels);
    /* Pointer for parallelization */
    imgptr    = (VImage *) VMalloc(sizeof(VImage) * 5);
    imgptr[0] = src;
    imgptr[1] = dest;
    imgptr[2] = phase;
    imgptr[3] = destCI;
    imgptr[4] = phaseCI;
    ptr   = (double *) malloc(sizeof(double) * 11);
    ptr[0] = mean;
    ptr[1] = (double)lag;
    ptr[2] = (double)swd;
    ptr[3] = (double)(piii / freqsec);
    ptr[4] = u2;
    ptr[5] = tr;
    ptr[6] = edf - (double)2.0;
    ptr[7] = zvalue;
    ptr[8] = tvalue;
    ptr[9] = (double)t11;
    ptr[10] = (double)posphase;
    free(cov2);
    /* Now go through all objects */
    i = 0;
    fprintf(stderr, "Slices:");
    for(VFirstAttr(in_list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(VPixelRepn(src) == VShortRepn) {
            imgptr[0] = src;
            fprintf(stderr, " %d", i);
            if(i >= first && i <= last && VImageNBands(src) > 1)
                CovarianceSequence(imgptr, time, ptr, i, (int)hemi);
            i++;
        }      /* end if VShort     */
    }        /* end loop over obj */
    /* Append coherence to attribute list */
    VSetAttr(VImageAttrList(dest), "name", NULL, VStringRepn, "coherence");
    VSetAttr(VImageAttrList(dest), "modality", NULL, VStringRepn, "coherence");
    if(voxel)
        VSetAttr(VImageAttrList(dest), "voxel", NULL, VStringRepn, voxel);
    if(extent)
        VSetAttr(VImageAttrList(dest), "extent", NULL, VStringRepn, extent);
    if(orientation)
        VSetAttr(VImageAttrList(dest), "orientation", NULL, VStringRepn, orientation);
    VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
    /* Append coherenceCI to attribute list */
    VSetAttr(VImageAttrList(destCI), "name", NULL, VStringRepn, "CIcoherence");
    VSetAttr(VImageAttrList(destCI), "modality", NULL, VStringRepn, "CIcoherence");
    if(voxel)
        VSetAttr(VImageAttrList(destCI), "voxel", NULL, VStringRepn, voxel);
    if(extent)
        VSetAttr(VImageAttrList(destCI), "extent", NULL, VStringRepn, extent);
    if(orientation)
        VSetAttr(VImageAttrList(destCI), "orientation", NULL, VStringRepn, orientation);
    VAppendAttr(out_list, "image", NULL, VImageRepn, destCI);
    /* Append phase to attribute list */
    VSetAttr(VImageAttrList(phase), "name", NULL, VStringRepn, "phase");
    VSetAttr(VImageAttrList(phase), "modality", NULL, VStringRepn, "phase");
    if(voxel)
        VSetAttr(VImageAttrList(phase), "voxel", NULL, VStringRepn, voxel);
    if(extent)
        VSetAttr(VImageAttrList(phase), "extent", NULL, VStringRepn, extent);
    if(orientation)
        VSetAttr(VImageAttrList(phase), "orientation", NULL, VStringRepn, orientation);
    VAppendAttr(out_list, "image", NULL, VImageRepn, phase);
    /* Append phase to attribute list */
    VSetAttr(VImageAttrList(phaseCI), "name", NULL, VStringRepn, "CIphase");
    VSetAttr(VImageAttrList(phaseCI), "modality", NULL, VStringRepn, "CIphase");
    if(voxel)
        VSetAttr(VImageAttrList(phaseCI), "voxel", NULL, VStringRepn, voxel);
    if(extent)
        VSetAttr(VImageAttrList(phaseCI), "extent", NULL, VStringRepn, extent);
    if(orientation)
        VSetAttr(VImageAttrList(phaseCI), "orientation", NULL, VStringRepn, orientation);
    VAppendAttr(out_list, "image", NULL, VImageRepn, phaseCI);
    /* Append mask image */
    if(mask)
        VAppendAttr(out_list, "image", NULL, VImageRepn, mask);
    /* Write out the results: */
    if(! VWriteFile(out_file, out_list))
        exit(1);
    fprintf(stderr, " done.\n");
    return 0;
}


/************************************************************
 *                                                          *
 *                  CovarianceSequence                      *
 *                                                          *
 ************************************************************/

void
CovarianceSequence(VImage *imgptr, double *time, double *ptr, int slice, int hemi) {
    int j, l, s, t, bands, sw;
    double piii = 6.2831853;
    double u, un, u1, ui, v, vs, mean, wert;
    double *cov, *covn, *cov1, c, p, cCI, pCI;
    double sqrt(double);
    double atan(double);
    double atanh(double);
    double tanh(double);
    double cos(double);
    double sin(double);
    double atan2(double, double);
    for(j = 0; j < VImageNRows(imgptr[0]); j++) {
        bands  = VImageNBands(imgptr[0]);
        cov    = (double *) malloc(sizeof(double) * (ptr[2] + 1));
        covn   = (double *) malloc(sizeof(double) * (ptr[2] + 1));
        cov1   = (double *) malloc(sizeof(double) * (ptr[2] + 1));
        for(l = 0; l < VImageNColumns(imgptr[0]); l++) {
            /* only for selected hemisphere */
            if(hemi == 1 ||
                    (hemi == 2 && l < (int)(VImageNColumns(imgptr[0]) / 2)) ||
                    (hemi == 3 && l > (int)(VImageNColumns(imgptr[0]) / 2))) {
                /* Compute mean of timeline */
                mean = sw = 0;
                for(s = 0; s < bands; s++) {
                    u = (double)VPixel(imgptr[0], s, j, l, VShort);
                    mean += u;
                }
                mean /= (double)bands;
                sw = 0;
                if(mean > 1 && bands > 1)
                    sw = 1;
                /* Compute covaraince sequence */
                if(sw == 1) {
                    for(s = 0; s <= (int)ptr[2]; s++) {
                        u = un = u1 = 0;
                        for(t = 0; t < bands - s; t++) {
                            v  = (double)VPixel(imgptr[0], t, j, l, VShort) - mean;
                            vs = (double)VPixel(imgptr[0], t + s, j, l, VShort) - mean;
                            u  += v * (time[t + s] - ptr[0]);
                            un += (time[t] - ptr[0]) * vs;
                            u1 += v * vs;
                        }
                        cov[s]  = u / (double)bands;
                        covn[s] = un / (double)bands;
                        cov1[s] = u1 / (double)bands;
                    }
                }
                /* Compute sample coherence */
                if(sw == 1) {
                    u = ui = u1 = 0;
                    for(t = -(int)ptr[2]; t <= (int)ptr[2]; t++) {
                        v  = klag((double)t / ptr[2], ptr[1]);
                        if(t < 0)
                            wert = covn[-t];
                        else
                            wert = cov[t];
                        u   += v * wert         * cos(ptr[3] * (double)t);
                        ui  += v * wert         * sin(ptr[3] * (double)t);
                        u1  += v * cov1[ABS(t)] * cos(ptr[3] * (double)t);
                    }
                    u  /= piii;
                    ui /= piii;
                    u1 /= piii;
                    c = sqrt((u * u + ui * ui) / (u1 * ptr[4]));
                    p = -atan2(ui, u);
                    if(ptr[10] > 0.5 && p < 0)
                        p += piii;
                    cCI = tanh(atanh(c) + (ptr[7] * sqrt(ptr[6]) - 1) / ptr[6]) - c;
                    pCI = sqrt((1 - c * c) / (ptr[6] * c * c)) * ptr[8];
                    if(ptr[9] > 0.5) {
                        p   = ptr[5] * p / ptr[3];
                        pCI = ptr[5] * pCI / ptr[3];
                    }
                    c   = 100.0 * c;
                    cCI = 100.0 * cCI;
                    VPixel(imgptr[1], slice, j, l, VFloat) = (VFloat)c;
                    VPixel(imgptr[2], slice, j, l, VFloat) = (VFloat)p;
                    VPixel(imgptr[3], slice, j, l, VFloat) = (VFloat)cCI;
                    VPixel(imgptr[4], slice, j, l, VFloat) = (VFloat)pCI;
                } else {
                    VPixel(imgptr[1], slice, j, l, VFloat) = (VFloat)0.0;
                    VPixel(imgptr[2], slice, j, l, VFloat) = (VFloat)0.0;
                    VPixel(imgptr[3], slice, j, l, VFloat) = (VFloat)0.0;
                    VPixel(imgptr[4], slice, j, l, VFloat) = (VFloat)0.0;
                }
            } /* end selected hemisphere */
        }   /* end for columns         */
        free(cov);
        free(covn);
        free(cov1);
    }    /* end for rows      */
}


/************************************************************
 *                                                          *
 *                         lag                              *
 *                                                          *
 ************************************************************/

double
klag(double x, double lag) {
    double epsilon;
    double z, y;
    double z1;
    double cos(double);
    double log(double);
    double exp(double);
    /* Parzen lag */
    if(lag == 1.0) {
        z = ABS(x);
        z1 = (double)1.0 - z;
        if(z <= 0.5)
            y = (double)1.0 - (double)6.0 * z * z * z1;
        else
            y = (double)2.0 * z1 * z1 * z1;
        if(z > 1)
            VError("z>1");
    }
    /* Naeve lag */
    if(lag == 2.0) {
        if(x > 0 && x <= 1)
            y = ((double)1.0 - x) * ((double)1.0 - x);
        else
            VError("x>1 || x<0");
    }
    /* Tukey lag */
    if(lag == 3.0) {
        y = (double)0.5 * ((double)1.0 + cos((double)3.141 * x));
        if(x > 1)
            y = 0;
    }
    /* Jones lag */
    if(lag == 4.0) {
        epsilon = (double)0.001;
        if(ABS(x) <= 1)
            y = exp(-(-log(epsilon)) * x * x);
        else
            y = 0;
    }
    return y;
}
