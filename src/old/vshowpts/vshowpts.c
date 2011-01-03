/****************************************************************
 *
 * vshowpts:
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
 * $Id: vshowpts.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LEN 512
#define SQR(x) ((x) * (x))

extern char *getLipsiaVersion();
extern void VTal2Pixel(float ca[3], float voxel[3], float extent[3],
                       float x, float y, float z,
                       int *band, int *row, int *col);

VImage
VShowPts(VString filename, VFloat reso, VShort system,
         VShort nnslices, VShort nnrows, VShort nncols) {
    FILE *fp;
    VImage dest = NULL;
    int i, n, b, r, c, bb, rr, cc, nbands, nrows, ncols, wn2, rad2;
    char buf[LEN];
    float x, y, z, radius, id;
    float ca[3], cp[3], extent[3], voxel[3];
    voxel[0] = voxel[1] = voxel[2] = reso;
    ca[0] = 80 / reso;
    ca[1] = 81 / reso;
    ca[2] = 90 / reso;
    cp[0] = 80 / reso;
    cp[1] = 109 / reso;
    cp[2] = 90 / reso;
    if(system == 0 || system ==  1) {     /* Talairach */
        extent[0] = 135;
        extent[1] = 175;
        extent[2] = 120;
    } else if(system == 2) {   /* MNI */
        extent[0] = 141;
        extent[1] = 177;
        extent[2] = 128;
    }
    /*
    ** ini output image
    */
    nbands = nnslices;
    nrows = nnrows;
    ncols = nncols;
    if(nnslices < 1)
        nbands = (int)(160.0 / reso);
    if(nnrows < 1)
        nrows  = (int)(200.0 / reso);
    if(nncols < 1)
        ncols  = (int)(160.0 / reso);
    dest = VCreateImage(nbands, nrows, ncols, VFloatRepn);
    if(! dest)
        return NULL;
    VFillImage(dest, VAllBands, 0);
    VSetAttr(VImageAttrList(dest), "modality", NULL, VStringRepn, "conimg");
    memset(buf, 0, LEN);
    sprintf(buf, " %g %g %g", extent[0], extent[1], extent[2]);
    VSetAttr(VImageAttrList(dest), "extent", NULL, VStringRepn, buf);
    VSetAttr(VImageAttrList(dest), "orientation", NULL, VStringRepn, "axial");
    memset(buf, 0, LEN);
    sprintf(buf, "%g %g %g", ca[0], ca[1], ca[2]);
    VSetAttr(VImageAttrList(dest), "ca", NULL, VStringRepn, buf);
    memset(buf, 0, LEN);
    sprintf(buf, "%g %g %g", cp[0], cp[1], cp[2]);
    VSetAttr(VImageAttrList(dest), "cp", NULL, VStringRepn, buf);
    memset(buf, 0, LEN);
    sprintf(buf, " %.2f %.2f %.2f", reso, reso, reso);
    VSetAttr(VImageAttrList(dest), "voxel", NULL, VStringRepn, buf);
    /*
    ** read text file
    */
    fp = fopen(filename, "r");
    if(!fp)
        VError("error opening file %s", filename);
    i = 1;
    while(!feof(fp)) {
        memset(buf, 0, LEN);
        fgets(buf, LEN, fp);
        if(strlen(buf) < 3)
            continue;
        if(buf[0] == '%' || buf[0] == '#')
            continue;
        if((n = sscanf(buf, "%f %f %f %f %f", &x, &y, &z, &id, &radius)) != 5)
            VError(" line %d: illegal input format", i);
        if(id == 0)
            VWarning(" id=0 not visible");
        if(system  == 0) {
            c = x;
            r = y;
            b = z;
        } else {
            VTal2Pixel(ca, voxel, extent, x, y, z, &b, &r, &c);
        }
        if(c < 0 || c >= ncols) {
            VWarning(" illegal x coordinate: %d, %f, line %d", c, x, i);
            continue;
        }
        if(r < 0 || r >= nrows) {
            VWarning(" illegal y coordinate: %d, %f, line %d", r, y, i);
            continue;
        }
        if(b < 0 || b >= nbands) {
            VWarning(" illegal z coordinate: %d, %f, line %d", b, z, i);
            continue;
        }
        wn2  = radius + 1;
        rad2 = (int)(radius * radius + 0.5);
        for(bb = b - wn2; bb <= b + wn2; bb++) {
            if(bb < 0 || bb >= nbands)
                continue;
            for(rr = r - wn2; rr <= r + wn2; rr++) {
                if(rr < 0 || rr >= nrows)
                    continue;
                for(cc = c - wn2; cc <= c + wn2; cc++) {
                    if(cc < 0 || cc >= ncols)
                        continue;
                    if(SQR(b - bb) + SQR(r - rr) + SQR(c - cc) > rad2)
                        continue;
                    VPixel(dest, bb, rr, cc, VFloat) = (VFloat)id;
                }
            }
        }
        i++;
    }
    fclose(fp);
    return dest;
}



VDictEntry TALDict[] = {
    { "voxel", 0 },
    { "talairach", 1 },
    { "mni", 2 },
    { NULL }
};

int main(int argc, char *argv[]) {
    static VFloat  reso     = 1.0;
    static VShort  system   = 1;
    static VString filename = "";
    static VShort  nslices  = 0;
    static VShort  nrows    = 0;
    static VShort  ncols    = 0;
    static VOptionDescRec options[] = {
        {
            "in", VStringRepn, 1, (VPointer) &filename, VRequiredOpt, NULL,
            "file containing coordinates"
        },
        {
            "system", VLongRepn, 1, &system, VOptionalOpt, TALDict,
            "Type of coordinate system (voxel,talairach,mni)"
        },
        {"resolution", VFloatRepn, 1, &reso, VOptionalOpt, NULL, "voxel size in mm" },
        {"nslices", VShortRepn, 1, &nslices, VOptionalOpt, NULL, "number of slices in output image"},
        {"nrows", VShortRepn, 1, &nrows, VOptionalOpt, NULL, "number of rows in output image"},
        {"ncolumns", VShortRepn, 1, &ncols, VOptionalOpt, NULL, "number of columns in output image"}
    };
    FILE *out_file;
    VAttrList out_list;
    VImage dest = NULL;
    char prg_name[50];
    sprintf(prg_name, "vshowpts V%s", getLipsiaVersion());
    fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, NULL, &out_file);
    if(reso <= 0)
        VError(" illegal resolution parameter %f", reso);
    /*
    ** process
    */
    dest = VShowPts(filename, reso, system, nslices, nrows, ncols);
    /*
    ** Output
    */
    out_list = VCreateAttrList();
    VHistory(VNumber(options), options, prg_name, &out_list, &out_list);
    VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
    if(! VWriteFile(out_file, out_list))
        VError("can't write output file");
    fprintf(stderr, "%s: done.\n", argv[0]);
    return (EXIT_SUCCESS);
}
