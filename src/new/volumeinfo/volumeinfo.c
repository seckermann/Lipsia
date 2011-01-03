/*
** volumeinfo - print infos about volumes in a set of volumes
** may be used for getting size of a mask
**
** G.Lohmann, MPI-CBS, 2005
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via/via.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>


/*
** give infos about volumes
*/
void
VolumeInfo(Volumes src) {
    Volume v;
    double size;
    double mean[3];
    int n;
    n = 0;
    for(v = src->first; v != NULL; v = v->next) {
        size = VolumeSize(v);
        VolumeCentroid(v, mean);
        fprintf(stderr, " %5d:  %.2f %.2f %.2f,  %.2f\n",
                n, mean[2], mean[1], mean[0], size);
        n++;
    }
}



int
main(int argc, char *argv[]) {
    FILE *in_file;
    VAttrList list;
    Volumes src = NULL;
    VImage src_image = NULL, tmp = NULL;
    VAttrListPosn posn;
    int nl = 0;
    char *prg = "volumeinfo: $Revision: 0.0 $";
    /* Parse command line arguments: */
    VParseFilterCmd(0, NULL, argc, argv, &in_file, NULL);
    /* Read source image(s): */
    if(!(list = VReadFile(in_file, NULL)))
        exit(1);
    /* Performs conversion on each image: */
    for(VFirstAttr(list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {
        switch(VGetAttrRepn(& posn)) {
        case VImageRepn:
            VGetAttrValue(& posn, NULL, VImageRepn, & src_image);
            if(VPixelRepn(src_image) == VBitRepn) {
                tmp = VLabelImage3d(src_image, NULL, 26, VShortRepn, &nl);
                src = VImage2Volumes(tmp);
            } else {
                src  = VImage2Volumes(src_image);
            }
            VolumeInfo(src);
            break;
        case VolumesRepn:
            VGetAttrValue(& posn, NULL, VolumesRepn, & src);
            VolumeInfo(src);
            break;
        default:
            continue;
        }
    }
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}
