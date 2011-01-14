/****************************************************************
 *
 * vtal: geomtric rotation into Talairach coordinate system
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
 *
 *****************************************************************/

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/VImage.h"
#include "viaio/mu.h"
#include "viaio/option.h"


/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct DpointStruct {
    double x;
    double y;
    double z;
} DPoint;

extern void getLipsiaVersion(char*,size_t);
extern VImage VTalairach(VImage, VDouble, VDouble, VDouble,
                         DPoint, DPoint, DPoint, DPoint, VLong, VLong, DPoint, VLong);


/* definitions of the Talairach coordinate system */
VDictEntry TALDict[] = {
    { "atlas", 0 },
    { "neurological", 1 },
    { NULL }
};


VDictEntry SIZEDict[] = {
    { "small", 0 },
    { "big", 1 },
    { "xxl", 2 },
    { "huge", 3 },
    { NULL }
};


int main(int argc, char *argv[]) {
    static VDouble d1 = 1.3;
    static VDouble d2 = 5.2;
    static VDouble t = 120;
    static DPoint  angle;
    static DPoint  ca;
    static DPoint  cp;
    static DPoint  p;
    static VLong   type = 0;
    static VLong   talairach = 0;
    static VLong   dstsize = 0;
    static DPoint  ext_manual;
    static VOptionDescRec options[] = {
        {
            "d1", VDoubleRepn, 1, &d1, VOptionalOpt, 0,
            "Erosion (peeling parameter)"
        },
        {
            "d2", VDoubleRepn, 1, &d2, VOptionalOpt, 0,
            "Dilation (peeling parameter)"
        },
        {
            "t", VDoubleRepn, 1, &t, VOptionalOpt, 0,
            "Segmentation threshold (peeling parameter)"
        },
        { "ca", VDoubleRepn, 3, &ca, VRequiredOpt, 0, "CA position" },
        { "cp", VDoubleRepn, 3, &cp, VRequiredOpt, 0, "CP position" },
        { "p",  VDoubleRepn, 3, &p, VOptionalOpt, 0, "3rd Point position" },
        { "angle", VDoubleRepn, 3, &angle, VRequiredOpt, 0, "Rotation angles" },
        {
            "type",  VLongRepn, 1, &type, VOptionalOpt, 0,
            "Type of operation: 0=peel+extent, 1=no_peel+extent, 2=no_peel+no_extent"
        },
        {
            "system", VLongRepn, 1, &talairach, VOptionalOpt, TALDict,
            "Type of talairach convention: 0=atlas, 1=neurological"
        },
        {
            "extent", VDoubleRepn, 3, &ext_manual, VOptionalOpt, 0,
            "extent parameter (if peeling impossible)"
        },
        {
            "size", VLongRepn, 1, &dstsize, VOptionalOpt, SIZEDict,
            "Size of output image, small=160x200x160, big=180x230x190, xxl=210x240x200, huge"
        }
    };
    FILE *in_file, *out_file;
    VAttrList list;
    VAttrListPosn posn;
    VImage src = NULL, dst = NULL;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vtal V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    if(talairach < 0 || talairach > 1)
        VError(" parameter 'system' must be either 0 or 1");
    /* Read source image(s): */
    if(!(list = VReadFile(in_file, NULL)))
        return 1;
    /* Operate on each source image: */
    for(VFirstAttr(list, &posn); VAttrExists(&posn); VNextAttr(&posn)) {
        if(VGetAttrRepn(&posn) == VImageRepn) {
            VGetAttrValue(&posn, NULL, VImageRepn, &src);
            dst = VTalairach(src, d1, d2, t,
                             ca, cp, p, angle, type, talairach, ext_manual, dstsize);
            if(dst == NULL)
                VError("err in vtal");
            VSetAttrValue(&posn, NULL, VImageRepn, dst);
        }
    }
    /* Write the results to the output file: */
    VHistory(VNumber(options), options, prg_name, &list, &list);
    if(!VWriteFile(out_file, list))
        return 1;
    fprintf(stderr, " %s: done.\n", argv[0]);
    return 0;
}

