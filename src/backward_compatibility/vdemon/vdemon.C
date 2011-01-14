/****************************************************************
 *
 * vdemon:
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Thomas Arnold, 2001, <lipsia@cbs.mpg.de>
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
 * $Id: vdemon.C 2999 2007-11-30 10:55:56Z karstenm $
 *
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <assert.h>

extern "C"
{
   #include <Vlib.h>
   #include <VImage.h>
   #include <option.h>
   #include <mu.h>
   
   extern void getLipsiaVersion(char*, size_t);
}

#include "maxwell.H"


/*------------------------------------------------------------------------------

This program implements a non-linear image matching with Maxwell's demons.

Note: The algorithm is described as "Demons 1" with bijective extension in

      Thirion, J.-P.
      "Image matching as a diffusion process: an analogy with Maxwell's demons"
      Medical Image Analysis 2(3), 243-260, 1998

------------------------------------------------------------------------------*/

void ReadImage (VString Name, VImage& Image, VAttrList& history_list)
{
   FILE*         file;   /* input file       */
   VAttrList     list, list1;   /* attribute list   */
   VAttrListPosn pos;    /* position in list */


   /* initialize results */
   Image   = NULL;

   /* open file */
   file = fopen (Name, "r");
   if (!file)
   {
      VError ("Failed to open input file '%s'", Name);
      return;
   }

   /* read file */
   list = VReadFile (file, NULL);
   if (!list)
   {
      VError ("Failed to read input file '%s'", Name);
      fclose (file);
      return;
   }

   /* Read History */
   list1 = VReadHistory(&list);
   if (list1==NULL) list1=VCreateAttrList();
   history_list = VCopyAttrList(list1);

   /* extract image */
   for (VFirstAttr (list, &pos); VAttrExists (&pos); VNextAttr (&pos))
   {
      if (VGetAttrRepn (&pos) != VImageRepn) continue;
      if (Image)
      {
         VDestroyImage (Image);
         Image = NULL;
         break;
      }
      VGetAttrValue (&pos, NULL, VImageRepn, &Image);
      VSetAttrValue (&pos, NULL, VImageRepn, NULL);
   }
   if (!Image &&  VAttrExists (&pos)) VError ("Input file '%s' contains multiple images",  Name);
   if (!Image && !VAttrExists (&pos)) VError ("Input file '%s' does not contain an image", Name);

   /* clean-up*/
   VDestroyAttrList (list);
   fclose (file);

} /* ReadImage */

/*----------------------------------------------------------------------------*/

VBoolean Compatible (VImage Image1, VImage Image2)
{
   VString attr1, attr2;   /* attributes */


   /* compare voxel representations */
   if (VPixelRepn (Image1) != VPixelRepn (Image2))
   {
      VError ("Images have different voxel representations");
      return FALSE;
   }

   /* compare image sizes */
   if ((VImageNBands   (Image1) != VImageNBands   (Image2)) ||
       (VImageNRows    (Image1) != VImageNRows    (Image2)) ||
       (VImageNColumns (Image1) != VImageNColumns (Image2)))
   {
      VError ("Images have different sizes");
      return FALSE;
   }

   /* compare talairach attributes */
   if (VGetAttr (VImageAttrList (Image1), "talairach", NULL, VStringRepn, (VPointer) &attr1) !=
       VGetAttr (VImageAttrList (Image2), "talairach", NULL, VStringRepn, (VPointer) &attr2))
   {
      VError ("Images have different talairach attributes");
      return FALSE;
   }

   /* compare talairach attributes */
   if ((VGetAttr (VImageAttrList (Image1), "talairach", NULL, VStringRepn, (VPointer) &attr1) == VAttrFound) &&
       (VGetAttr (VImageAttrList (Image2), "talairach", NULL, VStringRepn, (VPointer) &attr2) == VAttrFound) &&
       (strcmp (attr1, attr2) != NULL))
   {
      VError ("Images have different talairach attributes");
      return FALSE;
   }

   return TRUE;

} /* Compatible */

/*----------------------------------------------------------------------------*/

VBoolean WriteField (VString Name, VImage X, VImage Y, VImage Z, VAttrList& history_list)
{
   FILE*         file;      /* output file      */
   VAttrList     list;      /* attribute list   */
   VAttrListPosn pos;       /* position in list */
   VBoolean      success;   /* success flag     */
   char history[]="history";

   /* open file */
   file = fopen (Name, "w");
   if (!file)
   {
      VError ("Failed to open output file '%s'", Name);
      return FALSE;
   }

   /* create list */
   list = VCreateAttrList();

   /* insert images */
   VAppendAttr (list, "X", NULL, VImageRepn, X);
   VAppendAttr (list, "Y", NULL, VImageRepn, Y);
   VAppendAttr (list, "Z", NULL, VImageRepn, Z);

   /* Prepend history */
   VPrependAttr(list, history ,NULL,VAttrListRepn,history_list);

   /* write file */
   success = VWriteFile (file, list);
   if (!success) VError ("Failed to write output file '%s'", Name);

   /* remove images */
   for (VFirstAttr (list, &pos); VAttrExists (&pos); VNextAttr (&pos))
      if (VGetAttrRepn (&pos) == VImageRepn)
         VSetAttrValue (&pos, NULL, VImageRepn, NULL);

   /* clean-up*/
   VDestroyAttrList (list);
   fclose (file);

   return success;

} /* WriteField */

/*----------------------------------------------------------------------------*/

int main (int argc, char* argv[])
{
   VString        sourcename;        /* name of source image         */
   VString        modelname;         /* name of model image          */
   VString        fieldname;         /* name of deformation field    */
   VFloat         sigma = 1.0;       /* deviation of Gaussian filter */
   VLong          iter = 4;          /* iterations at finest scale   */
   VLong          scale = 3;         /* coarsest scale of matching   */
   VBoolean       verbose = TRUE;    /* verbose flag                 */
   VOptionDescRec options[] =        /* options of program           */
   {
      {"source",  VStringRepn,  1, &sourcename, VRequiredOpt, NULL, "Anatomical source image as fixed reference"},
      {"model",   VStringRepn,  1, &modelname,  VRequiredOpt, NULL, "Anatomical model image to be matched"},
      {"field",   VStringRepn,  1, &fieldname,  VRequiredOpt, NULL, "Resulting 3D deformation field"},
//    {"sigma",   VFloatRepn,   1, &sigma,      VOptionalOpt, NULL, "Standard deviation of Gaussian filter. Optional"},
//    {"iter",    VLongRepn,    1, &iter,       VOptionalOpt, NULL, "Number of iterations at finest scale. Optional"},
//    {"scale",   VLongRepn,    1, &scale,      VOptionalOpt, NULL, "Coarsest scale of matching. Optional"},
      {"verbose", VBooleanRepn, 1, &verbose,    VOptionalOpt, NULL, "Show status messages. Optional"}
   };

   VAttrList source_history=NULL;   /* history of source image */
   VAttrList model_history=NULL;    /* history of model image  */

   VImage S;            /* source image */
   VImage M;            /* model image  */
   VImage Dx, Dy, Dz;   /* field images */
   VAttrList history_list=NULL;

   int factor;   /* scaling factor */

   VBoolean success;   /* success flag */


   /* print information */
   char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vdemon V%s", ver);
  
   fprintf (stderr, "%s\n", prg_name); fflush (stderr);

   /* parse command line */
   if (!VParseCommand (VNumber (options), options, &argc, argv))
   {
      if (argc > 1) VReportBadArgs (argc, argv);
      VReportUsage (argv[0], VNumber (options), options, NULL);
      exit (1);
   }

   /* read source image */
   if (verbose) {fprintf (stderr, "Reading source image '%s' ...\n", sourcename); fflush (stderr);}
   ReadImage (sourcename, S, source_history);
   if (!S) exit (2);
   if (VPixelRepn (S) != VUByteRepn)
   {
      VError ("Source image does not contain anatomical data");
      exit (3);
   }

   /* read model image */
   if (verbose) {fprintf (stderr, "Reading model image '%s' ...\n", modelname); fflush (stderr);}
   ReadImage (modelname, M, model_history);
   if (!M) exit (4);
   if (VPixelRepn (M) != VUByteRepn)
   {
      VError ("Model image does not contain anatomical data");
      exit (5);
   }

   /* compare images */
   if (verbose) {fprintf (stderr, "Comparing images ...\n"); fflush (stderr);}
   if (!Compatible (S, M)) exit (6);


   /* check scalability */
   factor = (int) ldexp (1, scale);
   if (((VImageNBands (S) % factor) != 0) || ((VImageNRows (S) % factor) != 0) || ((VImageNColumns (S) % factor) != 0))
   {
      VError ("Images dimensions must be devisible by %d", factor);
      exit (7);
   }

   /* match images */
   if (verbose) {fprintf (stderr, "Matching images ...\n"); fflush (stderr);}
   success = DemonMatch (S, M, Dx, Dy, Dz, sigma, iter, scale, verbose);
   if (!success) exit (8);


   /* set attribute list */
   VImageAttrList (Dx) = VCopyAttrList (VImageAttrList (S));
   VSetAttr (VImageAttrList (Dx), "name", NULL, VStringRepn, "X");
   VImageAttrList (Dy) = VCopyAttrList (VImageAttrList (S));
   VSetAttr (VImageAttrList (Dy), "name", NULL, VStringRepn, "Y");
   VImageAttrList (Dz) = VCopyAttrList (VImageAttrList (S));
   VSetAttr (VImageAttrList (Dz), "name", NULL, VStringRepn, "Z");

   /* Prepend History */
   VPrependHistory(VNumber(options),options,prg_name,&source_history);

   /* write deformation field */
   if (verbose) {fprintf (stderr, "Writing 3D deformation field '%s' ...\n", fieldname); fflush (stderr);}
   success = WriteField (fieldname, Dx, Dy, Dz, source_history);
   if (!success) exit (9);


   /* clean-up 
      VDestroyAttrList (sourcehistory);
      VDestroyAttrList (modelhistory); 
      VDestroyImage (S);
      VDestroyImage (M);
      VDestroyImage (Dx);
      VDestroyImage (Dy);
      VDestroyImage (Dz); */
   
   /* exit */
   if (verbose) {fprintf (stderr, "Finished.\n"); fflush (stderr);}
   return 0;

} /* main */

/*----------------------------------------------------------------------------*/
