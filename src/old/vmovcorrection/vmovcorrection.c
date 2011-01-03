/****************************************************************
 *
 * Program: vmovcorrection
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2007, <lipsia@cbs.mpg.de>
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
 * $Id: vmovcorrection.c 3780 2010-09-30 08:12:57Z tuerke $
 *
 *****************************************************************/


#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

extern void VSpatialFilter1(VAttrList);
extern VImage VMotionCorrection3d(VAttrList, VLong, VLong, VLong);
extern void VApplyMotionCorrection3d(VAttrList, VImage, VString);
extern char *getLipsiaVersion();


void
VReleaseStorage(VAttrList list) {
    VImage src;
    VAttrListPosn posn;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        VDestroyImage(src);
        src = NULL;
    }
}




int
main(int argc, char *argv[]) {
    static VLong   i0      = 5;
    static VLong   maxiter = 200;
    static VLong   step    = 3;
    static VString filename = "";
    static VOptionDescRec  options[] = {
        {"tref", VLongRepn, 1, (VPointer) &i0, VOptionalOpt, NULL, "reference time step"},
        {"report", VStringRepn, 1, (VPointer) &filename, VOptionalOpt, NULL, "report file"},
        {"step", VLongRepn, 1, (VPointer) &step, VOptionalOpt, NULL, "Step size (e.g. 2 or 3)"},
        {"iterations", VLongRepn, 1, (VPointer) &maxiter, VOptionalOpt, NULL, "Max number of iterations"}
    };
    FILE *in_file = NULL, *out_file = NULL;
    VImage motion = NULL;
    VAttrList list = NULL;
    char prg[50];
    sprintf(prg, "vmovcorrection V%s", getLipsiaVersion());
    fprintf(stderr, "%s\n", prg);
    /* parse command line */
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    if(step < 1)
        VError(" illegal step size %d", step);
    if(i0 < 0)
        VError(" illegal reference time step %d", i0);
    /* read data */
    if(!(list = VReadFile(in_file, NULL)))
        VError(" error reading data, perhaps insufficient memory ?");
    /* spatial smoothing */
    VSpatialFilter1(list);
    /* estimate motion parameters using smoothed data */
    motion = VMotionCorrection3d(list, i0, step, maxiter);
    /* re-read original data from file */
    VReleaseStorage(list);
    list = NULL;
    rewind(in_file);
    if(!(list = VReadFile(in_file, NULL)))
        VError(" error reading data, perhaps insufficient memory ?");
    fclose(in_file);
    /* apply motion correction */
    VApplyMotionCorrection3d(list, motion, filename);
    /* write output */
    VHistory(VNumber(options), options, prg, &list, &list);
    if(! VWriteFile(out_file, list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    exit(0);
}
