/****************************************************************
 *
 * Program: vmulticomp
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
 * $Id: vmulticomp.c 3385 2008-08-20 14:18:34Z lohmann $
 *
 *****************************************************************/


/*
** Multiple comparison correction using monte carlo simulations.
**
** Lit:
**  Lohmann et al. (2008):
**  The multiple comparison problem in fMRI - a new method based on anatomical priors,
**  MICCAI, Workshop on Analysis of Functional Medical Images, New York, Sept.10, 2008.
**
** G.Lohmann, MPI-CBS
*/
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


extern VImage VMulticomp2d(VImage, VImage, VLong, VLong, VDouble, VDouble, VDouble);
extern VImage VMulticomp3d(VImage, VImage, VLong, VLong, VDouble, VDouble, VDouble);
extern void getLipsiaVersion(char*,size_t);



int
main(int argc, char *argv[]) {
    static VDouble p0      = 0.05;    /* corrected p-threshold */
    static VDouble z0      = 2.576;   /* initial cluster threshold */
    static VDouble fwhm    = 5;       /* spatial smoothness in mm */
    static VLong   seed    = 555;
    static VBoolean symm   = FALSE;
    static VLong   numiter = 1000;
    static VOptionDescRec  options[] = {
        {"z", VDoubleRepn, 1, (VPointer) &z0, VOptionalOpt, NULL, "initial threshold for defining clusters"},
        {"p", VDoubleRepn, 1, (VPointer) &p0, VOptionalOpt, NULL, "corrected p-threshold"},
        {"fwhm", VDoubleRepn, 1, (VPointer) &fwhm, VOptionalOpt, NULL, "fwhm of spatial smoothness in mm "},
        {"seed", VLongRepn, 1, (VPointer) &seed, VOptionalOpt, NULL, "seed"},
        {
            "symmetry", VBooleanRepn, 1, (VPointer) &symm, VOptionalOpt, NULL,
            "Whether to include hemispheric symmetry as an additional feature"
        },
        {"iter", VLongRepn, 1, (VPointer) &numiter, VOptionalOpt, NULL, "number of iterations"}
    };
    FILE *in_file, *out_file = NULL;
    VAttrList list = NULL, out_list = NULL;
    VAttrListPosn posn;
    VImage src = NULL, dest = NULL;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vmulticomp V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(symm == FALSE)
            dest = VMulticomp2d(src, NULL, seed, numiter, fwhm, z0, p0);
        else
            dest = VMulticomp3d(src, NULL, seed, numiter, fwhm, z0, p0);
        break;
    }
    if(src == NULL)
        VError(" no input image found");
    /* output */
    out_list = VCreateAttrList();
    VAppendAttr(out_list, "multicomp", NULL, VImageRepn, dest);
    VHistory(VNumber(options), options, prg_name, &list, &out_list);
    if(! VWriteFile(out_file, out_list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}
