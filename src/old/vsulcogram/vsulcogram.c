/****************************************************************
 *
 * Program: vsulcogram
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2002, <lipsia@cbs.mpg.de>
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
 * $Id: vsulcogram.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <via.h>

extern VGraph VSulci(VImage,VFloat,VBoolean);
extern VImage VCleanImage(VImage,VImage,VImage,int);
extern char * getLipsiaVersion();


#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SQR(x) ((x)*(x))

int main (int argc, char **argv)
{
  static VFloat depth=5.0;
  static VBoolean median=TRUE;
  static VOptionDescRec options[] = {
    {"depth",VFloatRepn,1,(VPointer) &depth,
     VOptionalOpt,NULL,"depth threshold (in mm)"},
    {"median",VBooleanRepn,1,(VPointer) &median,
     VOptionalOpt,NULL,"Whether to remove median sulci"}
  };
  FILE *in_file,*out_file;
  VAttrList list;
  VAttrListPosn posn;
  VImage src=NULL;
  VGraph dest=NULL;

  char prg_name[50];	
  sprintf(prg_name,"vsulcogram V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);
  
  /* Parse command line arguments: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file,&out_file);


  list = VReadFile (in_file,NULL);
  if (! list) exit (1);
  fclose(in_file);
  
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);
    if (VPixelRepn(src) != VBitRepn) continue;

    dest = VSulci(src,depth,median);
    VSetAttrValue (& posn, NULL, VGraphRepn, dest);
    break;
  }
  if (src == NULL) VError(" wm not found");


  /* Write the results to the output file: */
  VHistory(VNumber(options),options,prg_name,&list,&list); 
  VWriteFile (out_file, list);
  fprintf (stderr, "%s: done.\n",argv[0]);
  return 0;
}



VGraph
VSulci(VImage src,VFloat depth,VBoolean median)
{
  VGraph graph=NULL,destgraph=NULL;
  VImage dest=NULL,tmp=NULL,closed_image=NULL;
  VImage depth_image=NULL,dist_image=NULL,label_image=NULL;
  VBit *bin_pp;
  VFloat *depth_pp,u,v,u0,v0;
  int i,n,nl,nbands,nrows,ncols,npixels;
  int b,r,c,bb,rr,cc,wn=2;
  VString str;
  float cp_z,cp_y,cp_x;

  str = VMalloc(80);
  if (VGetAttr(VImageAttrList(src),"cp",NULL,VStringRepn,(VPointer) &str) != VAttrFound)
    VError(" attribute 'cp' not found");
  sscanf(str,"%f %f %f",&cp_x, &cp_y, &cp_z);


  nrows  = VImageNRows (src);
  ncols  = VImageNColumns (src);
  nbands = VImageNBands (src);
  npixels = nbands * nrows * ncols;


  /* close sulci, get depth image */
  closed_image = VDTClose(src,NULL,(VDouble) 14.0);
  bin_pp = VPixelPtr(closed_image,0,0,0);
  for (i=0; i<npixels; i++) {
    *bin_pp = (*bin_pp > 0 ? 0 : 1);
    bin_pp++;
  }
  depth_image = VChamferDist3d(closed_image,NULL,VFloatRepn);


  /* get distance from white matter borders */
  dist_image = VChamferDist3d(src,NULL,VFloatRepn);


  dest = VCreateImage(nbands,nrows,ncols,VBitRepn);
  VFillImage(dest,VAllBands,0);
  VCopyImageAttrs (src,dest);


  /* process */
  for (b=0; b<nbands; b++) {
    for (r=0; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	
	if (VPixel(src,b,r,c,VBit) > 0) continue; /* WM */
	if (VPixel(closed_image,b,r,c,VBit) > 0) continue; /* LUFT */

	v0 = VPixel(dist_image,b,r,c,VFloat);
	u0 = VPixel(depth_image,b,r,c,VFloat);

	n = 0;
	for (bb=b-wn; bb<=b+wn; bb++) {
	  if (bb < 0 || bb >= nbands) continue;
	  for (rr=r-wn; rr<=r+wn; rr++) {
	    if (rr < 0 || rr >= nrows) continue;
	    for (cc=c-wn; cc<=c+wn; cc++) {
	      if (cc < 0 || cc >= ncols) continue;

              if (VPixel(src,bb,rr,cc,VBit) > 0) continue;
	      u = VPixel(depth_image,bb,rr,cc,VFloat);
	      v = VPixel(dist_image,bb,rr,cc,VFloat);

	      /* gleiche tiefe, aber weiter von WM weg, also kein median-pixel */
	      if (ABS(u-u0) < 1.33 && v > v0+1.5) n++; 
	    }
	  }
	}

	if (n < 1) VPixel(dest,b,r,c,VBit) = 1;
      }
    }
  }
 
  /* thinning */
  /*
  tmp  = VSmoothImage3d (dest,NULL,(VLong) 0,(VLong) 2);
  */
  tmp  = VDTClose(dest,NULL,(VDouble) 1.5);
  dest = VThin3d(tmp,dest,(int)26);
  

  /* depth threshold */
  depth_pp = VPixelPtr(depth_image,0,0,0);
  bin_pp = VPixelPtr(dest,0,0,0);
  for (i=0; i<npixels; i++) {
    if (ABS((*depth_pp) - depth) > 1.5) *bin_pp = 0;
    depth_pp++;
    bin_pp++;
  }
  

  /* clean lines */
  tmp  = VCopyImage(dest,tmp,VAllBands);
  dest = VCleanImage(tmp,dest,depth_image,(int)26);


  /* remove median sulci */
  if (median == TRUE) {
    for (b=0; b<nbands; b++) {
      for (r=0; r<nrows; r++) {
        for (c=ncols/2-12; c<ncols/2+12; c++) {
	  VPixel(dest,b,r,c,VBit) = 0;
        }
      }
    }
  }

  /* remove cerebellum lines */
  for (b=cp_z+30; b<nbands; b++) {
    for (r=nrows/2+10; r<nrows; r++) {
      for (c=0; c<ncols; c++) {
	VPixel(dest,b,r,c,VBit) = 0;
      }
    }
  }

  label_image = VLabelImage3d (dest,NULL,26,VShortRepn,&nl);
  dest = VDeleteSmall (label_image,dest,10);

  /* convert to graph representation */
  graph = VImage2Graph (dest,NULL,(VDouble) -32000,(VDouble) 32000,TRUE,TRUE);
  destgraph = VGraphPrune(graph,(int) 5);

  fprintf (stderr,"\n");
  return destgraph;
}
