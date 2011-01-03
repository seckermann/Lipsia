/****************************************************************
 *
 * Program: vcatdesign
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
 * $Id: vcatdesign.c 3181 2008-04-01 15:19:44Z karstenm $
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
#include <ctype.h>

extern char *getLipsiaVersion();

VImage
ConcatDesigns(VImage *design, int nimages) {
    VImage dest = NULL;
    VString buf = NULL;
    VDouble x;
    int i, r, c, row, col;
    int nrows, ncols;
    nrows = ncols = 0;
    for(i = 0; i < nimages; i++) {
        ncols += VImageNColumns(design[i]);
        nrows += VImageNRows(design[i]);
    }
    dest = VCreateImage(1, nrows, ncols, VFloatRepn);
    VFillImage(dest, VAllBands, 0);
    VCopyImageAttrs(design[0], dest);
    VSetAttr(VImageAttrList(dest), "nsessions", NULL, VShortRepn, (VShort)nimages);
    buf = (VString) VMalloc(80);
    buf[0] = '\0';
    for(i = 0; i < nimages; i++)
        sprintf(buf, "%s %d", buf, (int)VImageNRows(design[i]));
    VSetAttr(VImageAttrList(dest), "session_lengths", NULL, VStringRepn, buf);
    buf[0] = '\0';
    for(i = 0; i < nimages; i++)
        sprintf(buf, "%s %d", buf, (int)VImageNColumns(design[i]));
    VSetAttr(VImageAttrList(dest), "session_covariates", NULL, VStringRepn, buf);
    row = col = 0;
    for(i = 0; i < nimages; i++) {
        for(r = 0; r < VImageNRows(design[i]); r++) {
            for(c = 0; c < VImageNColumns(design[i]); c++) {
                x = VGetPixel(design[i], 0, r, c);
                if(row + r >= VImageNRows(dest))
                    VError(" row: %d\n", row + r);
                if(col + c >= VImageNColumns(dest))
                    VError(" column: %d\n", col + c);
                VSetPixel(dest, 0, row + r, col + c, x);
            }
        }
        row += VImageNRows(design[i]);
        col += VImageNColumns(design[i]);
    }
    return dest;
}



int
main(int argc, char *argv[]) {
    static VArgVector in_files;
    static VString out_filename;
    static VBoolean in_found, out_found;
    static VOptionDescRec  options[] = {
        {"in", VStringRepn, 0, & in_files, & in_found, NULL, "Input files" },
        {"out", VStringRepn, 1, & out_filename, & out_found, NULL, "Output file" }
    };
    FILE *fp = NULL;
    VStringConst in_filename;
    VAttrList list = NULL, out_list = NULL;
    VImage *design = NULL, tmp = NULL, dest = NULL;
    VAttrListPosn posn;
    int  i, nimages;
    char prg[50];
    sprintf(prg, "vcatdesign V%s", getLipsiaVersion());
    fprintf(stderr, "%s\n", prg);
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
    /* Read each input file */
    nimages = in_files.number;
    design = (VImage *) VMalloc(sizeof(VImage) * nimages);
    for(i = 0; i < nimages; i++) {
        in_filename = ((VStringConst *) in_files.vector)[i];
        fprintf(stderr, " reading:  %s\n", in_filename);
        fp = VOpenInputFile(in_filename, TRUE);
        list = VReadFile(fp, NULL);
        if(! list)
            VError("Error reading file %s", in_filename);
        fclose(fp);
        for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & tmp);
            if(VPixelRepn(tmp) == VFloatRepn || VPixelRepn(tmp) == VDoubleRepn) {
                design[i] = tmp;
                break;
            }
        }
    }
    /* process */
    dest = ConcatDesigns(design, nimages);
    out_list = VCreateAttrList();
    VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
    /* Output: */
    VHistory(VNumber(options), options, prg, &list, &out_list);
    fp = VOpenOutputFile(out_filename, TRUE);
    if(!fp)
        VError(" error opening outout file %s", out_filename);
    if(! VWriteFile(fp, out_list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}

