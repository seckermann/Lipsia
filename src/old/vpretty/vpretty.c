/****************************************************************
 *
 * Program: vpretty
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2004, <lipsia@cbs.mpg.de>
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
 * $Id: vpretty.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


/* From the Vista library: */
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))

extern char * getLipsiaVersion();

int
 main (int argc, char *argv[])
{
  static VDouble pos     =  3.09;
  static VDouble neg     = -3.09;
  static VShort  minsize =  81;

  static VOptionDescRec options[] = {
    {"pos",VDoubleRepn,1,(VPointer) &pos,
     VOptionalOpt,NULL,"Pos threshold on zmap"},
    {"neg",VDoubleRepn,1,(VPointer) &neg,
     VOptionalOpt,NULL,"Neg threshold on zmap"},
     {"minsize",VShortRepn,1,(VPointer) &minsize,
      VOptionalOpt,NULL,"Minimal size per area (in mm^3)"},
  };
  FILE *in_file,*out_file;
  VAttrList list;
  VAttrListPosn posn;
  VImage src=NULL,tmp=NULL,label_image=NULL;
  int i,xminsize,npixels,nbands,nrows,ncols,nl=0;
  VFloat *src_pp;
  VShort *lab_pp;
  VBit   *bin_pp;
  float voxel[3],x0,x1,x2;
  float tiny=1.0e-10;
  VString str; 
  char prg[50];	
  sprintf(prg,"vpretty V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);


  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file,&out_file);
  if (neg > 0) VError(" parameter '-neg' must be negative");
  if (pos < 0) VError(" parameter '-pos' must be positive");


  /* Read the input file: */
  list = VReadFile (in_file,NULL);
  if (! list) exit (1);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);
    if (VPixelRepn(src) != VFloatRepn) continue;

    if (VGetAttr(VImageAttrList(src),"voxel",NULL,VStringRepn,(VPointer) &str) != VAttrFound)
      VError(" attribute 'voxel' not found");
    sscanf(str,"%f %f %f",&x0, &x1, &x2);
    voxel[0] = x0;
    voxel[1] = x1;
    voxel[2] = x2;
    xminsize = VRint((float)minsize /(voxel[0] * voxel[1] * voxel[2]));
    if (xminsize < 1 && minsize > 0) {
      VWarning(" parameter 'minsize' has no effect (0 voxels). Units are in mm^3 !");
    }

    fprintf(stderr," spatial resolution: %.2f %.2f %.2f,    voxel volume: %.2f mm^3\n",
	    x0,x1,x2,x0*x1*x2);
    if (xminsize > 0)
      fprintf(stderr," areas with fewer than %d voxels are deleted\n",xminsize);

    nbands  = VImageNBands(src);
    nrows   = VImageNRows(src);
    ncols   = VImageNColumns(src);
    npixels = VImageNPixels(src);

    tmp = VCreateImage(nbands,nrows,ncols,VBitRepn);


    /*
    ** delete background
    */
    src_pp = VImageData(src);
    for (i=0; i<npixels; i++) {
      if (ABS((*src_pp)) <= tiny) *src_pp = 0;
      src_pp++;
    }

    /*
    ** positive threshold
    */
    src_pp = VImageData(src);
    bin_pp = VImageData(tmp);      
    for (i=0; i<npixels; i++) {
      *bin_pp = 0;
      if (*src_pp >= pos) *bin_pp = 1;
      src_pp++;
      bin_pp++;
    }

    label_image = VLabelImage3d(tmp,NULL,(int)26,VShortRepn,&nl);
    if (xminsize > 0 && nl > 0) {
      tmp = VDeleteSmall (label_image,tmp,(int) xminsize);
    }
    else {
      lab_pp = VImageData(label_image);
      bin_pp = VImageData(tmp);
      for (i=0; i<npixels; i++) {
	*bin_pp = 0;
	if (*lab_pp > 0) *bin_pp = 1;
	lab_pp++;
	bin_pp++;
      }
    }
    src_pp = VImageData(src);
    bin_pp = VImageData(tmp);
    for (i=0; i<npixels; i++) {
      if ((*bin_pp < 1) && (*src_pp > tiny)) *src_pp = 0;
      src_pp++;
      bin_pp++;
    }


    /*
    ** negative threshold
    */
    src_pp = VImageData(src);
    bin_pp = VImageData(tmp);      
    for (i=0; i<npixels; i++) {
      *bin_pp = 0;
      if (*src_pp <= neg) {
	*bin_pp = 1;
      }
      src_pp++;
      bin_pp++;
    }

    label_image = VLabelImage3d(tmp,label_image,(int)26,VShortRepn,&nl);
    if (xminsize > 0 && nl > 0) {
      tmp = VDeleteSmall (label_image,tmp,(int) xminsize);
    }
    else {
      lab_pp = VImageData(label_image);
      bin_pp = VImageData(tmp);
      for (i=0; i<npixels; i++) {
	*bin_pp = 0;
	if (*lab_pp > 0) *bin_pp = 1;
	lab_pp++;
	bin_pp++;
      }
    }
    src_pp = VImageData(src);
    bin_pp = VImageData(tmp);
    for (i=0; i<npixels; i++) {
      if ((*bin_pp < 1) && (*src_pp < tiny)) *src_pp = 0;
      src_pp++;
      bin_pp++;
    }

  }

  /* Write the results to the output file: */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list))
  fprintf (stderr, "%s: done.\n",argv[0]);
  return 0;
}
