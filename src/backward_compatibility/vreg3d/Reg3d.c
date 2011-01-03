/****************************************************************
 *
 * vreg3d:
 * register a 2D slice to a 3D volume
 *
 * MPI of Cognitive Neuroscience, Leipzig
 *
 * Author G.Lohmann, Apr. 1998, <lohmann@cns.mpg.de>
 *
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <via.h>

extern VImage VGetTrans(VImage, VImage, int, int, float, float, float, float [3], float, float, VShort, VShort);
extern VImage IniShift(VImage, VImage, int, float [], int, int);


VImage
VReg3d(VImage ref, VImage src, int band1, int band2, float pitch,
       VShort scanwidth, VShort type) {
    VImage transform = NULL, src1 = NULL, src2 = NULL, tmp = NULL;
    int nbands2, nbands1, nrows1, ncols1;
    int i, j, k;
    float offset;
    float scaleb, scaler, scalec;
    float roll = 0, yaw = 0;
    float shift[3], shiftcorr = 0;
    float shift2d[2], scale2d[2];
    VString str;
    int  minval =  0; /* image background */
    int  maxval = 256; /* image background */
    VUByte *src_pp, *tmp_pp;
    /*
    ** get image background and foreground
    */
    tmp = VIsodataImage3d(src, NULL, 2L, 0L);
    src_pp = VImageData(src);
    tmp_pp = VImageData(tmp);
    minval = 256;
    maxval = 0;
    for(i = 0; i < VImageNPixels(src); i++) {
        j = *src_pp++;
        k = *tmp_pp++;
        if(k > 0) {
            if(j < minval)
                minval = j;
            if(j > maxval)
                maxval = j;
        }
    }
    VDestroyImage(tmp);
    /*
    ** scale image
    */
    nbands2 = VImageNBands(src);
    nbands1 = VImageNBands(ref);
    nrows1  = VImageNRows(ref);
    ncols1  = VImageNColumns(ref);
    if(VGetAttr(VImageAttrList(src), "voxel", NULL, VStringRepn, (VPointer) & str) != VAttrFound)
        VError(" attribute 'voxel' missing in input file.");
    sscanf(str, "%f %f %f", &scalec, &scaler, &scaleb);
    offset = scaleb;
    shiftcorr = (nbands2 - 1 - band2) * offset;
    shift2d[0] = (float)nrows1 * 0.5 - scaler * (float)VImageNRows(src) * 0.5;
    shift2d[1] = (float)ncols1 * 0.5 - scalec * (float)VImageNColumns(src) * 0.5;
    scale2d[0] = scaler;
    scale2d[1] = scalec;
    src1 = VBiLinearScale2d(src, NULL, nrows1, ncols1, shift2d, scale2d);
    for(i = 0; i < 3; i++)
        shift[i] = 0;
    src2 = IniShift(ref, src1, minval, shift, band1, band2);
    transform = VGetTrans(ref, src2, minval, maxval, pitch, roll, yaw, shift, shiftcorr,
                          offset, scanwidth, type);
    return transform;
}

