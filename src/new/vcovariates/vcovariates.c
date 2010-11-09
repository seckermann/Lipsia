/*
** construct a design file from an ASCII file
** no hemodynamic modelling
**
** G.Lohmann, Dec 2006
*/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

extern int VStringToken (char *,char *,int,int);

#define LEN  10000   /* max number of characters per line in file */
#define NCOV   200   /* max number of additional covariates     */


VImage 
VCovariates(VString cfile,VFloat tr)
{
  VImage dest=NULL;
  FILE *fp=NULL;
  char buf[LEN],token[80];
  int i,j,ncols,nrows;
  float x;


  fp = fopen(cfile,"r");
  if (fp == NULL) VError(" err opening file %s",cfile);


  nrows = ncols = 0;
  while (!feof(fp)) {
    for (j=0; j<LEN; j++) buf[j] = '\0';
    fgets(buf,LEN,fp);
    if (buf[0] == '#' || buf[0] == '%') continue;
    if (strlen(buf) < 3) continue;

    j = 0;
    while (VStringToken(buf,token,j,30)) {
      sscanf(token,"%f",&x);
      if (j >= NCOV) VError(" too many additional covariates (%d), max is %d",j,NCOV-1);
      j++;
    }
    if (j > ncols) ncols = j;
    nrows++;
  }

  fprintf(stderr,"# num rows: %d,  num cols: %d\n",nrows,ncols);

  dest = VCreateImage(1,nrows,ncols+1,VFloatRepn);
  VSetAttr(VImageAttrList(dest),"modality",NULL,VStringRepn,"X");
  VSetAttr(VImageAttrList(dest),"name",NULL,VStringRepn,"X");
  VSetAttr(VImageAttrList (dest),"repetition_time",NULL,VLongRepn,(VLong)(tr*1000.0));
  VSetAttr(VImageAttrList (dest),"ntimesteps",NULL,VLongRepn,(VLong)nrows);

  VSetAttr(VImageAttrList (dest),"derivatives",NULL,VShortRepn,(VFloat) 0);
  VSetAttr(VImageAttrList (dest),"delay",NULL,VFloatRepn,(VFloat) 0);
  VSetAttr(VImageAttrList (dest),"undershoot",NULL,VFloatRepn,(VFloat) 0);
  VSetAttr(VImageAttrList (dest),"understrength",NULL,VFloatRepn,(VFloat) 0);

  VSetAttr(VImageAttrList(dest),"nsessions",NULL,VShortRepn,(VShort)1);
  VSetAttr(VImageAttrList(dest),"designtype",NULL,VShortRepn,(VShort)1);
  VFillImage(dest,VAllBands,0);
  for (j=0; j<nrows; j++) VPixel(dest,0,j,ncols,VFloat) = 1;


  rewind(fp);
  i=0;
  while (!feof(fp)) {
    for (j=0; j<LEN; j++) buf[j] = '\0';
    fgets(buf,LEN,fp);
    if (buf[0] == '#' || buf[0] == '%') continue;
    if (strlen(buf) < 3) continue;
    if (i > nrows) VError(" too many rows in file (%d), max is %d",i,nrows);

    j = 0;
    while (VStringToken(buf,token,j,30)) {
      sscanf(token,"%f",&x);
      if (j >= NCOV) VError(" too many additional covariates (%d), max is %d",j,NCOV-1);
      VPixel(dest,0,i,j,VFloat) = x;
      j++;
    }
    if (j < 1) continue;
    if (i >= nrows) continue;
    i++;
  }
  fclose(fp);

  
  if (i != nrows) VError(" file: inconsistent number of rows: %d %d",i,nrows);
  return dest;
}




int
main (int argc,char *argv[])
{
  static VString cfile = "";
  static VFloat tr = 2;
  static VOptionDescRec  options[] = {
    {"in",VStringRepn,1,(VPointer) &cfile,VRequiredOpt,NULL,"file"},
    {"tr",VFloatRepn,1,(VPointer) &tr,VRequiredOpt,NULL,"TR in seconds"}
  };
  FILE *out_file=NULL;
  VAttrList list=NULL;
  VImage dest=NULL;


  VParseFilterCmd(VNumber(options),options,argc,argv,NULL,&out_file);
  if (tr > 500) VError(" tr must be given in seconds, not milliseconds");

  dest = VCovariates(cfile,tr);

  list = VCreateAttrList();
  VAppendAttr(list,"image",NULL,VImageRepn,dest);


  /* Output: */
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf(stderr," %s: done.\n",argv[0]);
  return 0;
}

