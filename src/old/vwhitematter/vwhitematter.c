/****************************************************************
 *
 * Program: vwhitematter
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
 * $Id: vwhitematter.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>

/* From the standard C library: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <via.h>


#define ABS(x) ((x) < 0 ? -(x) : (x))
#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#define IMIN(a,b) ((a) < (b) ? (a) : (b))


int m = 0;

#define BACKGROUND 0   /* background value after brain peeling */
#define GREY  1        /* grey matter */
#define WHITE 0        /* white matter */

VBoolean verbose = FALSE; /* print histograms */

/* Later in this file: */

extern VImage VTopSmoothImage3d(VImage, VImage, VImage, VLong, VLong);
extern VImage VLocalContrast(VImage, VImage, VShort, VDouble, VDouble);
extern VImage VLeeImage3d(VImage, VImage, VLong, VDouble);
extern VImage VCortex(VImage, VImage, VDouble, VLong, VLong, VLong, VBoolean, VShort, VBoolean);
extern int cortex_point(VImage, VImage, int, int, int, short, VDouble, VLong, VLong);
extern int VCheckPointDir(VImage, int, int, int, VBoolean, VBoolean, VBoolean, VBoolean, VBoolean, VBoolean);
extern int FinalPoint(VImage, int, int, int);
extern void getLipsiaVersion(char*,size_t);
static VBoolean geometry = FALSE;


int main(int argc, char *argv[]) {
    /* Command line options: */
    static VDouble t  = 3.0;
    static VLong  low  = 0;
    static VLong  high = 0;
    static VLong  add  = 0;
    static VShort maxiter = 100;
    static VShort wsize = 0;
    static VDouble sigma = 2.0;
    static VBoolean tauto = TRUE;
    static VBoolean topology = FALSE;
    static VOptionDescRec options[] = {
        {
            "t", VDoubleRepn, 1, &t, VOptionalOpt, NULL,
            "threshold"
        },
        {
            "low", VLongRepn, 1, &low, VOptionalOpt, NULL,
            "lower threshold"
        },
        {
            "high", VLongRepn, 1, &high, VOptionalOpt, NULL,
            "upper threshold"
        },
        {
            "add", VLongRepn, 1, &add, VOptionalOpt, NULL,
            "add to grey value thresholds"
        },
        {
            "auto", VBooleanRepn, 1, & tauto, VOptionalOpt, NULL,
            "Whether to use automatic threshold selection"
        },
        {
            "geometry", VBooleanRepn, 1, & geometry, VOptionalOpt, NULL,
            "Whether to use geometrical constraints"
        },
        {
            "iter", VShortRepn, 1, & maxiter, VOptionalOpt, NULL,
            "Max number of iterations"
        },
        {
            "wsize", VShortRepn, 1, & wsize, VOptionalOpt, NULL,
            "Window size for local contrast enhancement (25 is a typical choice)"
        },
        {
            "sigma", VDoubleRepn, 1, & sigma, VOptionalOpt, NULL,
            "Sigma for local contrast enhancement"
        },
        {
            "topology", VBooleanRepn, 1, & topology, VOptionalOpt, NULL,
            "Whether to enforce topological correctness "
        }
    };
    FILE *in_file, *out_file;
    VAttrList list;
    VAttrListPosn posn;
    VImage src = NULL, tmp = NULL, result = NULL;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vwhitematter V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments and identify files: */
    VParseFilterCmd(VNumber(options), options, argc, argv, & in_file, & out_file);
    /* Read the input file: */
    list = VReadFile(in_file, NULL);
    if(! list)
        exit(1);
    /* For each attribute read... */
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(wsize > 0) {
            tmp = VLeeImage3d(src, NULL, (VLong)5, (VDouble) 10.0);
            src = VLocalContrast(tmp, src, wsize, sigma, (int)0);
            result = VCortex(src, NULL, t, low, high, add, tauto, maxiter, topology);
        } else {
            result = VCortex(src, NULL, t, low, high, add, tauto, maxiter, topology);
        }
        if(! result)
            exit(1);
        VSetAttrValue(& posn, NULL, VImageRepn, result);
        VDestroyImage(src);
    }
    /* Write out the results: */
    VHistory(VNumber(options), options, prg_name, &list, &list);
    if(! VWriteFile(out_file, list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}


VImage
VCortex(VImage src, VImage dest, VDouble threshold, VLong low,
        VLong high, VLong add, VBoolean tauto, VShort maxiter, VBoolean topology) {
    int b = 0, r = 0, c = 0, nbands, nrows, ncols, npixels;
    int i, j, n, nn, m, ndel, sumdel;
    int background = BACKGROUND;
    VImage short_image, result = NULL;
    VUByte *srcbuf, u;
    VBit  *destbuf;
    short d;
    VPoint *arr;
    double sum = 0, mean = 0;
    double sum1, sum2, nx, v;
    VBoolean smooth = TRUE;
    int greyval, flag;
    VBoolean dir0, dir1, dir2, dir3, dir4, dir5;
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    nbands = VImageNBands(src);
    npixels = nbands * nrows * ncols;
    /*
    ** get threshold parameters
    */
    if(low < BACKGROUND)
        low = BACKGROUND;
    if(tauto || high == 0) {    /* automatic threshold selection */
        srcbuf  = (VUByte *) VPixelPtr(src, 0, 0, 0);
        m = sum = 0;
        for(i = 0; i < npixels; i++) {
            if(*srcbuf > 0) {
                sum += *srcbuf;
                m++;
            }
            srcbuf++;
        }
        mean = sum / (double) m;   /* average grey value */
        low  = mean + add;
        high = low + 20;
    }
    fprintf(stderr, " background: %d   mean: %g  low: %d high: %d  t: %g\n",
            background, mean, low, high, threshold);
    /*
    ** depth labelling
    */
    if(dest == NULL)
        dest = VCreateImage(nbands, nrows, ncols, VBitRepn);
    srcbuf  = (VUByte *) VPixelPtr(src, 0, 0, 0);
    destbuf = (VBit *)   VPixelPtr(dest, 0, 0, 0);
    n = 0;
    for(i = 0; i < npixels; i++) {
        u = *srcbuf;
        if(u > background)
            n++;
        *destbuf = (u > background) ? 0 : 1;
        srcbuf++;
        destbuf++;
    }
    short_image = VEuclideanDist3d(dest, NULL, VShortRepn);
    destbuf = (VBit *) VPixelPtr(dest, 0, 0, 0);
    for(i = 0; i < npixels; i++) {
        *destbuf = (*destbuf > 0) ? 0 : 1;
        destbuf++;
    }
    /*
    ** main loop
    */
    arr = (VPoint *) VMalloc(sizeof(VPoint) * (n + 2));
    sumdel = 0;
    for(greyval = 1; greyval <= high; greyval++) {
        nn = 1;
        for(b = 0; b < nbands; b++) {
            for(r = 0; r < nrows; r++) {
                for(c = 0; c < ncols; c++) {
                    if(VPixel(src, b, r, c, VUByte) > greyval)
                        continue;
                    if(VPixel(dest, b, r, c, VBit) == 0)
                        continue;
                    arr[nn].b = b;
                    arr[nn].r = r;
                    arr[nn].c = c;
                    arr[nn].val = (float)VPixel(short_image, b, r, c, VShort);
                    nn++;
                }
            }
        }
        VPoint_hpsort(nn - 1, arr);   /* sort by depth (oder auch nicht) */
        sum1 = sum2 = nx = 0;
        ndel = 1;
        while(ndel > 0) {
            ndel = 0;
            for(j = 1; j < nn; j++) {
                b = arr[j].b;
                r = arr[j].r;
                c = arr[j].c;
                if(VPixel(dest, b, r, c, VBit) == 0)
                    continue;
                if(b < 1 || r < 1 || c < 1)
                    continue;
                if(b >= nbands - 1 || r >= nrows - 1 || c >= ncols - 1)
                    continue;
                /* check if border voxel */
                dir0 = dir1 = dir2 = dir3 = dir4 = dir5 = FALSE;
                flag = 0;
                if(VPixel(dest, b, r, c - 1, VBit) == 0) {
                    dir2 = TRUE;
                    flag = 1;
                }
                if(VPixel(dest, b, r, c + 1, VBit) == 0) {
                    dir3 = TRUE;
                    flag = 1;
                }
                if(VPixel(dest, b, r - 1, c, VBit) == 0) {
                    dir0 = TRUE;
                    flag = 1;
                }
                if(VPixel(dest, b, r + 1, c, VBit) == 0) {
                    dir1 = TRUE;
                    flag = 1;
                }
                if(VPixel(dest, b - 1, r, c, VBit) == 0) {
                    dir4 = TRUE;
                    flag = 1;
                }
                if(VPixel(dest, b + 1, r, c, VBit) == 0) {
                    dir5 = TRUE;
                    flag = 1;
                }
                if(flag == 0)
                    continue;
                /* grey/white matter border */
                d = VPixel(short_image, b, r, c, VShort);
                m = cortex_point(src, dest, b, r, c, d, threshold, low, high);
                if(m < 1)
                    continue;
                /* enforce topological correctness */
                if(topology) {
                    if(VSimplePoint(dest, b, r, c, 26) == 0)
                        continue;
                }
                /* enforce geometry constraints */
                if(geometry && VPixel(src, b, r, c, VUByte) > low) {
                    if(FinalPoint(dest, b, r, c) < 2)
                        continue;
                    if(VCheckPointDir(dest, b, r, c, dir0, dir1, dir2, dir3, dir4, dir5) == 0)
                        continue;
                }
                if(m > 0) {
                    v = VPixel(src, b, r, c, VUByte);
                    sum1 += v;
                    sum2 += v * v;
                    nx++;
                    ndel++;
                    VPixel(dest, b, r, c, VBit) = 0;
                    sumdel++;
                }
            }
        }
        if(greyval % 25 == 0)
            fprintf(stderr, " %3d: %7d pts deleted\n", greyval, sumdel);
    }
    /* smoothing, postprocessing */
    if(smooth)
        result = VTopSmoothImage3d(dest, src, NULL, (VLong)0, (VLong)1);
    else
        result = VCopyImage(dest, NULL, VAllBands);
    fprintf(stderr, " genus: %d\n", VGenusLee(result, (VShort) 26));
    VCopyImageAttrs(src, result);
    return result;
}



/*
** b,r,c is a cortex point if its grey value does not deviate too
** much from the grey values of adjacent points which have already been removed
*/
int
cortex_point(VImage src, VImage dest, int b, int r, int c, VShort d,
             VDouble threshold, VLong low, VLong high) {
    int bb, rr, cc, b0, b1, r0, r1, c0, c1;
    float u, v;
    float sum1, sum2;
    float ssum1, ssum2, mean1, mean2;
    int n1, n2;
    int wn2 = 2;
    if(d < 25)
        return GREY;    /* outmost 2.0 mm are assumed to be cortex */
    u = VPixel(src, b, r, c, VUByte);
    if(u < low)
        return GREY;
    if(u > high)
        return WHITE;
    b0 = IMAX(0, b - wn2);
    b1 = IMIN(VImageNBands(src) - 1, b + wn2);
    r0 = IMAX(0, r - wn2);
    r1 = IMIN(VImageNRows(src) - 1, r + wn2);
    c0 = IMAX(0, c - wn2);
    c1 = IMIN(VImageNColumns(src) - 1, c + wn2);
    sum1 = sum2 = ssum1 = ssum2 = n1 = n2 = 0;
    for(bb = b0; bb <= b1; bb++) {
        for(rr = r0; rr <= r1; rr++) {
            for(cc = c0; cc <= c1; cc++) {
                v = VPixel(src, bb, rr, cc, VUByte);
                if(v <= 0.1)
                    continue;
                if(VPixel(dest, bb, rr, cc, VBit) == 0) {      /* schon als GM identifiziert */
                    sum1  += v;
                    ssum1 += v * v;
                    n1++;
                } else { /* noch WM */
                    sum2  += v;
                    ssum2 += v * v;
                    n2++;
                }
            }
        }
    }
    if(n1 <= 1)
        return GREY;
    mean1 = sum1 / (float)n1;
    if(u <= mean1)
        return GREY;
    if(mean1 < 1)
        return GREY;
    if(n2 <= 1)
        mean2 = u;
    else
        mean2 = sum2 / (float)n2;
    if(mean2 / mean1 < threshold)
        return GREY;
    else
        return WHITE;
}


/*
** get number n of foreground voxels in 6-adj.
** if n < 2: final point, do not delete
*/
int
FinalPoint(VImage src, int b, int r, int c) {
    int nbands, nrows, ncols;
    int n;
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    nbands = VImageNBands(src);
    n = 0;
    if(b > 0)
        if(VPixel(src, b - 1, r, c, VBit) > 0)
            n++;
    if(b < nbands - 1)
        if(VPixel(src, b + 1, r, c, VBit) > 0)
            n++;
    if(r > 0)
        if(VPixel(src, b, r - 1, c, VBit) > 0)
            n++;
    if(r < nrows - 1)
        if(VPixel(src, b, r + 1, c, VBit) > 0)
            n++;
    if(c > 0)
        if(VPixel(src, b, r, c - 1, VBit) > 0)
            n++;
    if(c < ncols - 1)
        if(VPixel(src, b, r, c + 1, VBit) > 0)
            n++;
    return n;
}

