/****************************************************************
 *
 * Program: vreg3d
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
 * $Id: vreg3d.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/file.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>


/* external declarations */
extern VImage VAxial2Coronal(VImage);
extern VImage VAxial2Sagittal(VImage);
extern VImage VReg3d (VImage,VImage,int,int,float,VShort,VShort);
extern void VRegRefine(VImage,VImage,VImage,int,int);
extern char * getLipsiaVersion();

VDictEntry MODDict[] = {
  { "small", 0 },
  { "big", 1 },
  { NULL }
};


VDictEntry TYPDict[] = {
  { "corr", 0 },
  { "MI", 1 },
  { NULL }
};




int main (int argc,char *argv[])
{
  /* Command line options: */
  static VString image_filename = "";
  static VLong band1      =  0;
  static VLong band2      = -1;
  static VShort range     =  0;
  static VFloat pitch     =  0;
  static VBoolean refine  =  FALSE;
  static VShort type      =  0;
  static VOptionDescRec options[] = {
    { "ref", VStringRepn, 1, & image_filename, VRequiredOpt, NULL,
      "File containing 3D anatomical reference image" },
    { "band1", VLongRepn, 1, & band1, VOptionalOpt, NULL,
      "first band to be used for registration" },
    { "band2", VLongRepn, 1, & band2, VOptionalOpt, NULL,
      "last band to be used for registration" },
    { "range", VShortRepn, 1, & range, VOptionalOpt, MODDict,
      "Search range for shift and rotation parameters (0: small, 1: big)" },
    { "type", VShortRepn, 1, & type, VOptionalOpt, TYPDict,
      "Type of goal function (linear correlation or mutual information)" },
    { "angle", VFloatRepn, 1, & pitch, VOptionalOpt, NULL,
      "Inital pitch angle" },
    { "refine", VBooleanRepn, 1, & refine, VOptionalOpt, NULL,
      "Whether or not to use additional data (EPI-T1) for refining the registration " }
  };
  FILE *in_file,*out_file,*fp;
  VAttrList list,out_list;
  VAttrListPosn posn;
  VImage ref=NULL,ref_cor=NULL,ref_sag=NULL,src,transform=NULL;
  VImage mdeft=NULL,epi_t1=NULL;
  VString str;
  float scalec,scaler,scaleb,high_res;
  char prg[50];	
  sprintf(prg,"vreg3d V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);

  /* Parse command line arguments and identify files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file,&out_file);
  if (range != 0 && range != 1) VError(" illegal range (must be 'small' or 'big')");
  if (band1 > band2 && band2 >= 0) VError("<band1> must be less than <band2>");


  /* Read the input file: */
  list = VReadFile (in_file,NULL);
  if (! list) VError(" can't read input file");
  fclose(in_file);


  high_res = 99999;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);
    if (VPixelRepn(src) != VUByteRepn) continue;

    if (VGetAttr (VImageAttrList (src), "voxel", NULL,VStringRepn, (VPointer) & str) 
	!= VAttrFound) 
      VError(" attribute 'voxel' missing in input file.");
    sscanf(str,"%f %f %f",&scalec,&scaler,&scaleb);
    if (scalec < high_res) high_res = scalec;
  }


  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & src);
    if (VPixelRepn(src) != VUByteRepn) continue;

    if (VGetAttr (VImageAttrList (src), "voxel", NULL,VStringRepn, (VPointer) & str) 
	!= VAttrFound) 
      VError(" attribute 'voxel' missing in input file.");
    sscanf(str,"%f %f %f",&scalec,&scaler,&scaleb);

    /* distinguish src and epi-t1 for refinement */
    if (scalec <= high_res+0.001) {
      mdeft = VCopyImage (src,NULL,VAllBands);
      continue;
    }
    else {
      epi_t1 = VCopyImage (src,NULL,VAllBands);
    }

    if (mdeft != NULL && epi_t1 != NULL) break;
  }
  if (mdeft == NULL) VError("Anatomical reference image not found");
  if (epi_t1 == NULL && refine == TRUE) VWarning(" no EPI-T1 image found");
  if (epi_t1 == NULL) refine = FALSE;
  

  if (band1 < 0) band1 = 0;
  if (band2 >= VImageNBands(mdeft) || band2 < band1 || band2 < 0) {
    band2 = VImageNBands(mdeft) -1;
  }


  /* 
  ** Read ref image: 
  */
  fp = VOpenInputFile (image_filename, TRUE);
  list = VReadFile (fp, NULL);
  if (! list)  VError("Error reading ref image");
  fclose(fp);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;

    VGetAttrValue (& posn, NULL, VImageRepn, & ref);

    if (VPixelRepn(ref) != VUByteRepn) VError(" ref image must be of type ubyte");
    if (VImageNBands(ref) < 100)
      VWarning(" ref image must be the MDEFT 3D image!!!");
    
    if (VGetAttr(VImageAttrList (mdeft),"orientation",NULL,VStringRepn,&str) != VAttrFound)
      VError("attribute 'orientation' missing");

    if (strcmp(str,"coronal") == 0) {
      ref_cor   = VAxial2Coronal (ref);
      transform = VReg3d(ref_cor,mdeft,(int)band1,(int)band2,pitch,range,type);
      if (refine) VRegRefine(ref_cor,epi_t1,transform,(int)band1,(int)band2);
      VCopyImageAttrs (ref,transform);
      VSetAttr(VImageAttrList(transform),"orientation",NULL,VStringRepn,"coronal");
    }

    else if (strcmp(str,"sagittal") == 0) {
      ref_sag   = VAxial2Sagittal (ref);
      transform = VReg3d(ref_sag,mdeft,(int)band1,(int)band2,pitch,range,type);
      if (refine) VRegRefine(ref_sag,epi_t1,transform,(int)band1,(int)band2);
      VCopyImageAttrs (ref,transform);
      VSetAttr(VImageAttrList(transform),"orientation",NULL,VStringRepn,"sagittal");
    }

    else {
      transform = VReg3d(ref,mdeft,(int)band1,(int)band2,pitch,range,type);
      if (refine) VRegRefine(ref,epi_t1,transform,(int)band1,(int)band2);
      VCopyImageAttrs (ref,transform);
      VSetAttr(VImageAttrList(transform),"orientation",NULL,VStringRepn,"axial");
    }
    break;
  }

  /*
  ** output
  */
  out_list = VCreateAttrList();
  VHistory(VNumber(options),options,prg,&list,&out_list);

  VSetAttr(VImageAttrList(transform),"zdim",NULL,VShortRepn,(VShort)VImageNBands(ref));
  VSetAttr(VImageAttrList(transform),"ydim",NULL,VShortRepn,(VShort)VImageNRows(ref));
  VSetAttr(VImageAttrList(transform),"xdim",NULL,VShortRepn,(VShort)VImageNColumns(ref));

  VAppendAttr(out_list,"transform",NULL,VImageRepn,transform);
  if (! VWriteFile (out_file, out_list)) VError(" can't write output file");
  fprintf (stderr, "%s: done.\n", argv[0]);
  return (0);
}

