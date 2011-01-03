/*
**
** get Talairach Coordinate
**
** G.Lohmann, April 2007
*/

#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


#define ABS(x) ((x) > 0 ? (x) : -(x))

extern void VPixel2Tal_Flt(float [3], float [3], float [3],
                           float, float, float, float *, float *, float *);

extern void VTal2Pixel_Flt(float [3], float [3], float [3],
                           float, float, float, float *, float *, float *);


void
VGetTalCoord(VImage src, float z0, float y0, float x0, float *u, float *v, float *w) {
    VString str1, str2, str3;
    float extent[3], voxel[3], ca[3];
    float x, y, z;
    *u = x0;
    *v = y0;
    *w = z0;
    if(VGetAttr(VImageAttrList(src), "extent", NULL,
                VStringRepn, (VPointer) & str1) == VAttrFound) {
        sscanf(str1, "%f %f %f", &x, &y, &z);
        extent[0] = x;
        extent[1] = y;
        extent[2] = z;
    } else
        return;
    if(VGetAttr(VImageAttrList(src), "voxel", NULL,
                VStringRepn, (VPointer) & str2) == VAttrFound) {
        sscanf(str2, "%f %f %f", &x, &y, &z);
        voxel[0] = x;
        voxel[1] = y;
        voxel[2] = z;
    } else
        return;
    if(VGetAttr(VImageAttrList(src), "ca", NULL,
                VStringRepn, (VPointer) & str3) == VAttrFound) {
        sscanf(str3, "%f %f %f", &x, &y, &z);
        ca[0] = x;
        ca[1] = y;
        ca[2] = z;
    } else
        return;
    VPixel2Tal_Flt(ca, voxel, extent, z0, y0, x0, u, v, w);
}



void
VGetVoxelCoord(VImage src, float x0, float y0, float z0, float *band, float *row, float *col) {
    VString str1, str2, str3;
    float extent[3], voxel[3], ca[3];
    float x, y, z;
    *band = z0;
    *row  = y0;
    *col  = x0;
    if(VGetAttr(VImageAttrList(src), "extent", NULL,
                VStringRepn, (VPointer) & str1) == VAttrFound) {
        sscanf(str1, "%f %f %f", &x, &y, &z);
        extent[0] = x;
        extent[1] = y;
        extent[2] = z;
    } else
        return;
    if(VGetAttr(VImageAttrList(src), "voxel", NULL,
                VStringRepn, (VPointer) & str2) == VAttrFound) {
        sscanf(str2, "%f %f %f", &x, &y, &z);
        voxel[0] = x;
        voxel[1] = y;
        voxel[2] = z;
    } else
        return;
    if(VGetAttr(VImageAttrList(src), "ca", NULL,
                VStringRepn, (VPointer) & str3) == VAttrFound) {
        sscanf(str3, "%f %f %f", &x, &y, &z);
        ca[0] = x;
        ca[1] = y;
        ca[2] = z;
    } else
        return;
    VTal2Pixel_Flt(ca, voxel, extent, x0, y0, z0, band, row, col);
}
