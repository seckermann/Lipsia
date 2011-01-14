/****************************************************************
 *
 * Program: vzmapborder
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
 * $Id: vzmapborder.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via.h>
#include <math.h>
#include <stdlib.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))


/* Later in this file: */
extern void PredispImage(VImage, VImage, VImage, VDouble, VDouble, VFloat);
extern void getLipsiaVersion(char*,size_t);

int main(int argc, char *argv[]) {
    /* Command line options: */
    static VArgVector in_files;
    static VString out_filename;
    static VBoolean in_found, out_found;
    static VDouble pos = 3.0;
    static VDouble neg = -100.0;
    static VString ref_filename = "";
    static VOptionDescRec options[] = {
        {
            "in", VStringRepn, 0, & in_files, & in_found, NULL,
            "Input files"
        },
        {
            "out", VStringRepn, 1, & out_filename, & out_found, NULL,
            "Output file"
        },
        {
            "ref", VStringRepn, 1, & ref_filename, VRequiredOpt, NULL,
            "File containing 3D MDEFT reference image"
        },
        {
            "pos", VDoubleRepn, 1, & pos, VOptionalOpt, NULL,
            "Positive threshold"
        },
        {
            "neg", VDoubleRepn, 1, & neg, VOptionalOpt, NULL,
            "Negative threshold"
        }
    };
    FILE *f, *ref_file;
    VStringConst in_filename;
    VAttrList list, out_list;
    VAttrListPosn posn;
    VImage src = NULL, ref = NULL, dest = NULL;
    VFloat zval = 1.0;
    int i, nbands, nrows, ncols;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vzmapborder V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments and identity files  */
    if(! VParseCommand(VNumber(options), options, & argc, argv) ||
            ! VIdentifyFiles(VNumber(options), options, "in", & argc, argv, 0) ||
            ! VIdentifyFiles(VNumber(options), options, "out", & argc, argv, -1))
        goto Usage;
    if(argc > 1) {
        VReportBadArgs(argc, argv);
Usage:
        VReportUsage(argv[0], VNumber(options), options, NULL);
        exit(EXIT_FAILURE);
    }
    if(neg > 0)
        VError("parameter '-neg' must be negative.");
    if(pos < 0)
        VError("parameter '-pos' must be positive.");
    /* Read the ref image: */
    ref_file = VOpenInputFile(ref_filename, TRUE);
    list = VReadFile(ref_file, NULL);
    if(! list)
        VError("Error reading reference image");
    fclose(ref_file);
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & ref);
    }
    if(ref == NULL)
        VError("reference image not found");
    /* if no scratch present, create dest image */
    if(dest == NULL) {
        nrows  = VImageNRows(ref);
        ncols  = VImageNColumns(ref);
        nbands = VImageNBands(ref);
        dest   = VCreateImage(nbands, nrows, ncols, VFloatRepn);
        VFillImage(dest, VAllBands, 0);
    }
    /* For each input file: */
    zval = 1;
    for(i = 0; i < in_files.number; i++) {
        in_filename = ((VStringConst *) in_files.vector)[i];
        fprintf(stderr, " %3d: %s\n", i + 1, in_filename);
        /* Read its contents: */
        if(strcmp(in_filename, "-") == 0)
            f = stdin;
        else {
            f = fopen((char *)in_filename, "r");
            if(! f)
                VError("Failed to open input file %s", in_filename);
        }
        if(!(list = VReadFile(f, NULL)))
            exit(EXIT_FAILURE);
        fclose(f);
        for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & src);
            if(VPixelRepn(src) != VFloatRepn)
                continue;      /* not a zmap */
            zval = i + 1;
            PredispImage(src, ref, dest, pos, neg, zval);
            break;
        }
    }
    /* Create outlist */
    out_list = VCreateAttrList();
    /* Make History */
    VHistory(VNumber(options), options, prg_name, &list, &out_list);
    VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
    /* Write out the results: */
    if(strcmp(out_filename, "-") == 0)
        f = stdout;
    else {
        f = fopen(out_filename, "w");
        if(! f)
            VError("Failed to open output file %s", out_filename);
    }
    if(! VWriteFile(f, out_list))
        exit(EXIT_FAILURE);
    fprintf(stderr, "%s done.\n", argv[0]);
    return EXIT_SUCCESS;
}



/*
 *  Prepare for display:
 *
 */
void
PredispImage(VImage src, VImage ref, VImage dest, VDouble pos, VDouble neg, VFloat zval) {
    int i, nbands, ncols, nrows, npixels;
    VFloat *float_pp, *dest_pp;
    VBit *bin_pp;
    VImage src0 = NULL, bin1_image = NULL;
    static VImage bin_image = NULL;
    double scalec, scaler, scaleb;
    VString str;
    float shift[3], scale[3];
    char normstr[] = "1 1 1";
    if(VPixelRepn(src) != VFloatRepn)
        VError(" image must be float");
    if(VPixelRepn(ref) != VUByteRepn)
        VError(" ref image must be ubyte");
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    nbands = VImageNBands(src);
    npixels = nbands * ncols * nrows;
    /*
    ** rescale geometrically if necessary
    */
    if(VGetAttr(VImageAttrList(src), "voxel", NULL,
                VStringRepn, (VPointer) & str) != VAttrFound)
        VError(" attribute 'voxel' missing in input file.");
    scalec = atof(strtok(str, " "));
    scaler = atof(strtok(NULL, " "));
    scaleb = atof(strtok(NULL, " "));
    if(ABS(scalec - 1.0) > 0.1
            || ABS(scaler - 1.0) > 0.1
            || ABS(scaleb - 1.0) > 0.1) {
        nbands = VImageNBands(ref);
        nrows  = VImageNRows(ref);
        ncols  = VImageNColumns(ref);
        npixels = nbands * ncols * nrows;
        scale[0] = scaleb;
        scale[1] = scaler;
        scale[2] = scalec;
        for(i = 0; i < 3; i++)
            shift[i] = 0;
        src0 = VTriLinearScale3d(src, NULL, nbands, nrows, ncols, shift, scale);
    } else
        src0 = NULL;
    if(bin_image == NULL)
        bin_image = VCreateImage(nbands, nrows, ncols, VBitRepn);
    bin_pp = VImageData(bin_image);
    if(src0 == NULL)
        float_pp = VImageData(src);
    if(src0 != NULL)
        float_pp = VImageData(src0);
    for(i = 0; i < npixels; i++) {
        if(*float_pp >= pos || *float_pp <= neg)
            *bin_pp = 1;
        else
            *bin_pp = 0;
        float_pp++;
        bin_pp++;
    }
    bin1_image = VBorderImage3d(bin_image, bin1_image);
    bin_pp  = VImageData(bin1_image);
    dest_pp = VImageData(dest);
    for(i = 0; i < npixels; i++) {
        if(*bin_pp > 0)
            *dest_pp = zval;
        dest_pp++;
        bin_pp++;
    }
    VCopyImageAttrs(ref, dest);
}

