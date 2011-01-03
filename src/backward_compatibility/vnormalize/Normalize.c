/*
** do linear scaling
**
**  ca,cp,fixpoint are given in voxel coordinates,
**  'extent' and 'voxel' are given in millimeters.
**
** scaling sizes are taken from:
** J.L.Lancaster, et. al. Human Brain Mapping, 10:120-131, 2000.
**
** For MNI-brain, see:
** http://www.mrc-cbu.cam.ac.uk/Imaging/Common/mnispace.shtml
**
** G.Lohmann
*/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/os.h>
#include <via.h>

#include <stdio.h>
#include <math.h>



VImage
VNormalize(VImage src, VImage dest, VShort type) {
    int nbands_new = 118; /* see Lancaster et.al. */
    int nrows_new  = 172;
    int ncols_new  = 136;
    int ncols_old = 0, nrows_old = 0, nbands_old = 0;
    int nbands, nrows, ncols, i, j;
    float x, y, z;
    float fix_x = 0, fix_y = 0, fix_z = 0;
    float ca_x = 0, ca_y = 0, ca_z = 0;
    float cp_x = 0, cp_y = 0, cp_z = 0;
    float vox_x = 0, vox_y = 0, vox_z = 0;
    VString extent, str, str1;
    float zscale, yscale, xscale;
    float shift[3] = {0, 0, 0}, scale[3] = {0, 0, 0}, mat[3][3];
    nbands_new = 120;
    nrows_new  = 175;
    ncols_new  = 135;
    /* use MNI brain extent */
    if(type == 1) {
        nbands_new = 128;
        nrows_new  = 177;
        ncols_new  = 141;
    }
    if(VGetAttr(VImageAttrList(src), "extent", NULL,
                VStringRepn, (VPointer) & extent) == VAttrFound) {
        sscanf(extent, "%f %f %f", &x, &y, &z);
        ncols_old  = x;
        nrows_old  = y;
        nbands_old = z;
    } else {
        VError(" attribute 'extent' missing.\n");
    }
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    nbands = VImageNBands(src);
    zscale = (float)(nbands_new) / (float)(nbands_old);
    yscale = (float)(nrows_new)  / (float)(nrows_old);
    xscale = (float)(ncols_new)  / (float)(ncols_old);
    scale[0] = zscale;
    scale[1] = yscale;
    scale[2] = xscale;
    /*
    ** src is a transformation matrix
    */
    if(VImageNBands(src) < 2  && VImageNRows(src) < 4 && VImageNColumns(src) < 5) {
        for(i = 0; i < 3; i++) {
            shift[i] = VGetPixel(src, 0, i, 0);
            for(j = 0; j < 3; j++) {
                mat[i][j] = VGetPixel(src, 0, i, j + 1);
            }
        }
        for(i = 0; i < 3; i++) {
            for(j = 0; j < 3; j++) {
                mat[i][j] *= scale[j];
            }
        }
        shift[0] += (1.0 - scale[0]) * (float)nbands * 0.5;
        shift[1] += (1.0 - scale[1]) * (float)nrows * 0.5;
        shift[2] += (1.0 - scale[2]) * (float)ncols * 0.5;
        dest = VCopyImage(src, dest, VAllBands);
        for(i = 0; i < 3; i++) {
            VSetPixel(dest, 0, i, 0, (VDouble)shift[i]);
            for(j = 0; j < 3; j++) {
                VSetPixel(dest, 0, i, j + 1, (VDouble)mat[i][j]);
            }
        }
    }
    /*
    ** src is a raster image
    */
    else {
        shift[0] = (1.0 - scale[0]) * (float)nbands * 0.5;
        shift[1] = (1.0 - scale[1]) * (float)nrows * 0.5;
        shift[2] = (1.0 - scale[2]) * (float)ncols * 0.5;
        dest = VTriLinearScale3d(src, dest, nbands, nrows, ncols, shift, scale);
    }
    /*
    ** update attributes
    */
    VCopyImageAttrs(src, dest);
    /* update extent attribute */
    str1 = VMalloc(100);
    sprintf(str1, "%d %d %d", ncols_new, nrows_new, nbands_new);
    VSetAttr(VImageAttrList(dest), "extent", NULL, VStringRepn, str1);
    /* update voxel resolution attribute */
    if(VGetAttr(VImageAttrList(src), "voxel", NULL, VStringRepn, (VPointer) & str) == VAttrFound) {
        sscanf(str, "%f %f %f", &x, &y, &z);
        vox_x = x / xscale;
        vox_y = y / yscale;
        vox_z = z / zscale;
        sprintf(str1, "%.3f %.3f %.3f", vox_x, vox_y, vox_z);
        VSetAttr(VImageAttrList(dest), "scaled_voxel", NULL, VStringRepn, str1);
    } else
        VError("attribute 'voxel' missing");
    /* get CA/CP */
    if(VGetAttr(VImageAttrList(src), "ca", NULL,
                VStringRepn, (VPointer) & str) == VAttrFound) {
        sscanf(str, "%f %f %f", &ca_x, &ca_y, &ca_z);
    } else {
        VError(" attribute 'ca' missing.\n");
    }
    if(VGetAttr(VImageAttrList(src), "cp", NULL,
                VStringRepn, (VPointer) & str) == VAttrFound) {
        sscanf(str, "%f %f %f", &cp_x, &cp_y, &cp_z);
    } else {
        VError(" attribute 'cp' missing.\n");
    }
    /* update fixpoint position */
    if(VGetAttr(VImageAttrList(src), "fixpoint", NULL,
                VStringRepn, (VPointer) & str) == VAttrFound) {
        sscanf(str, "%f %f %f", &fix_x, &fix_y, &fix_z);
    } else {
        fix_x = ca_x;
        fix_y = (cp_y + ca_y) * 0.5;
        fix_z = cp_z;
    }
    /* update ca, ca is given in voxel coordinates, (0,0,0) is top left border */
    x = ca_x * xscale + fix_x * (1.0 - xscale);
    y = ca_y * yscale + fix_y * (1.0 - yscale);
    z = ca_z * zscale + fix_z * (1.0 - zscale);
    sprintf(str1, "%.3f %.3f %.3f", x, y, z);
    VSetAttr(VImageAttrList(dest), "ca", NULL, VStringRepn, str1);
    /* update cp, cp is given in voxel coordinates, (0,0,0) is top left border */
    x = cp_x * xscale + fix_x * (1.0 - xscale);
    y = cp_y * yscale + fix_y * (1.0 - yscale);
    z = cp_z * zscale + fix_z * (1.0 - zscale);
    sprintf(str1, "%.3f %.3f %.3f", x, y, z);
    VSetAttr(VImageAttrList(dest), "cp", NULL, VStringRepn, str1);
    return dest;
}
