/****************************************************************
 *
 * Program: niftov
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Karsten Mueller, 2006, <lipsia@cbs.mpg.de>
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
 * $Id$
 *
 *****************************************************************/
 
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via/via.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <nifti1.h>
#include <nifti1_io.h>
#include <znzlib.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

static VDictEntry OrientationDict[] = {
  { "axial"   , 0},
  { "sagittal", 1},
  { "coronal" , 2},
  { NULL }
};

static VDictEntry PrecisionDict[] = {
  { "auto"    ,  0 },
  { "original",  1 },
  { NULL }
};


extern VImage VSetHeader(VImage, VString, VString, VShort);
extern VImage VNiftSort(VImage, int, int, int, VBoolean, VBoolean, VBoolean);
extern VAttrList VNiftFuncReorder(VAttrList, nifti_image *, VShort, VFloat);
extern char * getLipsiaVersion();


VImage
VSetHeader(VImage dest,VString descrip,VString vox,VShort orientation)
{
  VSetAttr(VImageAttrList (dest),"bandtype",NULL,VStringRepn, "spatial");
  VSetAttr(VImageAttrList (dest),"name",NULL,VStringRepn,descrip);
  VSetAttr(VImageAttrList (dest),"voxel",NULL,VStringRepn,vox);
  VSetAttr(VImageAttrList (dest),"convention",NULL,VStringRepn, "natural");
  if (orientation==0) VSetAttr(VImageAttrList(dest), "orientation", NULL, VStringRepn, "axial");
  if (orientation==1) VSetAttr(VImageAttrList(dest), "orientation", NULL, VStringRepn, "sagittal");
  if (orientation==2) VSetAttr(VImageAttrList(dest), "orientation", NULL, VStringRepn, "coronal");

  return dest;
}

int 
main (int argc,char *argv[])
{
  /* Command line options: */
  static VString niftifile = NULL;
  static VShort orientation = 0;
  static VShort precision = 0;
  static VFloat black = 0.1;      /* lower histogram cut-off for contrast scaling */
  static VFloat white = 0.1;      /* upper histogram cut-off for contrast scaling */
  static VBoolean flipx = TRUE;
  static VBoolean flipy = FALSE;
  static VBoolean flipz = FALSE;
  static VFloat reptime = 0;
  static VOptionDescRec  options[] = {
    {"in",VStringRepn, 1, &niftifile, VRequiredOpt, 0,"NIfTI-1 input file"},
    {"precision", VShortRepn,1,&precision,VOptionalOpt,PrecisionDict,"precision of anatomical data"},
    {"black",VFloatRepn,1,&black,VOptionalOpt,NULL,"lower histogram cutoff for contrast scaling"},
    {"white",VFloatRepn,1,&white,VOptionalOpt,NULL,"upper histogram cutoff for contrast scaling"},
    {"xflip",VBooleanRepn,1,(VPointer) &flipx, VOptionalOpt,NULL,"Flip x axis"},
    {"yflip",VBooleanRepn,1,(VPointer) &flipy, VOptionalOpt,NULL,"Flip y axis"},
    {"zflip",VBooleanRepn,1,(VPointer) &flipz, VOptionalOpt,NULL,"Flip z axis"}, 
    {"tr",VFloatRepn,1,(VPointer) &reptime,VOptionalOpt,NULL,"repetition time in seconds"},
    {"orientation", VShortRepn, 1, &orientation, VOptionalOpt, OrientationDict,"Orientation of slices" },
  };
  FILE *out_file;
  VShort *pp_short;
  VUByte *pp_ubyte;
  VFloat *pp_float;
  int npixels, step, i;
  int icod,jcod,kcod;
  int funcsort=0;
  VImage dest=NULL,tmp=NULL;
  VString voxel=NULL;
  nifti_image *src;
  VAttrList out_list, out_list1;
  char vox[100];
  char prg_name[50];	
  sprintf(prg_name,"niftov V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);
  VWarning("It is highly recommended to use vvinidi for data conversion from nifti to vista!!!");
  /* Parse command line arguments and identify files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,NULL /* &in_file */ ,&out_file);

  /* Creat outlist */
  out_list = VCreateAttrList();

  /* history */
  VHistory(VNumber(options),options,prg_name,&out_list,&out_list); 

  if (is_nifti_file(niftifile)!=1) VError("Input file is not NIfTI-1"); 
 
  /* Read the nifti image */
  src = nifti_image_read(niftifile,1);
  sprintf(vox,"%f %f %f\0",ABS(src->pixdim[1]),ABS(src->pixdim[2]),ABS(src->pixdim[3]));
  if (src->sform_code)
    nifti_mat44_to_orientation(src->sto_xyz,&icod,&jcod,&kcod);
  else
    nifti_mat44_to_orientation(src->qto_xyz,&icod,&jcod,&kcod);
  npixels = src->dim[1]*src->dim[2]*src->dim[3];

  /* check plausibility */
  if (src->dim[3]>50 && flipz)
    VWarning("The dataset looks like a 3D data set because it has more than 50 slices. Check if you really want to flip the z axis. Please check the image orientation");
  if (src->dim[3]<50 && !flipz)
    VWarning("The dataset has less than 50 slices. In Lipsia, 2D slices are ordered upside down. It might be useful to set '-zflip true'. Please check the image orientation");

  /* create vista image */ 
  if (src->datatype!=2 && src->datatype!=4 && src->datatype!=16)
    VError("Data type not yet implemented");

  /* UByte */
  if (src->datatype==2) {
    pp_ubyte = (VUByte *) src->data;
    dest = VCreateImage(src->dim[3],src->dim[2],src->dim[1],VUByteRepn);
    memcpy (VPixelPtr(dest,0,0,0), pp_ubyte, VPixelSize(dest) * npixels);
    for (i=0; i<npixels; i++) pp_ubyte++;
    dest = VNiftSort(dest, icod, jcod, kcod, flipx, flipy, flipz);
    dest = VSetHeader(dest,(VString)src->descrip,(VString)vox,orientation);
    VAppendAttr(out_list,"image",NULL,VImageRepn,dest);  
  }

  /* Float */
  if (src->datatype==16) {
    pp_float = (VFloat *) src->data; 
    dest = VCreateImage(src->dim[3],src->dim[2],src->dim[1],VFloatRepn); 
    memcpy (VPixelPtr(dest,0,0,0), pp_float, VPixelSize(dest) * npixels);
    for (i=0; i<npixels; i++) pp_float++;
    dest = VNiftSort(dest, icod, jcod, kcod, flipx, flipy, flipz);
    dest = VSetHeader(dest,(VString)src->descrip,(VString)vox,orientation);
    VAppendAttr(out_list,"image",NULL,VImageRepn,dest);  
  }

  /* Short */
  if (src->datatype==4) {
    pp_short = (VShort *) src->data;
    for (step=0; step<src->dim[4]; step++) {
      dest = VCreateImage(src->dim[3],src->dim[2],src->dim[1],VShortRepn);
      memcpy (VPixelPtr(dest,0,0,0), pp_short, VPixelSize(dest) * npixels);
      for (i=0; i<npixels; i++) pp_short++; 
      dest = VNiftSort(dest, icod, jcod, kcod, flipx, flipy, flipz); 

      if (src->dim[4]>1) {
	funcsort=1;
      } else {
	if (precision == 0) {
	  tmp = VContrastShortUByte(dest, (int)0, (int)255, black / 100.0, white / 100.0);
	  VDestroyImage(dest);
	  dest=tmp;
	}
      }
      dest = VSetHeader(dest,(VString)src->descrip,(VString)vox,orientation); 
      VAppendAttr(out_list,"image",NULL,VImageRepn,dest);     
    }    
  }
  
  /* unload image */
  nifti_image_unload (src);


  /* Resorting functional images */
  if (funcsort==1) {
    out_list1 = VNiftFuncReorder(out_list,src,orientation,reptime);
    if (! VWriteFile (out_file, out_list1)) exit (1);
  } else {
    if (! VWriteFile (out_file, out_list)) exit (1);
  }

  /* Destroy nifti image */
  nifti_image_free(src);


  /* Write out the results: */
  fprintf(stderr,"done.\n");
  return 0;
}


