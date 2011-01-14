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
 *  $Id: vgauss.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/os.h>
#include <viaio/VImage.h>

static int verbose = 0;
extern void getLipsiaVersion(char*,size_t);

int main(int argc, char **argv) {
    static VDouble sigma = 1.0;
    static VLong size = 0;
    static VBoolean size_found;
    static VOptionDescRec options[] = {
        {
            "sigma", VDoubleRepn, 1, & sigma, VOptionalOpt, NULL,
            "Standard deviation of Gaussian filter"
        },
        {
            "size", VLongRepn, 1, & size, & size_found, NULL,
            "Filter size (pixels)"
        },
        {"verbose", VShortRepn, 1, (VPointer) &verbose, VOptionalOpt, NULL, "Show program messages"}
    };
    FILE *in_file, *out_file;
    VAttrList list;
    VImage src, dest;
    VAttrListPosn posn;
    int nimages;
    int objects = 0;
   char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vgauss V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments: */
    VParseFilterCmd(VNumber(options), options, argc, argv,
                    & in_file, & out_file);
    /* Filter size defaults to 6.0 * sigma + 1: */
    if(! size_found) {
        size = 6.0 * sigma + 1;
        if((size & 1) == 0)
            size++;
    }
    /* Read source image(s): */
    if(!(list = VReadFile(in_file, NULL)))
        exit(EXIT_FAILURE);
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        objects++;
    }
    /* Performs Gaussian smoothing on each image: */
    nimages = 0;
    fprintf(stderr, "Processing %d/%d\r", nimages, objects);
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        dest = VGaussianConv(src, NULL, VAllBands, sigma, size);
        if(dest == NULL)
            exit(EXIT_FAILURE);
        VSetAttrValue(& posn, NULL, VImageRepn, dest);
        VDestroyImage(src);
        nimages++;
        fprintf(stderr, "Processing %d/%d\r", nimages, objects);
    }
    /* Make History */
    VHistory(VNumber(options), options, prg_name, &list, &list);
    /* Write the results to the output file: */
    if(VWriteFile(out_file, list))
        if(verbose >= 1)
            fprintf(stderr, "%s: Convolved %d image%s with a Gaussian filter.\n",
                    argv[0], nimages, nimages == 1 ? "" : "s");
    fprintf(stderr, "\n%s done.\n", argv[0]);
    return EXIT_SUCCESS;
}
