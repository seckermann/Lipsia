/*
** transform data to make them Gaussian normal distributed
**
** Ref: van Albada, Robinson (2007).
**    J. Neuroscience Methods, 161,205-211.
**
** G.Lohmann, MPI-CBS, Oct 2009
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
#include <gsl/gsl_cdf.h>

#define NSLICES 500   /* max number of slices */

#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))


/*
double
inverse_erf(double x)
{
  if (x <= -1) return -1/sqrt(2.0);
  if (x >=  1) return 1/sqrt(2.0);
  return gsl_cdf_ugaussian_Pinv((x+1)*0.5)/sqrt(2.0);
}
*/

double
Pinv(double x) {
    if(x <= -1)
        return -1;
    if(x >=  1)
        return 1;
    return gsl_cdf_ugaussian_Pinv(x);
}


void
VTransNormal(VImage *src, int n) {
    int i, j, k, b, r, c, nslices, nrows, ncols;
    double u, nx, sum, sum1, sum2, mean, sigma;
    double smax, smin, tiny = 1.0e-6;
    double *x = NULL, *edf = NULL;
    nslices = VImageNBands(src[0]);
    nrows   = VImageNRows(src[0]);
    ncols   = VImageNColumns(src[0]);
    smax = VPixelMaxValue(src[0]);
    smin = VPixelMinValue(src[0]);
    nx   = (double)n;
    x    = (double *) VCalloc(n, sizeof(double));
    edf  = (double *) VCalloc(n, sizeof(double));
    for(b = 0; b < nslices; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                k = 0;
                for(i = 0; i < n; i++) {
                    u = VGetPixel(src[i], b, r, c);
                    x[i] = u;
                    if(ABS(u) > tiny)
                        k++;
                }
                if(k < n)
                    continue;
                /* empirical mean, std */
                sum1 = sum2 = 0;
                for(i = 0; i < n; i++) {
                    sum1 += x[i];
                    sum2 += x[i] * x[i];
                }
                mean = sum1 / nx;
                sigma = sqrt((sum2 - nx * mean * mean) / (nx - 1.0));
                /* empirical distribution function */
                for(i = 0; i < n; i++) {
                    sum = 0;
                    u = x[i];
                    for(j = 0; j < n; j++)
                        if(x[j] <= u)
                            sum++;
                    edf[i] = sum / nx;
                }
                /* apply transform to normal distribution */
                for(i = 0; i < n; i++) {
                    u = mean + sigma * Pinv(edf[i]);
                    if(isnan(u))
                        u = 0;
                    if(isinf(u))
                        u = 0;
                    if(u < smin)
                        u = smin;
                    if(u > smax)
                        u = smax;
                    VSetPixel(src[i], b, r, c, u);
                }
            }
        }
    }
}



int main(int argc, char *argv[]) {
    static VArgVector in_files;
    static VOptionDescRec options[] = {
        { "in", VStringRepn, 0, & in_files, VRequiredOpt, NULL, "Input files" }
    };
    FILE *fp = NULL;
    VStringConst in_filename;
    VString out_filename;
    VAttrList list;
    VAttrListPosn posn;
    VImage src, *src1;
    int i, j, n = 0;
    char *prg_name = "vgaussianize";
    fprintf(stderr, "%s\n", prg_name);
    if(!VParseCommand(VNumber(options), options, & argc, argv)) {
        VReportUsage(argv[0], VNumber(options), options, NULL);
        exit(0);
    }
    /* read images  */
    n = in_files.number;
    src1 = (VImage *) VMalloc(sizeof(VImage) * n);
    for(i = 0; i < n; i++) {
        in_filename = ((VStringConst *) in_files.vector)[i];
        fprintf(stderr, " %3d:  %s\n", i, in_filename);
        fp = VOpenInputFile(in_filename, TRUE);
        list = VReadFile(fp, NULL);
        if(! list)
            VError("Error reading image");
        fclose(fp);
        src1[i] = NULL;
        for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & src);
            src1[i] = src;
            break;
        }
    }
    /* transformation to normality */
    VTransNormal(src1, n);
    /* output */
    out_filename = (char *) VMalloc(1024);
    for(i = 0; i < n; i++) {
        in_filename = ((VStringConst *) in_files.vector)[i];
        fp = VOpenInputFile(in_filename, TRUE);
        list = VReadFile(fp, NULL);
        if(! list)
            VError("Error reading image");
        fclose(fp);
        for(j = 0; j < 1024; j++)
            out_filename[j] = '\0';
        sprintf(out_filename, "gauss_%s", in_filename);
        fprintf(stderr, " %3d:  %s\n", i, out_filename);
        for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
            if(VGetAttrRepn(& posn) != VImageRepn)
                continue;
            VGetAttrValue(& posn, NULL, VImageRepn, & src);
            VSetAttrValue(& posn, NULL, VImageRepn, src1[i]);
            break;
        }
        fp = VOpenOutputFile(out_filename, TRUE);
        if(! VWriteFile(fp, list))
            exit(1);
        fclose(fp);
    }
    fprintf(stderr, "%s: done.\n", argv[0]);
    exit(0);
}
