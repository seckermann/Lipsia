/*
 *  Program: vflip3d
 *
 *  Flip axis
 */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern void getLipsiaVersion(char*,size_t);

/*
 *  Flip3dImage
 *
 */
VImage Flip3dImage(VImage src, VImage dest, VBand band, VBoolean x_axis, VBoolean y_axis, VBoolean z_axis) {
    int nx, ny, nz;
    int i, j, k;
    nx = VImageNColumns(src);
    ny = VImageNRows(src);
    nz = VImageNBands(src);
    /* Check the destination image.
       If it is NULL, then create one of the appropriate size and type. */
    dest = VSelectDestImage("Flip3dImage", dest,
                            nz, ny, nx, VPixelRepn(src));
    if(! dest)
        return NULL;
    for(i = 0; i < nx; i++)
        for(j = 0; j < ny; j++)
            for(k = 0; k < nz; k++) {
                if(! x_axis && ! y_axis && ! z_axis)
                    VSetPixel(dest, k, j, i, VGetPixel(src, k, j, i));
                if(x_axis && ! y_axis && ! z_axis)
                    VSetPixel(dest, k, j, i, VGetPixel(src, k, j, nx - i - 1));
                if(! x_axis &&   y_axis && ! z_axis)
                    VSetPixel(dest, k, j, i, VGetPixel(src, k, ny - j - 1, i));
                if(! x_axis && ! y_axis &&   z_axis)
                    VSetPixel(dest, k, j, i, VGetPixel(src, nz - k - 1, j, i));
                if(x_axis &&   y_axis && ! z_axis)
                    VSetPixel(dest, k, j, i, VGetPixel(src, k, ny - j - 1, nx - i - 1));
                if(x_axis && ! y_axis &&   z_axis)
                    VSetPixel(dest, k, j, i, VGetPixel(src, nz - k - 1, j, nx - i - 1));
                if(! x_axis &&   y_axis &&   z_axis)
                    VSetPixel(dest, k, j, i, VGetPixel(src, nz - k - 1, ny - j - 1, i));
                if(x_axis &&   y_axis &&   z_axis)
                    VSetPixel(dest, k, j, i, VGetPixel(src, nz - k - 1, ny - j - 1, nx - i - 1));
            };
    /* Let the destination inherit any attributes of the source image: */
    VCopyImageAttrs(src, dest);
    return dest;
};




int main(int argc, char *argv[]) {
    static VBoolean x_axis = FALSE;
    static VBoolean y_axis = FALSE;
    static VBoolean z_axis = FALSE;
    static VOptionDescRec options[] = {
        { "x", VBooleanRepn, 1, (VPointer) &x_axis, VOptionalOpt, NULL, "Flip x-axis" },
        { "y", VBooleanRepn, 1, (VPointer) &y_axis, VOptionalOpt, NULL, "Flip y-axis" },
        { "z", VBooleanRepn, 1, (VPointer) &z_axis, VOptionalOpt, NULL, "Flip z-axis" },
    };
    FILE *in_file, *out_file;
    VAttrList list;
    VAttrListPosn posn;
    VImage src = NULL, result = NULL;
	char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vflip3d V%s", ver);
	fprintf(stderr, "%s\n", prg_name);
	VWarning("It is recommended to use the program vswapdim since vflip3d does not support extended header informations");
    /* Parse command line arguments and identify files: */
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, &out_file);
    /* Read the input file: */
    list = VReadFile(in_file, NULL);
    if(! list)
        exit(1);
    /* For each attribute read... */
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        if(VGetAttrRepn(& posn) != VImageRepn)
            continue;
        VGetAttrValue(& posn, NULL, VImageRepn, & src);
        result = Flip3dImage(src, NULL, VAllBands, x_axis, y_axis, z_axis);
        if(! result)
            exit(1);
        VSetAttrValue(& posn, NULL, VImageRepn, result);
        VDestroyImage(src);
    }
    /* Write out the results: */
    if(! VWriteFile(out_file, list))
        exit(1);
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}


