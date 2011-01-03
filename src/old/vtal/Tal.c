/****************************************************************
 *
 * vtal: geometric rotation into Talairach coordinate system
 *       and brain peeling.
 *
 *
 *     A predecessor of this program was called 'vcoord'.
 *     'vtal' differs from 'vcoord' in the following aspects:
 *
 *     point 'oc' differently defined.
 *     automatic CA/CP-detection removed.
 *     new switch: peeling can be switched off
 *     new computation of extent
 *     coordinates of bounding Box is displayed.
 *     3point formula for mid-sagittal plane definition added
 *     new peeling procedure
 *
 * Copyright (C) 1999 MPI of Cognitive Neuroscience, Leipzig
 *
 * Author Gaby Lohmann, Feb/Mar. 1999, <lohmann@cns.mpg.de>
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
 *****************************************************************/

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/mu.h"
#include "via.h"


/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


typedef struct DpointStruct {
    double x;
    double y;
    double z;
} DPoint;

#define PI 3.14159265

extern VImage VPeel(VImage, VImage, VDouble, VDouble, VDouble);


DPoint
normalize(DPoint u) {
    DPoint v;
    double norm;
    v.x = v.y = v.z = 0;
    norm = sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
    if(norm != 0) {
        v.x = u.x / norm;
        v.y = u.y / norm;
        v.z = u.z / norm;
    }
    return v;
}


DPoint
cross(DPoint u, DPoint v) {
    DPoint w;
    w.x = u.y * v.z  -  u.z * v.y;
    w.y = u.z * v.x  -  u.x * v.z;
    w.z = u.x * v.y  -  u.y * v.x;
    return w;
}


DPoint
minus(DPoint u, DPoint v) {
    DPoint w;
    w.x = u.x - v.x;
    w.y = u.y - v.y;
    w.z = u.z - v.z;
    return w;
}




/*
** compute rotation matrix
*/
void
rotation_matrix(double roll, double pitch, double yaw, double rot[3][3]) {
    double cr, cy, cp, sr, sy, sp;
    cr = cos(roll);
    cp = cos(pitch);
    cy = cos(yaw);
    sr = sin(roll);
    sp = sin(pitch);
    sy = sin(yaw);
    rot[0][0] = cr * cy + sr * sp * sy;
    rot[0][1] = sr * cp;
    rot[0][2] = sr * sp * cy - sy * cr;
    rot[1][0] = cr * sp * sy - sr * cy;
    rot[1][1] = cr * cp;
    rot[1][2] = sr * sy + cy * cr * sp;
    rot[2][0] = cp * sy;
    rot[2][1] = - sp;
    rot[2][2] = cp * cy;
}


void
MatMult(double rot[3][3], double x[3], double y[3]) {
    int i, j, dim = 3;
    double sum;
    for(i = 0; i < dim; i++) {
        sum = 0;
        for(j = 0; j < dim; j++)
            sum += rot[i][j] * x[j];
        y[i] = sum;
    }
}



VImage
VTalairach(VImage src, VDouble d1, VDouble d2, VDouble t,
           DPoint ca, DPoint cp, DPoint p, DPoint angle,
           VLong type, VLong talairach, DPoint ext_manual, VLong dstsize) {
    VImage transform = NULL, dst = NULL;
    DPoint oc;
    DPoint normal;
    VImage peel = NULL;
    DPoint uu, vv;
    int nbands, nrows, ncols;
    double b0, r0, c0;
    double rot[3][3];
    double roll = 0, pitch = 0, yaw = 0, shift[3], fix[3];
    double x[3], y[3];
    int i, j;
    char info[128];
    VString voxel;
    float vox_x, vox_y, vox_z;
    vox_x = vox_y = vox_z = 1;
    if(VGetAttr(VImageAttrList(src), "voxel", NULL,
                VStringRepn, (VPointer) & voxel) == VAttrFound) {
        sscanf(voxel, "%f %f %f", &vox_x, &vox_y, &vox_z);
        fprintf(stderr, " voxel: %f  %f  %f\n", vox_x, vox_y, vox_z);
    }
    nbands = 160;  /* image size */
    nrows  = 200;
    ncols  = 160;
    /* fixed point */
    c0 = 80;
    r0 = 95;
    b0 = 90;
    if(dstsize == 1) {    /* large matrix */
        nbands = 160 + 20;
        nrows  = 200 + 30;
        ncols  = 160 + 30;
        c0 = (float)ncols * 0.5;
        r0 = (float)nrows * 0.5;
        b0 += 15;
    }
    if(dstsize == 2) {    /* very large matrix (xxl) */
        nbands = 210;
        nrows  = 240;
        ncols  = 200;
        b0 = 110;
        r0 = 120;
        c0 = 100;
    }
    if(dstsize == 3) {    /* huge matrix */
        nbands = (int)(180.0 / vox_z);
        nrows  = (int)(240.0 / vox_y);
        ncols  = (int)(180.0 / vox_z);
        if(nbands % 10 > 0)
            nbands += 10 - (nbands % 10);
        if(nrows % 10  > 0)
            nrows  += 10 - (nrows % 10);
        if(ncols % 10  > 0)
            ncols  += 10 - (ncols % 10);
        c0 = (float)ncols * 0.5;
        r0 = (float)nrows * 0.5;
        b0 = (float)nbands * 0.5 + 15 * vox_z;
        fprintf(stderr, " output image dimensions: %3d %3d %3d\n", nbands, nrows, ncols);
    }
    /* object center, point on mid-sagittal plane */
    oc.x = (ca.x - cp.x) * 0.5 + cp.x;
    oc.y = (ca.y - cp.y) * 0.5 + cp.y;
    oc.z = (ca.z - cp.z) * 0.5 + cp.z;
    shift[0] = oc.z;
    shift[1] = oc.y;
    shift[2] = oc.x;
    fix[0] = b0;
    fix[1] = r0;
    fix[2] = c0;
    roll = pitch = yaw = 0;
    /* roll:  nicken (ja)
    ** pitch: schuetteln (nein)
    ** yaw:   neigen (Ohr), nach links > 0; nach rechts < 0
    */
    /* 3 point form of plane */
    if(p.x > 0 && p.y > 0 && p.z > 0) {
        uu = minus(cp, ca);
        uu = normalize(uu);
        vv = minus(cp, p);
        vv = normalize(vv);
        normal = cross(uu, vv);
        normal = normalize(normal);
        uu    = minus(cp, ca);
        uu.x  = 0;
        uu    = normalize(uu);
        roll  = acos((double)uu.z) - PI * 0.5;
        uu    = minus(cp, ca);
        uu.z  = 0;
        uu    = normalize(uu);
        pitch = PI * 0.5 - acos((double)uu.x);
        vv    = minus(cp, p);
        vv.z  = 0;
        vv    = normalize(vv);
        yaw   = acos((double)vv.x) - PI * 0.5;
        rotation_matrix(roll, pitch, yaw, rot);
    }
    /* compute midsagittal plane  */
    if(p.x < 1 || p.y < 1 || p.z < 1) {
        uu    = minus(cp, ca);
        uu.x  = 0;
        uu    = normalize(uu);
        roll  = acos((double)uu.z) - PI * 0.5;
        uu    = minus(cp, ca);
        uu.z  = 0;
        uu    = normalize(uu);
        pitch = PI * 0.5 - acos((double)uu.x);
        pitch =  angle.z * PI / 180.0; /* geaendert, 8.2.2005, GL */
        yaw   = -angle.y * PI / 180.0;
        rotation_matrix(roll, pitch, yaw, rot);
    }
    /*
    ** apply geometrical transformation, trilinear resampling
    */
    transform = VCreateImage(1, 3, 4, VDoubleRepn);
    VFillImage(transform, VAllBands, 0);
    if(transform == NULL)
        return NULL;
    for(i = 0; i < 3; i++) {
        VPixel(transform, 0, i, 0, VDouble) = fix[i];
        for(j = 0; j < 3; j++) {
            VPixel(transform, 0, i, j + 1, VDouble) = rot[i][j];
        }
    }
    b0 = shift[0];
    r0 = shift[1];
    c0 = shift[2];
    dst = VTriLinearSample3d(src, NULL, transform, b0, r0, c0, nbands, nrows, ncols);
    /*
    ** set header entries
    */
    VCopyImageAttrs(src, dst);
    /* rotate CA/CP coordinate */
    x[0] = ca.z - shift[0];
    x[1] = ca.y - shift[1];
    x[2] = ca.x - shift[2];
    MatMult(rot, x, y);
    ca.z = y[0] + fix[0];
    ca.y = y[1] + fix[1];
    ca.x = y[2] + fix[2];
    x[0] = cp.z - shift[0];
    x[1] = cp.y - shift[1];
    x[2] = cp.x - shift[2];
    MatMult(rot, x, y);
    cp.z = y[0] + fix[0];
    cp.y = y[1] + fix[1];
    cp.x = y[2] + fix[2];
    sprintf(info, "%.3f %.3f %.3f", ca.x, ca.y, ca.z);
    VSetAttr(VImageAttrList(dst), "ca", NULL, VStringRepn, info);
    sprintf(info, "%.3f %.3f %.3f", cp.x, cp.y, cp.z);
    VSetAttr(VImageAttrList(dst), "cp", NULL, VStringRepn, info);
    /* set other header entries */
    if(talairach == 0)
        VSetAttr(VImageAttrList(dst), "talairach", 0, VStringRepn, "atlas");
    else if(talairach == 1)
        VSetAttr(VImageAttrList(dst), "talairach", 0, VStringRepn, "neurological");
    VSetAttr(VImageAttrList(dst), "convention", 0, VStringRepn, "natural");
    VSetAttr(VImageAttrList(dst), "location", 0, VStringRepn, "0");
    sprintf(info, "%G %G %G", fix[2], fix[1], fix[0]);
    VSetAttr(VImageAttrList(dst), "fixpoint", 0, VStringRepn, info);
    /*
    ** brain peeling
    */
    if(type == 0 || type == 1) {
        peel = VPeel(dst, NULL, d1, d2, t);
        if(type == 0) {     /* output peeled brain */
            /* dst = VCopyImage(peel,dst,VAllBands); */
            dst = peel;
        }
        if(type == 1)    /* output unpeeled brain with extent information */
            VCopyImageAttrs(peel, dst);
    }
    /*
    ** header entries
    */
    if(ext_manual.x > 1 && ext_manual.y > 1) {
        sprintf(info, "%G %G %G", ext_manual.x, ext_manual.y, ext_manual.z);
        fprintf(stderr, " extent= %s\n", info);
        VSetAttr(VImageAttrList(dst), "extent", 0, VStringRepn, info);
    }
    return dst;
}


