/*
** power spectral density (PSD)
**
** G.Lohmann, MPI-CBS, Dec 2006
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

#define ABS(x) ((x) > 0 ? (x) : -(x))

extern void getLipsiaVersion(char*,size_t);

VDictEntry TYPEDict[] = {
    { "range", 0 },
    { "median", 1 },
    { "centroid", 2 },
    { "overall", 3 },
    { NULL }
};


int
main(int argc, char *argv[]) {
    static VFloat  tr = -1;
    static VShort  minval = 0;
    static VShort  type = 0;
    static VDouble low  = -1;
    static VDouble high = -1;
    static VOptionDescRec  options[] = {
        {"tr", VFloatRepn, 1, (VPointer) &tr, VOptionalOpt, NULL, "temporal resolution in seconds"},
        {"type", VShortRepn, 1, (VPointer) &type, VOptionalOpt, TYPEDict, "type of output"},
        {"low", VDoubleRepn, 1, (VPointer) &low, VOptionalOpt, NULL, "low threshold in secs"},
        {"high", VDoubleRepn, 1, (VPointer) &high, VOptionalOpt, NULL, "high threshold in secs"},
        {"minval", VShortRepn, 1, (VPointer) &minval, VOptionalOpt, NULL, "minval"}
    };
    FILE *in_file = NULL, *out_file = NULL;
    VAttrList list = NULL, out_list = NULL;
    VAttrListPosn posn;
    VImage src = NULL, dest = NULL;
    double *in = NULL, *out = NULL;
    fftw_plan p1;
    int i, j, k, n, nc, r, c, nrows, ncols;
    float freq, f0, f1, xtr;
    double alpha;
    double v = 0, u, vsum, wsum, sum;
	char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vpowerspectrum V%s", ver);
	fprintf(stderr, "%s\n", prg_name);    
	VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    /* get image dimensions */
    n = k = nrows = ncols = 0;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(VPixelRepn(src) != VShortRepn)
            continue;
        if(VImageNBands(src) > n)
            n = VImageNBands(src);
        if(VImageNRows(src) > nrows)
            nrows = VImageNRows(src);
        if(VImageNColumns(src) > ncols)
            ncols = VImageNColumns(src);
        k++;
    }
    if(VGetAttr(VImageAttrList(src), "repetition_time", NULL,
                VFloatRepn, (VPointer) & xtr) == VAttrFound) {
        tr = (xtr / 1000.0);
    }
    if(tr < 0)
        VError(" illegal tr: %f", tr);
    fprintf(stderr, " TR= %g secs\n", tr);
    /* get freq range */
    alpha = 1.0 / ((double)n * tr);
    f0 = tr * 0.5;
    f1 = 0;
    if(low >= 0)
        f0 = 1.0 / low;    /* schnell */
    if(high >= 0)
        f1 = 1.0 / high;    /* langsam */
    if(type < 3) {
        fprintf(stderr, " frequency range:  [%.4f,%.4f]\n", f0, f1);
        if(low > high)
            VError(" illegal freq range: %f %f", low, high);
    }
    /* create output image */
    dest = VCreateImage(k, nrows, ncols, VFloatRepn);
    VFillImage(dest, VAllBands, 0);
    /* alloc memory */
    nc  = (n / 2) + 1;
    in  = (double *)fftw_malloc(sizeof(double) * n);
    out = (double *)fftw_malloc(sizeof(double) * n);
    for(i = 0; i < n; i++)
        in[i] = out[i] = 0;
    /* make plans */
    p1 = fftw_plan_r2r_1d(n, in, out, FFTW_R2HC, FFTW_FORWARD);
    alpha = 1.0 / ((double)n * tr);
    k = -1;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(VPixelRepn(src) != VShortRepn)
            continue;
        if(k < 0)
            VCopyImageAttrs(src, dest);
        k++;
        if(VImageNRows(src) < 2)
            continue;
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                /* get data */
                if(VPixel(src, 0, r, c, VShort) < minval)
                    continue;
                for(j = 0; j < n; j++)
                    in[j] = VPixel(src, j, r, c, VShort);
                /*  fft */
                fftw_execute(p1);
                v = vsum = 0;
                switch(type) {
                    /* percentage of power inside given range */
                case 0:
                    v = vsum = 0;
                    for(i = 1; i < nc; i++) {
                        freq = (double)i * alpha;
                        u = (out[i] * out[i] + out[n - i] * out[n - i]);
                        if(freq <= f0 && freq >= f1)
                            v += u;
                        vsum += u;
                    }
                    if(vsum > 0)
                        v /= vsum;
                    v *= 100.0;
                    VPixel(dest, k, r, c, VFloat) = v;
                    break;
                    /* get spectral median */
                case 1:
                    vsum = 0;
                    for(i = 1; i < nc; i++)
                        vsum += (out[i] * out[i] + out[n - i] * out[n - i]);
                    sum = v = 0;
                    for(i = 1; i < nc; i++) {
                        freq = (double)i * alpha;
                        if(freq <= f0 && freq >= f1) {
                            sum += (out[i] * out[i] + out[n - i] * out[n - i]) / vsum;
                            if(sum > 0.5) {
                                v = 1.0 / freq;
                                goto skip;
                            }
                        }
                    }
skip:
                    VPixel(dest, k, r, c, VFloat) = v;
                    break;
                    /* spectral centroid */
                case 2:
                    vsum = 0;
                    for(i = 1; i < nc; i++) {
                        freq = (double)i * alpha;
                        if(freq <= f0 && freq >= f1)
                            vsum += (out[i] * out[i] + out[n - i] * out[n - i]);
                    }
                    wsum = 0;
                    for(i = 1; i < nc; i++) {
                        freq = (double)i * alpha;
                        if(freq <= f0 && freq >= f1)
                            v = (out[i] * out[i] + out[n - i] * out[n - i]);
                        wsum += v / freq;
                    }
                    v = 0;
                    if(vsum > 0)
                        v = wsum / vsum;
                    VPixel(dest, k, r, c, VFloat) = v;
                    break;
                    /* get overall power */
                case 3:
                    vsum = 0;
                    for(i = 1; i < nc; i++)
                        vsum += (out[i] * out[i] + out[n - i] * out[n - i]);
                    v = 10000.0 * (double)n;
                    v = vsum / v;
                    VPixel(dest, k, r, c, VFloat) = v;
                    break;
                default:
                    VError(" illegal type");
                }
            }
        }
    }
    out_list = VCreateAttrList();
    VSetAttr(VImageAttrList(dest), "modality", NULL, VStringRepn, "conimg");
    VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
    VHistory(VNumber(options), options, prg_name, &list, &out_list);
    if(! VWriteFile(out_file, out_list))
        exit(1);
    fprintf(stderr, " %s: done.\n", argv[0]);
    exit(0);
}
