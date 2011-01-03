/*
** Brain peeling
** assumes that voxels are isotropic with 1x1x1mm resolution.
**
** G.Lohmann, Feb 2001
**
*/

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "via.h"
#include "viaio/mu.h"



/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))

#define BACKGROUND 30


/*
**  find bounding box (parameter 'extent')
*/
void
BoundingBox(VImage src, XPoint *or1, XPoint *or2, XPoint *extent) {
    int nbands, nrows, ncols;
    int b0, b1, r0, r1, c0, c1, r2, c2;
    int db, dr, dc;
    int i, b, r, c;
    VShort t_low = 50;    /* lower threshold for background pixels  */
    VShort t_high = 200;  /* upper threshold for grey matter pixels */
    XPoint fix;
    float x, y, z;
    VString str;
    /* get fixed point */
    if(VGetAttr(VImageAttrList(src), "fixpoint", NULL,
                VStringRepn, (VPointer) & str) == VAttrFound) {
        sscanf(str, "%f %f %f", &x, &y, &z);
        fix.x = x;
        fix.y = y;
        fix.z = z;
    } else {
        fix.x = 80;
        fix.y = 95;
        fix.z = 90;
    }
    nbands = VImageNBands(src);
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    c0 = ncols;
    c1 = 0;
    r0 = nrows;
    r1 = 0;
    b0 = nbands;
    b1 = 0;
    c2 = (int)VRint((double)(fix.x));
    r2 = (int)VRint((double)(fix.y + 5));
    r2 = (int)VRint((double)(fix.y + 4));
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                if(ABS(c - c2) < 10)
                    continue;      /* avoid brain stem and falx */
                i = (int)VRint((double)VGetPixel(src, b, r, c));
                if(i < t_low)
                    continue;
                if(i > t_high)
                    continue;
                if(ABS(c - c2) > 5)
                    if(b < b0)
                        b0 = b;
                if(r < r2 && ABS(c - c2) > 15) {
                    if(b > b1) {
                        b1 = b;
                    }
                }
                if(r < r0)
                    r0 = r;
                if(r > r1)
                    r1 = r;
                if(c < c0)
                    c0 = c;
                if(c > c1)
                    c1 = c;
            }
        }
    }
    db = b1 - b0;
    dr = r1 - r0;
    dc = c1 - c0;
    (*or1).x = c0;
    (*or1).y = r0;
    (*or1).z = b0;
    (*or2).x = c1;
    (*or2).y = r1;
    (*or2).z = b1;
    (*extent).x = dc;
    (*extent).y = dr;
    (*extent).z = db;
    fprintf(stderr, " bounding box:  ");
    fprintf(stderr, "  x= %d %d,   y= %d %d,   z= %d %d.\n",
            c0, c1, r0, r1, b0, b1);
}



VBoolean
SetT(XPoint point, int b, int r, int c) {
    double d, dz, dy, dx;
    double radius = 10.0;
    dz = (point).z - b;
    dy = (point).y - r;
    dx = (point).x - c;
    d = sqrt(dz * dz + dy * dy + dx * dx);
    if(d < radius)
        return TRUE;
    else
        return FALSE;
}


VImage
VHistoEqualizer(VImage src, VImage dest, VDouble background) {
    VUByte *ubyte_pp, *dest_pp;
    int nbands, nrows, ncols, npixels;
    double sum;
    double histo[256], lut[256], nx;
    int i, j;
    nbands = VImageNBands(src);
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    npixels = nbands * nrows * ncols;
    if(VPixelRepn(src) != VUByteRepn)
        VError(" input image must be ubyte.");
    if(dest == NULL)
        dest = VCreateImage(nbands, nrows, ncols, VUByteRepn);
    else
        VFillImage(dest, VAllBands, 0);
    for(j = 0; j < 256; j++)
        histo[j] = lut[j] = 0;
    nx = 0;
    ubyte_pp = (VUByte *) VPixelPtr(src, 0, 0, 0);
    for(i = 0; i < npixels; i++) {
        j = *ubyte_pp++;
        if(j < background)
            continue;
        nx++;
        histo[j]++;
    }
    sum = 0;
    for(j = 0; j < 256; j++)
        sum += histo[j];
    for(j = (int)VRint((double)background); j < 256; j++)
        histo[j] /= sum;
    for(i = (int)VRint((double)background); i < 256; i++) {
        sum = 0;
        for(j = (int)VRint((double)background); j <= i; j++)
            sum += histo[j];
        lut[i] = sum * 255.0f;
    }
    ubyte_pp = (VUByte *) VPixelPtr(src, 0, 0, 0);
    dest_pp  = (VUByte *) VPixelPtr(dest, 0, 0, 0);
    for(i = 0; i < npixels; i++) {
        j = *ubyte_pp;
        *dest_pp = (VUByte)VRint(lut[j]);
        ubyte_pp++;
        dest_pp++;
    }
    VCopyImageAttrs(src, dest);
    return dest;
}

/*
** separate histogram equalization for left and right hemispheres
*/
VImage
VHistoEqualizerLR(VImage src, VImage dest, VDouble background) {
    int nbands, nrows, ncols;
    double sum = 0;
    double histo[256], lut[256], nx;
    int i, j;
    int b, r, c, c2;
    nbands = VImageNBands(src);
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    if(VPixelRepn(src) != VUByteRepn)
        VError(" input image must be ubyte.");
    if(dest == NULL)
        dest = VCreateImage(nbands, nrows, ncols, VUByteRepn);
    else
        VFillImage(dest, VAllBands, 0);
    c2 = (int)VRint((double)ncols * (double)0.5);
    /*
    ** left hemisphere
    */
    for(j = 0; j < 256; j++)
        histo[j] = lut[j] = 0;
    nx = 0;
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < c2; c++) {
                j = VPixel(src, b, r, c, VUByte);
                if(j < background)
                    continue;
                nx++;
                histo[j]++;
            }
        }
    }
    sum = 0;
    for(j = 0; j < 256; j++)
        sum += histo[j];
    for(j = (int)VRint((double)background); j < 256; j++)
        histo[j] /= sum;
    for(i = (int)VRint((double)background); i < 256; i++) {
        sum = 0;
        for(j = (int)VRint((double)background); j <= i; j++)
            sum += histo[j];
        lut[i] = sum * 255.0f;
    }
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < c2; c++) {
                j = VPixel(src, b, r, c, VUByte);
                j = (int)VRint(lut[j]);
                VPixel(dest, b, r, c, VUByte) = j;
            }
        }
    }
    /*
    ** right hemisphere
    */
    for(j = 0; j < 256; j++)
        histo[j] = lut[j] = 0;
    nx = 0;
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = c2 + 1; c < ncols; c++) {
                j = VPixel(src, b, r, c, VUByte);
                if(j < background)
                    continue;
                nx++;
                histo[j]++;
            }
        }
    }
    sum = 0;
    for(j = 0; j < 256; j++)
        sum += histo[j];
    for(j = (int)VRint((double)background); j < 256; j++)
        histo[j] /= sum;
    for(i = (int)VRint((double)background); i < 256; i++) {
        sum = 0;
        for(j = (int)VRint((double)background); j <= i; j++)
            sum += histo[j];
        lut[i] = sum * 255.0f;
    }
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = c2; c < ncols; c++) {
                j = VPixel(src, b, r, c, VUByte);
                j = (int)VRint(lut[j]);
                VPixel(dest, b, r, c, VUByte) = j;
            }
        }
    }
    VCopyImageAttrs(src, dest);
    return dest;
}




/*
** brain peeling
*/
VImage
VPeel(VImage src, VImage dest, VDouble d1, VDouble d2, VDouble alpha) {
    VImage tmp, tmp1, bin_image, dt_image, label_image, mask_image;
    int b, r, c, nbands, nrows, ncols, npixels;
    int i, nl;
    double t, u;
    int grey_matter, background;
    VUByte *src_pp, *dest_pp, *tmp_pp;
    VBit *bin_pp;
    VFloat *float_pp;
    XPoint or1, or2, extent, fix;
    char info[120];
    VString str;
    float x, y, z;
    int b0, r0, r1, c0, c1;
    /* get fixed point */
    if(VGetAttr(VImageAttrList(src), "fixpoint", NULL,
                VStringRepn, (VPointer) & str) == VAttrFound) {
        sscanf(str, "%f %f %f", &x, &y, &z);
        fix.x = x;
        fix.y = y;
        fix.z = z;
    } else {
        fix.x = 80;
        fix.y = 95;
        fix.z = 90;
    }
    nbands = VImageNBands(src);
    nrows = VImageNRows(src);
    ncols = VImageNColumns(src);
    npixels = nbands * nrows * ncols;
    /* histogram equalization */
    background = BACKGROUND;
    tmp  = VHistoEqualizerLR(src, NULL, background);
    tmp1 = VHistoEqualizerLR(tmp, NULL, 5);
    tmp  = VCopyImage(tmp1, tmp, VAllBands);
    /* segmentation threshold */
    grey_matter = (int)VRint((double)alpha);
    /* binarize (crude white matter segmentation) */
    bin_image = VCreateImage(nbands, nrows, ncols, VBitRepn);
    src_pp  = (VUByte *) VPixelPtr(tmp, 0, 0, 0);
    bin_pp  = (VBit *) VPixelPtr(bin_image, 0, 0, 0);
    for(i = 0; i < npixels; i++) {
        if(*src_pp < grey_matter)
            *bin_pp = 1;
        else
            *bin_pp = 0;
        src_pp++;
        bin_pp++;
        tmp_pp++;
    }
    /* eyes */
    b0 = (int)VRint((double)(fix.z + 5));         /* 100 */
    r1 = (int)VRint((double)(fix.y - 40));         /*  50 */
    for(b = b0; b < nbands; b++) {
        for(r = 0; r < r1; r++) {
            for(c = 0; c < ncols; c++) {
                u = VPixel(tmp, b, r, c, VUByte);
                if(u > 245) {
                    VPixel(bin_image, b, r, c, VBit) = 1;
                }
            }
        }
    }
    /* bottom 1 */
    b0 = (int)VRint((double)(fix.z + 25));         /* 115 */
    r1 = (int)VRint((double)(fix.y +  5));         /* 100 */
    for(b = b0; b < nbands; b++) {
        for(r = 0; r < r1; r++) {
            for(c = 0; c < ncols; c++) {
                u = VPixel(tmp, b, r, c, VUByte);
                if(u > 245) {
                    VPixel(bin_image, b, r, c, VBit) = 1;
                }
            }
        }
    }
    /* bottom 2 */
    b0 = (int)VRint((double)(fix.z + 35));         /* 125 */
    for(b = b0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                u = VPixel(tmp, b, r, c, VUByte);
                if(u > 245) {
                    VPixel(bin_image, b, r, c, VBit) = 1;
                }
            }
        }
    }
    /* erode */
    dt_image = VChamferDist3d(bin_image, NULL, VFloatRepn);
    bin_pp  = (VBit *) VPixelPtr(bin_image, 0, 0, 0);
    for(i = 0; i < npixels; i++) {
        *bin_pp = *bin_pp > 0 ? 0 : 1;
        bin_pp++;
    }
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                t = d1;
                if(b > fix.z + 20 && r < fix.y)
                    t = 2.0 * d1;
                if(b > fix.z + 35 && r < fix.y + 25)
                    t = 3.0 * d1;
                if(b > fix.z + 30 && r > fix.y + 40)
                    t = 2.0 * d1;
                if(t < 3.0 && b > fix.z + 35 && r > fix.y + 10 && r < fix.y + 40 && ABS(c - fix.x) > 15)
                    t = 2.5 * d1;
                if(b > fix.z + 55)
                    t = 4.0 * d1;
                /* temporal base */
                if(b > fix.z + 30 && b < fix.z + 40 && ABS(c - fix.x) > 15
                        && r > fix.y - 22 && r < fix.y + 5)
                    t = 1.5 * d1;
                u = VPixel(dt_image, b, r, c, VFloat);
                if(u < t)
                    VPixel(bin_image, b, r, c, VBit) = 0;
            }
        }
    }
    /* select largest component */
    label_image = VLabelImage3d(bin_image, NULL, 6, VShortRepn, &nl);
    bin_image = VSelectBig(label_image, bin_image);
    /* dilate */
    mask_image = VDTDilate(bin_image, NULL, d2);
    bin_image  = VDTClose(mask_image, bin_image, (VDouble)15.0);
    /* get distance to border */
    bin_pp  = (VBit *) VPixelPtr(bin_image, 0, 0, 0);
    for(i = 0; i < npixels; i++) {
        *bin_pp = *bin_pp > 0 ? 0 : 1;
        bin_pp++;
    }
    dt_image = VChamferDist3d(bin_image, dt_image, VFloatRepn);
    bin_pp  = (VBit *) VPixelPtr(bin_image, 0, 0, 0);
    for(i = 0; i < npixels; i++) {
        *bin_pp = *bin_pp > 0 ? 0 : 1;
        bin_pp++;
    }
    /* falx */
    r0 = (int)VRint((double)(fix.y + 5));         /* 100 */
    c0 = (int)VRint((double)(fix.x - 5));         /*  75 */
    c1 = (int)VRint((double)(fix.x + 5));         /*  85 */
    for(b = 0; b < nbands; b++) {
        for(r = r0; r < nrows; r++) {
            for(c = c0; c <= c1; c++) {
                if(b > fix.z + 10 && r < fix.y + 50)
                    continue;
                if(VPixel(mask_image, b, r, c, VBit) > 0)
                    continue;
                if(VPixel(dt_image, b, r, c, VFloat) > 2.0)
                    continue;
                VPixel(bin_image, b, r, c, VBit) = 0;
            }
        }
    }
    /* unten 1 */
    b0 = (int)VRint((double)(fix.z + 20));         /* 110 */
    r0 = (int)VRint((double)(fix.y - 20));         /*  75 */
    r1 = (int)VRint((double)(fix.y +  8));         /* 103 */
    c0 = (int)VRint((double)(fix.x - 10));         /*  70 */
    c1 = (int)VRint((double)(fix.x + 10));
    for(b = b0; b < nbands; b++) {
        for(r = r0; r < r1; r++) {
            for(c = c0; c <= c1; c++) {
                if(VPixel(mask_image, b, r, c, VBit) > 0)
                    continue;
                if(VPixel(dt_image, b, r, c, VFloat) > 3.0)
                    continue;
                VPixel(bin_image, b, r, c, VBit) = 0;
            }
        }
    }
    /* unten 2 */
    b0 = (int)VRint((double)(fix.z + 20));         /* 110 */
    c0 = (int)VRint((double)(fix.x - 40));         /*  40 */
    c1 = (int)VRint((double)(fix.x + 40));         /* 120 */
    for(b = b0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = c0; c <= c1; c++) {
                if(VPixel(tmp, b, r, c, VUByte) < 240)
                    continue;
                if(VPixel(dt_image, b, r, c, VFloat) > 2.0)
                    continue;
                VPixel(bin_image, b, r, c, VBit) = 0;
            }
        }
    }
    src_pp  = (VUByte *) VPixelPtr(src, 0, 0, 0);
    tmp_pp  = (VUByte *) VPixelPtr(tmp, 0, 0, 0);
    bin_pp  = (VBit *) VPixelPtr(bin_image, 0, 0, 0);
    float_pp = (VFloat *) VPixelPtr(dt_image, 0, 0, 0);
    for(i = 0; i < npixels; i++) {
        if(*src_pp < 20 && *float_pp < 2.5 && *bin_pp > 0) {
            *bin_pp = 0;
        }
        src_pp++;
        bin_pp++;
        float_pp++;
        tmp_pp++;
    }
    mask_image = VSmoothImage3d(bin_image, mask_image, 1, 2);
    /* restore */
    dest = VCreateImage(nbands, nrows, ncols, VUByteRepn);
    VFillImage(dest, VAllBands, 0);
    src_pp  = (VUByte *) VPixelPtr(src, 0, 0, 0);
    dest_pp = (VUByte *) VPixelPtr(dest, 0, 0, 0);
    bin_pp  = (VBit *) VPixelPtr(mask_image, 0, 0, 0);
    for(i = 0; i < npixels; i++) {
        if(*bin_pp == 1)
            *dest_pp = *src_pp;
        else
            *dest_pp = 0;
        src_pp++;
        dest_pp++;
        bin_pp++;
    }
    /* get extent */
    BoundingBox(dest, &or1, &or2, &extent);
    VCopyImageAttrs(src, dest);
    sprintf(info, "%g %g %g", or1.x, or1.y, or1.z);
    VSetAttr(VImageAttrList(dest), "origin", NULL, VStringRepn, info);
    sprintf(info, "x: %g %g, y: %g %g, z: %g %g",
            or1.x, or2.x, or1.y, or2.y, or1.z, or2.z);
    VSetAttr(VImageAttrList(dest), "boundingBox", NULL, VStringRepn, info);
    sprintf(info, "%g %g %g", extent.x, extent.y, extent.z);
    VSetAttr(VImageAttrList(dest), "extent", NULL, VStringRepn, info);
    return dest;
}
