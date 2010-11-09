/*
** apply output of 'vmulticomp'
**
** G.Lohmann, lohmann@cbs.mpg.de, Aug 2007
*/

/* From the Vista library: */
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via/via.h>


/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>


extern char * getLipsiaVersion();
extern void VPixel2Tal(float [3],float [3],float [3],int,int,int,float *,float *,float *);
extern VBoolean CheckValue(VImage multicomp,int size,VFloat zval,float sym);
extern float VolumeSym(Volume vol,VImage src,float sign,VFloat zthr);
extern float VolumeZMax(Volume vol,VImage src,float sign,int *bb0,int *rr0,int *cc0);
extern void VolumeZero(Volume vol,VImage image);

extern VImage VDoMulticomp2d(VImage,VImage,VBoolean);
extern VImage VDoMulticomp3d(VImage,VImage,VBoolean);



int
main (int argc, char *argv[])
{  
  static VString filename = "";
  static VBoolean verbose = FALSE;
  static VOptionDescRec options[] = {
    {"file",VStringRepn,1,(VPointer) &filename,VRequiredOpt,NULL,"multicomp file"},
    {"verbose",VBooleanRepn,1,(VPointer) &verbose,VOptionalOpt,NULL,"verbose"}
  };
  FILE *in_file,*out_file,*fp;
  VAttrList list,list1;
  VAttrListPosn posn;
  VString str;
  VFloat p0=0.05;
  VImage src=NULL;
  VImage multicomp=NULL,dest=NULL;
  VBoolean symm = FALSE;

  char prg[50];
  /* sprintf(prg,"vdomulticomp V%s", getLipsiaVersion()); */
  sprintf(prg,"vdomulticomp");
  fprintf (stderr, "%s\n", prg);


  /* 
  ** Parse command line arguments: 
  */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file,&out_file);


  /* 
  ** Read multicomp image 
  */
  fp = VOpenInputFile (filename, TRUE);
  list1 = VReadFile (fp, NULL);
  if (! list1)  VError("Error reading multicomp image");
  fclose(fp);

  for (VFirstAttr (list1, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    if (strcmp(VGetAttrName(&posn),"multicomp") != 0) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & multicomp);
    if (VPixelRepn(multicomp) != VFloatRepn) continue;

    symm = FALSE;
    if (VGetAttr (VImageAttrList (multicomp), "hemi_symm", NULL,
		  VStringRepn, (VPointer) & str) == VAttrFound)
      if (strncmp(str,"on",2) == 0) symm = TRUE;

    p0 = 0.05;
    if (VGetAttr (VImageAttrList (multicomp), "corrected-p-threshold", NULL,
		  VFloatRepn, (VPointer) & p0) != VAttrFound) p0 = 0.05;
    fprintf(stderr," Using corrected p-threshold: %.4f\n",p0);
    break;
  }
  if (multicomp == NULL) VError(" multicomp image not found");
  if (VImageNBands(multicomp) < 2) symm = FALSE;



  /* 
  ** Read the input file: 
  */
  list = VReadFile (in_file,NULL);
  if (! list) exit (1);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);
    if (VPixelRepn(src) != VFloatRepn) continue;
    if (VGetAttr (VImageAttrList (src), "modality", NULL,
		  VStringRepn, (VPointer) & str) != VAttrFound)
      VError(" attribute 'modality' not found");

    if (strcmp(str,"zmap") != 0) {
      src = NULL;
      continue;
    }
    
    if (symm == FALSE) {
      dest = VDoMulticomp2d(src,multicomp,verbose);
    }
    else {
      dest = VDoMulticomp3d(src,multicomp,verbose);
    }

    VSetAttrValue (& posn, NULL,VImageRepn,dest);
    break;
  }
  if (src == NULL) VError(" input zmap not found");


  /* 
  ** Output
  */
  VHistory(VNumber(options),options,prg,&list1,&list);
  if (! VWriteFile (out_file, list)) VError(" error writing output file");
  fprintf (stderr, "%s: done.\n",argv[0]);
  return 0;
}
