/****************************************************************
 *
 * vwhiteglm:
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
 * $Id: whitereg.c 3351 2008-05-28 12:22:24Z proeger $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/headerinfo.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "via2mat.h"
#include "mathtools.h"
#include "gsl_utils.h"
#include "BlockIO.h"

#define LEN  10000

extern VBoolean VReadBandDataFD(int, VImageInfo *, int, int, VImage *);
extern int VStringToken(char *, char *, int, int);

/*
** general linear regression
*/
VAttrList *
VWhiteRegression(ListInfo *linfo, int numsub, VImage design, VShort numlags, VShort type, VShort numcon, VArgVector contrast, VString confile) {
    FILE *fp = NULL;
    VAttrList *out_list_gsl = NULL;
    VImageInfo *xinfo;
    VFloat *con = NULL, *dfs_gsl = NULL;
    VImage *src = NULL, *dest_gsl = NULL, *sd_gsl = NULL,
            rho_vol_gsl = NULL;
    VString buf = NULL;
    gsl_matrix_float *X_gsl = NULL, *CON_gsl = NULL, *SRC_gsl = NULL;
    gsl_matrix_float *pinvX_gsl = NULL, *invM_gsl = NULL;
    int dim[5];
    int nbands = 0, nslices = 0, nrows = 0, ncols = 0, reptime = 0, npix = 0;
    int i = 0, j = 0, k = 0, n = 0, m = 0, nr = 0, nc = 0, nt = 0, tr = 0;
    int slice = 0, fd = 0, slicedata = 1;
    float val = 0;
    char *constring = NULL;
    char buff[LEN], token[32];
    /*
     * switch gsl error handling off
     */
    gsl_set_error_handler_off();
    /*
     ** read input data
     */
    nslices = nbands = nrows = ncols = 0;
    for(k = 0; k < numsub; k++) {
        n  = linfo[k].nslices;
        nr = linfo[k].nrows;
        nc = linfo[k].ncols;
        nt = linfo[k].ntimesteps;
        tr = linfo[k].itr;
        nbands += nt;
        if(reptime == 0)
            reptime = tr;
        else if(reptime != tr)
            VError(" inconsistent repetition times: %d %d", tr, reptime);
        if(nslices == 0)
            nslices = n;
        else if(nslices != n)
            VError(" inconsistent image dimensions, slices: %d %d", n, nslices);
        if(nrows == 0)
            nrows = nr;
        else if(nrows != nr)
            VError(" inconsistent image dimensions, rows: %d %d", nr, nrows);
        if(ncols == 0)
            ncols = nc;
        else if(ncols != nc)
            VError(" inconsistent image dimensions, cols: %d %d", nc, ncols);
    }
    npix = (nrows * ncols);
    fprintf(stderr, " num input files: %d,  image dimensions: %d x %d x %d\n",
            numsub, nslices, nrows, ncols);
    /*
     ** get design dimensions
     */
    m = VImageNRows(design);      /* number of timesteps   */
    n = VImageNColumns(design);   /* number of covariates */
    fprintf(stderr, " ntimesteps=%d,  num covariates=%d,  num contrasts=%d\n", m, n, numcon);
    if(n >= MBETA)
        VError(" too many covariates (%d), max is %d", n, MBETA);
    if(m != nbands)
        VError(" design dimension inconsistency: %d %d", m, nbands);
    /*
    ** contrast vector
    */
    con = (VFloat *)VMalloc(sizeof(VFloat) * n * numcon);
    if(contrast.number > 0) {
        if(n *numcon != contrast.number)
            VError(" contrast vector has bad length %d, correct length is %d", contrast.number, n * numcon);
        for(i = 0; i < n * numcon; i++)
            con[i] = ((VFloat *)contrast.vector)[i];
    } else {
        fp = VOpenInputFile(confile, TRUE);
        if(!fp)
            VError(" error reading contrast file");
        nr = 0;
        i = 0;
        while(!feof(fp)) {
            memset(buff, 0, LEN);
            if(!fgets(buff, LEN, fp))
                break;
            if(strlen(buff) < 1)
                continue;
            if(buff[0] == '%' || buff[0] == '#')
                continue;
            j = 0;
            while(VStringToken(buff, token, j, 30)) {
                if(!sscanf(token, "%f", &val))
                    VError(" illegal input string: %s", token);
                con[i++] = (VFloat)val;
                j++;
            }
            if(j < 1)
                continue;
            else if(n != j)
                VError(" check length of contrast vector no %d", nr + 1);
            nr++;
            if(nr > numcon)
                VError(" mismatch between numcon parameter and number of contrasts in confile");
        }
        if(nr != numcon)
            VError(" mismatch between numcon and number of contrasts in confile");
        fclose(fp);
    }
    /*
    ** display and set contrast cector
    */
    fprintf(stderr, " contrast matrix:\n");
    for(i = 0; i < numcon; i++) {
        for(j = 0; j < n; j++)
            fprintf(stderr, "  %.2f", con[i * n + j]);
        fprintf(stderr, "\n");
    }
    CON_gsl = gsl_matrix_float_alloc(numcon, n);
    memcpy((VFloat *)gsl_matrix_float_ptr(CON_gsl, 0, 0),
           con, sizeof(VFloat)*n * numcon);
    VFree(con);
    /*
    ** output types
    */
    switch(type) {
    case 0:    /* conimg  */
        buf = VNewString("conimg");
        break;
    case 1:    /* t-image */
        buf = VNewString("tmap");
        break;
    case 2:    /* zmap    */
        buf = VNewString("zmap");
        break;
    case 3:    /* Fmap    */
        buf = VNewString("Fmap");
        break;
    default:
        VError(" illegal type");
    }
    fprintf(stderr, " output type: %s\n", buf);
    fprintf(stderr, " working...\n");
    /*
     ** imagedimensions
     */
    dim[0] = npix;
    dim[1] = nslices;
    dim[2] = (int)numlags;
    dim[3] = (int)type;
    dim[4] = (int)numcon;
    /*
     * design matrix
    */
    X_gsl = (gsl_matrix_float *)VFloat2Mat_gsl(design);
    /* initialize rho_vol -- gsl version */
    rho_vol_gsl = VCreateImage(npix, nslices, numlags, VFloatRepn);
    VFillImage(rho_vol_gsl, VAllBands, 0);
    /* prewhitening */
    prewhite(&pinvX_gsl, &invM_gsl, X_gsl, dim);
    /* initialize dfs_gsl pointer */
    dfs_gsl = (VFloat *)VMalloc(sizeof(VFloat) * (numcon + 1));
    /*
     ** degrees of freedom
     */
    dfs(&dfs_gsl, pinvX_gsl, X_gsl, CON_gsl, dim);
    /*
     ** create destination images
     */
    src = (VImage *)VMalloc(sizeof(VImage) * numsub);
    for(k = 0; k < numsub; k++)
        src[k] = VCreateImage(linfo[k].ntimesteps, nrows, ncols, VShortRepn);
    /*
     ** first pass via slices
     */
    for(slice = 0; slice < nslices; slice++) {
        fprintf(stderr, " first pass slice: %3d\r", slice);
        slicedata = 1;
        for(k = 0; k < numsub; k++) {
            if(linfo[k].zero[slice] == 0)
                slicedata = 0;
        }
        if(slicedata == 1) {
            for(k = 0; k < numsub; k++) {
                VFillImage(src[k], VAllBands, 0);
                fd = open(linfo[k].filename, O_RDONLY);
                if(fd == -1)
                    VError("could not open file %s", linfo[k].filename);
                nt = linfo[k].ntimesteps;
                if(! VReadBandDataFD(fd, &linfo[k].info[slice], 0, nt, &src[k]))
                    VError(" error reading data");
                close(fd);
            }
            SRC_gsl = VShort2Mat_gsl(src, numsub);
            whitecov(&rho_vol_gsl, SRC_gsl, invM_gsl, pinvX_gsl, X_gsl, dim, slice);
            gsl_matrix_float_free(SRC_gsl);
        } /* if slicedata are available */
    } /* loop over slices */
    /*
    ** create outlist and destination image
    */
    dest_gsl = (VImage *)VMalloc(sizeof(VImage) * numcon);
    sd_gsl   = (VImage *)VMalloc(sizeof(VImage) * numcon);
    out_list_gsl = (VAttrList *)VMalloc(sizeof(VAttrList) * numcon);
    for(i = 0; i < numcon; i++) {
        out_list_gsl[i] = VCreateAttrList();
        dest_gsl[i] = VCreateImage(nslices, nrows, ncols, VFloatRepn);
        VFillImage(dest_gsl[i], VAllBands, 0);
        if(type == 0) {
            sd_gsl[i] = VCreateImage(nslices, nrows, ncols, VFloatRepn);
            VFillImage(sd_gsl[i], VAllBands, 0);
        } else
            sd_gsl[i] = NULL;
    }
    /*
     ** second pass via slices
     */
    for(slice = 0; slice < nslices; slice++) {
        fprintf(stderr, " second pass slice: %3d\r", slice);
        slicedata = 1;
        for(k = 0; k < numsub; k++) {
            if(linfo[k].zero[slice] == 0)
                slicedata = 0;
        }
        if(slicedata == 1) {
            for(k = 0; k < numsub; k++) {
                VFillImage(src[k], VAllBands, 0);
                fd = open(linfo[k].filename, O_RDONLY);
                if(fd == -1)
                    VError("could not open file %s", linfo[k].filename);
                nt = linfo[k].ntimesteps;
                if(! VReadBandDataFD(fd, &linfo[k].info[slice], 0, nt, &src[k]))
                    VError(" error reading data");
                close(fd);
            }
            SRC_gsl = VShort2Mat_gsl(src, numsub);
            whitecov2(&dest_gsl, &sd_gsl, rho_vol_gsl, SRC_gsl, X_gsl, CON_gsl,
                      dfs_gsl, dim, slice);
            gsl_matrix_float_free(SRC_gsl);
        } /* if slicedata is available */
    } /* loop over slices */
    /*
     ** create image headers and return the outlist
     */
    xinfo = linfo[0].info;
    char str[10];
    for(i = 0; i < numcon; i++) {
        constring = (char *)VMalloc(sizeof(char) * 10 * n);
        constring[0] = '\0';
        for(k = 0; k < n; k++) {
            sprintf(str, "%1.2f ", gsl_matrix_float_get(CON_gsl, i, k));
            strcat((char *)constring, (const char *)str);
        }
        VSetAttr(VImageAttrList(dest_gsl[i]), "df", NULL, VFloatRepn, dfs_gsl[i]);
        VSetAttr(VImageAttrList(dest_gsl[i]), "modality", NULL, VStringRepn, buf);
        VSetAttr(VImageAttrList(dest_gsl[i]), "name", NULL, VStringRepn, buf);
        VSetAttr(VImageAttrList(dest_gsl[i]), "patient", NULL, VStringRepn, xinfo->patient);
        VSetAttr(VImageAttrList(dest_gsl[i]), "voxel", NULL, VStringRepn, xinfo->voxel);
        VSetAttr(VImageAttrList(dest_gsl[i]), "repetition_time", NULL, VLongRepn, reptime);
        VSetAttr(VImageAttrList(dest_gsl[i]), "contrast", NULL, VStringRepn, constring);
        if(xinfo->fixpoint[0] != 'N')
            VSetAttr(VImageAttrList(dest_gsl[i]), "fixpoint", NULL, VStringRepn, xinfo->fixpoint);
        if(xinfo->ca[0] != 'N') {
            VSetAttr(VImageAttrList(dest_gsl[i]), "ca", NULL, VStringRepn, xinfo->ca);
            VSetAttr(VImageAttrList(dest_gsl[i]), "cp", NULL, VStringRepn, xinfo->cp);
            VSetAttr(VImageAttrList(dest_gsl[i]), "extent", NULL, VStringRepn, xinfo->extent);
            VSetAttr(VImageAttrList(dest_gsl[i]), "talairach", NULL, VStringRepn, xinfo->talairach);
        }
        VAppendAttr(out_list_gsl[i], "image", NULL, VImageRepn, dest_gsl[i]);
        if(type == 0) {
            VCopyImageAttrs(dest_gsl[i], sd_gsl[i]);
            VSetAttr(VImageAttrList(sd_gsl[i]), "modality", NULL, VStringRepn, "std_dev");
            VSetAttr(VImageAttrList(sd_gsl[i]), "name", NULL, VStringRepn, "std_dev");
            VAppendAttr(out_list_gsl[i], "image", NULL, VImageRepn, sd_gsl[i]);
        }
    }
    /**
     * free some memory
     */
    gsl_matrix_float_free(X_gsl);
    gsl_matrix_float_free(CON_gsl);
    VFree(dfs_gsl);
    for(k = 0; k < numsub; k++)
        VDestroyImage(src[k]);
    VDestroyImage(rho_vol_gsl);
    return out_list_gsl;
}

