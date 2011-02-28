/* From the Vista library: */

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <nifti1.h>
#include <nifti1_io.h>
#include <znzlib.h>


#define LEN 64

#define ABS(x) ((x) < 0 ? -(x) : (x))


VAttrList
VNiftFuncReorder(VAttrList list,nifti_image *src,VShort orientation, VFloat reptime)
{
  VAttrList list1;
  int i, bands, rows, cols, timesteps;
  int rr,bb,cc,tt,time_units, tr1=0; 
  VLong tr=0;
  VImage *image=NULL, fnc;
  VAttrListPosn posn;
  char vox[100], mpilvista[128];

  rows      = src->dim[2];
  cols      = src->dim[1];
  bands     = src->dim[3];
  timesteps = src->dim[4];
  time_units  = src->time_units;

  /* TR from header */
  if (time_units==8)
    tr = (VLong)(ABS(src->pixdim[4])*1000.0);
  if (time_units==16)
    tr = (VLong)(ABS(src->pixdim[4]));
  if (tr<1) 
    VWarning(" could not read repetition time (TR) from nifti header ");

  /* TR from command line */
  if (reptime > 32) VWarning(" Check if repetition time is specified in seconds");
  tr1 =  (int)(reptime * 1000.0);
  if (tr1 > 1) {
    if (tr != tr1) {
      VWarning(" TR in header is different from specified TR in command line");
      fprintf(stderr," Using TR from command line TR=%d ms (TR in header is %d ms)\n",tr1,tr);
    }
    tr = tr1;
  } else {
    VError(" specify the repetion time using '-tr' ");
  }

  sprintf(vox,"%f %f %f\0",ABS(src->pixdim[1]),ABS(src->pixdim[2]),ABS(src->pixdim[3]));

  /* Creat outlist */
  list1 = VCreateAttrList();

  /* loop over all slices */
  image = (VImage *)VMalloc(sizeof (VImage) * bands);
  for (i=0;i<bands;i++) image[i] = VCreateImage(timesteps, rows, cols, VShortRepn);
    
  /* Going through the list */
  tt=0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, &fnc);
    if (VPixelRepn(fnc) != VShortRepn) continue;

    for (rr=0;rr<rows;rr++) {
      for (cc=0;cc<cols;cc++) {
	for (bb=0;bb<bands;bb++) {
	  /* VPixel(image[bb],tt,rr,cc,VShort)=VPixel(fnc,bands-bb-1,rr,cc,VShort); */
	  VPixel(image[bb],tt,rr,cc,VShort)=VPixel(fnc,bb,rr,cc,VShort);
	}
      }
    }
    tt++;
    if (tt>timesteps) VError("Problem with reordering of functional data");
  }
  
  /* Append on new List */
  for (i=0;i<bands;i++) {
    VSetAttr(VImageAttrList (image[i]),"bandtype",NULL,VStringRepn, "temporal");
    VSetAttr(VImageAttrList (image[i]),"name",NULL,VStringRepn,(VString)src->descrip);
    VSetAttr(VImageAttrList (image[i]),"voxel",NULL,VStringRepn,(VString)vox);
    if (orientation==0) VSetAttr(VImageAttrList(image[i]), "orientation", NULL, VStringRepn, "axial");
    if (orientation==1) VSetAttr(VImageAttrList(image[i]), "orientation", NULL, VStringRepn, "sagittal");
    if (orientation==2) VSetAttr(VImageAttrList(image[i]), "orientation", NULL, VStringRepn, "coronal");
    sprintf(mpilvista," repetition_time=%d packed_data=1 %d ",tr,(VLong)timesteps);
    VSetAttr(VImageAttrList(image[i]),"MPIL_vista_0", NULL, VStringRepn, (VString)mpilvista );
    VSetAttr(VImageAttrList(image[i]),"repetition_time",NULL,VLongRepn,tr);
    VSetAttr(VImageAttrList(image[i]),"ntimesteps",NULL,VLongRepn,(VLong)timesteps);
    VSetAttr(VImageAttrList(image[i]),"convention",NULL,VStringRepn, "natural");
    VAppendAttr(list1,"image",NULL,VImageRepn,image[i]);  
  }

  return list1;
}
