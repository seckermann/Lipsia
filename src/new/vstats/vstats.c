/*
** estimate distribution parameters: std, skewness, kurtosis
**
** G.Lohmann, MPI-CBS, Jan 2007
*/

#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/os.h>
#include <viaio/VImage.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>


#define NIMAGES 2000   /* max number of slices */

#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))

extern char *getLipsiaVersion();

VDictEntry TypeDict[] = {
    { "mean", 0 },
    { "std", 1 },
    { "skewness", 2 },
    { "kurtosis", 3 },
    { "min", 4 },
    { "max", 5 },
    { "median", 6 },
    { "stderr", 7 },
    { "numpos", 8 },
    { "sum", 9 },
    { "cv", 10 },
    { NULL }
};

VImage
VStats(VImage *src, VImage dest, int n, VShort type) {
    int i, k, b, r, c, nslices, nrows, ncols;
    double u, v, tiny = 1.0e-7;
    double upos, uneg;
    gsl_vector *vec = NULL;
    extern void gsl_sort_vector(gsl_vector *);
    nslices = VImageNBands(src[0]);
    nrows   = VImageNRows(src[0]);
    ncols   = VImageNColumns(src[0]);
    if(type >= 1 && type <= 3) {
        dest = VCreateImage(nslices, nrows, ncols, VFloatRepn);
        VCopyImageAttrs(src[0], dest);
    } else {
        dest = VCopyImage(src[0], NULL, VAllBands);
    }
    VFillImage(dest, VAllBands, 0);
    VSetAttr(VImageAttrList(dest), "num_images", NULL, VShortRepn, (VShort)n);
    VSetAttr(VImageAttrList(dest), "patient", NULL, VStringRepn, (VString)TypeDict[type].keyword);
    fprintf(stderr, " %s\n", TypeDict[type].keyword);
    vec = gsl_vector_calloc(n);
    for(b = 0; b < nslices; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                u = 0;
                k = 0;
                for(i = 0; i < n; i++) {
                    if(b >= VImageNBands(src[i]))
                        continue;
                    if(r >= VImageNRows(src[i]))
                        continue;
                    if(c >= VImageNColumns(src[i]))
                        continue;
                    u = VGetPixel(src[i], b, r, c);
                    gsl_vector_set(vec, i, u);
                    if(ABS(u) > tiny)
                        k++;
                }
                if(k < 2)
                    continue;
                switch(type) {
                case 0:
                    u = gsl_stats_mean(vec->data, vec->stride, n);
                    break;
                case 1:
                    u = gsl_stats_sd(vec->data, vec->stride, n);
                    break;
                case 2:
                    u = gsl_stats_skew(vec->data, vec->stride, n);
                    break;
                case 3:
                    u = gsl_stats_kurtosis(vec->data, vec->stride, n);
                    break;
                case 4:
                    u = gsl_stats_min(vec->data, vec->stride, n);
                    break;
                case 5:
                    u = gsl_stats_max(vec->data, vec->stride, n);
                    break;
                case 6:
                    gsl_sort_vector(vec);
                    u = gsl_stats_median_from_sorted_data(vec->data, vec->stride, n);
                    break;
                case 7:
                    u = gsl_stats_sd(vec->data, vec->stride, n);
                    u /= sqrt((double)n);
                    break;
                case 8:
                    upos = uneg = 0;
                    for(i = 0; i < n; i++) {
                        u = gsl_vector_get(vec, i);
                        if(u > 0)
                            upos++;
                        if(u < 0)
                            uneg++;
                    }
                    u = upos;
                    if(upos < uneg)
                        u = -uneg;
                    break;
                case 9:  /* sum */
                    u = 0;
                    for(i = 0; i < n; i++) {
                        u += gsl_vector_get(vec, i);
                    }
                    if(u > VRepnMaxValue(VFloatRepn))
                        u = VRepnMaxValue(VFloatRepn) - 1;
                    break;
                case 10:  /* coefficient of variation */
                    u = gsl_stats_mean(vec->data, vec->stride, n);
                    u = ABS(u);
                    if(u < 1.0e-6)
                        continue;
                    v = gsl_stats_sd(vec->data, vec->stride, n);
                    if(ABS(v) < 1.0e-6)
                        continue;
                    u = v / u;
                    if(u > 1000)
                        u = 0;
                    break;
                default:
                    VError(" illegal type");
                }
                VSetPixel(dest, b, r, c, u);
            }
        }
    }
    return dest;
}



int main(int argc, char *argv[]) {
    static VArgVector in_files;
    static VString out_filename;
    static VShort type = 0;
    static VBoolean in_found, out_found;
    static VOptionDescRec options[] = {
        {"in", VStringRepn, 0, & in_files, & in_found, NULL, "Input files" },
        {"type", VShortRepn, 1, (VPointer) &type, VOptionalOpt, TypeDict, "output type"},
        {"out", VStringRepn, 1, & out_filename, & out_found, NULL, "Output file" }
    };
    FILE *fp = NULL;
    VStringConst in_filename;
    VAttrList list, out_list;
    VAttrListPosn posn;
    VImage src, src1[NIMAGES], dest = NULL;
    int i, n;
	char prg_name[100];
	sprintf(prg_name, "vstats V%s", getLipsiaVersion());
	fprintf(stderr, "%s\n", prg_name);
    /* Parse command line arguments: */
    if(! VParseCommand(VNumber(options), options, & argc, argv) ||
            ! VIdentifyFiles(VNumber(options), options, "in", & argc, argv, 0) ||
            ! VIdentifyFiles(VNumber(options), options, "out", & argc, argv, -1))
        goto Usage;
    if(argc > 1) {
        VReportBadArgs(argc, argv);
Usage:
        VReportUsage(argv[0], VNumber(options), options, NULL);
        exit(EXIT_FAILURE);
    }
    /* ini */
    n = in_files.number;
    if(n >= NIMAGES)
        VError(" too many images, max is %d", NIMAGES);
    /* read images  */
    for(i = 0; i < n; i++)
        src1[i] = NULL;
    for(i = 0; i < n; i++) {
        in_filename = ((VStringConst *) in_files.vector)[i];
        fprintf(stderr, " %3d:  %s\n", i, in_filename);
        fp = VOpenInputFile(in_filename, TRUE);
        list = VReadFile(fp, NULL);
        if(! list)
            VError("Error reading image");
        fclose(fp);
        for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & src);
            src1[i] = src;
            if(i > 0 && VImageNPixels(src1[i]) != VImageNPixels(src1[0])) {
                VWarning("inconsistent number of pixels, [0]: %d, [%d]: %d",
                         VImageNPixels(src1[0]), i, VImageNPixels(src1[i]));
            }
            break;
        }
        if(src1[i] == NULL)
            VError(" image %d not found", i);
    }
    /* get statistics */
    dest = VStats(src1, dest, n, type);
    /*
    ** output
    */
    out_list = VCreateAttrList();
    VHistory(VNumber(options), options, prg_name, &list, &out_list);
    VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
    fp = VOpenOutputFile(out_filename, TRUE);
    if(! VWriteFile(fp, out_list))
        exit(1);
    fclose(fp);
    fprintf(stderr, "%s: done.\n", argv[0]);
    exit(0);
}
