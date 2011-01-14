/****************************************************************
 *
 * vdifftimecourse:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Jane Neumann, 2004, <lipsia@cns.mpg.de>
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
 * $Id: vdifftimecourse.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via/via.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define ABS(x) ((x) > 0 ? (x) : -(x))
#define MAXSLICE 100

extern void getLipsiaVersion(char*,size_t);

int main(int argc, char *argv[]) {
    /* Command line options: */
    static VString firstfile = "";
    static VString secondfile = "";
    static VOptionDescRec options[] = {
        {
            "first", VStringRepn, 1, (VPointer) &firstfile,
            VRequiredOpt, NULL, "first file"
        },
        {
            "second", VStringRepn, 1, (VPointer) &secondfile,
            VRequiredOpt, NULL, "second file"
        }
    };
    FILE *first_file, *second_file, *out_file;
    VAttrList first_list, second_list, out_list;
    VAttrListPosn fposn, sposn;
    VImage fsrc = NULL, ssrc = NULL, out = NULL;
    VString voxel = NULL;
    int rr = 0, cc = 0, bands = 0, nrows = 0, ncols = 0, nbands = 0, slice = 0;
    int nrows2 = 0, ncols2 = 0, nbands2 = 0;
    double nvalue, fvalue, svalue, rvalue;
   char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vdifftimecourse V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments and identify files: */
    VParseFilterCmd(VNumber(options), options, argc, argv, NULL, &out_file);
    first_file = VOpenInputFile(firstfile, TRUE);
    first_list = VReadFile(first_file, NULL);
    if(! first_list)
        VError("Error reading first file");
    fclose(first_file);
    second_file = VOpenInputFile(secondfile, TRUE);
    second_list = VReadFile(second_file, NULL);
    if(! second_list)
        VError("Error reading second file");
    fclose(second_file);
    /* Create attribute list for output image and r-value image */
    out_list = VCreateAttrList();
    /* Make History */
    VHistory(VNumber(options), options, prg_name, &first_list, &out_list);
    /* Count number of rows, columns, slices in first file
       Check if all functional slices have the same dimensions */
    for(VFirstAttr(first_list, & fposn); VAttrExists(& fposn); VNextAttr(& fposn)) {
        if(VGetAttrRepn(& fposn) != VImageRepn)
            continue;
        VGetAttrValue(& fposn, NULL, VImageRepn, & fsrc);
        /* copy anatomy to output file */
        if(VPixelRepn(fsrc) == VUByteRepn) {
            VAppendAttr(out_list, "image", NULL, VImageRepn, fsrc);
        }
        if(VPixelRepn(fsrc) == VShortRepn) {
            /* if voxel size unknown set from current object */
            if(voxel == NULL)
                VGetAttr(VImageAttrList(fsrc), "voxel", NULL, VStringRepn, (VPointer) &voxel);
            /* compare rows, colums, bands */
            if(nbands < 2) {
                nbands = (int)VImageNBands(fsrc);
                nrows  = (int)VImageNRows(fsrc);
                ncols  = (int)VImageNColumns(fsrc);
            }
        }
    }
    fprintf(stderr, "bands %d, rows %d, columns %d\n", nbands, nrows, ncols);
    /*
      Count number of rows, columns, slices in second file
      Check if it has same dimensions as first file
    */
    for(VFirstAttr(second_list, & sposn); VAttrExists(& sposn); VNextAttr(& sposn)) {
        if(VGetAttrRepn(& sposn) != VImageRepn)
            continue;
        VGetAttrValue(& sposn, NULL, VImageRepn, & ssrc);
        if(VPixelRepn(ssrc) == VUByteRepn)
            continue;
        if(VPixelRepn(ssrc) == VShortRepn) {
            if(nbands2 < 2) {
                nbands2 = (int)VImageNBands(ssrc);
                nrows2  = (int)VImageNRows(ssrc);
                ncols2  = (int)VImageNColumns(ssrc);
            }
        }
    }
    if((nbands2 != nbands) || (nrows2 != nrows) || (ncols2 != ncols)) {
        fprintf(stderr, "bands %d, rows %d, columns %d\n", (int)VImageNBands(ssrc), (int)VImageNRows(ssrc), (int)VImageNColumns(ssrc));
        VError("Different number of voxels in input images\n");
    }
    /* loop */
    slice = -1;
    for(VFirstAttr(second_list, & sposn); VAttrExists(& sposn); VNextAttr(& sposn)) {
        if(VGetAttrRepn(& sposn) != VImageRepn)
            continue;
        VGetAttrValue(& sposn, NULL, VImageRepn, & ssrc);
        if(VPixelRepn(ssrc) == VShortRepn)
            break;
    }
    for(VFirstAttr(first_list, & fposn); VAttrExists(& fposn); VNextAttr(& fposn)) {
        if(VGetAttrRepn(& fposn) != VImageRepn)
            continue;
        VGetAttrValue(& fposn, NULL, VImageRepn, & fsrc);
        if(VPixelRepn(fsrc) != VShortRepn)
            continue;
        VGetAttrValue(& sposn, NULL, VImageRepn, & ssrc);
        slice++;
        /* create new output slice, set voxel attribute */
        out = VCreateImage(nbands, nrows, ncols, VShortRepn);
        if(voxel)
            VSetAttr(VImageAttrList(out), "voxel", NULL, VStringRepn, voxel);
        VFillImage(out, VAllBands, 0);
        /* for data sets with empty slices */
        if((int)VImageNRows(fsrc) < 2)
            goto next1;
        for(rr = 0; rr < nrows; rr++) {
            for(cc = 0; cc < ncols; cc++) {
                nvalue = VPixel(fsrc, 0, rr, cc, VShort);
                for(bands = 0; bands < nbands; bands++) {
                    fvalue = VPixel(fsrc, bands, rr, cc, VShort);
                    svalue = VPixel(ssrc, bands, rr, cc, VShort);
                    rvalue = fvalue - svalue + nvalue;
                    VPixel(out, bands, rr, cc, VShort) = (VShort) rvalue;
                }
            }    /* for cols */
        }      /* for rows */
next1:
        VCopyImageAttrs(fsrc, out);
        VAppendAttr(out_list, "image", NULL, VImageRepn, out);
        VNextAttr(& sposn);
    }  /* for slices */
    if(! VWriteFile(out_file, out_list))
        exit(1);
    fprintf(stderr, "done\n");
    fclose(out_file);
    return 0;
}


