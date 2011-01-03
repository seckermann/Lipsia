/*
** Regional homogeneity - ReHo
**
** G.Lohmann, Sept 2010
*/
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_cblas.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NSLICES 2000
#define TINY 1.0e-6


#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))

extern double VKendall_W(gsl_matrix *, int, int);


VAttrList
VReHo(VAttrList list, VShort minval, VShort first, VShort length) {
    VAttrList out_list = NULL;
    VAttrListPosn posn;
    VImage src[NSLICES];
    VImage dest = NULL;
    int b, r, c, bb, rr, cc, nslices, nrows, ncols, ntimesteps, last, i1, nt;
    int i, k, nvoxels, otype = 0;
    double v, *ptr;
    gsl_matrix *data = NULL;
    /*
    ** get image dimensions
    */
    i = ntimesteps = nrows = ncols = 0;
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src[i]);
        if(VPixelRepn(src[i]) != VShortRepn)
            continue;
        if(VImageNBands(src[i]) > ntimesteps)
            ntimesteps = VImageNBands(src[i]);
        if(VImageNRows(src[i])  > nrows)
            nrows = VImageNRows(src[i]);
        if(VImageNColumns(src[i]) > ncols)
            ncols = VImageNColumns(src[i]);
        i++;
        if(i >= NSLICES)
            VError(" too many slices,max is %d", NSLICES);
    }
    nslices = i;
    /* get time steps to include */
    if(length < 1)
        length = ntimesteps - 2;
    last = first + length - 1;
    if(last >= ntimesteps)
        last = ntimesteps - 1;
    if(first < 0)
        first = 1;
    nt = last - first + 1;
    i1 = first + 1;
    if(nt < 2)
        VError(" not enough timesteps, nt= %d", nt);
    fprintf(stderr, "# ntimesteps: %d, first= %d, last= %d, nt= %d\n",
            (int)ntimesteps, (int)first, (int)last, (int)nt);
    /*
    ** alloc dest image
    */
    dest = VCreateImage(nslices, nrows, ncols, VFloatRepn);
    if(!dest)
        VError(" error allocating dest image");
    VFillImage(dest, VAllBands, 0);
    VCopyImageAttrs(src[0], dest);
    VSetAttr(VImageAttrList(dest), "modality", NULL, VStringRepn, "conimg");
    VSetAttr(VImageAttrList(dest), "name", NULL, VStringRepn, "ReHo");
    nvoxels = 27;
    data = gsl_matrix_calloc(nvoxels, nt);
    /*
    ** main process loop
    */
    for(b = 1; b < nslices - 1; b++) {
        fprintf(stderr, " slice: %5d  of %d\r", b, nslices);
        if(VImageNBands(src[b]) < 2)
            continue;
        for(r = 1; r < nrows - 1; r++) {
            for(c = 1; c < ncols - 1; c++) {
                if(VPixel(src[b], 0, r, c, VShort) < minval)
                    continue;
                i = 0;
                for(bb = b - 1; bb <= b + 1; bb++) {
                    if(VImageNBands(src[bb]) < 2)
                        goto next;
                    for(rr = r - 1; rr <= r + 1; rr++) {
                        for(cc = c - 1; cc <= c + 1; cc++) {
                            if(VPixel(src[bb], 0, rr, cc, VShort) < minval)
                                goto next;
                            ptr = gsl_matrix_ptr(data, i, 0);
                            for(k = first; k <= last; k++) {
                                *ptr++ = (float) VPixel(src[bb], k, rr, cc, VShort);
                            }
                            i++;
                        }
                    }
                }
                v = VKendall_W(data, nt, otype);
                VPixel(dest, b, r, c, VFloat) = v;
next:
                ;
            }
        }
    }
    fprintf(stderr, "\n");
    /* output */
    out_list = VCreateAttrList();
    VAppendAttr(out_list, "image", NULL, VImageRepn, dest);
    return out_list;
}



int
main(int argc, char *argv[]) {
    static VShort   first  = 2;
    static VShort   length = 0;
    static VShort   minval = 0;
    static VOptionDescRec  options[] = {
        {"minval", VShortRepn, 1, (VPointer) &minval, VOptionalOpt, NULL, "signal threshold"},
        {"first", VShortRepn, 1, (VPointer) &first, VOptionalOpt, NULL, "first timestep to use"},
        {"length", VShortRepn, 1, (VPointer) &length, VOptionalOpt, NULL, "length of time series to use, '0' to use full length"}
    };
    FILE *in_file, *out_file;
    VAttrList list = NULL, out_list = NULL;
    char *prg = "vReHo";
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    /*
    ** read functional data
    */
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    /*
    ** process
    */
    out_list = VReHo(list, minval, first, length);
    /* output */
    VHistory(VNumber(options), options, prg, &list, &out_list);
    if(! VWriteFile(out_file, out_list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}
