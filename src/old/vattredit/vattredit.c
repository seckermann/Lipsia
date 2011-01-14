/****************************************************************
 *
 *  Copyright 1993, 1994 University of British Columbia
 *
 *  Permission to use, copy, modify, distribute, and sell this software and its
 *  documentation for any purpose is hereby granted without fee, provided that
 *  the above copyright notice appears in all copies and that both that
 *  copyright notice and this permission notice appear in supporting
 *  documentation. UBC makes no representations about the suitability of this
 *  software for any purpose. It is provided "as is" without express or
 *  implied warranty.
 *
 *  Author: Arthur Pope, UBC Laboratory for Computational Intelligence
 *
 *  $Id: vattredit.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>
#include <viaio/VGraph.h>
#include <viaio/Volumes.h>

/* From the standard C library: */
#include <stdio.h>
#include <math.h>

extern VImage VEditImage(VImage, VImage);
extern void getLipsiaVersion(char*,size_t);

int main(int argc, char *argv[]) {
    static VLong objnr = -1;
    static VString name = "";
    static VString value = "";
    static VOptionDescRec options[] = {
        {
            "obj", VLongRepn, 1, (VPointer) &objnr,
            VOptionalOpt, NULL, "object number, all objects (-1)"
        },
        {
            "name", VStringRepn, 1, (VPointer) &name,
            VRequiredOpt, NULL, "attribute name"
        },
        {
            "value", VStringRepn, 1, (VPointer) &value,
            VRequiredOpt, NULL, "attribute value"
        }
    };
    FILE *in_file, *out_file;
    VAttrList list, olist;
    VAttrListPosn posn;
    VImage isrc;
    VGraph gsrc;
    Volumes vsrc;
    VString buf;
    int nobj;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vattredit V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments and identify files: */
    VParseFilterCmd(VNumber(options), options, argc, argv,
                    &in_file, & out_file);
    /* Read source image(s): */
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    /* Process each image: */
    nobj = 0;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        olist = NULL;
        if(nobj == objnr || objnr < 0) {
            if(VGetAttrRepn(& posn) == VImageRepn) {
                VGetAttrValue(& posn, NULL, VImageRepn, & isrc);
                olist = VImageAttrList(isrc);
            } else if(VGetAttrRepn(& posn) == VGraphRepn) {
                VGetAttrValue(& posn, NULL, VGraphRepn, & gsrc);
                olist = VGraphAttrList(gsrc);
            } else if(VGetAttrRepn(& posn) == VolumesRepn) {
                VGetAttrValue(& posn, NULL, VolumesRepn, & vsrc);
                olist = VolumesAttrList(vsrc);
            }
            if(olist != NULL) {
                if(VGetAttr(olist, name, NULL, VStringRepn, &buf) == VAttrFound)
                    fprintf(stderr, " object %3d, old value: %s\n", nobj, buf);
                VSetAttr(olist, name, NULL, VStringRepn, value);
                fprintf(stderr, " object %3d, new value: %s\n", nobj, value);
            }
        }
        nobj++;
    }
    /* Make History */
    VHistory(VNumber(options), options, prg_name, &list, &list);
    /* Write out the results: */
    if(! VWriteFile(out_file, list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}
