/****************************************************************
 *
 * FuncTrans.c
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 1998, <lipsia@cbs.mpg.de>
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
 * $Id: TransTimeStep.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <via.h>


#define AXIAL    0
#define CORONAL  1
#define SAGITTAL 2

extern VImage VAxial2Coronal_short(VImage, VImage);
extern VImage VAxial2Sagittal_short(VImage, VImage);



VImage
VTransTimeStep(VImage src, VImage src_tmp, VImage dest, VImage dest_tmp, VImage transform,
               int nbands1, int nrows1, int ncols1, int orient, VFloat resolution,
               float zscale, float yscale, float xscale) {
    float b0, r0, c0;
    float scale[3], shift[3];
    zscale /= resolution;
    yscale /= resolution;
    xscale /= resolution;
    scale[0] = zscale;
    scale[1] = yscale;
    scale[2] = xscale;
    shift[0] = 0;
    shift[1] = (float)nrows1 * 0.5 - yscale * (float)VImageNRows(src) * 0.5;
    shift[2] = (float)ncols1 * 0.5 - xscale * (float)VImageNColumns(src) * 0.5;
    src_tmp = VTriLinearScale3d(src, src_tmp, nbands1, nrows1, ncols1, shift, scale);
    /*
    ** resample image
    */
    b0 = (float)nbands1 * 0.5;
    r0 = (float)nrows1  * 0.5;
    c0 = (float)ncols1  * 0.5;
    dest = VTriLinearSample3d(src_tmp, dest, transform, b0, r0, c0, nbands1, nrows1, ncols1);
    switch(orient) {
    case AXIAL:
        return dest;
    case CORONAL:
        dest_tmp = VAxial2Coronal_short(dest, dest_tmp);
        return dest_tmp;
    case SAGITTAL:
        dest_tmp = VAxial2Sagittal_short(dest, dest_tmp);
        return dest_tmp;
    }
    return NULL;
}
