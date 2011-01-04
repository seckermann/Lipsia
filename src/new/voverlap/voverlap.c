/*
** get overlap between different clusterings
**
**  G.Lohmann, MPI-CBS, 2008
*/


#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_permutation.h>


#define ABS(x) ((x) > 0 ? (x) : -(x))

extern char *getLipsiaVersion();

/*
double
Dist1(VImage map1,VImage map2,int *lut)
{
  int i,ii,jj;
  double sum;

  sum = 0;
  for (i=0; i<VImageNColumns(map1); i++) {
    ii = VPixel(map1,0,4,i,VFloat);
    ii--;
    jj = VPixel(map2,0,4,i,VFloat);
    jj--;
    ii = lut[ii];

    if (ii == jj) sum++;
  }
  return sum;
}
*/


double
Dist(VImage map1, VImage map2, int *lut) {
    int i, j, ii, jj, j0;
    int b, r, c, bb, rr, cc;
    double sum;
    sum = 0;
    for(i = 0; i < VImageNColumns(map1); i++) {
        ii = VPixel(map1, 0, 4, i, VFloat);
        b = VPixel(map1, 0, 0, i, VFloat);
        r = VPixel(map1, 0, 1, i, VFloat);
        c = VPixel(map1, 0, 2, i, VFloat);
        j0 = -1;
        for(j = 0; j < VImageNColumns(map2); j++) {
            bb = VPixel(map2, 0, 0, j, VFloat);
            rr = VPixel(map2, 0, 1, j, VFloat);
            cc = VPixel(map2, 0, 2, j, VFloat);
            if(bb == b && rr == r && cc == c) {
                j0 = j;
                break;
            }
        }
        if(j0 < 0)
            VError(" maps are inconsistent");
        jj = VPixel(map2, 0, 4, j0, VFloat);
        ii--;
        jj--;
        ii = lut[ii];
        if(ii == jj)
            sum++;
    }
    return sum;
}


int
main(int argc, char *argv[]) {
    static VBoolean verbose = FALSE;
    static VString filename = "";
    static VOptionDescRec  options[] = {
        {"ref", VStringRepn, 1, (VPointer) &filename, VRequiredOpt, NULL, "reference cluster result"},
        {"verbose", VBooleanRepn, 1, (VPointer) &verbose, VOptionalOpt, NULL, "verbose"},
    };
    FILE *in_file, *out_file, *fp;
    VAttrList list1 = NULL, list2 = NULL;
    VAttrListPosn posn;
    VImage src, src1 = NULL, map1 = NULL;
    VImage src2 = NULL, map2 = NULL;
    VImage hsrc = NULL, wsrc = NULL, Hsrc = NULL, Wsrc = NULL;
    gsl_permutation *perm = NULL, *p0 = NULL;
    int i, j, ii, n1, n2, dim1, dim2;
    int *tab = NULL, *lut = NULL;
    int b, r, c;
    double u, umax;
	char prg_name[100];
	sprintf(prg_name, "voverlap V%s", getLipsiaVersion());
	fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    /*
    ** read 1st file
    */
    if(!(list1 = VReadFile(in_file, NULL)))
        exit(1);
    fclose(in_file);
    for(VFirstAttr(list1, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src1);
        if(VGetAttr(VImageAttrList(src1), "map", NULL, VImageRepn, &map1) != VAttrFound)
            continue;
        break;
    }
    if(map1 == NULL)
        VError(" map 1 not found");
    /*
    ** read 2nd file
    */
    fp = VOpenInputFile(filename, TRUE);
    list2 = VReadFile(fp, NULL);
    if(! list2)
        VError("Error reading image");
    fclose(fp);
    for(VFirstAttr(list2, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src2);
        if(VGetAttr(VImageAttrList(src2), "map", NULL, VImageRepn, &map2) != VAttrFound)
            continue;
        break;
    }
    if(map2 == NULL)
        VError(" map 2 not found");
    n1 = VImageNColumns(map1);
    n2 = VImageNColumns(map2);
    if(n1 != n2)
        VError(" n1 != n2");
    dim1 = 0;
    for(i = 0; i < n1; i++) {
        j = VPixel(map1, 0, 4, i, VFloat);
        if(j > dim1)
            dim1 = j;
    }
    dim2 = 0;
    for(i = 0; i < n2; i++) {
        j = VPixel(map2, 0, 4, i, VFloat);
        if(j > dim2)
            dim2 = j;
    }
    tab = (int *) VCalloc(dim1, sizeof(int));
    lut = (int *) VCalloc(dim1, sizeof(int));
    /*
    ** loop thru permutations
    */
    perm = gsl_permutation_alloc(dim1);
    gsl_permutation_init(perm);
    p0 = gsl_permutation_alloc(dim1);
    gsl_permutation_init(p0);
    umax = 0;
    do {
        for(i = 0; i < dim1; i++)
            lut[i] = gsl_permutation_get(perm, i);
        u = Dist(map1, map2, lut);
        if(u > umax) {
            p0 = perm;
            umax = u;
            for(i = 0; i < dim1; i++)
                tab[i] = lut[i];
        }
    } while(gsl_permutation_next(perm) == GSL_SUCCESS);
    for(i = 0; i < n1; i++) {
        ii = VPixel(map1, 0, 4, i, VFloat);
        VPixel(map1, 0, 4, i, VFloat) = (VFloat) tab[ii - 1] + 1;
    }
    for(VFirstAttr(list1, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        if(VPixelRepn(src) != VFloatRepn)
            continue;
        if(strncmp(VGetAttrName(&posn), "H", 1) == 0)
            continue;
        if(strncmp(VGetAttrName(&posn), "W", 1) == 0)
            continue;
        for(i = 0; i < n1; i++) {
            b = VPixel(map1, 0, 0, i, VFloat);
            r = VPixel(map1, 0, 1, i, VFloat);
            c = VPixel(map1, 0, 2, i, VFloat);
            ii = VPixel(map1, 0, 4, i, VFloat);
            VPixel(src, b, r, c, VFloat) = (VFloat)ii;
        }
        VSetAttrValue(& posn, NULL, VImageRepn, src);
        break;
    }
    for(VFirstAttr(list1, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & hsrc);
        if(VPixelRepn(src) != VFloatRepn)
            continue;
        if(strncmp(VGetAttrName(&posn), "H", 1) != 0)
            continue;
        Hsrc = VCopyImage(hsrc, NULL, VAllBands);
        for(i = 0; i < VImageNRows(hsrc); i++) {
            ii = gsl_permutation_get(p0, i);
            for(j = 0; j < VImageNColumns(hsrc); j++) {
                u = VGetPixel(hsrc, 0, i, j);
                VSetPixel(Hsrc, 0, ii, j, u);
            }
        }
        VSetAttrValue(& posn, NULL, VImageRepn, Hsrc);
        break;
    }
    for(VFirstAttr(list1, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & wsrc);
        if(VPixelRepn(src) != VFloatRepn)
            continue;
        if(strncmp(VGetAttrName(&posn), "W", 1) != 0)
            continue;
        Wsrc = VCopyImage(wsrc, NULL, VAllBands);
        for(i = 0; i < VImageNColumns(wsrc); i++) {
            ii = gsl_permutation_get(p0, i);
            for(j = 0; j < VImageNRows(wsrc); j++) {
                u = VGetPixel(wsrc, 0, j, i);
                VSetPixel(Wsrc, 0, j, ii, u);
            }
        }
        VSetAttrValue(& posn, NULL, VImageRepn, Wsrc);
        break;
    }
    if(! VWriteFile(out_file, list1))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}


