/****************************************************************
 *
 * Program: vimagemask
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2007, <lipsia@cbs.mpg.de>
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
 * $Id: vimagemask.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>

extern char * getLipsiaVersion();

VImage
VMaskImage(VImage src,VImage mask,VImage dest,VDouble xmin,VDouble xmax,VShort type)
{
  int b,r,c;
  double u;

  if (VImageNBands(src) != VImageNBands(mask))
    VError(" inconsistent number of slices");
  if (VImageNRows(src) != VImageNRows(mask))
    VError(" inconsistent number of rows");
  if (VImageNColumns(src) != VImageNColumns(mask))
    VError(" inconsistent number of columns");

  dest = VCopyImage(src,NULL,VAllBands);

  for (b=0; b<VImageNBands(src); b++) {
    for (r=0; r<VImageNRows(src); r++) {
      for (c=0; c<VImageNColumns(src); c++) {
	u = VGetPixel(mask,b,r,c);

	if (type == 1) {
	  if (u < xmin || u > xmax)
	    VSetPixel(dest,b,r,c,0);
	}
	else {
	  if (u >= xmin && u <= xmax)
	    VSetPixel(dest,b,r,c,0);
	}

      }
    }
  }
  return dest;
}

VDictEntry TypeDict[] = {
  { "outside", 0 },
  { "inside", 1 },
  { NULL }
};



int 
main (int argc,char *argv[])
{ 
  static VString filename = "";
  static VDouble xmin = 0;
  static VDouble xmax = 0;
  static VShort  type = 0;
  static VOptionDescRec  options[] = {
    {"mask",VStringRepn,1,(VPointer) &filename,VOptionalOpt,NULL,"mask image"},
    {"min",VDoubleRepn,1,(VPointer) &xmin,VRequiredOpt,NULL,"min threshold"},
    {"max",VDoubleRepn,1,(VPointer) &xmax,VRequiredOpt,NULL,"max threshold"},
    {"type",VShortRepn,1,(VPointer) &type,VOptionalOpt,TypeDict,"outside range vs. inside range"}
  };
  FILE *in_file,*out_file,*fp;
  VAttrList list=NULL,list1=NULL;
  VAttrListPosn posn;
  VImage src=NULL,dest=NULL,mask=NULL;
  char prg[50];	
  sprintf(prg,"vimagemask V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);

  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,&out_file);
  if (type < 0 || type > 1) VError(" illegal value of parameter '-type'");
  if (xmin > xmax) VError(" 'xmax' must be >= 'xmin'");


  /* read mask */
  fp = VOpenInputFile (filename, TRUE);
  list1 = VReadFile (fp, NULL);
  if (! list1)  VError("Error reading mask");
  fclose(fp);

  for (VFirstAttr (list1, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & mask);
    break;
  }
  if (mask == NULL) VError(" no mask image found");
  

  /* read image to be masked, and process */
  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    dest = VMaskImage(src,mask,NULL,xmin,xmax,type);
    VSetAttrValue (& posn, NULL,VImageRepn,dest);
  }
  if (src == NULL) VError(" no input image found");


  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
