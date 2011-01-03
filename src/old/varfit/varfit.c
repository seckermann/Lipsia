/****************************************************************
 *
 * Program: varfit
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Sebastian Bode, 2008, <lipsia@cbs.mpg.de>
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
 * $Id: varfit.c 3186 2008-04-01 15:40:11Z karstenm $
 *
 *****************************************************************/

#include    "arfit_via.h"
#include    "arfit_error.h"

#include    <time.h>


VDictEntry sel_dict[] = {
    {"sbc"   ,   arfit_selector_sbc },
    {"fpe"   ,   arfit_selector_fpe },
    { NULL }
};
VDictEntry alg_dict[] = {
    {"Schloegl"  ,   arfit_algorithm_schloegl  },
    {"Schneider" ,   arfit_algorithm_schneider },
    { NULL }
};
VDictEntry mode_dict[] = {
    {"Yule_Walker_unbiased"  ,   arfit_mode_ywunbiased },
    {"Yule_Walker_biased"    ,   arfit_mode_ywbiased },
    {"Vieira_Morf_unbiased"  ,   arfit_mode_vmunbiased },
    {"Vieira_Morf_biased"    ,   arfit_mode_vmbiased },
    {"Nutall_Strand_unbiased",   arfit_mode_nsunbiased },
    {"Nutall_Strand_biased"  ,   arfit_mode_nsbiased },
    { NULL }
};


/* varfit function
    output all arvalues of given order pmin to pmax criterion selector and zero
*/
int varfit(FILE *out_file, arfit_file_info *info, int pmin, int pmax, int zero, arfit_selector selector, arfit_algorithm algorithm, arfit_mode mode, long threshold) {
    int sl, row, column, i, selacc = 0;
    long gofcount = 0;
    double var, gofacc = 0.0;
    clock_t start, end;
    char name[ 64 ];
    VAttrList       out_list;
    VAttrListPosn   iterator;
    VImage          *arimages;
    VImage          selimage;
    VImage          varimage;
    VImage          original;
    VImage          gofimage;
    arfit_input     *input  = NULL;
    arfit_output    *output = NULL;
    /* check parameters */
    if(!out_file || !info || pmin < 0 || pmin > pmax) {
        VWarning("varfit failed due to invalid arguments");
        return -1;
    }
    if(!(arimages = malloc(pmax * sizeof(VImage)))) {
        VWarning("Not enough memory to create ar images");
        return -1;
    }
    out_list = VCreateAttrList();
    for(VFirstAttr(info->vAttributes, &iterator); VAttrExists(&iterator); VNextAttr(&iterator))
        if(VGetAttrRepn(&iterator) == VImageRepn) {
            VGetAttrValue(&iterator, NULL, VImageRepn, &original);
            if(VPixelRepn(original) == VUByteRepn && VImageNRows(original) > info->nrows)
                VAppendAttr(out_list, "image", NULL, VImageRepn, original);
        }
    for(i = 0; i < pmax; ++i) {
        arimages[ i ] = VCreateImage(info->ntotalslices, info->nrows, info->ncolumns, VFloatRepn);
        VCopyImageAttrs(original, arimages[ i ]);
        sprintf(name, "AR(%d)", i + 1);
        VSetAttr(VImageAttrList(arimages[ i ]), "name", NULL, VStringRepn, name);
        VAppendAttr(out_list, "image", NULL, VImageRepn, arimages[ i ]);
    }
    selimage = VCreateImage(info->ntotalslices, info->nrows, info->ncolumns, VUByteRepn);
    varimage = VCreateImage(info->ntotalslices, info->nrows, info->ncolumns, VFloatRepn);
    gofimage = VCreateImage(info->ntotalslices, info->nrows, info->ncolumns, VFloatRepn);
    VCopyImageAttrs(original, selimage);
    VCopyImageAttrs(original, varimage);
    VCopyImageAttrs(original, gofimage);
    VSetAttr(VImageAttrList(selimage), "name", NULL, VStringRepn, (selector == arfit_selector_sbc) ? "sbc" : "fpe");
    VSetAttr(VImageAttrList(varimage), "name", NULL, VStringRepn, "var");
    VSetAttr(VImageAttrList(gofimage), "name", NULL, VStringRepn, "gof");
    VAppendAttr(out_list, "image", NULL, VImageRepn, varimage);
    VAppendAttr(out_list, "image", NULL, VImageRepn, selimage);
    VAppendAttr(out_list, "image", NULL, VImageRepn, gofimage);
    arfit_input_alloc(input);
    input->selector     = selector;
    input->zero         = 0;
    input->pmin         = pmin;
    input->pmax         = pmax;
    input->threshold    = threshold;
    /* begin time logging*/
    start = clock();
    for(sl = 0; sl < info->nfunctionalslices; ++sl)
        for(row = 0; row < info->nrows; ++row)
            for(column = 0; column < info->ncolumns; ++column) {
                var = 0.0;
                /*info->nsamples = NUMBER_OF_SAMPLES;*/
                if(input->v == NULL)
                    add_sample_to_input(info, input, info->nFunctionalImageIndices[ sl ],  row, column);
                else
                    reset_input_data(info, input, info->nFunctionalImageIndices[ sl ], row, column, 0);
                switch(algorithm) {
                case arfit_algorithm_schneider:
                    arfit_schneider(input, &output);
                    break;
                case arfit_algorithm_schloegl:
                    arfit_schloegl(input, mode, &output);
                    break;
                default:
                    break;
                }
                /* output ar model parameters */
                for(i = 0; i < input->pmax; ++i) {
                    VPixel(arimages[ i ], info->nFunctionalImageIndices[ sl ], row, column, VFloat) = (VFloat) gsl_matrix_get(output->A, 0, i);
                }
                /* output optimal selection order */
                VPixel(selimage, info->nFunctionalImageIndices[ sl ], row, column, VUByte) = (VUByte) output->popt;
                /* output trace of C */
                for(i = 0; i < output->C->size1; ++i)
                    var += gsl_matrix_get(output->C, i, i);
                VPixel(varimage, info->nFunctionalImageIndices[ sl ], row, column, VFloat) = (VFloat) var;
                /* check if gof is valid */
                if(output->iprocessed == 1) {
                    /* compute goodness of fit value for the current voxel */
                    var = (VFloat) arfit_gof(input, 0, output);
                    /* output goodness of fit value to vista file */
                    VPixel(gofimage, info->nFunctionalImageIndices[ sl ], row, column, VFloat) = var;
                    /* accumulate gof and selection values and counter */
                    gofacc += 1.0f - arfit_mse(output);
                    selacc += output->popt;
                    ++gofcount;
                } else
                    VPixel(gofimage, info->nFunctionalImageIndices[ sl ], row, column, VFloat) = 0.0;
            }
    /* end time logging */;
    end = clock();
    fprintf(stderr, "\n varfit( %ld samples order %d to %d ) processed in %.2f secs\n", gofcount, input->pmin, input->pmax, (double)(end - start) / (double)CLOCKS_PER_SEC);
    if(gofcount > 0) {
        gofacc /= (double) gofcount;
        selacc /= gofcount;
    }
    fprintf(stderr, "\n----------\nOverall gof:\t%f\n----------\nOverall popt:\t%d\n----------\n", gofacc, selacc);
    if(arimages)
        free(arimages);
    if(input)
        arfit_input_free(input);
    if(output)
        arfit_output_free(output);
    if(!VWriteFile(out_file, out_list))
        return -1;
    return 0;
}

/* main function
    parameter handling and programm entry point
*/
int main(int argc, char *argv[]) {
    static VLong            minval = 2000, selector = arfit_selector_sbc, algorithm = arfit_algorithm_schloegl, mode = arfit_mode_vmunbiased;
    static VShort           pmin = 1, pmax = 2;
    /* static varaibles for command line parsing */
    static VOptionDescRec   options[] = {
        { "pmin",       VShortRepn,     1, &pmin,       VOptionalOpt, NULL,     "Minimum order" },
        { "pmax",       VShortRepn,     1, &pmax,       VOptionalOpt, NULL,     "Maximum order" },
        { "selector",   VLongRepn,      1, &selector,   VOptionalOpt, sel_dict, "Order selection criteria" },
        { "algorithm",  VLongRepn,      1, &algorithm,  VOptionalOpt, alg_dict, "Implementation" },
        { "mode",       VLongRepn,      1, &mode,       VOptionalOpt, mode_dict, "Mode for Schloegl implementation" },
        { "minval",     VLongRepn,      1, &minval,     VOptionalOpt, NULL,     "Threshold" }
    };
#ifndef     _DEBUG
    gsl_set_error_handler_off();
#endif
    /* variables used to get timecourse data */
    FILE *in_file, *out_file;
    /* variables used for arfit methods */
    arfit_file_info *info   = NULL;
    /* Parse command line arguments and identify files: */
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    /* get functional time series data information */
    info = get_file_info(in_file, NULL);
    fprintf(stderr, "\n Starting varfit...\n");
    /* output ar parameters of every voxel given in vista input file */
    if(!varfit(out_file, info, pmin, pmax, 1, (arfit_selector) selector, (arfit_algorithm) algorithm, (arfit_mode) mode, minval))
        fprintf(stderr, "\n...succeeded\n\n");
    else
        fprintf(stderr, "\n...falied\n\n");
    free_file_info(info);
    return 0;
}
