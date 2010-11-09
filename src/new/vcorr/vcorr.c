/*
** get corr
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

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_sort.h>

#define NSLICES  2000    /* max number of slices */
#define BSIZE      80    /* buffer size */


typedef struct SpointStruct{
  VShort x;
  VShort y;
  VShort z;
} SPoint;


VDictEntry CorrDict[] = {
  { "pearson", 0 },
  { "spearman", 1 },
  { "MI", 2 },
  { "discord", 3 },
  { NULL }
};

VDictEntry TypeDict[] = {
  { "corr", 0 },
  { "r2z", 1 },
  { "p", 2 },
  { "z", 4 },
  { NULL }
};

extern void   ROI(VImage *,VImage,int,int,gsl_vector *);
extern double VMutualInformation(gsl_vector *,gsl_vector *,int);
extern double CorrelationPearson(gsl_vector *,gsl_vector *,int);
extern double CorrelationSpearman(gsl_vector *,gsl_vector *,int);
extern double VKendallDist(double *,double *,int);


int
main(int argc, char *argv[])
{
  static VShort start  = 0;
  static VShort len    = -1;
  static VShort ctype  = 0;
  static VShort otype  = 0;
  static VString filename = "";
  static SPoint addr;
  static VShort minval = 0;
  static VOptionDescRec  options[] = {
    {"seed",VShortRepn,3,(VPointer) &addr,VOptionalOpt,NULL,"voxel address"},
    {"mask",VStringRepn,1,(VPointer) &filename,VOptionalOpt,NULL,"ROI mask"},
    {"start",VShortRepn,1,(VPointer) &start,VOptionalOpt,NULL,"start of time series "},
    {"len",VShortRepn,1,(VPointer) &len,VOptionalOpt,NULL,"len of time series "},
    {"corr",VShortRepn,1,(VPointer) &ctype,VOptionalOpt,CorrDict,"type of correlation"},
    {"type",VShortRepn,1,(VPointer) &otype,VOptionalOpt,TypeDict,"output type"},
    {"minval", VShortRepn,1,(VPointer) &minval,VOptionalOpt,NULL,"minval"}
  };
  FILE *in_file=NULL,*out_file=NULL,*fp=NULL;
  VAttrList list=NULL,list1=NULL,out_list=NULL;
  VAttrListPosn posn;
  VImage xsrc=NULL,dest=NULL,mask=NULL,src[NSLICES];
  double *ptr,v;
  gsl_vector *array1=NULL,*array2=NULL;
  int j,k,n,b,r,c,nslices,nrows,ncols;
  char *prg = "vcorr";


  VParseFilterCmd(VNumber(options),options,argc,argv,&in_file,&out_file);

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);


  /* get image dimensions */
  n = k = nrows = ncols = 0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & xsrc);
    if (VPixelRepn(xsrc) != VShortRepn) continue;
    if (VImageNBands(xsrc) > n) n = VImageNBands(xsrc);
    if (VImageNRows(xsrc) > nrows) nrows = VImageNRows(xsrc);
    if (VImageNColumns(xsrc) > ncols) ncols = VImageNColumns(xsrc);
    src[k] = xsrc;
    if (k >= NSLICES) VError(" too many slices, max is %d",NSLICES);
    k++;
  }
  if (len < 0) len = n;
  nslices = k;
  if (start+len > n) VError(" illegal length %d, max= %d",start+len,n);
  if (len == 0) len = n;

  array1 = gsl_vector_calloc (len);
  array2 = gsl_vector_calloc (len);


  /* alloc memory */
  dest = VCreateImage(nslices,nrows,ncols,VFloatRepn);
  VCopyImageAttrs (src[0],dest);
  VFillImage(dest,VAllBands,0);

  /* read ROI mask if present */
  if (strlen(filename) > 1) {
    fp = VOpenInputFile (filename, TRUE);
    list1 = VReadFile (fp, NULL);
    if (! list1)  VError("Error reading mask");
    fclose(fp);

    for (VFirstAttr (list1, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & mask);
      if (VPixelRepn(mask) != VBitRepn && VPixelRepn(mask) != VUByteRepn && VPixelRepn(mask) != VShortRepn) {
	mask = NULL;
	continue;
      }
      break;
    }
    if (mask == NULL) VError(" no mask found");
    ROI(src,mask,start,(int)minval,array1);
  }

  else {  /* single voxel as reference */
    b = addr.z;
    r = addr.y;
    c = addr.x;
    if (VImageNRows(src[b]) < 2) 
      VError(" illegal slice addr, empty slice: %d",b);
    if (c < 0 || c >= ncols) VError(" illegal column address (%d), must be < %d",c,ncols);
    if (r < 0 || r >= nrows) VError(" illegal row address (%d), must be < %d",r,nrows);
    if (b < 0 || b >= nslices) VError(" illegal slice address (%d), must be < %d",b,nslices);
				    
    ptr = array1->data;
    for (j=start; j<start+len; j++) {
      (*ptr++) = VPixel(src[b],j,r,c,VShort);
    }
  }


  /* process */
  for (b=0; b<nslices; b++) {
    if (VImageNRows(src[b]) < 2) continue;

    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {

        if (VPixel(src[b],0,r,c,VShort) < minval) continue;

	ptr = array2->data;
        for (j=start; j<start+len; j++)
          (*ptr++) = VPixel(src[b],j,r,c,VShort);

	v = 0;
	switch(ctype) {
	case 0:
	  v = CorrelationPearson(array1,array2,(int)otype);
	  break;

	case 1:
	  v = CorrelationSpearman(array1,array2,(int)otype);
	  break;

	case 2:
	  v = VMutualInformation(array1,array2,(int)otype);
	  break;

	case 3:
	  v = VKendallDist(array1->data,array2->data,(int)array1->size);
	  break;

	default:
	  VError(" illegal type");
	}

	VPixel(dest,b,r,c,VFloat) = v;
      }
    }
  }


  /*
  ** output
  */
  out_list = VCreateAttrList();
  VHistory(VNumber(options),options,prg,&list,&out_list);
  VSetAttr(VImageAttrList(dest),"modality",NULL,VStringRepn,"conimg");
  VAppendAttr(out_list,"image",NULL,VImageRepn,dest);
  if (! VWriteFile (out_file, out_list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  exit(0);
}
