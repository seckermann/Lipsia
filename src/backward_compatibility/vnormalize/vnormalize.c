/****************************************************************
 *
 * vnormalize:
 * apply a linear normalization
 *
 * Copyright (C) 1998 MPI of Cognitive Neuroscience, Leipzig
 *
 * Author Gaby Lohmann, Apr. 1998, <lohmann@cns.mpg.de>
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
 *****************************************************************
 *
 * History:
 * ========
 *  Heiko Mentzel -- 07/10/2001 -- std. output
 *
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>



/* Later in this file: */
extern VImage VNormalize(VImage, VImage, VShort);
extern char *getLipsiaVersion();

VDictEntry TALDict[] = {
    { "talairach", 0 },
    { "mni", 1 },
    { NULL }
};


int main(int argc, char *argv[]) {
    /* Command line options: */
    static VShort type = 0;
    static VOptionDescRec  options[] = {
        {"system", VShortRepn, 1, (VPointer) &type, VOptionalOpt, TALDict, "System to be used"}
    };
    FILE *in_file, *out_file;
    VAttrList list;
    VAttrListPosn posn;
    VImage src = NULL, dest = NULL;
    char prg[50];
    sprintf(prg, "vnormalize V%s", getLipsiaVersion());
    fprintf(stderr, "%s\n", prg);
    /* Parse command line arguments and identify files: */
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    if(type < 0 || type > 1)
        VError(" illegal parameter value, 'type' must be 0 or 1");
    /* Read the input file: */
    list = VReadFile(in_file, NULL);
    if(! list)
        exit(EXIT_FAILURE);
    fclose(in_file);
    /*
    ** process each object
    */
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        dest = VNormalize(src, NULL, type);
        VSetAttrValue(& posn, NULL, VImageRepn, dest);
    }
    /*
    ** output
    */
    VHistory(0, NULL, prg, &list, &list);
    if(! VWriteFile(out_file, list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}
