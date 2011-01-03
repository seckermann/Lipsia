/****************************************************************
 *
 * Program: vaddcovariates
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
 * $Id: vaddcovariates.c 3181 2008-04-01 15:19:44Z karstenm $
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

extern int VStringToken(char *, char *, int, int);
extern char *getLipsiaVersion();

#define LEN  1600   /* max number of characters per line in file */
#define NCOV  200   /* max number of additional covariates     */


VImage
VAddCovariates(VImage design, VString cfile) {
    VImage dest = NULL, tmp = NULL;
    FILE *fp = NULL;
    char buf[LEN], token[80];
    int i, j, r, c, ncols, nrows, add_cols;
    double u;
    float x;
    add_cols = 0;
    nrows = VImageNRows(design);
    ncols = VImageNColumns(design);
    /* read first file */
    fp = fopen(cfile, "r");
    if(fp == NULL)
        VError(" err opening file %s", cfile);
    tmp = VCreateImage(1, nrows, NCOV, VFloatRepn);
    VFillImage(tmp, VAllBands, 0);
    i = 0;
    while(!feof(fp)) {
        for(j = 0; j < LEN; j++)
            buf[j] = '\0';
        fgets(buf, LEN, fp);
        if(buf[0] == '#' || buf[0] == '%')
            continue;
        if(strlen(buf) < 2)
            continue;
        if(i > nrows)
            VError(" too many rows in file (%d), max is %d", i, nrows);
        j = 0;
        while(VStringToken(buf, token, j, 30)) {
            sscanf(token, "%f", &x);
            if(j >= NCOV)
                VError(" too many additional covariates (%d), max is %d", j, NCOV - 1);
            VPixel(tmp, 0, i, j, VFloat) = x;
            j++;
        }
        if(j < 1)
            continue;
        if(add_cols  > 0 && j > add_cols)
            VError(" inconsistent number of columns (%d, %d), line %d", j, add_cols, i + 1);
        if(add_cols == 0 && j > add_cols)
            add_cols = j;
        if(i >= nrows)
            continue;
        i++;
    }
    fclose(fp);
    if(i != nrows)
        VError(" file: inconsistent number of rows: %d %d", i, nrows);
    /*
    ** create new design file
    */
    dest = VCreateImage(1, nrows, ncols + add_cols, VPixelRepn(design));
    VFillImage(dest, VAllBands, 1);
    VCopyImageAttrs(design, dest);
    for(r = 0; r < nrows; r++) {
        for(c = 0; c < ncols; c++) {
            u = VGetPixel(design, 0, r, c);
            VSetPixel(dest, 0, r, c, u);
        }
    }
    /* add covariates */
    for(r = 0; r < nrows; r++) {
        for(c = 0; c < add_cols; c++) {
            u = VPixel(tmp, 0, r, c, VFloat);
            VSetPixel(dest, 0, r, c + ncols - 1, u);
        }
    }
    fprintf(stderr, " %d columns added to design file\n", add_cols);
    return dest;
}




int
main(int argc, char *argv[]) {
    static VString cfile = "";
    static VOptionDescRec  options[] = {
        {"file", VStringRepn, 1, (VPointer) &cfile, VOptionalOpt, NULL, "file"}
    };
    FILE *in_file, *out_file;
    VAttrList list = NULL;
    VAttrListPosn posn;
    VImage design = NULL, dest = NULL;
    char prg_name[50];
    sprintf(prg_name, "vaddcovariates V%s", getLipsiaVersion());
    fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & design);
        if(VPixelRepn(design) != VFloatRepn && VPixelRepn(design) != VDoubleRepn)
            continue;
        dest = VAddCovariates(design, cfile);
        VSetAttrValue(& posn, NULL, VImageRepn, dest);
        break;
    }
    if(design == NULL)
        VError(" design matrix not found ");
    /* Output: */
    if(! VWriteFile(out_file, list))
        exit(1);
    fprintf(stderr, " %s: done.\n", argv[0]);
    return 0;
}

