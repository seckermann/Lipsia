/*
 *  Program: Dotrans
 *
 *  apply an affine transformation conatined in an transformation matrix.
 *
 *  G.Lohmann, <lohmann@cns.mpg.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <via.h>


#define AXIAL    0
#define CORONAL  1
#define SAGITTAL 2

extern VImage VAxial2Sagittal(VImage, VImage);
extern VImage VAxial2Coronal(VImage, VImage);
extern VImage VShuffleSlices(VImage, VImage);


VImage
VDoTrans(VImage src, VImage dest, VImage transform, VFloat resolution, VLong type) {
    VImage src1 = NULL, dest_cor = NULL, dest_sag = NULL, trans = NULL;
    int   i, nbands1 = 0, nrows1 = 0, ncols1 = 0;
    int   xdim, ydim, zdim;
    int   orient = AXIAL;
    float b0, r0, c0;
    float scaleb, scaler, scalec;
    float scale[3], shift[3];
    VString str, str1, str2, str3, str4, str5, str6;
    VShort buf;
    VDouble u;
    float transResX = 1.0, transResY = 1.0, transResZ = 1.0;
    VString _transResString;
    if(VGetAttr(VImageAttrList(transform), "voxel", NULL, VStringRepn, (VPointer) &_transResString) == VAttrFound) {
        sscanf(_transResString, "%f %f %f", &transResX, &transResY, &transResZ);
    }
    /*
    ** get matrix size
    */
    xdim = 160;
    ydim = 200;
    zdim = 160;
    if(VGetAttr(VImageAttrList(transform), "zdim", NULL,
                VShortRepn, (VPointer) & buf) == VAttrFound) {
        zdim = buf;
    }
    if(VGetAttr(VImageAttrList(transform), "ydim", NULL,
                VShortRepn, (VPointer) & buf) == VAttrFound) {
        ydim = buf;
    }
    if(VGetAttr(VImageAttrList(transform), "xdim", NULL,
                VShortRepn, (VPointer) & buf) == VAttrFound) {
        xdim = buf;
    }
    /*
    ** get slice orientation
    */
    if(VGetAttr(VImageAttrList(src), "orientation", NULL,
                VStringRepn, (VPointer) & str) != VAttrFound)
        VError(" attribute 'orientation' missing in input file.");
    if(strcmp(str, "coronal") == 0) {
        orient  = CORONAL;
        nbands1 = ydim;
        nrows1  = zdim;
        ncols1  = xdim;
    } else if(strcmp(str, "axial") == 0) {
        orient  = AXIAL;
        nbands1 = zdim;
        nrows1  = ydim;
        ncols1  = xdim;
    } else if(strcmp(str, "sagittal") == 0) {
        orient  = SAGITTAL;
        nbands1 = zdim;
        nrows1  = xdim;
        ncols1  = ydim;
    } else
        VError("orientation must be axial or coronal");
    nbands1 /= (resolution / transResX);
    nrows1  /= (resolution / transResY);
    ncols1  /= (resolution / transResZ);
    /*
    ** scale to size
    */
    if(VGetAttr(VImageAttrList(src), "voxel", NULL,
                VStringRepn, (VPointer) & str) != VAttrFound)
        VError(" attribute 'voxel' missing in input file.");
    sscanf(str, "%f %f %f", &scalec, &scaler, &scaleb);
    scaleb /= (resolution / transResX);
    scaler /= (resolution / transResY);
    scalec /= (resolution / transResZ);
    scale[0] = scaleb;
    scale[1] = scaler;
    scale[2] = scalec;
    shift[0] = 0;
    shift[1] = (float)nrows1 * 0.5 - scaler * (float)VImageNRows(src) * 0.5;
    shift[2] = (float)ncols1 * 0.5 - scalec * (float)VImageNColumns(src) * 0.5;
    src1 = VShuffleSlices(src, NULL);
    switch(type) {
    case 0:
        src  = VTriLinearScale3d(src1, NULL, nbands1, nrows1, ncols1, shift, scale);
        break;
    case 1:
        src  = VNNScale3d(src1, NULL, nbands1, nrows1, ncols1, shift, scale);
        break;
    default:
        VError(" illegal resampling type");
    }
    /*
    ** resample image
    */
    trans = VCopyImage(transform, NULL, VAllBands);
    for(i = 0; i < 3; i++) {
        u = VPixel(trans, 0, i, 0, VDouble);
        u /= resolution;
        VPixel(trans, 0, i, 0, VDouble) = u;
    }
    b0 = (float)nbands1 * 0.5;
    r0 = (float)nrows1  * 0.5;
    c0 = (float)ncols1  * 0.5;
    switch(type) {
    case 0:
        dest = VTriLinearSample3d(src, dest, trans, b0, r0, c0, nbands1, nrows1, ncols1);
        break;
    case 1:
        dest = VNNSample3d(src, dest, trans, b0, r0, c0, nbands1, nrows1, ncols1);
        break;
    }
    /*
    ** update header
    */
    float _cax, _cay, _caz, _cpx, _cpy, _cpz;
    float _fixpointx, _fixpointy, _fixpointz;
    str1 = str2 = str3 = str4 = NULL;
    str1 = (VString) VMalloc(80);
    sprintf(str1, "%.2f %.2f %.2f", resolution, resolution, resolution);
    if(VGetAttr(VImageAttrList(trans), "ca", NULL,
                VStringRepn, (VPointer) & str2) != VAttrFound) {
        str2 = NULL;
    } else {
        sscanf(str2, "%f %f %f", &_cax, &_cay, &_caz);
        _cax /= (resolution / transResX);
        _cay /= (resolution / transResY);
        _caz /= (resolution / transResZ);
        sprintf(str2, "%.2f %.2f %.2f", _cax, _cay, _caz);
    }
    if(VGetAttr(VImageAttrList(trans), "cp", NULL,
                VStringRepn, (VPointer) & str3) != VAttrFound) {
        str3 = NULL;
    } else {
        sscanf(str3, "%f %f %f", &_cpx, &_cpy, &_cpz);
        _cpx /= (resolution / transResX);
        _cpy /= (resolution / transResY);
        _cpz /= (resolution / transResZ);
        sprintf(str3, "%.2f %.2f %.2f", _cpx, _cpy, _cpz);
    }
    if(VGetAttr(VImageAttrList(trans), "extent", NULL,
                VStringRepn, (VPointer) & str4) != VAttrFound)
        str4 = NULL;
    if(VGetAttr(VImageAttrList(trans), "fixpoint", NULL,
                VStringRepn, (VPointer) & str5) != VAttrFound) {
        str5 = NULL;
    } else {
        sscanf(str5, "%f %f %f", &_fixpointx, &_fixpointy, &_fixpointz);
        _fixpointx /= (resolution / transResX);
        _fixpointy /= (resolution / transResY);
        _fixpointz /= (resolution / transResZ);
        sprintf(str5, "%.2f %.2f %.2f", _fixpointx, _fixpointy, _fixpointz);
    }
    if(VGetAttr(VImageAttrList(trans), "talairach", NULL,
                VStringRepn, (VPointer) & str6) != VAttrFound) {
        str6 = NULL;
    }
    if(orient == CORONAL) {
        dest_cor = VAxial2Coronal(dest, NULL);
        VDestroyImage(dest);
        VCopyImageAttrs(src, dest_cor);
        VSetAttr(VImageAttrList(dest_cor), "voxel", NULL, VStringRepn, str1);
        if(str2)
            VSetAttr(VImageAttrList(dest_cor), "ca", NULL, VStringRepn, str2);
        if(str3)
            VSetAttr(VImageAttrList(dest_cor), "cp", NULL, VStringRepn, str3);
        if(str4)
            VSetAttr(VImageAttrList(dest_cor), "extent", NULL, VStringRepn, str4);
        if(str5)
            VSetAttr(VImageAttrList(dest_cor), "fixpoint", NULL, VStringRepn, str5);
        if(str6)
            VSetAttr(VImageAttrList(dest_cor), "talairach", NULL, VStringRepn, str6);
        return dest_cor;
    } else if(orient == SAGITTAL) {
        dest_sag = VAxial2Sagittal(dest, NULL);
        VDestroyImage(dest);
        VCopyImageAttrs(src, dest_sag);
        VSetAttr(VImageAttrList(dest_sag), "voxel", NULL, VStringRepn, str1);
        if(str2)
            VSetAttr(VImageAttrList(dest_sag), "ca", NULL, VStringRepn, str2);
        if(str3)
            VSetAttr(VImageAttrList(dest_sag), "cp", NULL, VStringRepn, str3);
        if(str4)
            VSetAttr(VImageAttrList(dest_sag), "extent", NULL, VStringRepn, str4);
        if(str5)
            VSetAttr(VImageAttrList(dest_sag), "fixpoint", NULL, VStringRepn, str5);
        if(str6)
            VSetAttr(VImageAttrList(dest_sag), "talairach", NULL, VStringRepn, str6);
        return dest_sag;
    } else {
        VCopyImageAttrs(src, dest);
        VSetAttr(VImageAttrList(dest), "voxel", NULL, VStringRepn, str1);
        if(str2)
            VSetAttr(VImageAttrList(dest), "ca", NULL, VStringRepn, str2);
        if(str3)
            VSetAttr(VImageAttrList(dest), "cp", NULL, VStringRepn, str3);
        if(str4)
            VSetAttr(VImageAttrList(dest), "extent", NULL, VStringRepn, str4);
        if(str5)
            VSetAttr(VImageAttrList(dest), "fixpoint", NULL, VStringRepn, str5);
        if(str6)
            VSetAttr(VImageAttrList(dest), "talairach", NULL, VStringRepn, str6);
        return dest;
    }
}
