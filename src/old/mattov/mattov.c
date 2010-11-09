/****************************************************************
 *
 * Program: mattov
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Karsten Mueller, 2005, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * $Id: mattov.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "mat.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

extern char * getLipsiaVersion();

int 
main (int argc,char *argv[])
{
  /* Command line options: */
  static VString mat = NULL;
  static VBoolean funconly = FALSE;
  static VOptionDescRec  options[] = {
    {"in",VStringRepn, 1, &mat, VRequiredOpt, 0,"Matfile"},
  };
  FILE *out_file;
  VAttrList out_list;
  VImage src=NULL;
  VBit    *bit_pp   =NULL, *pbSRC=NULL;
  VShort  *short_pp =NULL, *psSRC=NULL;
  VUByte  *ubyte_pp =NULL, *puSRC=NULL;
  VFloat  *float_pp =NULL, *pfSRC=NULL;
  VDouble *double_pp=NULL, *pdSRC=NULL;
  VString mode=NULL;
  mxArray *SRC=NULL;
  MATFile *fp;
  int numdims=0, *dims=NULL;
  int bands=0, rows=0, cols=0, pixels=0;
  const char *name=NULL;
  char prg_name[50];	
  sprintf(prg_name,"mattov V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);

  /* Parse command line arguments and identify files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,NULL /* &in_file */ ,&out_file);

  /* Creat outlist */
  out_list = VCreateAttrList();

  /* history */
  VHistory(VNumber(options),options,prg_name,&out_list,&out_list); 

  /* strings */
  mode = (VString)malloc(sizeof(char));
  strcpy(mode, "r");
  fp = matOpen(mat,mode);

  /* load data into mxarray */
  name = (const char *)malloc(sizeof(char)* 50);
  SRC = matGetNextVariable(fp,&name);



  /*******************************************************************
   *                                                                 *
   *                  L O O P over objects in matlab file            *
   *                                                                 *
   *******************************************************************/
  while (SRC != NULL) {
   
    numdims = mxGetNumberOfDimensions(SRC);
    dims = (int *)mxGetDimensions(SRC);

    /* rows and columns */
    rows = dims[1]; cols = dims[0];
    if (numdims > 2) bands = dims[2];
    else bands = 1;
    pixels = rows*cols*bands;
    if (numdims > 3) VError("4-D arrays not yet supported");
    if (mxIsComplex(SRC)) VError("Complex data are not yet supported");

    /* Float ONLY */
    if (mxIsSingle(SRC)) {
      src = VCreateImage(bands,rows,cols,VFloatRepn);
      pfSRC    = (VFloat*) mxGetPr (SRC);
      float_pp = (VFloat*) VPixelPtr(src,0,0,0);
      memcpy (float_pp, pfSRC, sizeof (VFloat) * pixels);
    }  

    /* Bit ONLY */
    if (mxIsLogical(SRC)) {
      src = VCreateImage(bands,rows,cols,VBitRepn);
      pbSRC  = (VBit*) mxGetPr (SRC);
      bit_pp = (VBit*) VPixelPtr(src,0,0,0);
      memcpy (bit_pp, pbSRC, sizeof (VBit) * pixels);
    }

    /* Short ONLY */
    if (mxIsInt16(SRC)) {
      src = VCreateImage(bands,rows,cols,VShortRepn);	  
      psSRC    = (VShort*) mxGetPr (SRC);
      short_pp = (VShort*) VPixelPtr(src,0,0,0);
      memcpy (short_pp, psSRC, sizeof (VShort) * pixels);
    }
    
    /* UByte ONLY */
    if (mxIsUint8(SRC)) {
      src = VCreateImage(bands,rows,cols,VUByteRepn);	   
      puSRC    = (VUByte*) mxGetPr (SRC);
      ubyte_pp = (VUByte*) VPixelPtr(src,0,0,0);
      memcpy (ubyte_pp, puSRC, sizeof (VUByte) * pixels);
    }   
    
    /* Double ONLY */
    if (mxIsDouble(SRC)) {
      src = VCreateImage(bands,rows,cols,VDoubleRepn); 
      pdSRC     = (VDouble*) mxGetPr (SRC);
      double_pp = (VDouble*) VPixelPtr(src,0,0,0);
      memcpy (double_pp, pdSRC, sizeof (VDouble) * pixels);
    }



    /* Append to outlist */
    VSetAttr(VImageAttrList (src),"name",NULL,VStringRepn,(VString)name);
    VSetAttr(VImageAttrList (src),"modality",NULL,VStringRepn,(VString)name);
    VAppendAttr(out_list,"image",NULL,VImageRepn,src);

    /* load next variable */
    mxDestroyArray(SRC);
    SRC = matGetNextVariable(fp,&name);
  }
   
  
  /* Terminate */
  matClose(fp);

  /* Write out the results: */
  if (! VWriteFile (out_file, out_list)) exit (1);
  fprintf(stderr,"done.\n");
  return 0;
}


