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
 * $Id: via2mat.c 3183 2008-04-01 15:27:39Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/headerinfo.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <gsl/gsl_matrix.h>

#include "via2mat.h"


/**
 * This method creates 2D GSL-Matrix from a 2D via image (only one band).
 * @param src A 2D via image from which the matrix should be generated.
 * @return A reference to the allocated gsl matrix.
 */
gsl_matrix_float *VFloat2Mat_gsl(VImage src) {
    gsl_matrix_float *mat = NULL;
    /* correct datatype? */
    if(VPixelRepn(src) != VFloatRepn)
        VError("src must be VFloat in routine VFloat2Mat");
    /* correct number of bands? */
    if(VImageNBands(src) > 1)
        VError("number of bands (%d) > 1in VFloat2Mat", VImageNBands(src));
    /* allocate  pixel matrix */
    mat = gsl_matrix_float_alloc(VImageNRows(src), VImageNColumns(src));
    /* copy pixel values */
    memcpy((VFloat *)mat->data, (VFloat *)VPixelPtr(src, 0, 0, 0),
           sizeof(VFloat) * VImageNRows(src)* VImageNColumns(src));
    /* transpose matrix */
    gsl_matrix_float *dest = gsl_matrix_float_alloc(mat->size2, mat->size1);
    gsl_matrix_float_transpose_memcpy(dest, mat);
    /* free memory */
    gsl_matrix_float_free(mat);
    return dest;
}

/* WARNING: This gsl implementation of VShort2Mat is heavily customized
 * for use within whitecov(). It returns a 2-dim matrix (bands,cols*rows instead of
 * the 3-dim array (cols,rows,bands) from the matlab method.
 */
gsl_matrix_float *VShort2Mat_gsl(VImage *src, int numsub) {
    int allbands = 0, rows = 0, cols = 0, k;
    gsl_matrix_float *dest;
    /* check geometry, count bands */
    for(k = 0; k < numsub; k++) {
        if(VPixelRepn(src[k]) != VShortRepn)
            VError("src must be VShort in routine VShort2Mat_gsl");
        /* check dimensions */
        if(rows == 0)
            rows = VImageNRows(src[k]);
        else {
            if(rows != VImageNRows(src[k]))
                VError("rows do not coincide in routine VShort2Mat_gsl");
        }
        if(cols == 0)
            cols = VImageNColumns(src[k]);
        else {
            if(cols != VImageNColumns(src[k]))
                VError("cols do not coincide in routine VShort2Mat_gsl");
        }
        allbands += VImageNBands(src[k]);
    }
    /* create array */
    dest = gsl_matrix_float_alloc(allbands, rows * cols);
    int n, cbands = 0;
    int r, c, j;
    double sum1 = 0, sum2 = 0, nxx;
    double mean = 0, sig = 1;
    float u = 0, norm = 0;
    gsl_vector_float *buffer;
    for(k = 0; k < numsub; k++) {
        n = VImageNBands(src[k]);
        buffer = gsl_vector_float_alloc(n);
        for(r = 0; r < rows; r++) {
            for(c = 0; c < cols; c++) {
                /* Computing mean and sig */
                sum1 = sum2 = 0;
                for(j = 0; j < n; j++) {
                    u = (float)VPixel(src[k], j, r, c, VShort);
                    gsl_vector_float_set(buffer, j, u);
                    sum1 += u;
                    sum2 += u * u;
                }
                nxx  = (double)n;
                mean = sum1 / nxx;
                sig  = sqrt((sum2 - nxx * mean * mean) / (nxx - 1.0));
                /* Applying mean and sig */
                if(sig > 1) {
                    norm = -mean + 100.0 * sig;
                    gsl_vector_float_add_constant(buffer, (const float)norm);
                    gsl_vector_float_scale(buffer, (const float)1.0 / sig);
                    for(j = 0; j < n; j++)
                        gsl_matrix_float_set(dest, j + cbands, r * cols + c, gsl_vector_float_get(buffer, j));
                } else {
                    for(j = 0; j < n; j++)
                        gsl_matrix_float_set(dest, j + cbands, r * cols + c, 0.0);
                }
            }
        }
        cbands += n;
        gsl_vector_float_free(buffer);
    }
    return dest;
}

