/****************************************************************
 *
 * vgetcovariates:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Karsten Mueller, 2001, <lipsia@cbs.mpg.de>
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
 * $Id: vgetcovariates.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "viaio/headerinfo.h"


VDictEntry TYPDict[] = {
    { "single", 0 },
    { "8adj",   1 },
    { "26adj",  2 },
    { NULL }
};

VDictEntry TALDict[] = {
    { "voxel"    , 0 },
    { "mm"       , 1 },
    { "talairach", 2 },
    { NULL }
};

VDictEntry OUTDict[] = {
    { "vista", 0 },
    { "txt",   1 },
    /* { "des",   2 }, */
    { NULL }
};

extern void VImageInfoIni(VImageInfo *);
extern void VglmGetRegr(VImageInfo *, FILE *, FILE *, int *, VString, VString, char *, char *, char *);

extern VBoolean  ReadHeader(FILE *);
extern VBoolean  VGetImageInfo(FILE *, VAttrList, int, VImageInfo *);
extern VAttrList ReadAttrList(FILE *);
extern char *getLipsiaVersion();

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

int
main(int argc, char *argv[]) {
    /* Command line options: */
    static VShort addr[3];
    static VShort type = 0;
    static VShort system = 0;
    static VShort outtype = 0;
    static VBoolean norm = 1;
    static VBoolean ppi = 0;
    static VBoolean orth = 1;
    static VBoolean conv = 0;
    static VString reportfile = " ";
    static VDouble minval = 2000;
    static VString regressor = " ";
    static VOptionDescRec  options[] = {
        { "outtype", VShortRepn, 1, &outtype, VOptionalOpt, OUTDict, "Type of output"},
        { "addr", VShortRepn, 3, &addr, VRequiredOpt, NULL, "Address of point to be processed"},
        { "type", VShortRepn, 1, &type, VOptionalOpt, TYPDict, "Type of region for regressor"},
        { "system", VShortRepn, 1, &system, VOptionalOpt, TALDict, "Type of coordinate system" },
        { "norm",   VBooleanRepn, 1, &norm, VOptionalOpt, NULL, "Normalize all regressors"},
        { "report", VStringRepn, 1, &reportfile, VOptionalOpt, NULL, "Report file with voxel timecourse"},
        { "minval", VDoubleRepn, 1, &minval, VRequiredOpt, 0, "Signal threshold" },
        { "regr", VStringRepn, 1, &regressor, VOptionalOpt, NULL, "Additional (psychological) regressor"},
        { "conv", VBooleanRepn, 1, &conv, VOptionalOpt, NULL, "Convolve additional (psychological) regressor with HRF"},
        { "ppi", VBooleanRepn, 1, &ppi, VOptionalOpt, NULL, "PPI analysis"},
        { "orth", VBooleanRepn, 1, &orth, VOptionalOpt, NULL, "Orthogonalize PPI interaction term"},
    };
    FILE *in_file, *out_file;
    VString mpilvista0;
    VStringConst in_filename;
    VAttrList list;
    VAttrListPosn posn;
    VImageInfo *imageInfo = NULL;
    VBoolean sw = 0, sw1 = 0;
    int nobject = 0;
    int i, nobj, trpre, tr = -1;
    int firstfuncobj = 0, funcobj = 0;
    int num_rows = 0, num_columns = 0, num_bands = 0;
    int hist_items = 0, pos, *ptrvi;
    char *token = NULL, *tmptoken = NULL;
    char voxelstr[50], extentstr[100], castr[100];
    char prg_name[50];
    sprintf(prg_name, "vgetcovariates V%s", getLipsiaVersion());
    fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments and identify files: */
    VParseFilterCmd(VNumber(options), options, argc, argv, & in_file, & out_file);
    /* Read header of input file */
    if(! ReadHeader(in_file))
        VError("error reading header");
    if(!(list = ReadAttrList(in_file)))
        VError("error reading attr list");
    /* Count the number of objects in vista file */
    nobject = 0;
    hist_items = 0;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(strncmp(VGetAttrName(&posn), "history", 7) == 0) {
            hist_items++;
            continue;
        }
        nobject++;
    }
    /* fprintf(stderr, "%2d objects read (%d history items)\n", nobject, hist_items); */
    if(imageInfo == NULL)
        imageInfo  = (VImageInfo *) VMalloc(sizeof(VImageInfo) * (nobject + hist_items));
    /* Read info about data */
    funcobj = 0;
    for(nobj = 0; nobj < nobject; nobj++) {
        /* to get position in attribute list, add offset 'hist_items' */
        pos = nobj + hist_items;
        VImageInfoIni(&imageInfo[nobj]);
        if(! VGetImageInfo(in_file, list, pos, &imageInfo[nobj]))
            VError("Error reading functional slice.");
        /* FUNCTIONAL: */
        if(imageInfo[nobj].repn == VShortRepn) {
            funcobj++;
            /* Which object is the first one which contains functional data ? */
            if(sw == 0) {
                firstfuncobj = nobj;
                sw = 1;
            }
            /* do that only for full objects */
            if(imageInfo[nobj].nbands > 1) {
                /* Read tr from header */
                if(strncmp(imageInfo[nobj].MPIL_vista_0, "N", 1) != 0) {
                    mpilvista0 = (VString) strdup(imageInfo[nobj].MPIL_vista_0);
                    token = strtok(mpilvista0, " ");
                    tmptoken = strpbrk(token, "=");
                    tmptoken++;
                    trpre = (int) atoi(tmptoken);
                    VFree(mpilvista0);
                }
                /* Read tr from header */
                if(imageInfo[nobj].repetition_time > 0)
                    trpre = (int)imageInfo[nobj].repetition_time;
                /* Get number of bands, rows, columns, tr for every file/object */
                if(sw1 == 0) {
                    num_bands = imageInfo[nobj].nbands;
                    num_rows = imageInfo[nobj].nrows;
                    num_columns = imageInfo[nobj].ncolumns;
                    strcpy(voxelstr , imageInfo[nobj].voxel);
                    strcpy(extentstr, imageInfo[nobj].extent);
                    strcpy(castr    , imageInfo[nobj].ca);
                    tr = trpre;
                    sw1 = 1;
                }
                /* Check number of columns, rows, bands and tr */
                if(num_rows    != imageInfo[nobj].nrows)
                    VError(" number of rows is different");
                if(num_columns != imageInfo[nobj].ncolumns)
                    VError(" number of columns is different");
                if(num_bands   != imageInfo[nobj].nbands)
                    VError(" number of time steps is different");
                if(tr != trpre)
                    VError(" tr in objects is different");
                if(strcmp(voxelstr, imageInfo[nobj].voxel))
                    VError(" different voxel size in objects");
                if(strcmp(extentstr, imageInfo[nobj].extent))
                    VError(" different extent size in objects");
                if(strcmp(castr, imageInfo[nobj].ca))
                    VError(" different ca in objects");
            }
        }
    }
    /* Prepare integer-pointer */
    ptrvi    = (int *) malloc(sizeof(int *) * 19);
    ptrvi[0] = (int)nobject;
    ptrvi[1] = (int)outtype;
    ptrvi[2] = (int)minval;
    ptrvi[3] = (int)firstfuncobj;
    ptrvi[4] = (int)funcobj;
    ptrvi[5] = (int)num_bands;
    ptrvi[6] = (int)num_rows;
    ptrvi[7] = (int)num_columns;
    ptrvi[8] = (int)num_bands;
    ptrvi[9] = (int)system;
    ptrvi[10] = (int)type;
    ptrvi[11] = (int)addr[0];
    ptrvi[12] = (int)addr[1];
    ptrvi[13] = (int)addr[2];
    ptrvi[14] = (int)norm;
    ptrvi[15] = (int)ppi;
    ptrvi[16] = (int)orth;
    ptrvi[17] = (int)tr;
    ptrvi[18] = (int)conv;
    /* Get the Regressor */
    VglmGetRegr(imageInfo, in_file, out_file, ptrvi, reportfile, regressor, voxelstr, extentstr, castr);
    /* The End */
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}




