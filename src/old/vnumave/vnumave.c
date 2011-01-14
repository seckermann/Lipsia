/****************************************************************
 *
 * vnumave:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2000, <lipsia@cbs.mpg.de>
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
 * $Id: vnumave.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/file.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include <viaio/VImage.h>

#include <math.h>
#include <stdio.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))

VDictEntry TYPEDict[] = {
    { "num", 0 },
    { "zmap", 1 },
    { NULL }
};

extern void getLipsiaVersion(char*,size_t);

void
VImageMin(VImage src, VImage dest, VImage count) {
    int i;
    VSByte *cnt_pp;
    VFloat *dest_pp, *src_pp, u;
    cnt_pp  = (VSByte *) VImageData(count);
    src_pp  = (VFloat *) VImageData(src);
    dest_pp = (VFloat *) VImageData(dest);
    for(i = 0; i < VImageNPixels(src); i++) {
        u = *src_pp;
        if(u > 0)
            *cnt_pp += 1;
        if(u < 0)
            *cnt_pp -= 1;
        u = ABS(u);
        if(u < *dest_pp)
            *dest_pp = u;
        cnt_pp++;
        src_pp++;
        dest_pp++;
    }
}



void
VImageSum(VImage src, VImage dest, VFloat pos, VFloat neg) {
    int i;
    VFloat *dest_pp;
    VFloat *src_pp;
    src_pp  = (VFloat *) VImageData(src);
    dest_pp = (VFloat *) VImageData(dest);
    for(i = 0; i < VImageNPixels(src); i++) {
        if((*src_pp >= pos) || (*src_pp <= neg))
            *dest_pp += 1;
        src_pp++;
        dest_pp++;
    }
}



int main(int argc, char *argv[]) {
    static VArgVector in_files;
    static VString out_filename;
    static VBoolean in_found, out_found;
    static VShort type = 0;
    static VFloat pos = 3.0;
    static VFloat neg = -10000;
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
            "type", VShortRepn, 1, & type, VOptionalOpt, TYPEDict,
            "Type of output, 0:num, 1:zmap"
        },
        {
            "pos", VFloatRepn, 1, & pos, VOptionalOpt, NULL,
            "Positive threshold for individual zmaps"
        },
        {
            "neg", VFloatRepn, 1, & neg, VOptionalOpt, NULL,
            "Negative threshold for individual zmaps"
        },
    };
    FILE *f;
    VStringConst in_filename;
    VAttrList list, out_list;
    VAttrListPosn posn;
    VImage src = NULL, dest = NULL, count = NULL;
    int i, nimages, nbands, nrows, ncols;
    VFloat *dest_pp;
    VSByte *cnt_pp;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vnumave V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments: */
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
    /* For each input file: */
    nimages = in_files.number;
    for(i = 0; i < nimages; i++) {
        in_filename = ((VStringConst *) in_files.vector)[i];
        fprintf(stderr, " %s\n", in_filename);
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
        for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & src);
            if(VPixelRepn(src) != VFloatRepn)
                continue;
            if(dest == NULL) {
                nbands = VImageNBands(src);
                nrows  = VImageNRows(src);
                ncols  = VImageNColumns(src);
                dest   = VCreateImage(nbands, nrows, ncols, VFloatRepn);
                VFillImage(dest, VAllBands, 0);
                VCopyImageAttrs(src, dest);
                VSetAttr(VImageAttrList(dest), "num_images", NULL, VShortRepn, (VShort)nimages);
                VSetAttr(VImageAttrList(dest), "patient", NULL, VStringRepn, "numave");
                if(type == 1) {
                    VFillImage(dest, VAllBands, VPixelMaxValue(dest));
                    count = VCreateImage(nbands, nrows, ncols, VUByteRepn);
                    VFillImage(count, VAllBands, 0);
                    if(nimages >= VPixelMaxValue(count))
                        VError(" too many input images");
                }
            }
            if(type == 0)
                VImageSum(src, dest, pos, neg);
            else
                VImageMin(src, dest, count);
            VDestroyImage(src);
        }
    }
    fprintf(stderr, "\n");
    if(dest == NULL)
        VError(" input image not found");
    /* post-process output image */
    if(type == 1) {
        dest_pp = (VFloat *) VImageData(dest);
        cnt_pp  = (VSByte *) VImageData(count);
        for(i = 0; i < VImageNPixels(dest); i++) {
            if(*cnt_pp < 0)
                *dest_pp = -(*dest_pp);
            if(ABS(*cnt_pp) < nimages)
                *dest_pp = 0;
            dest_pp++;
            cnt_pp++;
        }
    }
    /* Create outlist */
    out_list = VCreateAttrList();
    /* Make History */
    VHistory(VNumber(options), options, prg_name, &list, &out_list);
    VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
    /* Open and write the output file: */
    if(strcmp(out_filename, "-") == 0)
        f = stdout;
    else {
        f = fopen(out_filename, "w");
        if(! f)
            VError("Failed to open output file %s", out_filename);
    }
    if(! VWriteFile(f, out_list))
        VError("Failed to write output file %s", out_filename);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return EXIT_SUCCESS;
}
