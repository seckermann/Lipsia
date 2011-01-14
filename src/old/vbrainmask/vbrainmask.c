/****************************************************************
 *
 * Program: vbrainmask
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2005, <lipsia@cbs.mpg.de>
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
 * $Id: vbrainmask.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <via.h>

extern void getLipsiaVersion(char*,size_t);

int
main(int argc, char *argv[]) {
    static VShort  minval = 0;
    static VDouble radius = 2;
    static VString filename = "";
    static VOptionDescRec  options[] = {
        {"minval", VShortRepn, 1, (VPointer) &minval, VRequiredOpt, NULL, "Signal threshold"},
        {"radius", VDoubleRepn, 1, (VPointer) &radius, VOptionalOpt, NULL, "Opening radius"},
        {
            "raw", VStringRepn, 1, (VPointer) &filename,
            VRequiredOpt, NULL, "fRMI data"
        }
    };
    FILE *in_file, *out_file, *fp;
    VAttrList list = NULL, list1 = NULL;
    VAttrListPosn posn;
    VImage src = NULL, dest = NULL, tmp = NULL, zmap = NULL, mask = NULL;
    int i, n, nbands, nrows, ncols, npixels;
    VBand band;
    VPointer src_pp, *dest_pp;
    VShort *dst_pp;
    VFloat *zmap_pp;
    VBit *bin_pp;
    VBoolean dim3 = FALSE;
    char *ptr1, *ptr2;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vbrainmask V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    /*
    ** extract first timestep
    */
    fp = VOpenInputFile(filename, TRUE);
    list = VReadFile(fp, NULL);
    if(! list)
        VError("Error reading raw data file");
    fclose(fp);
    /*
    ** count number of slices
    */
    band = 0;
    nbands = nrows = ncols = 0;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(VPixelRepn(src) != VShortRepn)
            continue;
        if(VImageNRows(src) > nrows)
            nrows = VImageNRows(src);
        if(VImageNColumns(src) > ncols)
            ncols = VImageNColumns(src);
        nbands++;
    }
    fprintf(stderr, " nslices=%d,  size: %d x %d\n", nbands, nrows, ncols);
    dest = VCreateImage(nbands, nrows, ncols, VShortRepn);
    VFillImage(dest, VAllBands, 0);
    n = 0;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(VPixelRepn(src) != VShortRepn)
            continue;
        if(n == 0)
            VCopyImageAttrs(src, dest);
        if(VImageNRows(src) < 2)
            dim3 = TRUE;
        if(VImageNRows(src) > 1) {
            if(VSelectBand("vtimestep", src, band, &npixels, &src_pp) == FALSE)
                VError("err reading data");
            dest_pp = VPixelPtr(dest, n, 0, 0);
            ptr1 = (char *) src_pp;
            ptr2 = (char *) dest_pp;
            memcpy(ptr2, ptr1, npixels * VPixelSize(src));
        }
        n++;
    }
    /*
    ** apply threshold to zmap
    */
    if(dim3 == FALSE)
        radius = 0;    /* 2D data, do not use 3D opening */
    mask = VCreateImage(VImageNBands(dest), VImageNRows(dest), VImageNColumns(dest), VBitRepn);
    VFillImage(mask, VAllBands, 0);
    if(!(list1 = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    for(VFirstAttr(list1, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & zmap);
        if(VPixelRepn(zmap) != VFloatRepn)
            continue;
        if(VImageNPixels(dest) != VImageNPixels(zmap))
            VError(" inconsistent image dimensions, number of voxels do not match");
        dst_pp = VImageData(dest);
        bin_pp = VImageData(mask);
        for(i = 0; i < VImageNPixels(dest); i++) {
            *bin_pp = 0;
            if(*dst_pp > minval)
                *bin_pp = 1;
            dst_pp++;
            bin_pp++;
        }
        /* clean mask */
        if(radius > 1)
            tmp = VDTOpen(mask, tmp, (VDouble) radius);
        else
            tmp = VCopyImage(mask, tmp, VAllBands);
        mask = VSmoothImage3d(tmp, mask, 1, 2);
        /* apply mask */
        zmap_pp = VImageData(zmap);
        bin_pp  = VImageData(mask);
        for(i = 0; i < VImageNPixels(zmap); i++) {
            if(*bin_pp == 0)
                *zmap_pp = 0;
            zmap_pp++;
            bin_pp++;
        }
        VSetAttrValue(& posn, NULL, VImageRepn, zmap);
    }
    VHistory(VNumber(options), options, prg_name, &list1, &list1);
    if(! VWriteFile(out_file, list1))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}
