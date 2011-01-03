/*
** vsmooth3d -- perform 3d smoothing filter.
**
** the smooth filter computes the most frequent value in a 6 (or 18)
** neighbourhood and replaces the center pixel with this value.
** The pixels within the neighbourhood are weighted.
** This program is useful for postprocessing of segmentation results.
** It will produce smooth boundaries between regions.
**
** Update: topologically correct smoothing: only simple points are changed.
**
** G.Lohmann <lohmann@cns.mpg.de>, Dec 2001
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C library: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <via.h>




VImage
VTopSmoothImage3d(VImage bit_image, VImage grey_image, VImage result, VLong neighb, VLong numiter) {
    long nbands, nrows, ncols, npixels;
    VRepnKind repn;
    long i, i0, i1, n, iter, npts, u;
    long b0, b1, r0, r1, c0, c1, b, r, c;
    long n1, n6, n18, n26;
    int isum;
    double sum, norm = 0;
    VBit *dest_pp, *bit_pp;
    VUByte *ubyte_pp;
    int background = 0;
    VPoint *array = NULL;
    repn   = VPixelRepn(bit_image);
    if(repn != VBitRepn)
        VError("Smooth3d: repn must be bit");
    nbands = VImageNBands(bit_image);
    nrows  = VImageNRows(bit_image);
    ncols  = VImageNColumns(bit_image);
    npixels = nbands * nrows * ncols;
    if(result == NULL)
        result = VCreateImage(nbands, nrows, ncols, repn);
    if(! result)
        return NULL;
    bit_pp = VPixelPtr(bit_image, 0, 0, 0);
    dest_pp = VPixelPtr(result, 0, 0, 0);
    for(i = 0; i < npixels; i++)
        *dest_pp++ = *bit_pp++;
    n1  = 8;
    n6  = 4;
    n18 = 2;
    n26 = 1;
    switch(neighb) {
    case 0:
        norm = n1 + 6 * n6;
        break;
    case 1:
        norm = n1 + 6 * n6 + 12 * n18;
        break;
    case 2:
        norm = n1 + 6 * n6 + 12 * n18 + 8 * n26;
        break;
    default:
        ;
    }
    n = 1;
    ubyte_pp = VPixelPtr(grey_image, 0, 0, 0);
    for(i = 0; i < npixels; i++)
        if(*ubyte_pp++ > background)
            n++;
    array = (VPoint *) VMalloc(sizeof(VPoint) * (n + 2));
    for(i = 0; i < n + 2; i++)
        array[i].val = array[i].b = array[i].r = array[i].c = 0;
    n = 1;
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                u = VPixel(grey_image, b, r, c, VUByte);
                if(u > background) {
                    array[n].b = b;
                    array[n].r = r;
                    array[n].c = c;
                    array[n].val = (float)u;
                    n++;
                }
            }
        }
    }
    npts = n;
    VPoint_hpsort(n - 1, array);
    iter = 0;
    n = 100;
    while(n > 1 && iter < numiter) {
        iter++;
        n = 0;
        for(i = 1; i < npts; i++) {
            b = array[i].b;
            r = array[i].r;
            c = array[i].c;
            b0 = (b < 1) ? 0 : b - 1;
            b1 = (b > nbands - 2) ?  nbands - 1 : b + 1;
            r0 = (r < 1) ? 0 : r - 1;
            r1 = (r > nrows - 2) ?  nrows - 1 : r + 1;
            c0 = (c < 1) ? 0 : c - 1;
            c1 = (c > ncols - 2) ?  ncols - 1 : c + 1;
            isum = 0;
            i1 = VPixel(bit_image, b, r, c, VBit);
            isum += (int) i1 * n1;
            isum += (int) VPixel(bit_image, b0, r, c, VBit) * n6;
            isum += (int) VPixel(bit_image, b, r0, c, VBit) * n6;
            isum += (int) VPixel(bit_image, b, r, c0, VBit) * n6;
            isum += (int) VPixel(bit_image, b1, r, c, VBit) * n6;
            isum += (int) VPixel(bit_image, b, r1, c, VBit) * n6;
            isum += (int) VPixel(bit_image, b, r, c1, VBit) * n6;
            if(neighb == 1) {
                isum += VPixel(bit_image, b0, r0, c, VBit) * n18;
                isum += (int) VPixel(bit_image, b, r0, c0, VBit) * n18;
                isum += (int) VPixel(bit_image, b0, r, c0, VBit) * n18;
                isum += (int) VPixel(bit_image, b1, r1, c, VBit) * n18;
                isum += (int) VPixel(bit_image, b, r1, c1, VBit) * n18;
                isum += (int) VPixel(bit_image, b1, r, c1, VBit) * n18;
                isum += (int) VPixel(bit_image, b1, r0, c, VBit) * n18;
                isum += (int) VPixel(bit_image, b, r1, c0, VBit) * n18;
                isum += (int) VPixel(bit_image, b1, r, c0, VBit) * n18;
                isum += (int) VPixel(bit_image, b0, r1, c, VBit) * n18;
                isum += (int) VPixel(bit_image, b, r0, c1, VBit) * n18;
                isum += (int) VPixel(bit_image, b0, r, c1, VBit) * n18;
            }
            if(neighb == 2) {
                isum += (int) VPixel(bit_image, b0, r0, c0, VBit) * n26;
                isum += (int) VPixel(bit_image, b1, r0, c0, VBit) * n26;
                isum += (int) VPixel(bit_image, b0, r1, c0, VBit) * n26;
                isum += (int) VPixel(bit_image, b0, r0, c1, VBit) * n26;
                isum += (int) VPixel(bit_image, b1, r1, c0, VBit) * n26;
                isum += (int) VPixel(bit_image, b1, r0, c1, VBit) * n26;
                isum += (int) VPixel(bit_image, b0, r1, c1, VBit) * n26;
                isum += (int) VPixel(bit_image, b1, r1, c1, VBit) * n26;
            }
            sum = (double) isum / (double) norm;
            i0 = 0;
            if(sum >= 0.5)
                i0 = 1;
            if(i1 == 1 && i0 == 0) {
                if(VSimplePoint(result, b, r, c, 26) == 0)
                    i0 = 1;
            } else if(i1 == 0 && i0 == 1) {
                VPixel(result, b, r, c, VBit) = 1;
                if(VSimplePoint(result, b, r, c, 26) == 0)
                    i0 = 0;
            }
            VPixel(result, b, r, c, VBit) = i0;
            if(i0 != i1)
                n++;
        }
        if(numiter > 1) {
            bit_pp  = (VBit *) VImageData(bit_image);
            dest_pp = (VBit *) VImageData(result);
            for(i = 0; i < npixels; i++)
                *bit_pp++ = *dest_pp++;
        }
    }
    /* Successful completion: */
    VCopyImageAttrs(bit_image, result);
    return result;
}


#ifdef MM
VDictEntry TYPDict[] = {
    { "6", 0 },
    { "18", 1 },
    { "26", 2 },
    { NULL }
};


int main(int argc, char **argv) {
    static VString filename = "";
    static VLong neighb = 2;
    static VLong iter = 1;
    static VOptionDescRec options[] = {
        {
            "n", VLongRepn, 1, &neighb, VOptionalOpt, TYPDict,
            "type of neighbourhood used for smoothing"
        },
        {
            "iter", VLongRepn, 1, &iter, VOptionalOpt, NULL,
            "number of iterations"
        },
        {
            "image", VStringRepn, 1, &filename, VOptionalOpt, NULL,
            "grey value image"
        },
    };
    FILE *in_file, *out_file, *fp;
    VAttrList list, list1;
    VImage src = NULL, dest, image = NULL;
    VAttrListPosn posn;
    /* Parse command line arguments: */
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    /* Read transformation image: */
    fp = VOpenInputFile(filename, TRUE);
    list1 = VReadFile(fp, NULL);
    if(! list1)
        VError("Error reading image");
    fclose(fp);
    for(VFirstAttr(list1, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & image);
        if(VPixelRepn(image) != VUByteRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & image);
        break;
    }
    if(image == NULL)
        VError(" image not found");
    /* Read source image(s): */
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        dest = VTopSmoothImage3d(src, image, NULL, neighb, iter);
        if(dest == NULL)
            exit(1);
        VSetAttrValue(& posn, NULL, VImageRepn, dest);
        VDestroyImage(src);
    }
    /* Write the results to the output file: */
    if(VWriteFile(out_file, list))
        fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}

#endif

