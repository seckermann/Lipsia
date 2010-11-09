/****************************************************************
 *
 * Program: vtomat
 * merge outlines of several zmaps into one quasi-zmap.
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Karsten Mueller, 2001, <lipsia@cbs.mpg.de>
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
 * $Id: vtomat.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

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
    {"out",VStringRepn, 1, &mat, VRequiredOpt, 0,"Matfile"},
    {"funconly",VBooleanRepn, 1, &funconly, VOptionalOpt, 0,"Functional data only (in one single 4D-object)"},
  };
  FILE *in_file;
  VAttrList in_list;
  VAttrListPosn posn;
  VImage src=NULL;
  VBit     *bit_pp=NULL, *pbSRC=NULL;
  VShort *short_pp=NULL, *psSRC=NULL;
  VUByte *ubyte_pp=NULL, *puSRC=NULL;
  VFloat *float_pp=NULL, *pfSRC=NULL;
  VDouble *double_pp=NULL, *pdSRC=NULL;
  VString mode=NULL, name=NULL;
  mxArray *SRC=NULL;
  MATFile *fp;
  int n=0, dims[4];
  int bands=0, rows=0, cols=0;

  char prg_name[50];	
  sprintf(prg_name,"vtomat V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);
  
  /* Parse command line arguments and identify files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file,/* &out_file */ NULL);

  /* Read the input file */
  in_list = VReadFile (in_file, NULL);
  if (!in_list) exit(1);

  /* strings */
  mode = (VString)malloc(sizeof(char));
  name = (VString)malloc(sizeof(char)* 50);
  strcpy(mode, "w");
  fp = matOpen(mat,mode);

  if (funconly==TRUE) {

    /*******************************************************************
     *                                                                 *
     *                   L O O P over objects in vista file            *
     *                                                                 *
     *******************************************************************/
    n=0;
    for (VFirstAttr (in_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & src);
      if (VPixelRepn(src) == VShortRepn) {
	if (bands < 2) bands = VImageNBands(src);
	if (rows  < 2) rows  = VImageNRows(src);
	if (cols  < 2) cols  = VImageNColumns(src);
	if (bands != VImageNBands(src))   VError("Different bands in functional objects");
	if (rows  != VImageNRows(src))    VError("Different rows  in functional objects");
	if (cols  != VImageNColumns(src)) VError("Different cols  in functional objects");
	n++;
      }
    }

    /*dimensions */
    dims[3]=n; dims[2]=bands; dims[1]=rows; dims[0]=cols;   
    SRC   = mxCreateNumericArray(4,dims,mxINT16_CLASS,mxREAL);
    psSRC = (VShort*) mxGetPr (SRC);

    /* copy data to SRC */
    for (VFirstAttr (in_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & src);
      if (VPixelRepn(src) == VShortRepn) {
	short_pp = (VShort*) VPixelPtr(src,0,0,0);
	memcpy (psSRC, short_pp, sizeof (VShort) * dims[0] * dims[1] * dims[2]);
	psSRC += dims[0] * dims[1] * dims[2];
      }
    }


    /* save data into file */
    if (matPutVariable(fp,"fnc",SRC))
      VError("Could not save data in MATLAB file");
    
    /* destroy array */
    mxDestroyArray(SRC);
   

 
  } else {

    
    /*******************************************************************
     *                                                                 *
     *                   L O O P over objects in vista file            *
     *                                                                 *
     *******************************************************************/
    n=0;
    for (VFirstAttr (in_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & src);
      
      /*dimensions */
      dims[3]=1; dims[2]=VImageNBands(src); dims[1]=VImageNRows(src); dims[0]=VImageNColumns(src);
      sprintf(name,"obj%d\0",n);
      
      /* Bit ONLY */
      if (VPixelRepn(src) == VBitRepn) {
	SRC    = mxCreateNumericArray(3,dims,mxLOGICAL_CLASS,mxREAL);
	pbSRC  = (VBit*) mxGetPr (SRC);
	bit_pp = (VBit*) VPixelPtr(src,0,0,0);
	memcpy (pbSRC, bit_pp, sizeof (VBit) * dims[0] * dims[1] * dims[2]);
      }
      
      /* Short ONLY */
      if (VPixelRepn(src) == VShortRepn) {
	SRC      = mxCreateNumericArray(3,dims,mxINT16_CLASS,mxREAL);
	psSRC    = (VShort*) mxGetPr (SRC);
	short_pp = (VShort*) VPixelPtr(src,0,0,0);
	memcpy (psSRC, short_pp, sizeof (VShort) * dims[0] * dims[1] * dims[2]);
      }
      
      /* UByte ONLY */
      if (VPixelRepn(src) == VUByteRepn) {
	SRC      = mxCreateNumericArray(3,dims,mxUINT8_CLASS,mxREAL);
	puSRC    = (VUByte*) mxGetPr (SRC);
	ubyte_pp = (VUByte*) VPixelPtr(src,0,0,0);
	memcpy (puSRC, ubyte_pp, sizeof (VUByte) * dims[0] * dims[1] * dims[2]);
      }
      
      /* Float ONLY */
      if (VPixelRepn(src) == VFloatRepn) {
	SRC      = mxCreateNumericArray(3,dims,mxSINGLE_CLASS,mxREAL);
	pfSRC    = (VFloat*) mxGetPr (SRC);
	float_pp = (VFloat*) VPixelPtr(src,0,0,0);
	memcpy (pfSRC, float_pp, sizeof (VFloat) * dims[0] * dims[1] * dims[2]);
      }
      
      /* Double ONLY */
      if (VPixelRepn(src) == VDoubleRepn) {
	SRC       = mxCreateNumericArray(3,dims,mxDOUBLE_CLASS,mxREAL);
	pdSRC     = (VDouble*) mxGetPr (SRC);
	double_pp = (VDouble*) VPixelPtr(src,0,0,0);
	memcpy (pdSRC, double_pp, sizeof (VDouble) * dims[0] * dims[1] * dims[2]);
      }
      
      /* save data into file */
      if (matPutVariable(fp,name,SRC))
	VError("Could not save data in MATLAB file");
      
      /* destroy array */
      mxDestroyArray(SRC);
      n++;
    }   
  }

  
  /* Terminate */
  matClose(fp);
  return 0;
}


