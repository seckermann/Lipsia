/*
** data scaling of fMRI time series
** each voxel gets a new time series with user-specified mean and sigma
**
** G.Lohmann, MPI-CBS, Feb 2007
*/
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))


void
VScaleVals(VAttrList list,VFloat scale)
{
  VImage src;
  VAttrListPosn posn;
  int r,c,j,k,l,n;
  double u;


  k = l = 0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    if (VPixelRepn(src) != VShortRepn) continue;
    if (VImageNRows(src) < 2) continue;

    n = VImageNBands(src);

    for (r=0; r<VImageNRows(src); r++) {
      for (c=0; c<VImageNColumns(src); c++) {

	for (j=0; j<n; j++) {
	  u = VPixel(src,j,r,c,VShort);
	  u *= scale;
	  u = (u+0.5);

	  if (u > VPixelMaxValue(src)) {
	    u = VPixelMaxValue(src);
	    k++;
	  }
	  if (u < VPixelMinValue(src)) {
	    u = VPixelMinValue(src);
	    l++;
	  }

	  VPixel(src,j,r,c,VShort) = (int)u;
	}
      }
    }
  }

  if (k > 0) VWarning(" overflow %d",k);
  if (l > 0) VWarning(" underflow %d",l);
}



void
VNormVals(VAttrList list,VShort minval)
{
  VImage src;
  VAttrListPosn posn;
  int r,c,j,n;
  double u,val,sum1,sum2,nx,mean,sigma;


  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    if (VPixelRepn(src) != VShortRepn) continue;
    if (VImageNRows(src) < 2) continue;

    n = VImageNBands(src);

    for (r=0; r<VImageNRows(src); r++) {
      for (c=0; c<VImageNColumns(src); c++) {

	if (VPixel(src,0,r,c,VShort) < minval) {
	  for (j=0; j<n; j++) {
	    VPixel(src,j,r,c,VShort) = 0;
	  }
	}

	else {

	  sum1 = sum2 = nx = 0;
	  for (j=0; j<n; j++) {
	    val = VPixel(src,j,r,c,VShort);
	    sum1 += val;
	    sum2 += val * val;
	    nx++;
	  }
	  mean = sum1/nx;
	  sigma = sqrt((sum2 - nx * mean * mean) / (nx - 1.0));

	  if (sigma < 0.1) {
	    for (j=0; j<n; j++) {
	      VPixel(src,j,r,c,VShort) = 0;
	    }
	  }


	  for (j=0; j<n; j++) {

	    val = VPixel(src,j,r,c,VShort);
	    val = (val - mean) / sigma;

	    val *= 1000;
	    val += 10000;

	    u = (int)(val + 0.5);
	    if (u > VPixelMaxValue(src)) u = VPixelMaxValue(src);
	    if (u < VPixelMinValue(src)) u = VPixelMinValue(src);
	    VPixel(src,j,r,c,VShort) = u;

	  }
	}
      }
    }
  }
}


int
main(int argc, char *argv[])
{
  static VFloat scale  = 0;
  static VShort minval = 0;
  static VOptionDescRec  options[] = {
    {"scale",VFloatRepn,1,(VPointer) &scale,VOptionalOpt,NULL,"scale"},
    {"minval",VShortRepn,1,(VPointer) &minval,VOptionalOpt,NULL,"signal threshold"}
  };
  FILE *in_file=NULL,*out_file=NULL;  
  VAttrList list=NULL;
  char *prg = "vnormvals";

  VParseFilterCmd(VNumber(options),options,argc,argv,&in_file,&out_file);

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);


  if (ABS(scale) < 0.1) {
    VNormVals(list,minval);
  }
  else {
    VScaleVals(list,scale);
  }


  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  exit(0);
}
