/*
** make an object symmetric, used for creating masks
**
** G.Lohmann, April 2005
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) > (y) ? (y) : (x))


VImage
VSymmetric(VImage src, VShort type) {
    VImage dest = NULL;
    int nbands, nrows, ncols, b, r, c, cc, c0;
    double u, v, w;
    nbands  = VImageNBands(src);
    nrows   = VImageNRows(src);
    ncols   = VImageNColumns(src);
    if(ncols % 2 == 0)
        c0 = ncols / 2;
    else
        c0 = ncols / 2 + 1;
    dest = VCopyImage(src, NULL, VAllBands);
    VFillImage(dest, VAllBands, 0);
    /* make left-right symmetric */
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            cc = ncols - 1;
            for(c = 0; c <= c0; c++) {
                u = VGetPixel(src, b, r, c);
                v = VGetPixel(src, b, r, cc);
                w = u;
                if(type == 0)
                    w = MIN(u, v);
                else
                    w = MAX(u, v);
                VSetPixel(dest, b, r, c, w);
                VSetPixel(dest, b, r, cc, w);
                cc--;
            }
        }
    }
    return dest;
}



VDictEntry TypeDict[] = {
    { "min", 0 },
    { "max", 1 },
    { NULL }
};


int main(int argc, char **argv) {
    static VShort type = 0;
    static VOptionDescRec  options[] = {
        {"type", VShortRepn, 1, (VPointer) &type, VOptionalOpt, TypeDict, "type (min or max) "},
    };
    FILE *in_file, *out_file;
    VAttrList list = NULL;
    VImage src = NULL, dest = NULL;
    VAttrListPosn posn;
    /* Parse command line arguments: */
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    /* Read source image (grey matter): */
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        dest = VSymmetric(src, type);
        VSetAttrValue(& posn, NULL, VImageRepn, dest);
    }
    if(src == NULL)
        VError(" src image not found");
    /* Write the results to the output file: */
    if(VWriteFile(out_file, list))
        fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}



