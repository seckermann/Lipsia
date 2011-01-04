/*
** spectral clustering
**
** Lit:
**   Ulrike von Luxburg, Tech Report TR-149, MPI-KYB
**
**   Shi,J. and Malik,J. (2000),
**   Normalized cuts and image segmentation,
**   IEEE-PAMI, 22(8), pp. 888-905
**
** G.Lohmann, May 2008
*/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sort.h>


#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SQR(x) ((x) * (x))

extern int *KMeans(gsl_matrix *, int, double *, double *);
extern gsl_matrix *dmat_x_mat(gsl_matrix *, gsl_matrix *, gsl_matrix *);
extern gsl_matrix *L_rw(VImage, gsl_matrix *, int, int);
extern gsl_matrix *L_sym(VImage, gsl_matrix *, int, int, VBoolean);
extern char *getLipsiaVersion();

void
vprintmat(gsl_matrix *A) {
    int i, j, n, m;
    n = A->size1;
    m = A->size2;
    n = m = 7;
    for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            fprintf(stderr, " %8.4f", gsl_matrix_get(A, i, j));
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}


VImage
VSpectralClustering(VImage src, int nclusters, VShort ntype, VBoolean raster, VBoolean lapack) {
    VImage dest = NULL, map = NULL, dmap = NULL;
    int *labels = NULL;
    int i, j, nvectors;
    int b, r, c, nslices, nrows, ncols;
    double u, v, aic = 0, bic = 0;
    gsl_matrix *cmat = NULL;
    if(VImageNRows(src) != VImageNColumns(src))
        VError(" matrix must be square");
    nvectors = VImageNRows(src);
    /*
    ** similarity matrix
    */
    for(i = 0; i < nvectors; i++) {
        for(j = 0; j <= i; j++) {
            u = VGetPixel(src, 0, i, j);
            v = VGetPixel(src, 0, j, i);
            if(u < 0)
                VSetPixel(src, 0, i, j, 0);
            if(v < 0)
                VSetPixel(src, 0, j, i, 0);
            if(ABS(u - v) > 0.0001)
                VError("matrix must be symmetric");
        }
        VSetPixel(src, 0, i, i, 0);
    }
    /*
    ** use laplacian matrix to get feature vectors for kmeans
    */
    switch(ntype) {
    case 0:     /* L_rw, Shi,Malik, 2000 */
        cmat = L_rw(src, cmat, nvectors, nclusters);
        break;
    case 1:     /* L_sym, Ng,Jordan,Weiss, 2002 */
        cmat = L_sym(src, cmat, nvectors, nclusters, lapack);
        break;
    default:
        VError(" illegal type %d", ntype);
    }
    /*
    ** k-means clustering
    */
    labels = KMeans(cmat, (int)nclusters, &bic, &aic);
    /*
    ** output
    */
    if(VGetAttr(VImageAttrList(src), "map", NULL, VImageRepn, &map) != VAttrFound)
        VError(" map not found");
    dmap = VCopyImage(map, NULL, VAllBands);
    for(i = 0; i < nvectors; i++) {
        VPixel(map, 0, 4, i, VFloat)  = (VFloat)(labels[i] + 1);
        VPixel(dmap, 0, 4, i, VFloat) = (VFloat)(labels[i] + 1);
    }
    /* if no raster image for output, return now */
    VSetAttr(VImageAttrList(dmap), "name", NULL, VStringRepn, "cluster_map");
    if(raster == FALSE)
        return dmap;
    nslices = VPixel(map, 0, 3, 0, VFloat);
    nrows   = VPixel(map, 0, 3, 1, VFloat);
    ncols   = VPixel(map, 0, 3, 2, VFloat);
    fprintf(stderr, " dims: %d %d %d\n", nslices, nrows, ncols);
    dest = VCreateImage(nslices, nrows, ncols, VFloatRepn);
    if(!dest)
        VError(" err allocating output image, dims: %d %d %d\n", nslices, nrows, ncols);
    VCopyImageAttrs(src, dest);
    VFillImage(dest, VAllBands, 0);
    for(i = 0; i < nvectors; i++) {
        b = VPixel(map, 0, 0, i, VFloat);
        r = VPixel(map, 0, 1, i, VFloat);
        c = VPixel(map, 0, 2, i, VFloat);
        if(b < 0 || b >= nslices)
            VError(" illegal slice coord %d", b);
        if(r < 0 || r >= nrows)
            VError(" illegal row coord %d", r);
        if(c < 0 || c >= ncols)
            VError(" illegal column coord %d", c);
        VPixel(dest, b, r, c, VFloat) = (VFloat)(labels[i] + 1);
    }
    VSetAttr(VImageAttrList(src), "map", NULL, VImageRepn, map);
    VSetAttr(VImageAttrList(src), "name", NULL, VStringRepn, "similarity_matrix");
    VSetAttr(VImageAttrList(dest), "map", NULL, VImageRepn, dmap);
    VSetAttr(VImageAttrList(dest), "name", NULL, VStringRepn, "clustering");
    return dest;
}


VDictEntry TypeDict[] = {
    { "Shi", 0 },
    { "Ng", 1 },
    { NULL }
};

int
main(int argc, char *argv[]) {
    static VBoolean lapack = TRUE;
    static VShort   nclusters = 3;
    static VBoolean raster = TRUE;
    static VShort   ntype = 1;
    static VOptionDescRec  options[] = {
        {"n", VShortRepn, 1, (VPointer) &nclusters, VOptionalOpt, NULL, "number of clusters"},
        /* {"raster",VBooleanRepn,1,(VPointer) &raster,VOptionalOpt,NULL,"Whether to output a raster image"}, */
        /* {"lapack",VBooleanRepn,1,(VPointer) &lapack,VOptionalOpt,NULL,"Whether to use lapack"}, */
        {
            "type", VShortRepn, 1, (VPointer) &ntype, VOptionalOpt, TypeDict,
            "Type of algorithm to use"
        }
    };
    FILE *in_file, *out_file;
    VAttrList list = NULL;
    VAttrListPosn posn;
    VImage src = NULL, dest = NULL, map = NULL;
    /* char *prg = "vspectralcluster: $Revision: 0.0 $"; */
	char prg_name[100];
	sprintf(prg_name, "vspectralcluster V%s", getLipsiaVersion());
	fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    if(nclusters < 2)
        VError(" number of clusters too small");
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        /* if (VPixelRepn(src) != VFloatRepn) continue; */
        if(VImageNRows(src) != VImageNColumns(src))
            continue;
        if(VImageNBands(src) > 1)
            continue;
        if(VGetAttr(VImageAttrList(src), "map", NULL, VImageRepn, &map) != VAttrFound)
            continue;
        dest = VSpectralClustering(src, nclusters, ntype, raster, lapack);
        VSetAttrValue(& posn, NULL, VImageRepn, dest);
        if(raster)
            VAppendAttr(list, "cluster", NULL, VImageRepn, src);
        break;
    }
    if(dest == NULL)
        VError(" similarity matrix not found");
    VHistory(VNumber(options), options, prg_name, &list, &list);
    if(! VWriteFile(out_file, list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}
