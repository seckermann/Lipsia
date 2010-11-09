/****************************************************************
 *
 * Program: vtimestep
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2001, <lipsia@cbs.mpg.de>
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
 * $Id: vtimestep.c 3602 2009-07-01 07:43:04Z lohmann $
 *
 *****************************************************************/
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <via/via.h>

VDictEntry TYPEDict[] = {
  { "ubyte", 0 },
  { "short", 1 },
  { "float", 2 },
  { NULL }
};

extern char * getLipsiaVersion();

int 
main (int argc,char *argv[])
{  
  static VShort step = 0;
  static VShort repn = 0;
  static VOptionDescRec  options[] = {
    {"step",VShortRepn,1,(VPointer) &step,VOptionalOpt,NULL,"Time step to be extracted"},
    {"repn",VShortRepn,1,(VPointer) &repn,
       VOptionalOpt,TYPEDict,"output representation type (ubyte,short,float)"}
  };
  FILE *in_file,*out_file;
  VAttrList list=NULL,out_list=NULL;
  VAttrListPosn posn;
  VImage src=NULL,dest=NULL,tmp=NULL;
  int n,nbands,nrows,ncols,npixels;
  VBand band;
  VPointer src_pp,*dest_pp;
  char *ptr1,*ptr2;
  char prg[50];	
  sprintf(prg,"vtimestep V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);

  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,&out_file);

  band = step;

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  
  /*
  ** count number of slices
  */
  nbands = nrows = ncols = 0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    if (VPixelRepn(src) != VShortRepn) continue;
    if (VImageNRows(src) > nrows) nrows = VImageNRows(src);
    if (VImageNColumns(src) > ncols) ncols = VImageNColumns(src);
    nbands++;
  }
  fprintf(stderr," nslices=%d,  size: %d x %d\n",nbands,nrows,ncols);
  if (nbands==0) VError(" no functional images found in input file");

  dest = VCreateImage(nbands,nrows,ncols,VShortRepn);
  VFillImage(dest,VAllBands,0);

  n = 0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    if (VPixelRepn(src) != VShortRepn) continue;
    if (n == 0) VCopyImageAttrs (src, dest);

    if (VImageNRows(src) > 1) {
      if (VSelectBand ("vtimestep",src,band,&npixels,&src_pp) == FALSE)
	VError("err reading data");

      dest_pp = VPixelPtr(dest,n,0,0);
      ptr1 = (char *) src_pp;
      ptr2 = (char *) dest_pp;
      memcpy (ptr2,ptr1, npixels * VPixelSize (src));
    }
    n++;
  }


  /* output */
  out_list = VCreateAttrList();
  VSetAttr(VImageAttrList(dest),"condition",NULL,VStringRepn,"0");
  VSetAttr(VImageAttrList(dest),"bandtype",NULL,VStringRepn,"spatial");

  switch (repn) {
  case 0:
    tmp = VContrast(dest,NULL,VUByteRepn,(VFloat) 3.0,(VFloat) 0.001);
    VAppendAttr(out_list,"image",NULL,VImageRepn,tmp);
    break;

  case 1:
    VAppendAttr(out_list,"image",NULL,VImageRepn,dest);
    break;

  case 2:
    tmp = VConvertImageCopy(dest,NULL,VAllBands,VFloatRepn);
    VAppendAttr(out_list,"image",NULL,VImageRepn,tmp);
    break;

  default:
    VError("illegal repn type");
  }

  VHistory(VNumber(options),options,prg,&list,&out_list);
  if (! VWriteFile (out_file, out_list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
