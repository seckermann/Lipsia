/****************************************************************
 *
 * vfunctrans:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 1999, <lipsia@cbs.mpg.de>
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
 * $Id: Functrans.c 3210 2008-04-03 15:04:02Z karstenm $
 *
 *****************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/VImage.h>
#include <viaio/headerinfo.h>


extern VBoolean VReadBandDataFD(int, VImageInfo *, int, int, VImage *);
extern VImage VFuncCompress(VImage, VShort);
extern VImage VTransTimeStep(VImage, VImage, VImage, VImage, VImage, int, int, int, int,
                             VFloat, float, float, float);


/* standard brain image matrix dimensions */
#define XNBANDS   160
#define XNROWS    200
#define XNCOLUMNS 160


#define AXIAL    0
#define CORONAL  1
#define SAGITTAL 2



/*
** loop thru timesteps
*/
VImage *
VFunctrans(VStringConst in_filename, VImageInfo *imageInfo, int nslices, VImage trans,
           VFloat resolution, int ntimesteps, VShort minval, VBoolean compress, int *dst_nslices) {
    VImage *dst_image = NULL;
    VImage tmp = NULL, dest = NULL, src_tmp = NULL;
    VImage *image, dest_tmp = NULL;
    char *str;
    VString str1, newstr, extent, convention = NULL, talairach = NULL;
    int i, j, n, b, nb, npix;
    int nbands, nrows, ncols;
    int nbands1 = 0, nrows1 = 0, ncols1 = 0;
    int dest_nbands, dest_nrows, dest_ncols;
    VShort *short1_pp, *short2_pp, ibuf;
    VBand *src_bands;
    float scaleb, scaler, scalec, x, y, z;
    float fix_c = 0, fix_r = 0, fix_b = 0;
    float ca_x = 0, ca_y = 0, ca_z = 0, cp_x = 0, cp_y = 0, cp_z = 0;
    int orient = AXIAL;
    int fd; /* file descriptor */
    float xnbands, xnrows, xncols;
    /*
    ** adjust transformation matrix to output resolution
    */
    for(i = 0; i < 3; i++) {
        x = VGetPixel(trans, 0, i, 0);
        x /= (float)resolution;
        VSetPixel(trans, 0, i, 0, (VDouble) x);
    }
    /*
    ** get input image dimensions
    */
    nbands     = nslices;
    nrows      = imageInfo[0].nrows;
    ncols      = imageInfo[0].ncolumns;
    ntimesteps = imageInfo[0].nbands;
    /*
    ** input voxel size
    */
    str = (char *) VMalloc(80);
    strcpy(str, imageInfo[0].voxel);
    sscanf(str, "%f %f %f", &scalec, &scaler, &scaleb);
    /*
    ** get slice orientation
    */
    if(VGetAttr(VImageAttrList(trans), "orientation", NULL,
                VStringRepn, (VPointer) & str1) != VAttrFound)
        VError(" attribute 'orientation' missing in transformation matrix ");
    if(strcmp(str1, "coronal")  == 0)
        orient = CORONAL;
    else if(strcmp(str1, "axial")    == 0)
        orient = AXIAL;
    else if(strcmp(str1, "sagittal") == 0)
        orient = SAGITTAL;
    else
        VError("orientation must be axial or coronal");
    float transResX = 1.0, transResY = 1.0, transResZ = 1.0;
    VString _transResString;
    if(VGetAttr(VImageAttrList(trans), "voxel", NULL, VStringRepn, (VPointer) &_transResString) == VAttrFound) {
        sscanf(_transResString, "%f %f %f", &transResX, &transResY, &transResZ);
    }
    /*
    ** allocate output images
    */
    xnbands = XNBANDS;
    xnrows  = XNROWS;
    xncols  = XNCOLUMNS;
    if(VGetAttr(VImageAttrList(trans), "zdim", NULL,
                VShortRepn, (VPointer) & ibuf) == VAttrFound) {
        xnbands = ibuf;
    }
    if(VGetAttr(VImageAttrList(trans), "ydim", NULL,
                VShortRepn, (VPointer) & ibuf) == VAttrFound) {
        xnrows = ibuf;
    }
    if(VGetAttr(VImageAttrList(trans), "xdim", NULL,
                VShortRepn, (VPointer) & ibuf) == VAttrFound) {
        xncols = ibuf;
    }
    dest_nbands = (int)(xnbands / (resolution / transResX));
    dest_nrows  = (int)(xnrows  / (resolution / transResY));
    dest_ncols  = (int)(xncols  / (resolution / transResZ));
    /*
    ** update fixpoint coordinate
    */
    if(VGetAttr(VImageAttrList(trans), "fixpoint", NULL,
                VStringRepn, (VPointer) & str1) == VAttrFound) {
        sscanf(str1, "%f %f %f", &fix_c, &fix_r, &fix_b);
    } else {
        fix_c = 80;
        fix_r = 95;
        fix_b = 90;
    }
    fix_c /= (resolution / transResX);
    fix_r /= (resolution / transResY);
    fix_b /= (resolution / transResZ);
    /*
    ** update ca,cp coordinates
    */
    if(VGetAttr(VImageAttrList(trans), "ca", NULL,
                VStringRepn, (VPointer) & str1) == VAttrFound) {
        sscanf(str1, "%f %f %f", &x, &y, &z);
    } else
        VError(" parameter 'ca' missing in transformation matrix ");
    ca_x = x / (resolution / transResX);
    ca_y = y / (resolution / transResY);
    ca_z = z / (resolution / transResZ);
    if(VGetAttr(VImageAttrList(trans), "cp", NULL,
                VStringRepn, (VPointer) & str1) == VAttrFound) {
        sscanf(str1, "%f %f %f", &x, &y, &z);
    } else
        VError(" parameter 'cp' missing in transformation matrix ");
    cp_x = x / (resolution / transResX);
    cp_y = y / (resolution / transResY);
    cp_z = z / (resolution / transResZ);
    if(VGetAttr(VImageAttrList(trans), "extent", NULL,
                VStringRepn, (VPointer) & extent) != VAttrFound) {
        VError(" parameter 'extent' missing in transformation matrix ");
    }
    if(VGetAttr(VImageAttrList(trans), "convention", NULL,
                VStringRepn, (VPointer) & convention) != VAttrFound) {
        VError(" parameter 'convention' missing in transformation matrix ");
    }
    if(VGetAttr(VImageAttrList(trans), "talairach", NULL,
                VStringRepn, (VPointer) & talairach) != VAttrFound) {
        VError(" parameter 'talairach' missing in transformation matrix ");
    }
    /* create destination images */
    newstr = (VString) VMalloc(60);
    dst_image = (VImage *) VMalloc(sizeof(VImage *) * dest_nbands);
    /* create temporal images */
    switch(orient) {
    case AXIAL:
        nbands1 = dest_nbands;
        nrows1  = dest_nrows;
        ncols1  = dest_ncols;
        dest_tmp = NULL;
        break;
    case CORONAL:
        nbands1 = dest_nrows;
        nrows1  = dest_nbands;
        ncols1  = dest_ncols;
        dest_tmp = VCreateImage(nbands1, nrows1, ncols1, VShortRepn);
        break;
    case SAGITTAL:
        nbands1 = dest_nbands;
        nrows1  = dest_ncols;
        ncols1  = dest_nrows;
        dest_tmp = VCreateImage(nbands1, nrows1, ncols1, VShortRepn);
        break;
    }
    src_tmp = VCreateImage(nbands1, nrows1, ncols1, VShortRepn);
    tmp     = VCreateImage(nbands, nrows, ncols, VShortRepn);
    dest    = VCreateImage(dest_nbands, dest_nrows, dest_ncols, VShortRepn);
    /*
    ** loop thru time steps
    */
    src_bands = (VBand *) VMalloc(sizeof(VBand) * nbands);
    image = (VImage *) VMalloc(sizeof(VImage) * nbands);
    for(j = 0; j < nbands; j++)
        image[j] = VCreateImage(1, nrows, ncols, VShortRepn);
    for(j = 0; j < nbands; j++)
        src_bands[j] = VAllBands;
    fd = open(in_filename, O_RDONLY);
    if(fd == -1)
        VError("could not open infile %s", in_filename);
    for(i = 0; i < ntimesteps; i++) {
        if(i % 50 == 0)
            fprintf(stderr, "Processing %4d of %4d\r", i, ntimesteps - 1);
        /* construct 3D image */
        nb = 1;
        for(j = 0; j < nbands; j++) {
            if(! VReadBandDataFD(fd, &imageInfo[j], i, nb, &image[nbands - j - 1]))
                VError(" error reading data");
        }
        for(j = 0; j < nbands; j++)
            src_bands[j] = 0;
        tmp = VCombineBands(nbands, image, src_bands, tmp);
        /* do transformation */
        if(orient == AXIAL) {
            dest = VTransTimeStep(tmp, src_tmp, dest, dest_tmp, trans, nbands1, nrows1, ncols1,
                                  orient, resolution, scaleb, scaler, scalec);
        } else {
            dest = VTransTimeStep(tmp, src_tmp, dest_tmp, dest, trans, nbands1, nrows1, ncols1,
                                  orient, resolution, scaleb, scaler, scalec);
        }
        /* copy to dest image */
        npix = dest_nrows * dest_ncols;
        for(b = 0; b < dest_nbands; b++) {
            /* test the first time step: is slice empty ? */
            if(i == 0) {
                short1_pp = (VShort *) VPixelPtr(dest, b, 0, 0);
                n = 0;
                for(j = 0; j < npix; j++)
                    if(*short1_pp++ > minval)
                        n++;
                if(n > 0 || !compress) {    /* a non-empty slice */
                    dst_image[b] = VCreateImage(ntimesteps, dest_nrows, dest_ncols, VShortRepn);
                } else { /* an empty slice */
                    dst_image[b] = VCreateImage(1, 1, 1, VShortRepn);
                }
                VFillImage(dst_image[b], VAllBands, 0);
                /* Don't use the attributes from the trans matrix !!! */
                /* VCopyImageAttrs (trans, dst_image[b]); */
                VSetAttr(VImageAttrList(dst_image[b]), "bandtype", NULL, VStringRepn, "temporal");
                VSetAttr(VImageAttrList(dst_image[b]), "orientation", NULL, VStringRepn, "axial");
                VSetAttr(VImageAttrList(dst_image[b]), "convention", NULL, VStringRepn, (VString)convention);
                VSetAttr(VImageAttrList(dst_image[b]), "talairach", NULL, VStringRepn, (VString)talairach);
                sprintf(newstr, "%.3f %.3f %.3f", resolution, resolution, resolution);
                VSetAttr(VImageAttrList(dst_image[b]), "voxel", NULL, VStringRepn, (VString)newstr);
                sprintf(newstr, "%.3f %.3f %.3f", fix_c, fix_r, fix_b);
                VSetAttr(VImageAttrList(dst_image[b]), "fixpoint", NULL, VStringRepn, (VString)newstr);
                sprintf(newstr, "%.3f %.3f %.3f", ca_x, ca_y, ca_z);
                VSetAttr(VImageAttrList(dst_image[b]), "ca", NULL, VStringRepn, (VString)newstr);
                sprintf(newstr, "%.3f %.3f %.3f", cp_x, cp_y, cp_z);
                VSetAttr(VImageAttrList(dst_image[b]), "cp", NULL, VStringRepn, (VString)newstr);
                VSetAttr(VImageAttrList(dst_image[b]), "extent", NULL, VStringRepn, (VString)extent);
                if(imageInfo->MPIL_vista_0)
                    VSetAttr(VImageAttrList(dst_image[b]), "MPIL_vista_0", NULL, VStringRepn, imageInfo->MPIL_vista_0);
                if(imageInfo->modality)
                    VSetAttr(VImageAttrList(dst_image[b]), "modality", NULL, VStringRepn, imageInfo->modality);
                if(imageInfo->name)
                    VSetAttr(VImageAttrList(dst_image[b]), "name", NULL, VStringRepn, imageInfo->name);
                if(imageInfo->angle)
                    VSetAttr(VImageAttrList(dst_image[b]), "angle", NULL, VStringRepn, imageInfo->angle);
                if(imageInfo->patient)
                    VSetAttr(VImageAttrList(dst_image[b]), "patient", NULL, VStringRepn, imageInfo->patient);
                if(imageInfo->ntimesteps)
                    VSetAttr(VImageAttrList(dst_image[b]), "ntimesteps", NULL, VLongRepn, imageInfo->ntimesteps);
                if(imageInfo->repetition_time)
                    VSetAttr(VImageAttrList(dst_image[b]), "repetition_time", NULL, VLongRepn, imageInfo->repetition_time);
                if(b < 10)
                    sprintf(newstr, "MR 01-0%d func", b);
                else
                    sprintf(newstr, "MR 01-%d func", b);
                VSetAttr(VImageAttrList(dst_image[b]), "name", NULL, VStringRepn, (VString)newstr);
                if(VImageNRows(dst_image[b]) < 2) {
                    VSetAttr(VImageAttrList(dst_image[b]), "ori_nrows", NULL, VShortRepn, dest_nrows);
                    VSetAttr(VImageAttrList(dst_image[b]), "ori_ncolumns", NULL, VShortRepn, dest_ncols);
                }
            }
            if(VImageNRows(dst_image[b]) > 2) {
                short1_pp = (VShort *) VPixelPtr(dest, b, 0, 0);
                short2_pp = (VShort *) VPixelPtr(dst_image[b], i, 0, 0);
                for(j = 0; j < npix; j++) {
                    *short2_pp++ = *short1_pp++;
                }
            }
        }
    }
    close(fd);
    VDestroyImage(tmp);
    VDestroyImage(dest);
    VDestroyImage(src_tmp);
    for(j = 0; j < nbands; j++)
        VDestroyImage(image[j]);
    *dst_nslices = dest_nbands;
    return dst_image;
}

