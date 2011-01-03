/*
** Multiple comparison correction using monte carlo simulations.
** Using 2 features: cluster size + max z-val per cluster
**
** G.Lohmann, MPI-CBS
*/
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_statistics.h>



#define CSIZE   2000   /* max cluster size */
#define MSIZE    500   /* max cluster zval */

#define SQR(x) ((x) * (x))
#define ABS(x) ((x) > 0 ? (x) : -(x))



/*
** convert z to p values
*/
double z2p(double z) {
    if(z < 0)
        return gsl_cdf_ugaussian_Q(-z);
    else
        return gsl_cdf_ugaussian_Q(z);
}


/*
** convert p to z values
*/
double p2z(double p) {
    return gsl_cdf_ugaussian_Qinv(p);
}





VImage
VMulticomp2d(VImage src, VImage dest, VLong seed, VLong numiter,
             VDouble fwhm, VDouble z0, VDouble p0) {
    VImage tmp = NULL, bin_image = NULL, label_image = NULL, kernel = NULL, mask = NULL;
    VImage zdest = NULL;
    VString buf = NULL;
    VFloat *dst_pp;
    VShort *short_pp;
    VBit *bin_pp, *bit_pp;
    int i, j = 0, k, b, r, c, nbands, nrows, ncols, npixels, nl, iter, nvox;
    int ii, jj, msize, csize;
    double u, zz, px, pthr, zthr, sum1, sum2, nx, mean, sig;
    double sigma = 0, sig0 = 0, zthr0 = 0, zscale = 0;
    float ptable[CSIZE][MSIZE], histo[CSIZE][MSIZE];
    float *clustersize = NULL, *clustermax = NULL;
    float tiny = 1.0e-7;
    float x, y, z, voxsize = 1;
    int tabsize = 0, cmaxsize = 0, zmaxsize = 0;
    gsl_rng *rx = NULL;
    const gsl_rng_type *T = NULL;
    extern VImage VGauss3d(VImage, VImage, VImage);
    extern VImage VSGaussKernel(double);
    extern void VZeroBorders(VImage, VImage);
    extern VImage VLabelImage3d(VImage, VImage, int, VRepnKind, int *);
    /*
    ** read smoothness estimation, if not specified on command line
    */
    /*
    if (fwhm < 0) {
      if (VGetAttr (VImageAttrList (src), "smoothness", NULL,
        VFloatRepn, (VPointer) & smoothness) == VAttrFound) {
        fwhm = smoothness;
      }
      else
        VError(" Please specify smoothness in fwhm");
    }
    */
    cmaxsize = zmaxsize = 0;
    msize = MSIZE;
    csize = CSIZE;
    for(i = 0; i < csize; i++)
        for(j = 0; j < msize; j++)
            ptable[i][j] = histo[i][j] = 0;
    /*
    ** ini data structs
    */
    pthr = z2p(z0);
    fprintf(stderr, "\n");
    fprintf(stderr, "  fwhm: %.3f mm,  seed: %d,  numiter: %d\n", fwhm, seed, numiter);
    fprintf(stderr, "  z: %.5f,  p: %.7f\n", z0, pthr);
    nbands = VImageNBands(src);
    nrows  = VImageNRows(src);
    ncols  = VImageNColumns(src);
    npixels = nbands * nrows * ncols;
    fprintf(stderr, "  image dims:  %d %d %d\n", ncols, nrows, nbands);
    mask = VCreateImage(nbands, nrows, ncols, VBitRepn);
    VFillImage(mask, VAllBands, 0);
    nvox = 0;
    for(b = 0; b < nbands; b++) {
        for(r = 0; r < nrows; r++) {
            for(c = 0; c < ncols; c++) {
                u = VGetPixel(src, b, r, c);
                if(ABS(u) > tiny) {
                    VPixel(mask, b, r, c, VBit) = 1;
                    nvox++;
                }
            }
        }
    }
    fprintf(stderr, "  num voxels:  %d\n", nvox);
    voxsize = x = y = z = 1;
    if(VGetAttr(VImageAttrList(src), "voxel", NULL,
                VStringRepn, (VPointer) & buf) == VAttrFound) {
        sscanf(buf, "%f %f %f", &x, &y, &z);
        voxsize = x * y * z;
        fprintf(stderr, "  voxel size:  %.2f x %.2f x %.2f = %.2f mm^3\n", x, y, z, voxsize);
    }
    sigma = fwhm / 2.35482;
    sigma /= x;
    fprintf(stderr, "\n");
    tabsize = npixels / 2;
    clustersize = (float *) VCalloc(tabsize, sizeof(float));
    clustermax = (float *) VCalloc(tabsize, sizeof(float));
    dest = VCreateImage(nbands, nrows, ncols, VFloatRepn);
    tmp  = VCreateImage(nbands, nrows, ncols, VFloatRepn);
    bin_image = VCreateImage(nbands, nrows, ncols, VBitRepn);
    VFillImage(dest, VAllBands, 0);
    VFillImage(tmp, VAllBands, 0);
    VFillImage(bin_image, VAllBands, 0);
    if(sigma > 0) {
        kernel = VSGaussKernel(sigma);
        VZeroBorders(mask, kernel);
    }
    /*
    ** ini random number generator
    */
    gsl_rng_env_setup();
    T  = gsl_rng_default;
    rx = gsl_rng_alloc(T);
    gsl_rng_set(rx, (unsigned long int)seed);
    /*
    ** main loop
    */
    sig0 = zthr0 = 0;
    for(iter = 0; iter < numiter; iter++) {
        if(iter % 10 == 0)
            fprintf(stderr, " iter: %5d\r", iter);
        /*
        ** fill with random numbers
        */
        dst_pp = VImageData(dest);
        for(i = 0; i < npixels; i++) {
            *dst_pp++ = gsl_ran_ugaussian(rx);
        }
        /*
        ** Gauss filter
        */
        if(sigma > 0)
            tmp = VGauss3d(dest, tmp, kernel);
        else
            tmp = VCopyImage(dest, tmp, VAllBands);
        /*
        ** sample mean, std
        */
        sum1 = sum2 = nx = 0;
        dst_pp = VImageData(tmp);
        bin_pp = VImageData(mask);
        for(i = 0; i < npixels; i++) {
            u = *dst_pp++;
            if(*bin_pp > 0) {
                sum1 += u;
                sum2 += u * u;
                nx++;
            }
            bin_pp++;
        }
        mean = sum1 / nx;
        sig  = sqrt((sum2 - nx * mean * mean) / (nx - 1.0));
        /*
        ** get threshold zthr
        */
        zthr = gsl_cdf_gaussian_Qinv(pthr, sig);
        /*
        ** get standard parameters
        */
        if(iter == 0) {
            sig0  = sig;
            zthr0 = zthr;
            zscale = 100.0 / sig0;
        }
        /*
        ** get connected components
        */
        dst_pp = VImageData(tmp);
        bin_pp = VImageData(bin_image);
        bit_pp = VImageData(mask);
        for(i = 0; i < npixels; i++) {
            *bin_pp = 0;
            u = *dst_pp++;
            if(u > zthr && *bit_pp > 0)
                *bin_pp = 1;
            bin_pp++;
            bit_pp++;
        }
        label_image = VLabelImage3d(bin_image, label_image, 26, VShortRepn, &nl);
        if(nl < 1)
            continue;    /* no clusters found */
        if(nl >= tabsize)
            VError(" table too small");
        /*
        ** get size and max value per cluster
        */
        for(i = 0; i < tabsize; i++)
            clustersize[i] = clustermax[i] = 0;
        short_pp = VImageData(label_image);
        dst_pp = VImageData(tmp);
        for(i = 0; i < npixels; i++) {
            j = *short_pp++;
            u = *dst_pp++;
            if(j >= tabsize)
                VError(" table too small, %d", j);
            if(j > 0) {
                clustersize[j]++;
                if(u > clustermax[j])
                    clustermax[j] = u;
            }
        }
        /*
        ** get dimensions of ptable and histo
        */
        csize = msize = 0;
        for(i = 0; i < tabsize; i++) {
            if(clustersize[i] > csize)
                csize = clustersize[i];
            u = clustermax[i];
            k = (int)((u - zthr0) * zscale + 1.0);
            if(k > msize)
                msize = k;
        }
        msize += 2;
        csize += 2;
        if(msize > MSIZE)
            msize = MSIZE;
        if(csize >= CSIZE) {
            VWarning(" clustersize too large %d %d\n", csize, CSIZE);
            csize = CSIZE;
        }
        if(csize > cmaxsize)
            cmaxsize = csize;
        if(msize > zmaxsize)
            zmaxsize = msize;
        /*
        ** get 2D histogram of cluster features
        */
        for(i = 0; i < csize; i++) {
            for(j = 0; j < msize; j++) {
                histo[i][j] = 0;
            }
        }
        for(i = 1; i < tabsize; i++) {
            if(clustersize[i] < 1)
                continue;
            j = clustersize[i];
            if(j >= csize)
                j = csize - 1;
            u = clustermax[i];
            k = (int)((u - zthr0) * zscale + 1.0);
            if(k >= msize)
                k = msize - 1;
            if(k < 0)
                k = 0;
            histo[j][k]++;
        }
        /*
        ** fill p-table
        */
        for(i = 1; i < csize; i++) {
            for(j = 1; j < msize; j++) {
                for(ii = i; ii < csize; ii++) {
                    for(jj = j; jj < msize; jj++) {
                        if(histo[ii][jj] > 0) {
                            ptable[i][j]++;
                            goto skip;
                        }
                    }
                }
skip:
                ;
            }
        }
    }
    fprintf(stderr, "\n");
    for(i = 1; i < CSIZE; i++) {
        for(j = 1; j < MSIZE; j++) {
            ptable[i][j] /= (float)numiter;
        }
    }
    /*
    ** create output image
    */
    i = cmaxsize + 20;
    j = zmaxsize + 10;
    if(i > CSIZE)
        i = CSIZE;
    if(j > MSIZE)
        j = MSIZE;
    zdest = VCreateImage(1, i, j, VFloatRepn);
    VFillImage(zdest, VAllBands, 0);
    VSetAttr(VImageAttrList(zdest), "name", NULL, VStringRepn, "multicomp");
    VSetAttr(VImageAttrList(zdest), "modality", NULL, VStringRepn, "multicomp");
    VSetAttr(VImageAttrList(zdest), "zthr", NULL, VFloatRepn, (VFloat)z0);
    VSetAttr(VImageAttrList(zdest), "fwhm", NULL, VFloatRepn, (VFloat)fwhm);
    if(buf != NULL && voxsize != 1)
        VSetAttr(VImageAttrList(zdest), "voxel", NULL, VStringRepn, buf);
    VSetAttr(VImageAttrList(zdest), "voxel_size", NULL, VFloatRepn, (VFloat)voxsize);
    VSetAttr(VImageAttrList(zdest), "num_voxels", NULL, VLongRepn, (VLong)nvox);
    VSetAttr(VImageAttrList(zdest), "num_iterations", NULL, VShortRepn, (VShort)numiter);
    VSetAttr(VImageAttrList(zdest), "seed", NULL, VLongRepn, (VLong)seed);
    VSetAttr(VImageAttrList(zdest), "corrected-p-threshold", NULL, VFloatRepn, (VFloat)p0);
    for(i = 1; i < VImageNRows(zdest); i++) {
        for(j = 1; j < VImageNColumns(zdest); j++) {
            u = (double)j;
            u = zthr0 + (u - 0.5) / zscale;
            px = gsl_cdf_gaussian_Q(u, sig0);
            zz = p2z(px);
            if(ptable[i][j] < p0)
                VPixel(zdest, 0, i, j, VFloat) = zz;
        }
    }
    return zdest;
}
