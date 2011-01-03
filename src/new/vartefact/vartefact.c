/*
** artefact removal of fMRI time series
** replace damaged timepoints by chebychev approximation
**
** G.Lohmann, MPI-CBS, Oct 2010
*/
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <fftw3.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_chebyshev.h>


#define ABS(x) ((x) > 0 ? (x) : -(x))
struct my_params {
    int nt;
    double *array;
};

double
func(double x, void *params) {
    struct my_params *par = params;
    int i;
    i = (int)x;
    return par->array[i];
}

void
HighPassFilter(double *z, int n, float tr) {
    int i, nc;
    static double *in = NULL;
    static double *highp = NULL;
    static fftw_complex *out = NULL;
    double sharp = 0.8, x, alpha, high = 60;
    fftw_plan p1, p2;
    /* repetition time */
    nc  = (n / 2) + 1;
    if(out == NULL) {
        in  = (double *) VCalloc(n, sizeof(double));
        out = fftw_malloc(sizeof(fftw_complex) * nc);
    }
    for(i = 0; i < n; i++)
        in[i] = z[i];
    /* make plans */
    p1 = fftw_plan_dft_r2c_1d(n, in, out, FFTW_ESTIMATE);
    p2 = fftw_plan_dft_c2r_1d(n, out, in, FFTW_ESTIMATE);
    alpha = (double)n * tr;
    if(highp == NULL)
        highp = (double *) VCalloc(nc, sizeof(double));
    sharp = 0.8;
    for(i = 1; i < nc; i++) {
        highp[i] = 1.0 / (1.0 +  exp((alpha / high - (double)i) * sharp));
    }
    /* forward fft */
    fftw_execute(p1);
    /* highpass */
    for(i = 1; i < nc; i++) {
        x = highp[i];
        out[i][0] *= x;
        out[i][1] *= x;
    }
    /* inverse fft */
    fftw_execute(p2);
    for(i = 0; i < n; i++)
        z[i] = in[i] / (double)n;
}


void
VArtefact(VAttrList list, VFloat alpha, VFloat minval, VBoolean verbose) {
    VImage src;
    VAttrListPosn posn;
    VString str;
    int r, c, j, n, slice;
    float tr = 0;
    double val, sum1, sum2, nx, mean, sigma, xmin, xmax;
    double xi, yi, u, *z = NULL;
    int *flag = NULL;
    gsl_cheb_series *cs = NULL;
    gsl_function F;
    struct my_params params;
    xmin = (VShort) VRepnMinValue(VShortRepn);
    xmax = (VShort) VRepnMaxValue(VShortRepn);
    /*
    ** counting artefacts
    */
    slice = 0;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(VPixelRepn(src) != VShortRepn)
            continue;
        if(slice % 5 == 0)
            fprintf(stderr, " slice= %5d\r", slice);
        slice++;
        if(VImageNRows(src) < 2)
            continue;
        /* Get TR */
        if(tr == 0) {
            if(VGetAttr(VImageAttrList(src), "repetition_time", NULL, VFloatRepn, &tr) == VAttrFound)
                tr /= 1000.0;
            else {
                if(VGetAttr(VImageAttrList(src), "MPIL_vista_0", NULL, VStringRepn, (VPointer) & str) == VAttrFound) {
                    sscanf(str, " repetition_time=%f", &tr);
                    tr /= 1000.0;
                } else
                    VError(" TR unknown");
            }
        }
        n = VImageNBands(src);
        /* ini chebychev polynomials */
        if(cs == NULL) {
            cs = gsl_cheb_alloc(15);
            F.function = func;
            F.params = (void *)&params;
            params.nt = n;
            params.array = (double *) VCalloc(n, sizeof(double));
            z = (double *) VCalloc(n, sizeof(double));
            flag = (int *) VCalloc(n, sizeof(int));
        }
        for(r = 0; r < VImageNRows(src); r++) {
            for(c = 0; c < VImageNColumns(src); c++) {
                if(VPixel(src, 0, r, c, VShort) < minval) {
                    for(j = 0; j < n; j++)
                        VPixel(src, j, r, c, VShort) = 0;
                    goto next;
                }
                /* remove baseline drift */
                for(j = 0; j < n; j++)
                    z[j] = VPixel(src, j, r, c, VShort);
                HighPassFilter(z, n, tr);
                /* get mean,sigma */
                sum1 = sum2 = nx = 0;
                for(j = 0; j < n; j++) {
                    sum1 += z[j];
                    sum2 += z[j] * z[j];
                    nx++;
                }
                mean = sum1 / nx;
                sigma = sqrt((sum2 - nx * mean * mean) / (nx - 1.0));
                if(sigma < 0.01) {
                    for(j = 0; j < n; j++)
                        VPixel(src, j, r, c, VShort) = 0;
                    continue;
                }
                /* identify and mark artefacts */
                for(j = 0; j < n; j++) {
                    u = VPixel(src, j, r, c, VShort);
                    if((u > mean + alpha * sigma) || (u < mean - alpha * sigma)) {
                        params.array[j] = mean;
                        flag[j] = 1;
                    } else if(u < minval) {
                        params.array[j] = minval;
                        flag[j] = 1;
                    } else {
                        params.array[j] = u;
                        flag[j] = 0;
                    }
                }
                /* replace damaged timepoints by chebychev approximation */
                gsl_cheb_init(cs, &F, 0.0, (double)n);
                for(j = 0; j < n; j++) {
                    if(flag[j] == 0)
                        continue;
                    xi = (double)j;
                    yi = gsl_cheb_eval(cs, xi);
                    if(yi < minval)
                        yi = 0;
                    if(verbose) {
                        u = VPixel(src, j, r, c, VShort);
                        fprintf(stderr, "# artefact addr %3d %3d %3d,  %5d: %f  %f\n",
                                slice - 1, r, c, j, u, yi);
                    }
                    val = (int)(yi + 0.49);
                    if(val > xmax)
                        val = xmax;
                    if(val < xmin)
                        val = xmin;
                    VPixel(src, j, r, c, VShort) = val;
                }
next:
                ;
            }
        }
    }
}


int
main(int argc, char *argv[]) {
    static VFloat alpha  = 3.5;
    static VFloat minval = 0;
    static VBoolean verbose = FALSE;
    static VOptionDescRec  options[] = {
        {"alpha", VFloatRepn, 1, (VPointer) &alpha, VOptionalOpt, NULL, "alpha"},
        {"minval", VFloatRepn, 1, (VPointer) &minval, VOptionalOpt, NULL, "Signal threshold"},
        {"verbose", VBooleanRepn, 1, (VPointer) &verbose, VOptionalOpt, NULL, "verbose"}
    };
    FILE *in_file = NULL, *out_file = NULL;
    VAttrList list = NULL;
    char *prg = "vartefact";
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    /* repair artefact timesteps */
    VArtefact(list, alpha, minval, verbose);
    /* output */
    VHistory(VNumber(options), options, prg, &list, &list);
    if(! VWriteFile(out_file, list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    exit(0);
}
