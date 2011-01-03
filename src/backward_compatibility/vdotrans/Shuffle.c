
/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/os.h>
#include <viaio/VImage.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VImage
VShuffleSlices(VImage src, VImage dest) {
    VPointer src_pp, *dest_pp;
    char *ptr1, *ptr2;
    int nbands, nrows, ncols, npixels;
    VBand band;
    nbands = VImageNBands(src);
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    dest = VSelectDestImage("VShuffleSlices", dest, nbands, nrows, ncols, VPixelRepn(src));
    if(! dest)
        VError(" err creating dest image");
    VCopyImageAttrs(src, dest);
    for(band = 0; band < nbands; band++) {
        VSelectBand("VShuffleSlices", src, band, &npixels, &src_pp);
        dest_pp = VPixelPtr(dest, (int)(nbands - band - 1), 0, 0);
        ptr1 = (char *) src_pp;
        ptr2 = (char *) dest_pp;
        memcpy(ptr2, ptr1, npixels * VPixelSize(src));
    }
    return dest;
}


