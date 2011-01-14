/****************************************************************
 *
 * vdotrans: apply an affine transformation
 *
 * Copyright (C) 1999 MPI of Cognitive Neuroscience, Leipzig
 *
 * Author Gaby Lohmann, Apr. 1999, <lohmann@cns.mpg.de>
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
 * $Id: vdotrans.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>

#define MINRESO 0.01
#define MAXRESO 10

extern VImage VDoTrans(VImage, VImage, VImage, VFloat, VLong);
extern void getLipsiaVersion(char*,size_t);

VDictEntry ITYPDict[] = {
    { "trilinear", 0 },
    { "NN", 1 },
    { NULL }
};

VDictEntry TransDict[] = {
    { "anat", 0 },
    { "zmap", 1 },
    { "epi_t1", 2 },
    { "all", 3 },
    { NULL }
};



#define HIGH_RES_ANAT 1
#define FUNCTIONAL    2
#define EPI_T1        3
#define ZMAP          4
#define OTHER         5

int
VGetModality(VImage src) {
    VAttrListPosn testposn;
    if(VPixelRepn(src) == VFloatRepn)
        return ZMAP;
    if((VImageNRows(src) > 150) && (VImageNColumns(src) > 150))
        return (int)HIGH_RES_ANAT;
    if(VPixelRepn(src) == VUByteRepn)
        return (int)EPI_T1;
    if(VLookupAttr(VImageAttrList(src), "slice_time", &testposn) == TRUE)
        return (int)FUNCTIONAL;
    return (int)OTHER;
}



int main(int argc, char *argv[]) {
    /* Command line options: */
    static VString image_filename = "";
    static VLong   type = 0;
    static VLong   sel  = 3;
    static VFloat  resolution = 1.0;
    static VOptionDescRec options[] = {
        {
            "trans", VStringRepn, 1, & image_filename, VRequiredOpt, NULL,
            "File containing transformation matrix"
        },
        {
            "resolution", VFloatRepn, 1, & resolution, VOptionalOpt, NULL,
            "Output voxel resolution in mm"
        },
        {
            "type", VLongRepn, 1, & type, VOptionalOpt, ITYPDict,
            "Type of interpolation: trilinear or NN (nearest neighbor)"
        },
        {
            "object", VLongRepn, 1, & sel, VOptionalOpt, TransDict,
            "Select object(s) to be transformed"
        }
    };
    FILE *in_file, *out_file, *fp;
    VAttrList list, list1, out_list;
    VAttrListPosn posn;
    VImage src = NULL, trans = NULL, dest = NULL;
    VFloat minreso = 0.001;
    int mod = 0;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vdotrans V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments and identify files: */
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    /* Read transformation image: */
    fp = VOpenInputFile(image_filename, TRUE);
    list1 = VReadFile(fp, NULL);
    if(! list1)
        VError("Error reading image");
    fclose(fp);
    for(VFirstAttr(list1, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        if(strcmp(VGetAttrName(&posn), "transform") != 0)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & trans);
        break;
    }
    if(trans == NULL)
        VError("transformation matrix not found");
    if(resolution < MINRESO || resolution > MAXRESO)
        VError("check resolution of destination image");
    /* Read the input file: */
    list = VReadFile(in_file, NULL);
    if(! list)
        VError(" can't read infile");
    fclose(in_file);
    /*
    ** transform selected objects
    */
    out_list = VCreateAttrList();
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        if(strcmp(VGetAttrName(&posn), "transform") == 0)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        mod = VGetModality(src);
        /*
        if (mod == FUNCTIONAL) continue;
        */
        switch(sel) {
        case 0:  /* anat */
            if(mod == HIGH_RES_ANAT) {
                dest = VDoTrans(src, NULL, trans, resolution, type);
                VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
                goto ende;
            }
            break;
        case 1:  /* zmap */
            if(mod == ZMAP) {
                dest = VDoTrans(src, NULL, trans, resolution, type);
                VAppendAttr(out_list, "zmap", NULL, VImageRepn, dest);
                goto ende;
            }
            break;
        case 2:  /* epi_t1 */
            if(mod == EPI_T1) {
                dest = VDoTrans(src, NULL, trans, resolution, type);
                VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
                goto ende;
            }
            break;
        default: /* other */
            dest = VDoTrans(src, NULL, trans, resolution, type);
            VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
        }
    }
    if(dest == NULL)
        VError(" no object transformed");
    /*
    ** output
    */
ende:
    VHistory(VNumber(options), options, prg_name, &list, &out_list);
    if(! VWriteFile(out_file, out_list))
        VError(" can't write output file");
    fprintf(stderr, "%s: done.\n", argv[0]);
    return EXIT_SUCCESS;
}
