/****************************************************************
 *
 * Program: vave
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
 * $Id: vave.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


#include "viaio/Vlib.h"
#include "viaio/file.h"
#include "viaio/mu.h"
#include "viaio/option.h"
#include "viaio/os.h"
#include <viaio/VImage.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define VRintPos(x) ((int)((x) + 0.5))
#define VRintNeg(x) ((int)((x) - 0.5))
#define VRint(x)  ((x) >= 0 ? VRintPos(x) : VRintNeg(x))

extern char * getLipsiaVersion();

VImage
Vave(VImage *src,VImage dest,int n)
{
  int i,b,r,c,nslices,nrows,ncols;
  double v,sum,nx,*data=NULL;
  VRepnKind repn;

  nslices = VImageNBands(src[0]);
  nrows   = VImageNRows(src[0]);
  ncols   = VImageNColumns(src[0]);


  dest = VCopyImage(src[0],NULL,VAllBands);
  VFillImage(dest,VAllBands,0);
  repn = VPixelRepn(src[0]);

  VSetAttr(VImageAttrList(dest),"num_images",NULL,VShortRepn,(VShort)n);
  VSetAttr(VImageAttrList(dest),"patient",NULL,VStringRepn,"average");

  nx   = (double) n;
  data = (double *) VCalloc(n,sizeof(double));

  for (b=0; b<nslices; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {

	sum = 0;
	for (i=0; i<n; i++)
	  sum += VGetPixel(src[i],b,r,c);
	sum /= nx;

	v = 0;
	switch(repn) {
        case VBitRepn:
	  v = 0;
          if (sum >= 0.5) v = 1;
          break;

        case VUByteRepn:
          v = (int)(sum + 0.5);
	  if (v > 255) v = 255;
	  if (v < 0) v = 0;
          break;

        case VSByteRepn:
          v = VRint(sum);
          break;

        case VShortRepn:
          v = VRint(sum);
          break;

        case VFloatRepn:
          v = sum;
          break;

        case VDoubleRepn:
          v = sum;
          break;

        default:
          VError(" illegal pixel repn");
        }
	VSetPixel(dest,b,r,c,v);

      }
    }
  }

  return dest;
}


int main (int argc, char *argv[])
{
  static VArgVector in_files;
  static VString out_filename;
  static VOptionDescRec options[] = {
    {"in", VStringRepn, 0, & in_files,VRequiredOpt, NULL,"Input files" },
    {"out", VStringRepn, 1, & out_filename,VRequiredOpt, NULL,"Output file" }
  };
  FILE *fp=NULL;
  VStringConst in_filename;
  VAttrList list,out_list;
  VAttrListPosn posn;
  VImage src=NULL,*src1,dest=NULL;
  int i,n,npix=0;
  char prg_name[50];	
  sprintf(prg_name,"vave V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);

  /*
  ** parse command line
  */
  if (! VParseCommand (VNumber (options), options, & argc, argv)) {
    VReportUsage (argv[0], VNumber (options), options, NULL);
    exit (EXIT_FAILURE);
  }
  if (argc > 1) {
    VReportBadArgs (argc, argv);
    exit (EXIT_FAILURE);
  }



  /* 
  ** read images  
  */
  n = in_files.number;
  src1 = (VImage *) VCalloc(n,sizeof(VImage));
  for (i = 0; i < n; i++) {
    src1[i] = NULL;
    in_filename = ((VStringConst *) in_files.vector)[i];
    fprintf(stderr," %3d:  %s\n",i,in_filename);
    fp = VOpenInputFile (in_filename, TRUE);
    list = VReadFile (fp, NULL);
    if (! list)  VError("Error reading image");
    fclose(fp);

    for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & src);     
      if (i == 0) npix = VImageNPixels(src);
      else if (npix != VImageNPixels(src)) VError(" inconsistent image dimensions");
      src1[i] = src;
      break;
    }
    if (src1[i] == NULL) VError(" no image found in %s",in_filename);
  }


  /* 
  ** compute average 
  */
  dest = Vave(src1,dest,n);


  /* 
  ** output
  */
  out_list = VCreateAttrList ();
  VHistory(VNumber(options),options,prg_name,&list,&out_list); 
  VAppendAttr (out_list,"image",NULL,VImageRepn,dest);

  fp = VOpenOutputFile (out_filename, TRUE);
  if (! VWriteFile (fp, out_list)) exit (1);
  fclose(fp);
  fprintf (stderr, "%s: done.\n", argv[0]);
  exit(0);
}
